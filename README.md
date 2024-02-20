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
  <a href="#how-to-install-skyscraper">Installation</a> •
  <a href="#how-to-use-skyscraper">Quick Usage</a> •
  <a href="https://gemba.github.io/skyscraper/">User Manual</a> •
  <a href="https://gemba.github.io/skyscraper/CHANGELOG/">Changelog</a>
</p>

---

## Preface

Since autumn 2023 this repo you have reached is the official successor of Lars'
awesome Skyscraper and also the official version used in RetroPie. Read all
about the change since then [here](https://gemba.github.io/skyscraper/CHANGELOG/). Happy scraping!

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

## 🎮Platforms Supported (set with '-p'):
Check the full list of platforms [here](https://gemba.github.io/skyscraper/PLATFORMS/).

## 🕹Frontends Supported (set with '-f'):
* EmulationStation
* AttractMode
* Pegasus
* RetroBat

## 📚Supported scraping modules (set with '-s')
Skyscraper supports a variety of different scraping sources called *scraping modules*. Use these to gather game data into the Skyscraper resource cache. Check the full list of scraping modules [here](https://gemba.github.io/skyscraper/SCRAPINGMODULES/) and read more about the resource cache [here](https://gemba.github.io/skyscraper/CACHE/).

## 🧑‍💻Code contributions
I welcome any contributions, although I would like to keep things backwards compatible.

## How to install Skyscraper
Follow the steps below to install the latest version of Skyscraper. Lines beginning with `$` signifies a command you need run in a terminal on the machine you wish to install it on.

NOTE! If you are using the RetroPie distribution, you have the option to install Skyscraper directly from the RetroPie-Setup script (*you need to update the script before installing it!*). Read more about all of that [here](https://retropie.org.uk/docs/Scraper/#skyscraper). If not, read on.

### Installation of Skyscraper Enhanced on RetroPie

This goes in the usual RetroPie stanza: Either run `sudo RetroPie-Setup/retropie_setup.sh` and folow the menus (_Manage packages_ -> _Manage optional packages_ -> then look for _Skyscraper_) or run `sudo RetroPie-Setup/retropie_packages.sh skyscraper`.

### Installation Prerequisites on Other Systems or Architectures
#### Linux
Skyscraper needs Qt5.3 or later to compile. For a Retropie, Ubuntu or other Debian derived distro, you can install Qt5 using the following commands:
```
$ sudo apt update
$ sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools p7zip-full
```
You might be asked for your sudo password. On RetroPie the default password is `raspberry`. To install Qt5 on other Linux distributions, please refer to their documentation.

#### macOS
Skyscraper works perfectly on macOS as well but is not officially supported as I don't own a Mac. But with the help of HoraceAndTheSpider and abritinthebay here's the commands needed to install the Qt5 and other prerequisites:
```
$ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
$ brew install gnu-tar
$ brew install wget
$ brew install qt@5
$ brew link qt@5 --force
```
If that went well, proceed to the default installation instructions in the next section. It should work and give you a working installation of Skyscraper.

### Download, compile and install
When you've installed the prerequisites as described above for Linux or macOS, you can install Skyscraper by typing in the following commands:
```
$ cd
$ mkdir skysource
$ cd skysource
$ wget -q -O - https://raw.githubusercontent.com/Gemba/skyscraper/master/update_skyscraper.sh | bash
```
The last command will download and run the latest update script from Github. The script installs the latest release of Skyscraper. During the installation you might be asked for your sudo password. On RetroPie the default password is `raspberry`.

When the script has completed you are ready to run Skyscraper!

### Updating Skyscraper
From Skyscraper 2.3.2 and newer you can update to the latest version simply by running the following commands:
```
$ cd
$ cd skysource
$ ./update_skyscraper.sh
```
You might be asked for your sudo password during the update. On RetroPie the default password is `raspberry`. If your version is older than 2.3.2 (check with `--help`) you need to follow the [installation instructions](#download-compile-and-install) instead.

### Installing the Development Version
If you want to build the latest `main/HEAD` version use the following commands. Make sure to have the before mentioned packages installed:
```
git clone --depth 1 https://github.com/Gemba/skyscraper.git
cd skyscraper
make --ignore-errors clean
rm --force .qmake.stash
QT_SELECT=5 qmake 
make -j$(nproc)
sudo make install
```

### How to uninstall Skyscraper
If you've installed Skyscraper using the instructions in this readme, you can uninstall it using the following commands:
```
$ cd
$ cd skysource
$ sudo make uninstall
$ cd
$ rm -Rf skysource
$ rm -Rf ~/.skyscraper
```
You might be asked for your sudo password during the processs. On RetroPie the default password is `raspberry`.

### Windows
Windows is not officially supported at this time! However, you [may roll your own](win32/README.md) Windows 64-bit version that works just fine. And just to be clear: You are on your own if you use this version - please don't ask me questions about it. Use the sources.

## How to use Skyscraper
IMPORTANT!!! In order for Skyscraper to work properly, it is necessary to quit your frontend before running it! If you're running EmulationStation, you can quit it by pressing F4.

Remember, you can completely customize the artwork Skyscraper exports. Check out the documentation [here](https://gemba.github.io/skyscraper/ARTWORK/). If you just want to use the default (pretty cool looking) artwork Skyscraper provides, read on.

### A simple use case
For first-time users I recommend reading the short and to-the-point [use case](https://gemba.github.io/skyscraper/USECASE/). Please read it and get back here when you got the gist of it.

### A quick run-down of Skyscraper
Skyscraper is a command line tool, and has many, many options for you to fiddle around with. I recommend taking a look at all of them to familiarize yourself with the possibilites:
```
$ Skyscraper --help
$ Skyscraper --flags help
$ Skyscraper --cache help
```
This will give you a description of everything Skyscraper can do if you feel adventurous! For a thorough description of all available options, check [here](https://gemba.github.io/skyscraper/CLIHELP/).

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

For almost any command line option, consider setting them in the `/home/<USER>/.skyscraper/config.ini` file as described [here](https://gemba.github.io/skyscraper/CONFIGINI/). This will make the options permanent so you don't need to type them in all the time.

#### Gathering data for a subset of roms
Skyscraper offers several ways of gathering data for a subset of roms. If you just want to scrape the roms that have no data in the cache whatsoever, you can do so with the `--onlymissing` command-line option. You can also check out the `--startat FILENAME` and `--endat FILENAME` options. If you just want to gather data for a couple of roms you can simply add the filename(s) to the end of the command-line (eg. `Skyscraper -p amiga -s openretro "/path/to/rom name 1.lha" "/path/to/rom name 2.lha"`). And probably the most advanced (and quite handy) way to gather data for a subset of roms is to make use of the `--cache report:missing=RESOURCE` option. This can generate a report containing the filenames that are missing a certain resource. You can then feed the report back into Skyscraper with the `--fromfile REPORTFILE` afterwards. Skyscraper will then only scrape the files contained in the report.

### config.ini
A lesser known, but extremely useful, feature of Skyscraper is to add your desired config variables to `/home/<USER>/.skyscraper/config.ini`. Any options set in this file will be used by default by Skyscraper. So if you always use, for example, `-i <SOME FOLDER>` on command line, you can set the matching option `inputFolder="<SOME FOLDER>"` in the config.

For a full description of all availabe config options, check [here](https://gemba.github.io/skyscraper/CONFIGINI/).

### Resource cache
One of Skyscraper's most powerful features is the resource cache. It's important to understand how this works in order to use Skyscraper to its full potential. Read more about it [here](https://gemba.github.io/skyscraper/CACHE/).

### Custom data
I addition to allowing scraping from locally cached resources, Skyscraper also allows you to import your own data into the resource cache with the `-s import` scraping module. You can also edit existing resources in the cache or add individual `user` resources with the `--cache edit` command. Lastly, you also have the option of importing existing EmulationStation game list data into the Skyscraper resource cache if you need it. You can do this with the `-s esgamelist` scraping module.

To read more about any of the features described above, please check out all of the documentation [here](https://gemba.github.io/skyscraper/).

### Artwork look and effects
Check the full artwork documentation [here](https://gemba.github.io/skyscraper/ARTWORK/)

## Skyscraper Configurable Platforms Enhancement

The main goal of this fork is to allow users to easily configurate and add platforms without having the needs to edit the code source directly.

This feature is achieved by adding new config files:
- [peas.json](peas.json): Describes now the supported platforms by Skyscraper. See all details [here](https://gemba.github.io/skyscraper/PLATFORMS/).
- [platforms_idmap.csv]platforms_idmap.csv): Maps the local platform name to the platform ID of screenscraper.fr, Mobygames or The Games DB web API.

These files are copied in the folder `/home/pi/.skyscraper` on RetroPie (or `/usr/local/etc/skyscraper/` in general) at the first run of the program if you want to edit them after an installation.  

## Previous Release Notes

Release notes for older releases which this fork builds on can be found [here](docs/OLDERRELEASES.md).
