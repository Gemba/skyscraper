<h1 align="center">
  <img src="docs/resources/skyscraper_banner.png" alt="Skyscraper" width="640px">
  <br>
</h1>

<h4 align="center">Powerful and versatile game data scraper written in Qt and C++.</h4>

<p align="center">
  <a href="https://github.com/Gemba/skyscraper/actions/workflows/ci.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/Gemba/skyscraper/ci.yml" alt="Build status">
  </a>
  <a href="https://github.com/Gemba/skyscraper/releases">
    <img src="https://img.shields.io/github/v/release/Gemba/skyscraper.svg" alt="Releases">
  </a>
  <a href="https://github.com/Gemba/skyscraper/issues">
    <img src="https://img.shields.io/badge/contributions-welcome-brightgreen.svg" alt="Issues">
  </a>
</p>

<p align="center">
  <a href="#-how-to-install-skyscraper">Installation</a> (<a href="#linux">Linux</a>|<a href="#macos">macOS</a>|<a href="#docker">Docker</a>|<a href="#windows">Windows</a>) •
  <a href="#-how-to-use-skyscraper">Quick Usage</a> •
  <a href="https://gemba.github.io/skyscraper/">User Manual</a> •
  <a href="https://gemba.github.io/skyscraper/CHANGELOG/">Changelog</a>
</p>

---

## 📬 Preface

