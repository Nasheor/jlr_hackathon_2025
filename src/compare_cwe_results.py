#!/usr/bin/env python3
"""
Compare predicted CWE scan results against a ground-truth CSV.
"""

import sys, argparse, pandas as pd, pathlib, textwrap

def load_predictions(path: pathlib.Path) -> pd.DataFrame:
    df = pd.read_csv(path)
    # keep only the columns we need and normalise filename
    df = df.rename(columns={"nearest_cwe": "pred_cwe",
                            "is_vulnerable": "pred_vuln",
                            "file": "filename"})
    df["filename"] = df["filename"].apply(lambda p: pathlib.Path(p).name)
    # pred_vuln may be bool, make it {0,1}
    df["pred_vuln"] = df["pred_vuln"].astype(bool).astype(int)
    return df[["filename", "pred_vuln", "pred_cwe"]]

def load_ground_truth(path: pathlib.Path) -> pd.DataFrame:
    df = pd.read_csv(path, header=None, names=["filename", "true_cwe", "true_vuln"])
    df["filename"] = df["filename"].apply(lambda p: pathlib.Path(p).name)
    return df

def evaluate(pred_df: pd.DataFrame, truth_df: pd.DataFrame) -> pd.DataFrame:
    merged = truth_df.merge(pred_df, on="filename", how="left", indicator=True)
    # fill missing preds (file not scanned)
    merged["pred_vuln"] = merged["pred_vuln"].fillna(0).astype(int)
    merged["pred_cwe"]  = merged["pred_cwe"].fillna("")
    # compute match
    def row_match(r):
        if r.true_vuln == 0:          # file labelled safe
            return r.pred_vuln == 0
        # vulnerable: need both vuln flag and CWE match
        return r.pred_vuln == 1 and (r.pred_cwe == r.true_cwe)
    merged["correct"] = merged.apply(row_match, axis=1)
    return merged

def main():
    ap = argparse.ArgumentParser(
        description="Compare CWE scan CSV vs. ground-truth CSV.")
    ap.add_argument("pred_csv",   help="CSV from scan_cwe_directory.py")
    ap.add_argument("truth_csv",  help="Ground-truth CSV (filename,cwe_id,vuln)")
    ap.add_argument("--mismatches", "-m", help="Write mismatching rows to this CSV")
    args = ap.parse_args()

    pred  = load_predictions(pathlib.Path(args.pred_csv))
    truth = load_ground_truth(pathlib.Path(args.truth_csv))
    df    = evaluate(pred, truth)

    total = len(df)
    correct = df.correct.sum()
    print(f"✔️  Correct: {correct}/{total}  ({correct/total:.1%})")
    print("Break-down:")
    print(df.groupby(["true_vuln","pred_vuln"]).size().rename("count"))

    if args.mismatches:
        mism = df[~df.correct]
        mism.to_csv(args.mismatches, index=False)
        print(f"❌  {len(mism)} mismatches written → {args.mismatches}")

if __name__ == "__main__":
    main()
