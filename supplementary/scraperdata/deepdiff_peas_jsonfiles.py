#! /usr/bin/env python3

# Compares platform definition files of Skyscraper and prints differences as new
# JSON object: Fo example, it identifies changes of your local platforms
# peas.json in comparison to the maintainer's peas.json.rp-dist.
#
# See docs/PLATFORMS.md for details.
#
# Your local changes will be printed and can also be stored in a local platform
# peas_local.json, this will not be altered by any upstream changes to
# peas.json.
#
# Once you have saved your local changes move the peas.json.rp-dist to
# peas.json.
#
# For the platform_idmap.csv changes to the maintainer's file run:

# You will need Deepdiff to run this script: sudo apt install python3-deepdiff

# (c) 2025 Gemba @ GitHub
# SPDX-License-Identifier: GPL-3.0-or-later

from deepdiff import DeepDiff
from pathlib import Path
import json
import sys


def load_peas_json(fn):
    if not fn.exists():
        print(f"[-] File {fn} not found. Please fix.")
        sys.exit(1)
    with open(fn, encoding="utf8") as fh:
        d = json.load(fh)

    for k, v in d.items():
        d[k].pop("scrapers", None)
    return d


def check_outfile():
    fn_localonly_peas = None
    if len(sys.argv) == 4:
        fn_localonly_peas = Path(sys.argv[3])
        if fn_localonly_peas.exists():
            print(
                f"[!] File {fn_localonly_peas} already exists and will not be overwritten."
            )
            fn_localonly_peas = None
    return fn_localonly_peas


if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print(
            f"[*] Usage: {sys.argv[0]} <source_peas.json> <dest_peas.json> [<outfile_peas.json>]"
        )
        print(
            f"    e.g: {sys.argv[0]} peas.json.rp-dist peas.json peas_local.json\n"
            "    identifies changes to your local peas.json compared to peas.json.rp-dist\n"
            "    and saves them in peas_local.json"
        )
        sys.exit(0)

    fn_upstream_peas = Path(sys.argv[1])
    d1 = load_peas_json(fn_upstream_peas)

    fn_changed_peas = Path(sys.argv[2])
    d2 = load_peas_json(fn_changed_peas)

    diff = DeepDiff(d1, d2, ignore_order=True, report_repetition=True)

    dout = {}
    for what, diffpath in diff.items():
        platform = None
        if what == "dictionary_item_added":
            platform = diffpath[0].split("'")[1]
        elif what == "iterable_item_added":
            platform = next(iter(diffpath)).split("'")[1]

        if platform:
            dout[platform] = d2[platform]

    platforms = dict(sorted(dout.items()))

    # sort subkeys
    for platform, sub_dict in platforms.items():
        for k, v in sub_dict.items():
            if type(v) == list:
                sub_dict[k] = sorted(v)
        platforms[platform] = dict(sorted(sub_dict.items()))

    fn_localonly_peas = check_outfile()

    if not platforms:
        print(f"[*] No additions to '{fn_upstream_peas}' detected. Quitting.")
        sys.exit(0)

    if fn_localonly_peas:
        with open(fn_localonly_peas, "w", encoding="utf8") as fh:
            json.dump(platforms, fh, indent=4, ensure_ascii=False)
        print(f"[*] Additions saved to '{fn_localonly_peas}'")
    else:
        print(f"[*] Detected additions to baseline file '{fn_upstream_peas}':")
        print(
            json.dumps(platforms, indent=4, ensure_ascii=False).encode("utf8").decode()
        )
        if len(sys.argv) == 3:
            print(
                f"[*] Re-run and provide a filename as third parameter to persist these."
            )
        else:
            check_outfile()
