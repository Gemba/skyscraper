## Skyscraper Enhanced and Reloaded

<figure markdown>
  ![](resources/skyscraper_banner.png)
</figure>

Welcome to the powerful one-stop scraping-tool to maintain your gamelists!

This repo is the official successor of [Lars Muldjord's
Skyscraper]((https://github.com/muldjord/skyscraper?tab=readme-ov-file#code-contributions-and-forks)).

### Key New Features at a Glance

- Platforms to scrape can be added via configuration files
- Several more platforms added and supported out-of-the-box
- Support for EmulationStation Desktop Edition (ES-DE) Gamelist format
- Added scraping of game manuals as PDF
- Welcoming the 10th scraping module: Ingest GameBase DB data
- Support for XDG Base Directory standard
- Commandline Bash completion on Linux systems
- Various configuration options added to fine-tune scraping and Gamelist
  creation
- The Skyscraper documentation is also available in mkdocs/material layout
- Verified to compile and run on Linux, macOS and Windows

See also [all new features](CHANGELOG.md)

### Backstory

This fork is based on commit `654a31b` (2022-10-26) from [this
fork](https://github.com/detain/skyscraper), which was a short-lived fork of
Skyscraper project after Lars retired his project.

Skyscraper focuses on RetroPie integration but it can also be used without
RetroPie. However, the RetroPie-Setup has a
[scriptmodule](https://github.com/RetroPie/RetroPie-Setup/blob/master/scriptmodules/supplementary/skyscraper.sh)
to install this Skyscraper fork.

In essence Skyscraper only relies on a Std-C++17 toolchain and the Qt framework.

Ready? Let's [dive in](USECASE.md) or use the navigation pane on the left.
