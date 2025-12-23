## Skyscraper Enhanced and Reloaded

<figure markdown>
  ![](resources/skyscraper_banner.png)
</figure>

The powerful and highly customizable scraping-tool to maintain your gamelists!

This repo is the official successor of [Lars Muldjord's
Skyscraper]((https://github.com/muldjord/skyscraper?tab=readme-ov-file#code-contributions-and-forks)).

### Benefits at a Glance

Additions since Skyscraper 3.7.7 (2023):

- Verified to compile and run on Linux, NixOS, macOS and Windows
- Commandline Bash completion on Linux systems
- Platforms (systems) to scrape can be added via configuration files
- Several more platforms added and supported out-of-the-box
- Added support for EmulationStation Desktop Edition (ES-DE) and Batocera
  Gamelist format
- Added scraping of game manuals as PDF, fanart and backcover
- Welcoming the 10th scraping module: Ingest GameBase DB data
- Support for XDG Base Directory standard
- Various configuration options added to fine-tune scraping and Gamelist
  creation
- This extensive Skyscraper documentation is available in [mkdocs/material
  layout](https://gemba.github.io/skyscraper/) and can be easily searched

See also [all new features](CHANGELOG.md)

### Backstory

This fork is based on commit `654a31b` (2022-10-26) from [Detain's
fork](https://github.com/detain/skyscraper), which was a short-lived fork of
Skyscraper project after Lars retired his project. I started to maintain
Skyscraper in autumn 2023.

Skyscraper focuses on RetroPie integration but it can also be used without
RetroPie. However, the RetroPie-Setup has a
[scriptmodule](https://github.com/RetroPie/RetroPie-Setup/blob/master/scriptmodules/supplementary/skyscraper.sh)
to install this Skyscraper fork.

In essence Skyscraper only relies on a Std-C++17 toolchain and the Qt framework
(Version 5 onwards). The [installation documentation](INSTALLATION.md) will have
your setup covered.

Ready? Let's [dive in](USECASE.md) or use the navigation pane on the left.
