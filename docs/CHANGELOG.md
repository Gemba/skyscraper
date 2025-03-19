## Changes

This page summarizes the changes of each Skyscraper release, a [changlog for
humans](https://keepachangelog.com).

### Version 3.17.0 (2025-TBA)

- Updated: Query keyword `romnom=` in `--query='...'` for scraper Screenscraper
  is now optional. It gets added automagically when no other query keyword is
  applied.
- Updated: `mamemap.csv` for MAME 0.275

### Version 3.16.0 (2025-03-10)

- Added: Support for local GameBase DB scraping. See
  [here](SCRAPINGMODULES.md#gamebase-db) and [here](CONFIGINI.md#gamebasefile)
  for details. _To the RetroPie users_: Please update the scriptmodule in the
  RetroPie-Setup first before updating Skyscraper.
- Added: Documentation on scraper modules supplied with [scraper
  capabilities](SCRAPINGMODULES.md#capabilities-of-scrapers).
- Added: Documentation on options for --query parameter per scraper modules. See
  [here](SCRAPINGMODULES.md#recognized-keywords-in-query).
- Added: IGDB scraper now supports Screenshot and Cover scraping, plus it
  allows querying with game ID `--query="id=..."`.
- Updated: ArcadeDB scraper now downloads HD video if present, failsafes to
  default video.
- Updated: Tip of the day is now word-wrapped and uses specific paths instead of
  `/home/USER/...`
- Changed: Backward compability limited to Qt 5.11 (Debian Buster), earlier Qt
  versions are no longer supported. However, Skyscraper is fully compatible to
  Qt6 on Linux, macOS and Windows.
- Fixed: Various edge cases.

### Version 3.15.0 (2025-01-24)

- Added: Separate local Platform configuration from upstream Platform
  configuration. Details
  [here](PLATFORMS.md#transferring-local-platform-changes). _To the RetroPie users_:
  Please update the scriptmodule in the RetroPie-Setup first before updating via
  `retropie_setup.sh`, otherwise you have to install the script named in the
  documentation above manually. Thanks for the nudge, @s1eve-mcdichae1.
- Added: Configuration option `gameListFilename`. See
  [documentation](CONFIGINI.md#gamelistfilename), there is also a [command line
  option](CLIHELP.md#-gamelistfilename-filename). Thanks #1, @Leukhos for the
  submission.
- Added: Support "all platform selection" for cache commands within one Skyscraper
  run. See respective documentation for [cache
  report:missing](CLIHELP.md#-cache-reportmissingall-textual-artwork-media-or-resource1resource2),
  [cache validate](CLIHELP.md#-cache-validate), [cache
  vacuum](CLIHELP.md#-cache-vacuum) and [cache
  purge](CLIHELP.md#-cache-purgekeywordmodule-andor-type). Thanks #2, @Leukhos
  the addition is very appreciated.
- Fixed: Improvements to the Pegasus frontend, to avoid inconsistency with
  existing aliases in the frontend output for the keywords `command/launch` and
  `workdir/cwd`. Thanks, @Leukhos for your third submission.
- Fixed: Some edge case bugs wiped out, thanks to all reporters!

### Version 3.14.0 (2024-12-08)

- Added: Support for [XDG Base Directories](XDG.md), thanks for the suggestion
  @ASHGOLDOFFICIAL.
- Added: Option to allow any delimiter between consecutive brackets and
  parentheses in gamelist title. See
  [`innerBracketsReplace`](CONFIGINI.md#innerbracketsreplace) for examples.
  Thanks for the suggestion, @retrobit.
- Added: Option to retain disc numbering from game filename, when no other
  bracket information is requested. See
  [`keepDiscInfo`](CONFIGINI.md#keepdiscinfo) for details. Thanks, @maxexcloo!
- Added: Option to override year comparison during scraping, if year is present
  in game filename. See
  [`ignoreYearInFilename`](CONFIGINI.md#ignoreyearinfilename).
- Added: [Platform 'Fujitsu
  FM-Towns'](https://github.com/Gemba/skyscraper/pull/95/files). Manually update
  your `peas.json` and `platformid_map.csv` to make use of it.
- Added: Option `--hint`, it shows a random Tip of the Day.
- Updated: Skyscraper's hardcoded `/home/<USER>` replaced with the actual user's
  home directory in messages. Thanks for highlighting it on the Mac,
  @cdaters
- Changed: When an invalid scrape module is provided with `-s` Skyscraper now
  exits. Before this change Skyscraper failed back to cache scraping silently.
- Updated: Check on RetroPie if an existing Skyscraper installation is updated
  at least with RetroPie-Setup 4.8.6 onwards to have the configurable platform
  information deployed (`peas.json`) and provide a remediation to the user, if
  this is violated
- Updated: A downloaded `whdload.xml` file for platform Amiga will be not
  downloaded again until the server indicates. However, manually removing
  `/home/<USER>/.skyscraper/whdload_cached_etag.txt` will force a new download.
- Fixed: Performing Ctrl-C in `--cache edit` mode will now dismiss any changes
  made instead of persisting them
- Fixed: Game rating calculation in Openretro scraping module

### Version 3.13.0 (2024-11-06)

- Added: Option to provide user file `peas_local.json` (same format as
  `peas.json`), to extend platform information or overwrite existing platform
  information
- Updated: `*_platform.json` files as reference for supported platforms of
  various scrapers
- Added: Support for Vircon32 platform in `peas.json`. Thanks for hinting,
  @vircon32
- Removed: `scrapers` entries in `peas.json`, as it did not provide any use.
- Removed: Deprecated flags and options: `includeFiles` superseded by
  `includePattern`; `excludeFiles` superseded by `excludePattern`;
  `gamelistFolder` superseded by `gameListFolder`; `fromfile` superseded by
  `includefrom`. These were deprecated since v3.7.0.

### Version 3.12.0 (2024-07-01)

- Added: Support for scraping of PDF manuals (for scrape modules screenscraper,
  import and esgamelist) and gamelist output with these manuals for frontends
  (ES-DE Frontend and some EmulationStation variants). See configurations
  options [`manuals=true`](CONFIGINI.md#manuals) and
  [`gameListVariants=enable-manuals`](CONFIGINI.md#gamelistvariants). Thanks for
  the initial PR, @pandino
- Added: For frontend ES-DE, evaluate environment variable `ESDE_APPDATA_DIR` if
  present. Thanks for the hint, @ASHGOLDOFFICIAL
- Added: Use also release year as hint on user interaction. This is useful when
  in interactive mode and more than one game with the same name is found.
  Skyscraper can be guided to prefer a specific game when the release year is
  added in parenthesis as part of the ROM name (or alias in `aliasMap.csv`).
  Verbose info in [#59](https://github.com/Gemba/skyscraper/pull/59). Thanks,
  @mjkaye
- Changed: Persistent config option `onlymissing` for counting and scraping only
  games which do not have any game data in the cache. This is a commodity config
  option to the already existing flag with the same name. Plus: If you use a
  scraper with a scraping limit for games to be scraped at once (e.g. MobyGames)
  you may stay below that limit. Documentation [here](CONFIGINI.md#onlymissing).
  Thanks for the suggestion, @sleve_mcdichael
- Update: Valid extensions (= `formats` in Skyscraper's `peas.json` file) with
  info from RetroPie's `platform.cfg` (commit
  [`5e0ab1f`](https://github.com/RetroPie/RetroPie-Setup/blob/5e0ab1f85994cbb51eb5539d2a7592a3578c15b8/platforms.cfg))
- Fixed: The [update
  script](https://github.com/Gemba/skyscraper/blob/master/update_skyscraper.sh)
  for recent macOS versions. Thanks, @calumbrodie

### Version 3.11.0 (2024-04-15)

- Added: Support for EmulationStation Desktop Edition (ES-DE Frontend). Use
  [`frontend=esde`](CONFIGINI.md#frontend) in `config.ini` and see
  [documentation](FRONTENDS.md#emulationstation-desktop-edition-es-de) on the
  default settings. Thanks for the hints and for testing, @maxexcloo, @Nargash
- Added: Entries in
  [`aliasMap.csv`](https://github.com/Gemba/skyscraper/blob/master/aliasMap.csv)
  are now also applicable for Screenscraper. Thanks, @retrobit.
- Added: Enhanced game name detection for ScummVM platform.
- Changed: In module Screenscraper, in some cases if multiple media types are
  queried (NB: most queries are single-typed), now the first type is selected
  with the first matching region. Only if this does exists in any provided
  region, try the next type with all provided regions (in short: type has
  precedence over region). Previously the regions have been tried to match on
  any media type (=region had precedence over type), which resulted in picking
  the alternative type. This approach gave less suitable media files, especially
  for screenshots (type:`ss`) and screenshot from title screen (type:
  `sstitle`).
- Fixed: Some corner-case bugs fixed, thanks to all reporters!

### Version 3.10.0 (2024-02-10)

- Feature: Preserve existing `<folder/>` nodes in gamelist or create skeleton
  `<folder/>` nodes when ROMs are stored in subfolders within a system folder,
  see [frontend documentation](FRONTENDS.md#metadata-preservation) and the [gamelist
  specification](https://github.com/RetroPie/EmulationStation/blob/master/GAMELISTS.md#folder).
- Feature: [Bash Completion on Linux
  installations](CLIHELP.md#programmable-completion). Use ++tab++ twice for
  completion of Skyscraper options. On RetroPie the scriptmodule will handle
  the installation. On non-RetroPie-Linux put the file
  `supplementary/bash-completion/Skyscraper.bash` into
  `$XDG_DATA_HOME/bash-completion/completions/` (`$XDG_DATA_HOME` is
  equivalent to `$HOME/.local/share`). Open a new bash -- et voila!
- Feature: Customizable installation folder when running `make install`. See
  `PREFIX` in `skyscraper.pro`.
- Feature: Improved macOS support. Unified update script
  `update_skyscraper.sh` (thanks, @jeantichoc) and Docker support via Dev
  Container (kudos, @retrobit).
- Fix: Various minor fixes reported from the community on different setups,
  thanks!

### 2023-12-01 (Version 3.9.2)

- Feature: Import of data in XML format is now more lax (does not rely on
  strict identical indention). Read also the hint
  [here](IMPORT.md#textual-data-definitions-file)
- Feature: Configuration option `tidyDesc` added. See documentation
  [here](CONFIGINI.md#tidydesc)
- Feature: Documentation reviewed and hosted with mkdocs for ease of access.
  See [here](https://gemba.github.io/skyscraper)
- Update: Added index of configuration parameters to `CONFIGINI.md`. Details
  see [here](CONFIGINI.md#index-of-options-with-applicable-sections)
- Update: Refactored `skyscraper.cpp` class. Factorised configuration settings
  into `settings.cpp`
- Update: Various other refactorings to remove duplicated code
- Fix: Quit Skyscraper when neither `-p <PLATFORM>` nor `--cache help` nor
  `--flags help` is provided
- Fix: Warning remediated when NULL image was applied in composer/gamebox
  rendering

### 2023-10-22 (Version 3.9.1)

- Feature: Mobygames scraper genres limited to two most relevant genre
  categories ('Basic Genre' and 'Gameplay')
- Update: Removed legacy and unused code
- Update: Code formatting (LLVM)
- Fix: TGDB scraper retrieves screens from `screenshot/` and as well
  `screenshots/` URL path as some platforms (supported since configurable
  platforms) have their screenshots served from `screenshots/`.

### 2023-10-20 (Version 3.9.0)

- Feature: Mobygames scraper respects game id from mobygames.com website via
  `--query=<gameid>`. Handy to hint to the right game information when usual
  search returns false positives.
- Feature: Scrapers which provide an web API (Screenscraper, Mobygames, The
  Games DB (tgdb)) have the full platform information shipped with this release
  (see `<scraper>_platforms.json` files). These files are used as reference.
- Feature: Less 'aliases' maintenance needed in former `platforms.json`.
- Update: Streamlined external platform configuration. File `platforms.json` is
  replaced by `peas.json` (platforms, extensions/formats, aliases and scrapers).
  Precise platform determination for Screenscraper, Mobygames and TGDB via
  `platform_idmap.csv` See [platforms documentation](PLATFORMS.md) for
  details.
- Fix: Failed media download when TGDB provides PNG files instead of JPG files
  and vice versa.
- Update: [Scriptmodule for this Skyscraper](https://github.com/RetroPie/RetroPie-Setup/blob/master/scriptmodules/supplementary/skyscraper.sh) now official part of RetroPie.

### 2023-09-23 (Version 3.8.1.2309)

- Feature: OpenRetro scraper retrieves now also score/rating, if available for a
  game. Precedence is to use reviews from external websites first (right
  header), then the Score label (above the game details). See this
  [example](https://openretro.org/amiga/shadow-of-the-beast)
- Feature: Additionally to the existing import with rating values of 0, 0.5, 1,
  1.5 ... 5 ("Star rating") it is possible to use 0.1 ... 1.0 scale for rating
  in import files (EmulationStation `gamelist.xml` internal rating range). See
  also [import formats](IMPORT.md#resource-formats)
- Fix: Wrong score/rating calculation from Mobygames scraper / Mobygames API.
  See also [this
  gist](https://gist.github.com/Gemba/13f0accddcecd68a356721ebac020d76) on how
  to update your existing Skyscraper `db.xml` files.
- Fix: Use of `--query` free-text search in OpenRetro scraper fixed. This bug
  did not occur when the switch is ommitted and an Amiga WHDLoad file is provided
  to Skyscraper.
- Fix: RetroPie Scriptmodule, removed surplus boolean negation (Thanks
  @s1eve-mcdichae1)
- Fix: RetroPie Scriptmodule, fixed use of legacy option `--unattend` (Thanks
  @windg)
- Update: RetroPie Scriptmodule, relaxed the remove function of the scriptmodule
  to not zap the Skyscraper cache. Plus various cleanups.

### 2023-09-15 (Version 3.8.0.2309)

- updated `mamemap.csv` from MAME 0.240 (Arcade).dat, fbneo.dat,
  mame2003-plus.xml and cleanup of surplus device information
- update script for make `mamemap.csv` does no longer rely on mame binary
- removed discontinued `*.php` scripts
- `platforms.json` sorted and formatted
- updated documentation especially to reflect the supported platforms
- scriptmodule file for RetroPie aligned to their naming convention
- Mobygames platform information refactored from hardwired `mobygames.cpp`.to
  `mobygames.json`

#### Included Pull Requests from Parent Skyscraper Repos

These pull requests from other repos have been merged into this fork.

- [muldjord #362](https://github.com/muldjord/skyscraper/pull/362)
- [detain #14](https://github.com/detain/skyscraper/pull/14) (extensions only)
- [detain #16](https://github.com/detain/skyscraper/pull/16)
- [detain #18](https://github.com/detain/skyscraper/pull/18)
- [detain #21](https://github.com/detain/skyscraper/pull/21)
- [detain #22](https://github.com/detain/skyscraper/pull/22)
- [detain #23](https://github.com/detain/skyscraper/pull/23)
- [detain #24](https://github.com/detain/skyscraper/pull/24)
