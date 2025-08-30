## Overview

If you are a fellow Linux user you may be familiar with the XDG Base Directory
Specification (BDS) as part of the freedesktop.org project. If not, you may read
the [brief specification](https://specifications.freedesktop.org/basedir-spec/latest/), the
[ArchWiki](https://wiki.archlinux.org/title/XDG_Base_Directory) has also
additional information and list tools that support the specification. You may also bend over to
[wikipedia](https://en.wikipedia.org/wiki/Freedesktop.org#Base_Directory_Specification) which
has a brief section on the topic.

!!! tip

    In this documentation you will note paths and file locations which relate to the
    "genuine Skyscraper" approach. With the table below you can map these
    locations to their counterpart in XDG BDS.

### File Locations: Genuine- and XDG-Skyscraper

|   Type   | Genuine Skyscraper Location                                          | XDG BDS Location                       |
| :------: | -------------------------------------------------------------------- | -------------------------------------- |
|  CONFIG  | All files, but no subfolders of `~/.skyscraper/*`                    | `$XDG_CONFIG_HOME/skyscraper/`         |
|  CACHE   | All of `~/.skyscraper/cache/`                                        | `$XDG_CACHE_HOME/skyscraper/`          |
|  IMPORT  | All of `~/.skyscraper/import/`                                       | `$XDG_DATA_HOME/skyscraper/import/`    |
| RESOURCE | All of `~/.skyscraper/resources/`                                    | `$XDG_DATA_HOME/skyscraper/resources/` |
|   LOG    | Skipped files/games during processing `~/.skyscraper/skipped*.txt`   | `$XDG_STATE_HOME/skyscraper/`          |
|  REPORT  | Cache reports `~/.skyscraper/reports/*`                              | `$XDG_STATE_HOME/skyscraper/reports/`  |
|   LOG    | Rare Screenscraper error log `~/.skyscraper/screenscraper_error.txt` | `$XDG_STATE_HOME/skyscraper/`          |

!!! warning "Skyscraper is strict"

    As per specification: All paths must be absolute, relative paths will be
    ignored by Skyscraper. Non-existing paths will be created by Skyscraper.

### How to Enable XDG

The XDG feature can be used with any modern Linux distribution, but should _not_
be enabled on RetroPie installments as they rely on the genuine Skyscraper
folder structure.

Review the Skyscraper project file (`skyscraper.pro`). Enable the line
`DEFINES+=XDG`, rebuild and install Skyscraper. Then transfer the files from the
genuine Skyscraper location to their XDG counterpart. If you start from scratch
Skyscraper will be deploy its configuration files to the XDG destinations (by
default sourced from `/usr/local/etc/skyscraper`).

If you want to verify if Skyscraper is compiled with XDG support run `Skyscraper
--version`. If you see a XDG label below the version number it means XDG is
enabled and supported.

To disable XDG support undo the steps above.

!!! note "The Fine Print"

    Skyscraper supports all user directories of the XDG BDS (except
    `$HOME/.local/bin` for user binaries). The system
    directories properties (`XDG_DATA_DIRS` and `XDG_CONFIG_DIRS`) are
    not utilized by Skyscraper.
