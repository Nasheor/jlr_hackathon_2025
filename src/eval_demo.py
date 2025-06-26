import os
import json
from openai import OpenAI
import re, pathlib

# Initialize OpenAI client
client = OpenAI(api_key=os.getenv("OPEN_API_KEY"))

# Allowed CWE list
ALLOWED_CWES = {"119", "122", "124", "129"}

# Regex to extract digits
CWE_DIGIT_RE = re.compile(r"(\d+)")

# Regex to extract JSON payload from the model response
JSON_OBJ_RE = re.compile(r"(\{.*\})", re.DOTALL)
# Regex to extract digits
CWE_DIGIT_RE = re.compile(r"(\d+)")

UNSAFE_API_RE = re.compile(  # common overflow-prone calls
    r"\b("
    r"memcpy|memmove|strcpy|strcat|sprintf|snprintf|gets"
    r")\s*\(",
    re.IGNORECASE
)

# ---------------------------------------------------------------- heuristics
def _looks_unsafe(snippet: str) -> bool:
    """
    Very coarse heuristic:
      • snippet calls an unsafe API
      • the 2 lines above the call DON'T contain sizeof/strlen/<=/< guarding
    """
    lines = [ln.strip() for ln in snippet.splitlines()]
    for i, ln in enumerate(lines):
        if UNSAFE_API_RE.search(ln):
            context = " ".join(lines[max(0, i-2):i])  # two lines above
            if any(tok in context for tok in ("sizeof", "strlen", "<=", "<", ">=", ">")):
                return False  # looks guarded
            return True       # unsafe call, no guard detected
    return False

def identify_cwe(code: str) -> dict:
    """
    Sends `code` to ChatGPT, asking it to:
      - Identify the CWE fault (only 119,122,124,129)
      - Extract the vulnerable snippet from within the code
    Returns a dict with vulnerable_code, nearest_cwe, and is_vulnerable.
    """
    system_prompt = (
        "You are a security assistant. For the C/C++ code provided, list *all* CWE IDs "
        "(only 119, 122, 124, 129).  For each CWE, also rate the severity "
        "as 'low', 'medium', or 'high'.  "
        "If no relevant CWE exists, return empty lists.  "
        "Respond JSON with exactly these keys:\n"
        '  "vulnerable_code": string,\n'
        '  "nearest_cwe":     list of strings,\n'
        '  "severity":        list of strings (same length),\n'
        '  "is_vulnerable":   boolean'
    )
    user_prompt = f"```c\n{code}\n```"

    # Call the API
    resp = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {"role": "system", "content": system_prompt},
            {"role": "user",   "content": user_prompt}
        ],
        temperature=0.0,
        max_tokens=300,
        n=1,
    )

    # ---------- parse GPT reply ----------
    raw = resp.choices[0].message.content.strip()
    m = JSON_OBJ_RE.search(raw)
    gpt = {"vulnerable_code": "", "nearest_cwe": [], "severity": [], "is_vulnerable": False}
    if m:
        try:
            gpt = json.loads(m.group(1))
        except json.JSONDecodeError:
            pass

    snippet = gpt.get("vulnerable_code", "").strip()
    # ── normalise nearest_cwe to a list[str] of just digits ──
    def extract_digits(item):
        m = CWE_DIGIT_RE.search(str(item))
        return m.group(1) if m else None

    raw_list = gpt.get("nearest_cwe") or []
    if not isinstance(raw_list, list):
        raw_list = [raw_list]

    cwe_list = [d for item in raw_list if (d := extract_digits(item)) in ALLOWED_CWES]
    # normalise severity list ---------
    severity_list = gpt.get("severity") or []
    if not isinstance(severity_list, list):
        severity_list = [severity_list]
    # pad / trim to match cwe_list length
    while len(severity_list) < len(cwe_list):
        severity_list.append("medium")
    severity_list = severity_list[: len(cwe_list)]

    gpt_says_vuln = bool(snippet) and bool(cwe_list)

    is_vuln = gpt_says_vuln
    if snippet is None:
        is_vuln = False

    return {
        "vulnerable_code": snippet,
        "nearest_cwe": cwe_list,
        "severity": severity_list,
        "is_vulnerable": is_vuln,
    }


if __name__ == "__main__":
    path = "../data/test/codebert_safe_test_file.c"
    with open(path, 'r', encoding='utf-8') as f:
        code_text = f.read()
    output = identify_cwe(code_text)
    out_path = pathlib.Path("../data/results/result.json")
    with out_path.open('w', encoding='utf-8') as f:
        json.dump(output, f, indent=2)
    print(f"Results written to {out_path}")
