The following instructions are provided as-is by Gemba @ GitHub.

# A friendly reminder from muldjord @ GitHub

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

Nothing I (Gemba) have to add here.

# Install Software

- Get "Git for Windows" and install with default values
- Checkout Skyscraper from [1] into a folder. This folder is
  `<skyscraper_git>` in following.
- Get Qt Creator from [2]:
  Follow link "Download the Qt Online Installer", then select "Offline installer".
- In Installer: Select _Developer and Designer Tools_, expand subtree and
  select _MinGW 7.3.0 64-Bit_, choose also from _Qt/Qt<version>/MinGW 7.3.0 64-Bit_

# Build Skyscraper

1. Start Qt Creator
2. Set Compiler (via _Manage Kits..._ in Qt Creator) to
   "Desktop Qt 5.12.12 MinGW 64-Bit" (set Default)
3. Open skyscraper project (Projects (Wrench icon) from toolbar left):
   - Adjust Build directory (e.g. `<skyscraper_git>\build`), do this for Debug
     and Release build configuration
4. Hit build (Hammer icon)
5. Check "Compile Output" register for any errors and build details

# Running

1. Open a command prompt
2. Change to `<skyscraper_git>\build\release`
3. Run `.\Skyscraper.exe --help`
4. Check that the available platforms are listed and are not empty
5. Smile :)

# Notes

- Tested with Qt 5.12.12 Open Source Ed., MinGW 7.3.0 64-bit, Qt Creator 12.0.1 (Community) on Windows 10 and 11 Pro (64Bit)
- Home Path is `%USERPROFILE%/.skyscraper` (instead of `/home/pi/.skyscraper`)

# Refs

[1] https://github.com/Gemba/skyscraper.git
[2] https://www.qt.io/download-open-source
