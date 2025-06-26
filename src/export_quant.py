"""
Merge LoRA → base, export ONNX, and INT8-quantise the file.
Usage:
    python tools/export_quant.py \
           --lora_dir models/codebert_mini_lora \
           --out_dir  models/quantised
"""
import argparse, pathlib, torch, tempfile, shutil
from transformers import AutoModel
from peft import PeftModel
from onnxruntime.quantization import quantize_dynamic, QuantType
from transformers import AutoModelForSequenceClassification

def merge_lora(lora_dir: str, merged_out: pathlib.Path):
    # ----------------------------- use the CLASSIFIER  ▼▼
    base = AutoModelForSequenceClassification.from_pretrained(
        "microsoft/codebert-base", num_labels=2
    )
    model  = PeftModel.from_pretrained(base, lora_dir)
    merged = model.merge_and_unload()        # returns full fp32 classifier

    save_path = merged_out / "fp32"
    save_path.mkdir(parents=True, exist_ok=True)
    merged.save_pretrained(save_path, safe_serialization=True)
    return merged            # return classifier (not encoder)

def export_onnx(model, onnx_path: pathlib.Path):
    seq_len = 16
    ids  = torch.randint(1, 100, (1, seq_len), dtype=torch.long)   # any tokens
    attn = torch.ones(1, seq_len, dtype=torch.long)                # all 1s
    ttype = torch.zeros(1, seq_len, dtype=torch.long)              # ← *** zeros ***

    torch.onnx.export(
        model,
        (ids, attn, ttype),
        onnx_path,
        input_names=["input_ids", "attention_mask", "token_type_ids"],
        output_names=["logits"],
        opset_version=16,
        dynamic_axes={
            "input_ids": {0: "batch", 1: "seq"},
            "attention_mask": {0: "batch", 1: "seq"},
            "token_type_ids": {0: "batch", 1: "seq"},
            "logits": {0: "batch"},
        },
    )

def quantise_int8(onnx_path: pathlib.Path, quant_out: pathlib.Path):
    quantize_dynamic(
        model_input=str(onnx_path),
        model_output=str(quant_out),
        weight_type=QuantType.QInt8,
        # optimize_model=False,
    )

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--lora_dir", required=True)
    p.add_argument("--out_dir",  default="../models/quantised")
    args = p.parse_args()

    out_dir = pathlib.Path(args.out_dir); out_dir.mkdir(parents=True, exist_ok=True)

    merged_dir = out_dir / "codebert_merged"
    print("🔄  Merging LoRA into base …")
    # model = merge_lora(args.lora_dir, merged_dir)
    #
    # fp32_onnx = out_dir / "codebert_fp32.onnx"
    merged_model = merge_lora(args.lora_dir, out_dir)

    fp32_onnx = out_dir / "codebert_fp32.onnx"
    print("📦  Exporting ONNX fp32 …")
    export_onnx(merged_model, fp32_onnx)

    int8_onnx = out_dir / "codebert_int8.onnx"
    print("🔧  Quantising → INT8 …")
    quantise_int8(fp32_onnx, int8_onnx)

    sz = int8_onnx.stat().st_size / 1e6
    print(f"✅  Saved INT8 model  → {int8_onnx}  ({sz:.1f} MB)")
