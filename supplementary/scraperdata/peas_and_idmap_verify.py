#! /usr/bin/env python3

# Use this script to print textual representations of  the IDs in
# platforms_idmap.csv: Translate the IDs from platforms_map.csv to human
# readably and check for differences between
#
# peas.json (platform handles and extensions (formats), aliases and scrapers)
#
# and
#
# platforms_idmap.csv (platform handle to platform id of scrapers)

# The peas.json file can also be provided as first paramter to the script. This
# may come in handy if you converted your platforms.json to peas_mine.json, with
# script convert_platforms.json.py

# (c) 2023 Gemba @ GitHub
# SPDX-License-Identifier: GPL-3.0-or-later

from pathlib import Path
import json
import sys
import pandas as pd

replacements = {
    "amigacd32": "cd32",
    "astrocde": "astrocade",
    "c16": "plus4",
    "genesis": "megadrive",
    "megacd": "segacd",
    "sharp x1": "x1",
}


def print_platform_tree():

    df = pd.read_csv(pid_map_fn)
    df = df[~df.folder.str.contains("#")]
    df = df.astype({col: int for col in df.columns[1:]})
    df = df.sort_values("folder")
    last_folder = df["folder"].values[-1]

    print(f"[*] {pid_map_fn.name}:")
    [print_data(row, last_folder) for row in df.values]

    return df


def print_data(r, last_folder):
    folder = r[0]
    scrs_id = r[1]
    moby_id = r[2]
    tgdb_id = r[3]

    last = folder == last_folder
    print(f"    {    '└' if last else '├'}── {folder}")

    scrs_name = "N/A"
    if scrs_id in systemes:
        if "nom_eu" in systemes[scrs_id]:
            scrs_name = systemes[scrs_id]["nom_eu"]
        elif "noms_commun" in systemes[scrs_id]:
            scrs_name = ", ".join(
                [
                    n
                    for k, n in enumerate(systemes[scrs_id]["noms_commun"].split(","))
                    if k < 3
                ]
            )
        else:
            scrs_name = "[!] no match"

    mobs_name = mobs[moby_id] if moby_id in mobs else "N/A"
    tgdb_name = tgdb_plafs[tgdb_id] if tgdb_id in tgdb_plafs else "N/A"

    print(f"    {' ' if last else '│'}   ├── ScrS {scrs_id:>4d}: {scrs_name}")
    print(f"    {' ' if last else '│'}   ├── Moby {moby_id:>4d}: {mobs_name}")
    print(f"    {' ' if last else '│'}   └── TGDB {tgdb_id:>4d}: {tgdb_name}")


def print_coverage(df):

    total = df.shape[0]
    scrs_count = len(df[(df["screenscraper_id"] != -1)])
    moby_count = len(df[(df["mobygames_id"] != -1)])
    tgdb_count = len(df[(df["tgdb_id"] != -1)])

    print("[*] Coverage")
    print(
        f"    Screenscraper: {scrs_count:3d}/{total} ({100.0 * scrs_count/total:.1f}%)"
    )
    print(
        f"    Moby Games   : {moby_count:3d}/{total} ({100.0 * moby_count/total:.1f}%)"
    )
    print(
        f"    The Games DB : {tgdb_count:3d}/{total} ({100.0 * tgdb_count/total:.1f}%)"
    )
    print("    Any not covered/matched platform can most likely be scraped by "
        "utilizing the 'aliases' list in the peas.json file.\n    Read the details: "
        "https://gemba.github.io/skyscraper/PLATFORMS/#updating-peasjson-and-platforms_idmapcsv")


def annotate_peas(f):
    if f in replacements:
        print(
            f"    {f:10s} is replaced with {replacements[f]:10s} in {pid_map_fn.name} (OK)"
        )
    else:
        print(
            f"    {f:10s} add to {pid_map_fn.name} if needed, see documentation PLATFORMS.md"
        )


def annotate_idmap(f):
    print(f"    {f}")


def print_diffs(df):
    diff_df = pd.merge(
        df["folder"], peas["folder"], how="left", indicator="Exist", sort=True
    )
    if (diff_df["Exist"] != "both").any():
        print(f"[*] Only in {pid_map_fn.name} and not in {peas_fn.name}:")
        _ = [
            annotate_idmap(f) for f in diff_df.loc[diff_df["Exist"] != "both"]["folder"]
        ]

    diff_df = pd.merge(
        df["folder"], peas["folder"], how="right", indicator="Exist", sort=True
    )
    if (diff_df["Exist"] != "both").any():
        print(f"[*] Only in {peas_fn.name} and not in {pid_map_fn.name}:")
        _ = [
            annotate_peas(f) for f in diff_df.loc[diff_df["Exist"] != "both"]["folder"]
        ]


def locate_file(f):
    cwd = Path(__file__).parent.resolve()
    if Path("/opt/retropie/supplementary") in cwd.parents:
        return Path("/opt/retropie/configs/all/skyscraper") / f
    elif Path("/usr/local/bin") in cwd.parents:
        return Path("/usr/local/etc/skyscraper") / f
    elif (cwd.parent.parent / f).exists():
        return cwd.parent.parent / f
    else:
        print(
            f"[!] can not locate {f}. Please add full path as parameter to this script."
        )
        sys.exit(1)


if __name__ == "__main__":

    if len(sys.argv) == 2:
        if not Path(sys.argv[1]).exists():
            print(
                "[!] Provide a Skyscraper peas.json filename.\n"
                f"    Usage: python3 {sys.argv[0]} <path/to/peas.json>"
            )
            sys.exit(1)
        peas_fn = Path(sys.argv[1])
    else:
        peas_fn = locate_file("peas.json")

    cfg_home = peas_fn.parent
    pid_map_fn = cfg_home / "platforms_idmap.csv"
    peas_fn = cfg_home / "peas.json"

    print(
        f"[+] Using platforms, aliases, extensions and scrapers definition file\n    {peas_fn}"
    )

    scrs_plafs_fn = cfg_home / "screenscraper_platforms.json"
    if not scrs_plafs_fn.exists():
        print(f"[!] File not found {scrs_plafs_fn}")
        print("    Check if installation of Skyscraper succeeded. Bailing out.")
        sys.exit(1)

    with open(cfg_home / "screenscraper_platforms.json") as fh:
        systemes = {int(k): v for k, v in json.load(fh).items()}

    with open(cfg_home / "mobygames_platforms.json") as fh:
        mobs = {int(k): v for k, v in json.load(fh).items()}

    with open(cfg_home / "tgdb_platforms.json") as fh:
        tgdb_plafs = {int(k): v for k, v in json.load(fh).items()}

    with open(peas_fn) as fh:
        peas = pd.DataFrame([k for k in json.load(fh).keys()], columns=["folder"])

    peas_local_fn = cfg_home / "peas_local.json"
    if peas_local_fn.exists():
        peas = peas.set_index("folder")
        with open(peas_local_fn) as fh:
            peas_local = pd.DataFrame([k for k in json.load(fh).keys()], columns=["folder"])
            peas_local.set_index("folder")
            peas.update(peas_local)
            peas = peas.reset_index()
        print(f"[+] Merging with local platform file: {peas_local_fn}")


    df = print_platform_tree()
    print_coverage(df)
    print_diffs(df)
