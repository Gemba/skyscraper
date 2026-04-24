#! /usr/bin/env python3

# convert platforms.json from detain/torresflo to Skyscraper 3.9.0+ format
#
# You do not have to use this script on your Skyscraper installation.
#
# Provide path/to/platforms.json as parameter. Output will be
# path/to/peas_mine.json which your then can compare with peas.json. You will
# only need this script if you made local changes to platforms.json.

# (c) 2023 Gemba @ GitHub
# SPDX-License-Identifier: GPL-3.0-or-later

from operator import itemgetter
from pathlib import Path
import json
import sys


SKYSCRAPER_HOME = Path(__file__).parent.parent.parent.resolve()
PLATFORMS_JSON = SKYSCRAPER_HOME / "platforms.json"


def reformat(dd):
    out[dd["name"]] = {}
    for k, v in dd.items():
        if k == "name":
            continue
        p = out[dd["name"]]
        p[k] = v


if __name__ == "__main__":

    if len(sys.argv) != 2 or not Path(sys.argv[1]).exists():
        print(
            "[!] Provide a Skyscraper platforms.json filename.\n"
            f"    Usage: python3 {sys.argv[0]} <path/to/platforms.json>\n"
            "    Inputfile will not be altered, a new JSON file will be written."
        )
        sys.exit(1)
    fn = Path(sys.argv[1])

    with open(fn) as fh:
        jd = json.load(fh)

    for entries in jd["platforms"]:
        for k, v in entries.items():
            if type(v) == list:
                entries[k] = sorted(v)

    jd["platforms"] = sorted(jd["platforms"], key=itemgetter("name"))
    jd = jd["platforms"]

    out = {}

    [reformat(dd) for dd in jd]

    of = fn.parent / ("peas_mine" + fn.suffix)
    with open(of, "w") as fh:
        json.dump(out, fh, ensure_ascii=False, sort_keys=True, indent=4)
    print(f"[+] New format platforms.json written to {of}")
