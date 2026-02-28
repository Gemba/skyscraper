#! /usr/bin/env python3

# Check screenscraper.json from detain/torresflo to platforms_idmap.csv for
# differences. You will only need this script if you made local changes to
# screenscraper.json.

# (c) 2023 Gemba @ GitHub
# SPDX-License-Identifier: GPL-3.0-or-later

from pathlib import Path

import json
import sys
import pandas as pd


def read_platforms_idmap():
    df = pd.read_csv(pid_map_fn)
    df = df[~df.folder.str.contains("#")]
    df = df.astype({col: int for col in df.columns[1:]})
    return df


def resolve(hdl, id):
    if id in systemes:
        print(
            f"[-] The id {id} used for this system ({hdl}) is not well known in Screenscraper systems."
        )
        names = ""
        if "noms_commun" in systemes[id]:
            names = "\n    " + "\n    ".join(systemes[id]["noms_commun"].split(","))
        else:
            names = systemes[id]["nom_eu"]
        print(f"[-] The id refers to this system: {names}")
    else:
        print(
            f"[!] Something's fishy: This id {id} (system: {hdl}) does not exist in Screenscrapers systems."
        )


def locate_file(f):
    cwd = Path(__file__).parent.resolve()
    if Path("/opt/retropie/supplementary") in cwd.parents:
        return Path("/opt/retropie/configs/all/skyscraper") / f
    elif Path("/usr/local/bin") in cwd.parents:
        return Path("/usr/local/etc/skyscraper") / f
    else:
        print(
            f"[!] cannot locate {f}. Please add full path as parameter to this script."
        )
        sys.exit(1)


if __name__ == "__main__":

    if len(sys.argv) == 2:
        if not Path(sys.argv[1]).exists():
            print(
                "[!] Provide a valid Skyscraper platforms_idmap.csv filename.\n"
                f"    Usage: python3 {sys.argv[0]} <path/to/platforms_idmap.csv>\n"
            )
            sys.exit(1)
        else:
            pid_map_fn = Path(sys.argv[1])
    else:
        pid_map_fn = locate_file("platforms_idmap.csv")

    print(f"[*] Using map file {pid_map_fn}")
    cfg_home = pid_map_fn.parent

    scrs_json = cfg_home / "screenscraper.json"
    if not scrs_json.exists():
        print(f"[-] File not found: {scrs_json}. Bailing out.")
        sys.exit(0)

    with open(scrs_json) as fh:
        jd = json.load(fh)

    jd = jd["platforms"]

    with open(cfg_home / "screenscraper_platforms.json") as fh:
        systemes = {int(k): v for k, v in json.load(fh).items()}

    dfs = pd.DataFrame(jd)
    dfs.rename(columns={"name": "handle", "id": "id"}, inplace=True)

    df = read_platforms_idmap()
    dfs = dfs.loc[~dfs["handle"].isin(df["folder"]), :]
    if dfs.shape[1]:
        [resolve(hdl, id) for hdl, id in zip(dfs["handle"], dfs["id"])]
