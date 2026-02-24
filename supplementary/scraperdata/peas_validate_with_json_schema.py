# /usr/bin/env python3

# Validates a JSON file against Skyscraper's PEAS JSON Schema.

# Usage:
# python3 peas_validate_with_json_schema.py <path/to/peas_local.json>

# Preparation:
# sudo apt install python3-jsonschema

# (c) 2026 Gemba @ GitHub
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import sys
from pathlib import Path
from jsonschema import validate

if len(sys.argv) != 2:
    print("[!] Provide a peas.json or peas_local.json file as parameter")
    sys.exit(1)

peas_file = Path(sys.argv[1])

if not peas_file.exists():
    print(f"[!] File does not exist: '{peas_file}'")
    sys.exit(1)

with open(peas_file, "r", encoding="utf-8") as pf:
    peas_json = json.load(pf)

with open(Path(__file__).parent / "peas-schema.json", "r", encoding="utf-8") as sf:
    schema = json.load(sf)

try:
    validate(instance=peas_json, schema=schema)
except Exception as e:
    print(f"[-] Validation failed for '{peas_file}':")
    raise e

print(f"[+] '{peas_file}' validated OK")