Since autumn 2023 this repo, which you have reached, is the official successor of Lars'
awesome Skyscraper and also the official version used in RetroPie. Read all
about the [changes since then](https://gemba.github.io/skyscraper/CHANGELOG/). Happy scraping!

Skyscraper was temporarily maintained by [Joe
Huss](https://github.com/detain/skyscraper) (2022-2023). The original project is by
[Lars Muldjord](https://github.com/muldjord/skyscraper) (2017-2022), who did all
the heavy lifting. 

In addition to this brief README, there is the extensive [user
manual](https://gemba.github.io/skyscraper). It contains everything from
[docs/](docs) but with an enhanced layout (mkdocs) which should be easier to
read and navigate.

----

A powerful and versatile yet easy to use game scraper written in C++ for use with multiple frontends running on a Linux system (macOS and Windows too, but not officially supported). It scrapes and caches various game resources from various scraping sources, including media such as screenshot, cover and video. It then gives you the option to generate a game list and artwork for the chosen frontend by combining all of the cached resources.

All Skyscraper features are [well-documented](https://gemba.github.io/skyscraper/) and there's also a [F.A.Q](https://gemba.github.io/skyscraper/FAQ/) with answers.

## 🎮 Platforms Supported (set with '-p'):
Skyscraper supports scraping of 125 platforms/systems out of the box. These are defined in the [peas.json](peas.json) and they also get displayed with `Skyscraper --help`.

You can configure and add platforms without having the needs to edit the code source (or file a pull request). This feature is achieved by adding new config files. You should only edit the `peas_local.json` / `platforms_idmap_local.csv` counterparts of these files:
- [peas.json](peas.json): Describes now the supported platforms and recognized gamefile extensions by Skyscraper. See all details in the [platforms documentation](https://gemba.github.io/skyscraper/PLATFORMS/).
- [platforms_idmap.csv](platforms_idmap.csv): Maps the local platform name to the platform ID of screenscraper.fr, Mobygames or The Games DB web API, this yields more accurate hits.

These files are copied into the folder `/home/pi/.skyscraper` on RetroPie (or `/usr/local/etc/skyscraper/` in general) at the first run of the program.

## 🕹 Frontends Supported (set with '-f'):
* EmulationStation
* ES-DE (EmulationStation Desktop Edition)
* AttractMode
* Pegasus
* RetroBat

## 📚 Supported scraping modules (set with '-s')
Skyscraper supports a variety of different scraping sources called *scraping modules*. Use these to gather game data into the Skyscraper resource cache. Check the full list of [scraping modules](https://gemba.github.io/skyscraper/SCRAPINGMODULES/) and read more about the [resource cache](https://gemba.github.io/skyscraper/CACHE/).

## 🧑‍💻 Code contributions
I welcome any contributions, although I would like to keep things backwards compatible.

## 🔨 How to install Skyscraper
Follow the steps below to install the latest version of Skyscraper. Lines beginning with `$` signifies a command you need run in a terminal on the machine you wish to install it on.

NOTE! If you are using the RetroPie distribution, you have the option to install Skyscraper directly from the RetroPie-Setup script (*you need to update the script before installing it!*). Read more about all of that [in the RetroPie documentation](https://retropie.org.uk/docs/Scraper/#skyscraper).

### Installation of Skyscraper on RetroPie and Programmable Completion
This goes in the usual RetroPie stanza: Either run `sudo RetroPie-Setup/retropie_setup.sh` and folow the menus (_Manage packages_ -> _Manage optional packages_ -> then look for _Skyscraper_) or run `sudo RetroPie-Setup/retropie_packages.sh skyscraper`. This will also automagically install programmable completion (aka. bash completion) for the Skyscraper command line (see [command line completion](https://gemba.github.io/skyscraper/CLIHELP#programmable-completion)).

### Installation Prerequisites on Other Systems or Architectures

_Qt5 is end-of-life by end of May 2025_: You can still use and compile Skyscraper with Qt5 with no restrictions, when your setup does not provide Qt6. However, if you have the option to use Qt6 I strongly recommend it, Skyscraper works well with Qt6. Use Qt6 especially when you use Skyscraper on a recent Linux distribution, macOS or Windows. In rare cases you may have to use `qmake6` instead of `qmake`. If you find a mismatch in the build scripts, please file an issue.

#### Linux
_For Qt6_:
```bash
$ sudo apt update
$ sudo apt install qt6-base-dev qmake6 qt6-base-dev-tools libqt6sql6-sqlite p7zip-full
```

_For Qt5 (these are legacy installation prerequisites!)_: Skyscraper needs Qt5.11 or later to compile. For Ubuntu or other Debian derived distro, you can install Qt5 using the following commands:
```bash
$ sudo apt update
$ sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5sql5-sqlite p7zip-full
# You may need these too, if they are not installed already
$ sudo apt install make g++ gcc git
```
To install Qt5 on other Linux distributions, please refer to their documentation.

#### macOS
Skyscraper works perfectly on macOS as well but is not officially supported as I don't own a Mac. Here are the commands needed to install the Qt6 and other prerequisites. You can skip the Qt5 _un_install if you don't have it installed:
```
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
$ brew uninstall qt@5
$ brew install gnu-tar
$ brew install wget
$ brew install qt
```
You may also need a recent installation of [XCode](https://xcodereleases.com/) for the development tools. Then proceed to the default installation instructions in the "Download, compile and install" section. It should work and give you a working installation of Skyscraper.

#### Docker 
Two Docker setups exist: One general in the `docker/` folder. The other resides in the `.devcontainer/` and its use is for [MS Dev Containers](https://microsoft.github.io/code-with-engineering-playbook/developer-experience/devcontainers/).

#### Windows
Windows is not officially supported at this time. However, you [may roll your own](win32/README.md) Windows 64-bit version that works just fine with a recent Windows. It even supports colored terminal output.

### Download, compile and install
When you've installed the prerequisites as described above for Linux or macOS, you can install Skyscraper by typing in the following commands:
```bash
$ cd
$ mkdir -p skysource && cd skysource
$ wget -q -O - https://raw.githubusercontent.com/Gemba/skyscraper/master/update_skyscraper.sh | bash -s --
```
The last command will download and run the latest update script from Github. The script installs the latest release of Skyscraper. During the installation you might be asked for your sudo password. On RetroPie the default password is `raspberry`.  
Default prefix for installation is `/usr/local/`. If you want to change this add `PREFIX=` after the pipe, thus it reads `PREFIX=/here/goes/skyscraper/ bash -s --`  
If you want to compile Skyscraper with XDG support add `xdg` at the very end, thus it reads `bash -s -- xdg`  
If you want also bash completion, then copy the [Skyscraper.bash](https://github.com/Gemba/skyscraper/blob/master/supplementary/bash-completion/Skyscraper.bash) to the folder of bash completion scripts according to your distribution. 

When the script has completed you are ready to run Skyscraper!

### Updating Skyscraper
You can update to the latest version by running the following commands:
```bash
$ cd
$ cd skysource
$ ./update_skyscraper.sh
```

Default prefix for installation is `/usr/local/`. If you want to change this add `PREFIX=` before the script e.g., `PREFIX=/here/goes/skyscraper ./update_skyscraper.sh`  
If you want to compile Skyscraper with XDG support supply the positional argument `xdg` to the script e.g., `./update_skyscraper.sh xdg`  
You might be asked for your sudo password during the update. On RetroPie the default password is `raspberry`.  

### Installing the Development Version
If you want to build the latest `main/HEAD` version use the following commands. Make sure to have the before mentioned packages installed:
```bash
git clone --depth 1 https://github.com/Gemba/skyscraper.git
cd skyscraper
[[ -f Makefile ]] && make --ignore-errors clean
rm --force .qmake.stash
# You may need to issue qmake6 with Qt6 for the next command instead of qmake
qmake  # Add also PREFIX=/path/to before qmake if you want a different PREFIX than /usr/local
make -j$(nproc)
sudo make install
```

### How to uninstall Skyscraper
If you've installed Skyscraper using the instructions in this readme, you can uninstall it using the following commands:
```bash
$ cd
$ cd skysource
$ sudo make uninstall
$ cd
$ rm -Rf skysource
$ rm -Rf ~/.skyscraper
```
You might be asked for your sudo password during the processs. On RetroPie the default password is `raspberry`.

## 🪄 How to use Skyscraper
_IMPORTANT_: In order for Skyscraper to work properly, it is necessary to quit your frontend before running it! If you're running EmulationStation, you can quit it by pressing F4.

Remember, you can completely customize the artwork Skyscraper exports (artwork). Read all about in the [artwork documentation](https://gemba.github.io/skyscraper/ARTWORK/). If you just want to use the default (IMHO pretty cool looking) artwork Skyscraper provides, read on.

### A simple use case
For first-time users I recommend reading the short and to-the-point [use case](https://gemba.github.io/skyscraper/USECASE/). Please read it and get back here when you got the gist of it.

### A quick run-down of Skyscraper
Skyscraper is a command line tool, and has many, many options for you to fiddle around with. I recommend taking a look at all of them to familiarize yourself with the possibilites:
```bash
$ Skyscraper --help
$ Skyscraper --flags help
$ Skyscraper --cache help
```
This will give you a description of everything Skyscraper can do if you feel adventurous! For a thorough description of all available options, check the extensive [command line documentation](https://gemba.github.io/skyscraper/CLIHELP/).

The most important ones are probably:
* `-p <PLATFORM>`
* `-s <SCRAPING MODULE>`
* `-u <USER:PASS>` or `-u <KEY>`
* `--cache refresh`
* `--flags videos`

If you have your roms in a non-default location (default for RetroPie users is `/home/<USER>/RetroPie/roms/<PLATFORM>`) or wish to export the game list or artwork to non-default locations, you will also need these:
* `-i <PATH>`
* `-g <PATH>`
* `-o <PATH>`

For almost any command line option, consider setting them in the `/home/<USER>/.skyscraper/config.ini` file as described in the detailed [config file documentation](https://gemba.github.io/skyscraper/CONFIGINI/). This will make the options permanent so you don't need to type them in all the time.

#### Gathering data for a subset of roms
Skyscraper offers several ways of gathering data for a subset of roms. If you just want to scrape the roms that have no data in the cache whatsoever, you can do so with the `--onlymissing` command-line option. You can also check out the `--startat FILENAME` and `--endat FILENAME` options. If you just want to gather data for a couple of roms you can simply add the filename(s) to the end of the command-line (eg. `Skyscraper -p amiga -s openretro "/path/to/rom name 1.lha" "/path/to/rom name 2.lha"`). And probably the most advanced (and quite handy) way to gather data for a subset of roms is to make use of the `--cache report:missing=RESOURCE` option. This can generate a report containing the filenames that are missing a certain resource. You can then feed the report back into Skyscraper with the `--includefrom REPORTFILE` afterwards. Skyscraper will then only scrape the files contained in the report.

### config.ini
A lesser known, but extremely useful, feature of Skyscraper is to add your desired config variables to `/home/<USER>/.skyscraper/config.ini`. Any options set in this file will be used by default by Skyscraper. So if you always use, for example, `-i <SOME FOLDER>` on command line, you can set the matching option `inputFolder="<SOME FOLDER>"` in the config.

For a full description of all availabe config options, check the [config file documentation](https://gemba.github.io/skyscraper/CONFIGINI/).

### Resource cache
One of Skyscraper's most powerful features is the resource cache. It's important to understand how this works in order to use Skyscraper to its full potential. Read more about it in the [cache documentation](https://gemba.github.io/skyscraper/CACHE/).

### Custom data
I addition to allowing scraping from locally cached resources, Skyscraper also allows you to import your own data into the resource cache with the `-s import` scraping module. You can also edit existing resources in the cache or add individual `user` resources with the `--cache edit` command. Lastly, you also have the option of importing existing EmulationStation game list data into the Skyscraper resource cache if you need it. You can do this with the `-s esgamelist` scraping module.

To read more about any of the features described above, please check out all of the [documentation](https://gemba.github.io/skyscraper/).

### Artwork look and effects
Check the full [artwork documentation](https://gemba.github.io/skyscraper/ARTWORK/)

## 📑 Previous Release Notes
Release notes for older releases which this fork builds on can be found in [docs/OLDERRELEASES.md](docs/OLDERRELEASES.md).
