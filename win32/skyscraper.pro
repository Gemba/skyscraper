win32 {

  CONFIG += console
  CONFIG(release, debug|release) {
    BUILD_CFG=release
  } else {
    BUILD_CFG=debug
  }
  DEST=$${OUT_PWD}/$${BUILD_CFG}

  !build_pass:message("Config files will be put into '$$(USERPROFILE)\\.skyscraper'.")
  !build_pass:message("Existing user's config files will not be overwritten, you may have to diff/merge them manually!")

  WINDEPLOYQT_BIN="$$[QT_INSTALL_PREFIX]/bin/windeployqt6.exe"
  QT_LIC_FOLDER="$$[QT_INSTALL_PREFIX]/../../Licenses"

  # Avoid MinGW windres.exe hiccup in generated Skyscraper_resource.rc
  VERSION=$$replace(VERSION, '-dev', '.999')

  # remove build artifacts
  QMAKE_POST_LINK += cmd /c del $$system_quote($$system_path($${DEST}\\\\*.cpp)) \
    $$system_quote($$system_path($${DEST}\\\\*.o)) \
    $$system_quote($$system_path($${DEST}\\\\*.h)) $$escape_expand(\\n\\t)

  # copy licenses
  QMAKE_POST_LINK += cmd /c copy /y $$system_quote($$system_path($${PWD}/../LICENSE)) \
    $$system_quote($$system_path($${DEST}/LICENSE.Skyscraper)) $$escape_expand(\\n\\t)
  QMAKE_POST_LINK += cmd /c copy /y $$system_quote($$system_path($${QT_LIC_FOLDER}/LICENSE)) \
    $$system_quote($$system_path($${DEST})) $$escape_expand(\\n\\t)
  QMAKE_POST_LINK += cmd /c copy /y $$system_quote($$system_path($${QT_LIC_FOLDER}/LICENSE.GPL3-EXCEPT)) \
    $$system_quote($$system_path($${DEST})) $$escape_expand(\\n\\t)

  # determine needed libs and copy into $${DEST}
  QMAKE_POST_LINK += cmd /c $$system_quote($$system_path($${WINDEPLOYQT_BIN})) \
    --no-system-dxc-compiler --no-system-d3d-compiler --no-opengl-sw \
    $$system_quote($$system_path($${DEST}\\Skyscraper.exe)) \
    $$escape_expand(\\n\\t)

  # create mandatory folders and deploy mandatory files
  QMAKE_POST_LINK += powershell -Command "md -Force '$$system_path($$(USERPROFILE)/RetroPie/roms)'" > NUL &
  QMAKE_POST_LINK += powershell -NoProfile -NoLogo -NonInteractive -ExecutionPolicy Bypass \
    -File $$system_quote($$system_path($${PWD}/deploy_mandatory_files.ps1)) \
    $$system_quote($${PWD}/..) $$system_quote($$system_path($$(USERPROFILE)/.skyscraper)) \
    $$escape_expand(\\n\\t)
}
