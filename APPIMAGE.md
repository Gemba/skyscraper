**Making AppImage for use on Batocera etc**<br>
*Note: this is how **I** did it. Some steps might be superflous, I don't know.*

## Build environment

- WSL2
- Debian Trixie/Sid
- Qt 6.10.2
- x86_64

## Tested target

- Batocera v43
- x86_64

# 1. Install dependencies
```
sudo apt update
sudo apt install -y \
  git make gcc g++ python3 wget file patchelf \
  qt6-base-dev qmake6 qt6-base-dev-tools \
  qt6-declarative-dev-tools \
  libqt6sql6-sqlite p7zip-full
```
# 2. Clone Skyscraper
```
mkdir -p ~/git
cd ~/git
git clone https://github.com/Gemba/skyscraper.git
cd skyscraper
```
# 3. Clean old qmake state
```
make distclean 2>/dev/null || true
rm -f Makefile .qmake.stash
```
# 4. Configure with AppImage-friendly paths
```
PREFIX=/usr SYSCONFDIR=/usr/etc qmake6 skyscraper.pro
```
# 5. Confirm paths are correct
```
grep SYSCONFDIR Makefile
```
You wanna see something like:
```
-DPREFIX="/usr" -DSYSCONFDIR="/usr/etc"
```
Not `/usr/local`.

# 6. Build
```
make -j"$(nproc)"
```
# 7. Create AppDir
```
rm -rf AppDir
make install INSTALL_ROOT="$PWD/AppDir"
```
# 8. Verify binary location
```
ls -l AppDir/usr/bin/Skyscraper
```
# 9. Add missing runtime libraries for Batocera compatibility
```
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/lib/
cp /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 AppDir/usr/lib/
```
# 10. Create desktop file
```
cat > skyscraper.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=Skyscraper
Exec=Skyscraper
Icon=skyscraper
Categories=Utility;
Terminal=true
EOF
```
# 11. Add icon
```
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps
cp docs/resources/skyscraper_logo.png \
   AppDir/usr/share/icons/hicolor/256x256/apps/skyscraper.png
```
# 12. Download linuxdeploy
```
wget -O linuxdeploy-x86_64.AppImage \
https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage

wget -O linuxdeploy-plugin-qt-x86_64.AppImage \
https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
```
# 13. Make qmlimportscanner visible
```
export PATH="$PATH:/usr/lib/qt6/libexec"
```
# 14. Build AppImage
```
ARCH=x86_64 ./linuxdeploy-x86_64.AppImage \
  --appdir AppDir \
  --desktop-file skyscraper.desktop \
  --output appimage
```
# 15. Test locally
```
./Skyscraper-x86_64.AppImage --help
```
# 16. Test contents
```
./Skyscraper-x86_64.AppImage --appimage-extract
find squashfs-root -name peas.json
find squashfs-root -name 'libstdc++.so*'
find squashfs-root -name 'libgcc_s.so*'
```

## Key takeaways:

- You need to run `make install`, otherwise `peas.json` won't be found.
- Do not use /usr/local, as AppImage will then look for /usr/local/etc/skyscraper on the host system.
- Use PREFIX=/usr SYSCONFDIR=/usr/etc.
- Batocera does not include CXXABI_1.3.15, so libstdc++.so.6 and libgcc_s.so.1 must be included.
- linuxdeploy-plugin-qt may require qmlimportscanner to be in PATH if you use --plugin qt.

## The most important line is:
```
PREFIX=/usr SYSCONFDIR=/usr/etc qmake6 skyscraper.pro
```
<br><br><br>

# Troubleshooting
If you are building from a completely empty AppDir, you may still need to run
`--plugin qt` once to include the Qt libraries.

If AppDir already contains the Qt libraries, the command above is sufficient.

## Check compiled paths
```
strings Skyscraper | grep -E 'usr/.*/etc/skyscraper'
```
## Verify AppImage contains required runtime libs
```
./Skyscraper-x86_64.AppImage --appimage-extract

find squashfs-root -name 'libstdc++.so*'
find squashfs-root -name 'libgcc_s.so*'
```
## If linuxdeploy cannot find the executable
Depending on how Skyscraper was built, the executable may end up in
`AppDir/usr/local/bin` instead of `AppDir/usr/bin`. In that case,
linuxdeploy may fail to detect it automatically.
```bash
mkdir -p AppDir/usr/bin
cp AppDir/usr/bin/Skyscraper AppDir/usr/bin/Skyscraper 2>/dev/null || \
cp AppDir/usr/local/bin/Skyscraper AppDir/usr/bin/
```
