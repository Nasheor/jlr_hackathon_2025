# JLR Hackathon 2025 · In-House Cyber-Security Scanner  
*A CPU-only static–analysis pipeline that flags C/C++ vulnerabilities, explains **why**, and runs in < 2 ms/function.*

---

## 🚀 Quick Start
```bash
# clone
git clone https://github.com/your-org/jlr-cyber-scan.git
cd jlr-hackathon_2025

# create & activate virtual-env
python -m venv venv
source venv/bin/activate      # Windows: venv\Scripts\activate

# install deps  (~4 min, CPU build)
pip install -r requirements.txt

# scan single file
1. Run the notebooks in the order of numbering
2. For Custom C, c++ files, place them in the data folder and amend the paths in the notebook accordingly
