#! /usr/bin/env bash

# Derived from
# https://gist.github.com/ghodsizadeh/5e586c55add1f01cc48c8bd6acb57c16
#
# Copyright (c) 2021 ghodsizadeh @ Github
# Copyright (c) 2025 Gemba @ Github
#
# SPDX-License-Identifier: MIT

# USAGE:
#
# sudo apt install mdb-tools sqlite3
# run: ./mdb2sqlite.sh <path/to/mdb-file.mdb>

input="$1"
[[ -z "$input" ]] && echo "[!] Provide MDB file as parameter." && exit 1

output="$(basename "${input%.*}.sqlite3")"
of="$(dirname "$input")/$output"
rm -f "$of"

rm -rf sqlite
mkdir -p sqlite/dumps

echo "[*] Source file: $input"
echo "[+] Dumping schema ..."
mdb-schema "$input" sqlite >sqlite/schema.sql

for i in $(mdb-tables "$input"); do
    echo "[+] Dumping $i ..."
    mdb-export -D "%Y-%m-%d %H:%M:%S" -H -I sqlite "$input" "$i" >"sqlite/dumps/$i.sql"
done

pushd sqlite >/dev/null || exit
sqlite3 <schema.sql "$of"

for f in dumps/*; do
    echo "[+] Importing $f ..." && (
        echo 'BEGIN;'
        cat "$f"
        echo 'COMMIT;'
    ) | sqlite3 "$of"
done

popd >/dev/null || exit
rm -rf sqlite

echo "[+] ... to destination file: '$of'"
echo "[*] Done."
