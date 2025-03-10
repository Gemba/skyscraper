# Windows

The following instructions are provided as-is by Gemba @ GitHub.

## A friendly reminder from muldjord @ GitHub

> First things first: Skyscraper runs under Windows but was not designed for it.
> So you might run into some issues I haven't thought about. So let me be very
> clear about this: I DO NOT SUPPORT THE WINDOWS VERSION! YOU ARE ON YOUR OWN!
>
> TO RUN IT:
> I recommend running it from a Cygwin terminal. That way the color codes work
> as intended and it basically looks as if you are running it under Linux.
>
> If you prefer, you can also run it under ye olde Windows command prompt or
> maybe even Powershell. Note that the output will look odd if you use either
> of these, as they don't support the color escape sequences.
>
> Good luck!

Nothing I (Gemba) have to add here, well, at least this: With a recent Windows
you don't need Cygwin for colorized/ANSI output any longer (see below).

## Install Software

- Get "Git for Windows" and install with default values
- Checkout Skyscraper from [1] into a folder. This folder is
  `<skyscraper_git>` in following.
- Get Qt Creator from [2]:
  Follow link "Download the Qt Online Installer", then select "Offline installer".
- In the Installer in the stage _Select Components_: Enable checkbox for _Qt_ ->
  _Qt <version>_ -> _LLVM-MinGW <version> 64-bit_ and also _Qt_ -> _Build Tools_
  -> _LLVM-MinGW <version> 64-bit_.
- Or: If you have already a Qt installation: Run _Tools_ -> _Maintenance Tool_
  -> _Start Maintenance Tool_ make the selection as outlined in the previous
  step.

## Build Skyscraper

1. Start Qt Creator
2. Set Compiler (via _Manage Kits..._ in Qt Creator) to
   "Desktop Qt <version> LLVM MinGW 64-Bit" (set Default)
3. Open skyscraper project (Projects (Wrench icon) from toolbar left):
   - In _Build and Run_ -> _Build Steps_ (expand _Details_) enter into
     _Additional Arguments_: `QT+=core5compat`, do this for Debug and Release
     build configuration
   - Adjust Build directory (e.g. `<skyscraper_git>\build`), do this for Debug
     and Release build configuration
4. Hit build (Hammer icon)
5. Check "Compile Output" register for any errors and build details

## Running

1. Open a command prompt (cmd.exe or powershell.exe)
2. Add LLVM MinGW to the path, for example `set
   PATH=<qt6-installfolder>\<qt-version>\llvm-mingw_64\bin;%PATH%`. If unsure
   what to add to the PATH run `qmake -query "QT_INSTALL_PREFIX"` and add
   `\bin`. Consider persisting this environment setting.
3. Change to `<skyscraper_git>\build\release`
4. Run `.\Skyscraper.exe --help`
5. Check that the supported platforms (`-p`) are listed and are not empty
6. Run `.\Skyscraper.exe --version`: You should get colored output (ANSI
   Terminal Control)
7. Smile :)

## Notes

- Tested with Qt 6.8.2 Open Source Ed., LLVM MinGW 17.0.6, Qt Creator 15.0.1
  (Community) on Windows 10 and 11 Pro (64Bit)
- Path for config files is `%USERPROFILE%/.skyscraper`

## Refs

[1] https://github.com/Gemba/skyscraper.git
[2] https://www.qt.io/download-open-source
