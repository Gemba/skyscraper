#!/usr/bin/env bash
#
# Build a Skyscraper .deb for Debian-based systems.
#
# Recipe (per docs, RetroPie scriptmodule, Debian trixie host):
#   pinned source -> qmake6 (PREFIX=/usr, SYSCONFDIR=/etc) -> make ->
#   make install INSTALL_ROOT=<staging> -> dpkg-shlibdeps Depends ->
#   dpkg-deb --build
#
# Usage:
#   packaging/build-deb.sh [--out-dir DIR] [--keep-tmp] [--pin-commit SHA]
#
# Output:
#   skyscraper_<VERSION>-1_amd64.deb  (default: repo root)
#
# Notes:
# - Builds from a clean `git archive HEAD` export, so the working tree stays
#   clean. HEAD must match PINNED_COMMIT (or the SHA passed via --pin-commit).
# - All temporary work (build tree, staging dir) lives under $REPO_ROOT/.tmp
#   (git-ignored). Nothing is written outside the repo.
# - SYSCONFDIR=/etc is a deliberate FHS choice: upstream default would be
#   /usr/etc with PREFIX=/usr. The binary probes SYSCONFDIR/skyscraper at
#   runtime and falls back to single-binary mode when absent.
# - libqt6sql6-sqlite and p7zip-full are runtime-only deps (dlopen/exec) that
#   dpkg-shlibdeps cannot see; they are appended to Depends manually.
# - No skyscraper.cfg / config.ini is shipped: the app seeds per-user config
#   from /etc/skyscraper examples on first run, falling back to built-in
#   defaults.

set -euo pipefail

PINNED_VERSION="3.21.1"
PINNED_COMMIT="ccd49ae99a13ce037476f1d3cc3b646afca2059e"
PKG_NAME="skyscraper"
PKG_REVISION="1"
ARCH="amd64"
# Neutral default; downstream forks override with DEB_MAINTAINER.
MAINTAINER="${DEB_MAINTAINER:-Gemba <Gemba@users.noreply.github.com>}"

OUT_DIR=""
KEEP_TMP=0
PIN_COMMIT=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --out-dir)
            OUT_DIR="$2"
            shift 2
            ;;
        --pin-commit)
            PIN_COMMIT="$2"
            shift 2
            ;;
        --keep-tmp)
            KEEP_TMP=1
            shift
            ;;
        -h|--help)
            sed -n '2,/^$/p' "$0"
            echo "Usage: $0 [--out-dir DIR] [--keep-tmp] [--pin-commit SHA]"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

if [[ -z "$OUT_DIR" ]]; then
    OUT_DIR="$REPO_ROOT"
fi
mkdir -p "$OUT_DIR"

# --- Pin check ---------------------------------------------------------------
# Default pin: last locally validated source. CI / releases pass the exact
# commit explicitly via --pin-commit (a hardcoded pin can never survive the
# commit that carries it, so the override is the release mechanism).
HEAD_COMMIT="$(git rev-parse HEAD)"
WANT_COMMIT="${PIN_COMMIT:-$PINNED_COMMIT}"
if [[ "$HEAD_COMMIT" != "$WANT_COMMIT" ]]; then
    echo "ERROR: HEAD ($HEAD_COMMIT) is not the expected commit ($WANT_COMMIT, v$PINNED_VERSION)." >&2
    echo "Check out the expected commit, or pass --pin-commit <sha> to build it deliberately." >&2
    exit 1
fi

TREE_VERSION="$(grep -oE '[0-9]+\.[0-9]+\.[0-9]+' VERSION.ini | head -n 1)"
if [[ "$TREE_VERSION" != "$PINNED_VERSION" ]]; then
    echo "ERROR: VERSION.ini says $TREE_VERSION, expected $PINNED_VERSION." >&2
    exit 1
fi

# --- Dependency check ----------------------------------------------------------
for tool in qmake6 make g++ dpkg-shlibdeps dpkg-deb; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool '$tool' not found." >&2
        echo "Install build deps: sudo apt-get install qt6-base-dev qmake6 qt6-base-dev-tools libqt6sql6-sqlite p7zip-full" >&2
        exit 1
    fi
done

DEB_VERSION="${PINNED_VERSION}-${PKG_REVISION}"
DEB_FILE="${PKG_NAME}_${DEB_VERSION}_${ARCH}.deb"

