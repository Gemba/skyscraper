win32 {

  CONFIG += console
  CONFIG(release, debug|release) {
    BUILD_CFG=release
  } else {
    BUILD_CFG=debug
  }
  DEST=$${OUT_PWD}/$${BUILD_CFG}

  !build_pass:message("Config files will be put into '$$(USERPROFILE)\.skyscraper'.")
  !build_pass:message("Existing user's config files will not be overwritten, you may have to diff/merge them manually!")

  WINDEPLOYQT_BIN="$$[QT_INSTALL_PREFIX]/bin/windeployqt.exe"
  QT_LIC_FOLDER="$$[QT_INSTALL_PREFIX]/../../Licenses"

  # Avoid MinGW windres.exe hiccup in generated Skyscraper_resource.rc
  VERSION=$$replace(VERSION, '-dev', '.999')

  # remove build artifacts
  QMAKE_POST_LINK += cmd /c del $$shell_quote($$shell_path($${DEST}\\\\*.cpp)) \
    $$shell_quote($$shell_path($${DEST}\\\\*.o)) \
    $$shell_quote($$shell_path($${DEST}\\\\*.h)) $$escape_expand(\\n\\t)

  # copy licenses
  QMAKE_POST_LINK += cmd /c copy /y $$shell_quote($$shell_path($${PWD}/../LICENSE)) \
    $$shell_quote($$shell_path($${DEST}/LICENSE.Skyscraper)) $$escape_expand(\\n\\t)
  QMAKE_POST_LINK += cmd /c copy /y $$shell_quote($$shell_path($${QT_LIC_FOLDER}/LICENSE)) \
    $$shell_quote($$shell_path($${DEST})) $$escape_expand(\\n\\t)
  QMAKE_POST_LINK += cmd /c copy /y $$shell_quote($$shell_path($${QT_LIC_FOLDER}/LICENSE.GPL3-EXCEPT)) \
    $$shell_quote($$shell_path($${DEST})) $$escape_expand(\\n\\t)

  # determine needed libs and copy into $${DEST}
  QMAKE_POST_LINK += cmd /c $$shell_quote($$shell_path($${WINDEPLOYQT_BIN})) \
    --$${BUILD_CFG} $$shell_quote($$shell_path($${DEST}\\Skyscraper.exe)) \
    $$escape_expand(\\n\\t)

  # create mandatory folders and deploy mandatory files
  QMAKE_POST_LINK += powershell -Command "md -Force '$$shell_path($$(USERPROFILE)/RetroPie/roms)'" > NUL &
  QMAKE_POST_LINK += powershell -NoProfile -NoLogo -NonInteractive -ExecutionPolicy Bypass \
    -File $$shell_quote($$shell_path($${PWD}/deploy_mandatory_files.ps1)) \
    $$shell_quote($${PWD}/..) $$shell_quote($$shell_path($$(USERPROFILE)/.skyscraper)) \
    $$escape_expand(\\n\\t)
}