TMP_BASE="$REPO_ROOT/.tmp/build-deb"
mkdir -p "$TMP_BASE"
BUILD_DIR="$(mktemp -d "$TMP_BASE/build-XXXXXX")"
STAGING_DIR="$(mktemp -d "$TMP_BASE/staging-XXXXXX")"

cleanup() {
    if [[ "$KEEP_TMP" -eq 0 ]]; then
        rm -rf "$BUILD_DIR" "$STAGING_DIR" 2>/dev/null || true
    else
        echo "Kept temp dirs: $BUILD_DIR $STAGING_DIR"
    fi
    # Remove the shlibdeps stub if we created it inside the export.
    rm -rf "$BUILD_DIR/debian" 2>/dev/null || true
}
trap cleanup EXIT

echo "==> Exporting clean tree of $HEAD_COMMIT to $BUILD_DIR"
git archive HEAD | tar -x -C "$BUILD_DIR"

echo "==> Building (qmake6 PREFIX=/usr SYSCONFDIR=/etc && make)"
pushd "$BUILD_DIR" >/dev/null
rm -f .qmake.stash
PREFIX=/usr SYSCONFDIR=/etc qmake6
make -j"$(nproc)"

echo "==> Staging install to $STAGING_DIR"
make install INSTALL_ROOT="$STAGING_DIR"
popd >/dev/null

if [[ ! -x "$STAGING_DIR/usr/bin/Skyscraper" ]]; then
    echo "ERROR: staged binary missing: $STAGING_DIR/usr/bin/Skyscraper" >&2
    exit 1
fi
if [[ ! -f "$STAGING_DIR/etc/skyscraper/peas.json" ]]; then
    echo "ERROR: staged data files missing under $STAGING_DIR/etc/skyscraper" >&2
    exit 1
fi

echo "==> Computing Depends via dpkg-shlibdeps"
# dpkg-shlibdeps requires a debian/control stub in the analysis directory.
mkdir -p "$BUILD_DIR/debian"
printf 'Source: %s\nSection: games\nPriority: optional\n\nPackage: %s\nArchitecture: %s\nDepends: ${shlibs:Depends}\nDescription: stub\n' \
    "$PKG_NAME" "$PKG_NAME" "$ARCH" >"$BUILD_DIR/debian/control"
SHLIBS_DEPENDS="$(cd "$BUILD_DIR" && dpkg-shlibdeps -O "$STAGING_DIR/usr/bin/Skyscraper" 2>/dev/null | sed 's/^shlibs:Depends=//')"
rm -rf "$BUILD_DIR/debian"
if [[ -z "$SHLIBS_DEPENDS" ]]; then
    echo "ERROR: dpkg-shlibdeps produced empty Depends." >&2
    exit 1
fi
# Runtime-only deps invisible to shlibdeps: Qt SQLite driver (dlopened) and
# p7zip-full (shelled out to for archive handling).
DEPENDS="${SHLIBS_DEPENDS}, libqt6sql6-sqlite, p7zip-full"
echo "    Depends: $DEPENDS"

echo "==> Writing DEBIAN/control"
mkdir -p "$STAGING_DIR/DEBIAN"
cat >"$STAGING_DIR/DEBIAN/control" <<EOF
Package: $PKG_NAME
Version: $DEB_VERSION
Section: games
Priority: optional
Architecture: $ARCH
Depends: $DEPENDS
Maintainer: $MAINTAINER
Description: Powerful and versatile game data scraper for EmulationStation
 Skyscraper gathers game metadata, artwork and videos from online and
 local sources and generates EmulationStation gamelists.
 .
 The package installs the Skyscraper binary to /usr/bin and shared
 data files to /etc/skyscraper.
Homepage: https://github.com/Gemba/skyscraper
EOF

echo "==> Building $DEB_FILE"
dpkg-deb --build "$STAGING_DIR" "$OUT_DIR/$DEB_FILE"

echo "==> Verifying"
dpkg-deb --field "$OUT_DIR/$DEB_FILE"
echo "--- contents (first 45 entries) ---"
# head closes the pipe early (SIGPIPE); without this, `set -o pipefail`
# would fail the build on a successful listing.
dpkg-deb --contents "$OUT_DIR/$DEB_FILE" | head -n 45 || true

echo "OK: $OUT_DIR/$DEB_FILE"
