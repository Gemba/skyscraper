## Scraping Module Overview

Skyscraper supports several online and local sources when scraping data for your roms. This makes Skyscraper a hugely versatile tool since it also caches any resources that are gathered from any of the modules. The cached data can then be used to generate a game list and composite artwork later.

Choosing a scraping module is as simply as setting the `-s <MODULE>` option when running Skyscraper on the command line. It also requires a platform to be set with `-p <PLATFORM>`. If you leave out the `-s` option Skyscraper goes into _game list generation_ mode and combines your cached data into a game list for the chosen platform and frontend. Read more about the resource cache [here](CACHE.md).

For scraping modules that support or require user credentials you have the option of either setting it on commandline with `-u <USER:PASSWD>` or `-u <KEY>` or better yet, by adding it permanently to the Skyscraper configuration at `/home/<USER>/.skyscraper/config.ini` as described [here](CONFIGINI.md#usercredscredentials-or-key)

### Capabilities of Scrapers

This table summarizes the game metadata provided by each scraping module. Hover
over a table cell to display the scraper as tooltip:

| Capability &rarr;<br>Scraper &darr;        |                Title                |            Release Date             |             Description             |            Max. Players             |              Developer              |              Publisher              |             Genre/Tags              |               Rating                |           Age Recommend.            |                Cover                |             Screenshot              |                  Video                   |                Wheel/Logo                |                 Marquee                  |               Manual (PDF)               |                 Texture                  |                 Fan Art                  |
| ------------------------------------------ | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :---------------------------------: | :--------------------------------------: | :--------------------------------------: | :--------------------------------------: | :--------------------------------------: | :--------------------------------------: | :--------------------------------------: |
| [Arcade DB](#arcadedb-by-motoschifo)       |        ✓ {title='Arcade DB'}        |        ✓ {title='Arcade DB'}        |        ✓ {title='Arcade DB'}        |        ✓ {title='Arcade DB'}        |     &nbsp; {title='Arcade DB'}      |        ✓ {title='Arcade DB'}        |        ✓ {title='Arcade DB'}        |     &nbsp; {title='Arcade DB'}      |     &nbsp; {title='Arcade DB'}      |        ✓ {title='Arcade DB'}        |        ✓ {title='Arcade DB'}        |          ✓ {title='Arcade DB'}           |          ✓ {title='Arcade DB'}           |          ✓ {title='Arcade DB'}           |        &nbsp; {title='Arcade DB'}        |        &nbsp; {title='Arcade DB'}        |        &nbsp; {title='Arcade DB'}        |
| [ES GameList](#emulationstation-game-list) |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |       ✓ {title='ES GameList'}       |         ✓ {title='ES GameList'}          |       &nbsp; {title='ES GameList'}       |       &nbsp; {title='ES GameList'}       |         ✓ {title='ES GameList'}          |       &nbsp; {title='ES GameList'}       |       &nbsp; {title='ES GameList'}       |
| [GameBase](#gamebase-db)                   |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |      &nbsp; {title='GameBase'}      |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |       ✓ ¹ {title='GameBase'}        |        ✓ {title='GameBase'}         |        ✓ {title='GameBase'}         |        &nbsp; {title='GameBase'}         |        &nbsp; {title='GameBase'}         |        &nbsp; {title='GameBase'}         |        &nbsp; {title='GameBase'}         |        &nbsp; {title='GameBase'}         |        &nbsp; {title='GameBase'}         |
| [Internet Game DB (IGDB)](#igdb)           | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | ✓ {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} | &nbsp; {title='Internet Game DB (IGDB)'} |
| [File Import](#custom-resource-import)     |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |       ✓ {title='File Import'}       |         ✓ {title='File Import'}          |         ✓ {title='File Import'}          |         ✓ {title='File Import'}          |         ✓ {title='File Import'}          |         ✓ {title='File Import'}          |       See ² {title='File Import'}        |
| [MobyGames](#mobygames)                    |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        ✓ {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |        &nbsp; {title='MobyGames'}        |
| [OpenRetro](#openretro)                    |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |     &nbsp; {title='OpenRetro'}      |        ✓ {title='OpenRetro'}        |        ✓ {title='OpenRetro'}        |        &nbsp; {title='OpenRetro'}        |        &nbsp; {title='OpenRetro'}        |          ✓ {title='OpenRetro'}           |        &nbsp; {title='OpenRetro'}        |        &nbsp; {title='OpenRetro'}        |        &nbsp; {title='OpenRetro'}        |
| [ScreenScraper](#screenscraper)            |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |      ✓ {title='ScreenScraper'}      |        ✓ {title='ScreenScraper'}         |        ✓ {title='ScreenScraper'}         |        ✓ {title='ScreenScraper'}         |        ✓ {title='ScreenScraper'}         |        ✓ {title='ScreenScraper'}         |      See ² {title='ScreenScraper'}       |
| [The Games DB](#thegamesdb-tgdb)           |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      ✓ {title='The Games DB'}       |      &nbsp; {title='The Games DB'}       |         ✓ {title='The Games DB'}         |         ✓ {title='The Games DB'}         |      &nbsp; {title='The Games DB'}       |      &nbsp; {title='The Games DB'}       |       See ² {title='The Games DB'}       |
| [World of Spectrum](#world-of-spectrum)    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    | &nbsp; {title='World of Spectrum'}  | &nbsp; {title='World of Spectrum'}  |    ✓ {title='World of Spectrum'}    |    ✓ {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |    &nbsp; {title='World of Spectrum'}    |

**Remarks**:  
 ¹ GameBase provides only an adult flag, thus it is either 18 or no age rating  
 ² Planned for a future release of Skyscraper

### Recognized Keywords in Query

| Module               | Supported Formats `--query=""` Parameter                                                                                       |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| arcadedb             | Only title                                                                                                                     |
| esgamelist           | No query supported                                                                                                             |
| gamebase             | Game filename, Game title and Game CRC (automatically detected). Except for CRC, globbing patterns (`*` and `'?`) can be used. |
| igdb                 | Only title                                                                                                                     |
| import               | No query supported                                                                                                             |
| mobygames            | Title or numeric MobyGames ID (see Identifiers section their website)                                                          |
| openretro            | Only title                                                                                                                     |
| screenscraper        | romnom=, crc=, md5=, sha1=; see [here](https://www.screenscraper.fr/webapi2.php?alpha=0&numpage=0#jeuInfos) for description    |
| thegamesdb, tgdb     | Only title                                                                                                                     |
| worldofspectrum, wos | Only title, but can be regular expression                                                                                      |

!!! tip "Aliases for Game Filenames"

    Except for the Import and EmulationStation Gamelist scraper you can also define aliases for each game filename. If an alias is found it is applied for searching the game's metadata. Consult the file [`aliasMap.csv`](https://github.com/Gemba/skyscraper/blob/master/aliasMap.csv) for details.

!!! warning "World of Spectrum"

    The World of Spectrum scraping module is currently not functional. See [#122](https://github.com/Gemba/skyscraper/issues/122) for workarounds.

## Characteristics for Each Scraping Module

### ScreenScraper

- Shortname: _`screenscraper`_
- Type: _Online_
- Website: _[www.screenscraper.fr](https://www.screenscraper.fr)_
- Type: _Rom checksum based, Exact file name based_
- User credential support: _Yes, and strongly recommended, but not required_
- API request limit: _20k per day for registered users_
- Thread limit: _1 or more depending on user credentials_
- Platform support: _[Check list under "Systémes"](https://www.screenscraper.fr)_ or see `screenscraper_platforms.json` sibling to your `config.ini`
- Media support: _`cover`, `screenshot`, `wheel`, `manual`, `marquee`, `video`_
- Example use: `Skyscraper -p snes -s screenscraper`

ScreenScraper is probably the most versatile and complete retro gaming database out there. It searches for games using either the checksums of the files or by comparing the _exact_ file name to entries in their database.

It can be used for gathering data for pretty much all platforms, but it does have issues with platforms that are ISO based. Still, even for those platforms, it does locate some games.

It has the best support for the `wheel` and `marquee` artwork types of any of the databases, and also contains videos and manuals for a lot of the games.

I strongly recommend supporting them by contributing data to the database, or by supporting them with a bit of money. This can also give you more threads to scrape with.

!!! note

    _Exact_ file name matching does not work well for the `arcade` derived platforms in cases where a data checksum doesn't match. The reason being that `arcade` and other arcade-like platforms are made up of several subplatforms. Each of those subplatforms have a high chance of containing the same file name entry. In those cases ScreenScraper can't determine a unique game and will return an empty result.

### TheGamesDB (TGDB)

- Shortname: _`thegamesdb`_, _`tgdb`_
- Type: _Online_
- Website: _[www.thegamesdb.net](http://www.thegamesdb.net)_
- Type: _File name search based_
- User credential support: _Not required_
- API request limit: _Limited to 3000 requests per IP per month_
- Thread limit: _None_
- Platform support: _[Link to list](https://thegamesdb.net/list_platforms.php)_ or see `tgdb_platforms.json` sibling to your `config.ini`
- Media support: _`cover`, `screenshot`, `wheel`, `marquee`_
- Example use: `Skyscraper -p snes -s thegamesdb`

For newer games there's no way around TheGamesDb. It recently had a huge redesign and their database remains one of the best out there. I would recommend scraping your roms with `screenscraper` first, and then use `thegamesdb` to fill out the gaps in your cache.

There's a small caveat to this module, as it has a monthly request limit (see above) per IP per month. But this should be plenty for most people.

Their API is based on a file name search. This means that the returned results do have a chance of being faulty. Skyscraper does a lot internally to make sure accepted data is for the correct game. But it is impossible to ensure 100% correct results, so do keep that in mind when using it. Consider using the `--flags interactive` command line flag if you want complete control of the accepted entries.

### ArcadeDB (by motoschifo)

- Shortname: _`arcadedb`_
- Type: _Online_
- Website: _[adb.arcadeitalia.net](http://adb.arcadeitalia.net), [youtube](https://www.youtube.com/c/ArcadeDatabase)_
- Contact: *arcadedatabase@gmail.com*
- Type: _Mame file name id based_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: _1_
- Platform support: _Exclusively arcade platforms using official MAME files_
- Media support: _`cover`, `screenshot`, `wheel`, `marquee`, `video`_
- Example use: `Skyscraper -p fba -s arcadedb`

Several Arcade databases using the MAME file name id's have existed throughout the years. Currently the best one, in my opinion, is the ArcadeDB made by motoschifo. It goes without saying that this module is best used for arcade platforms such as `fba`, `arcade` and any of the mame sub-platforms.

As it relies on the MAME file name id when searching, there's no use trying to use this module for any non-MAME files. It won't give you any results.

This module also supports videos for many games.

### OpenRetro

- Shortname: _`openretro`_
- Type: _Online_
- Website: _[www.openretro.org](https://www.openretro.org)_
- Type: _WHDLoad uuid based, File name search based_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: _1_
- Platform support: _Primarily Amiga, but supports others as well. [Check list here to the right](https://openretro.org/browse/amiga/a)_
- Media support: _`cover`, `screenshot`, `marquee`_
- Example use: `Skyscraper -p amiga -s openretro`

If you're looking to scrape the Amiga RetroPlay LHA files, there's no better way to do this than using the `openretro` module. It is by far the best WHDLoad Amiga database on the internet when it comes to data scraping, and maybe even the best Amiga game info database overall.

The database also supports many non-Amiga platforms, but there's no doubt that Amiga is the strong point.

### MobyGames

- Shortname: _`mobygames`_
- Type: _Online_
- Website: _[www.mobygames.com](https://www.mobygames.com)_
- Type: _File name search based_
- User credential support: _None required_
- API request limit: _1 request per 10 seconds_
- Rom limit per run: _35_
- Thread limit: _1_
- Platform support: _[List](https://www.mobygames.com/browse/games)_ or see `mobygames_platforms.json` sibling to your `config.ini`
- Media support: _`cover`, `screenshot`_
- Example use:
  - `Skyscraper -p fba -s mobygames <SINGLE FILE TO SCRAPE>`
  - `Skyscraper -p fba -s mobygames --startat <FILE TO START AT> --endat <FILE TO END AT>`

MobyGames. What can I say. If you haven't heard about this database before you've been missing out. It's one of the best and oldest games databases on the internet. You'll probably come across references to MobyGames on other sites when searching for retro games. There's a reason for that - it's that good.

There's a caveat to the module as it has quite strong restrictions for the number of requests that are allowed at any given time. This restriction is global for the entire Skyscraper user base, which means that it might quit on you if other users are currently scraping from it. For this reason it has been strongly limited inside of Skyscraper by forcing a maximum number of rom scrapings per run.

Please use this module sparingly. And only ever use it to scrape those last few roms you can't get any data for using any of the other sources.

### IGDB

![IGDB logo](resources/igdb.png)

- Shortname: _`igdb`_
- Type: _Online_
- Website: _[www.igdb.com](https://www.igdb.com)_
- Type: _File name search based_
- User credential support: _Yes, free private API client-id and secret-key required! Read more below_
- API request limit: _A maximum of 4 requests per seconds is allowed_
- Thread limit: _4 (each being limited to 1 request per second)_
- Platform support: _[List](https://www.igdb.com/platforms)_
- Media support: _`cover`, `screenshot`_
- Example use:
  - `Skyscraper -p fba -s igdb <SINGLE FILE TO SCRAPE>`
  - `Skyscraper -p fba -s igdb --startat <FILE TO START AT> --endat <FILE TO END AT>`

IGDB is a relatively new database on the market. But absolutely not a bad one at that. It has a couple caveats though, as the database doesn't distinguish between platform versions of the same game when it comes to any artwork resources (they are working to implement this at some point). This makes it less usable in a retro game scraping context as many of the games differ drastically visually between the old platforms. For that reason alone, this module will only provide textual data for your roms for the time being.

It is _required_ to register with the Twitch dev program (IGDB is owned by Twitch) and create a free client-id and secret-key pair for use with Skyscraper. The process of getting this free client-id and secret-key pair is quite easy. Just follow the following steps:

- Go [here](https://dev.twitch.tv/login) and sign up for an account
- [Enable](https://www.twitch.tv/settings/security) two-factor authentication (required)
- [Register](https://dev.twitch.tv/console/apps/create) an application (call it whatever you like)
- [Manage](https://dev.twitch.tv/console/apps) the application
- Add `http://localhost` as OAuth redirect URL
- Generate a secret-key by clicking `New secret`
- Add your client-id and secret-key pair to the Skyscraper config ini (`/home/<USER>/.skyscraper/config.ini`):

```
[igdb]
userCreds="CLIENTID:SECRETKEY"
```

Substitute CLIENTID and SECRETKEY with your own details. And that's it, you should now be able to use the IGDB module.

### World of Spectrum

- Shortname: _`worldofspectrum`_, _`wos`_
- Type: _Online_
- Website: _[www.worldofspectrum.org](http://www.worldofspectrum.org)_
- Type: _File name search based_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: _None_
- Platform support: _Exclusively ZX Spectrum games_
- Media support: _`cover`, `screenshot`_
- Example use: `Skyscraper -p zxspectrum -s worldofspectrum`

If you're looking specifically for ZX Spectrum data, this is the module to use. World of Spectrum is probably the most complete ZX Spectrum resource and information database in existence. I strongly recommend visiting the site if you have any interest in these little machines. It's a cornucopia of information on the platform.

### EmulationStation Game List

- Shortname: _`esgamelist`_
- Type: _Local_
- Website: _[https://emulationstation.org](https://emulationstation.org)_
- Type: _Exact file name match_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: _None_
- Platform support: _All_
- Media support: _`screenshot`, `manual`, `marquee`, `video`_
- Example use: `Skyscraper -p snes -s esgamelist`

This module allows you to import data from an existing EmulationStation game list into the Skyscraper cache. This is useful if you already have a lot of data and artwork in a gamelist.xml file and you wish to use it with Skyscraper.

Skyscraper will search for the `gamelist.xml` file at `<INPUT FOLDER>/gamelist.xml` which by default is `/home/<USER>/RetroPie/roms/<PLATFORM>/gamelist.xml`. If that file isn't found it will look for it at `/home/<USER>/.skyscraper/import/<PLATFORM>/gamelist.xml`.

### Custom Resource Import

- Shortname: _`import`_
- Type: _Local_
- Website: _[Documentation@github](IMPORT.md)_
- Type: _Exact file name match_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: _None_
- Platform support: _All_
- Media support: _`cover`, `screenshot`, `wheel`, `manual`, `marquee`, `video`_
- Example use: `Skyscraper -p snes -s import`

Read a thorough description of this module [here](IMPORT.md).

### GameBase DB

- Shortname: _`gamebase`_
- Type: _Local_
- Website: _[about the format](https://www.bu22.com/wiki/home)_
- Type: _Exact filename, title or CRC match, for filename and title wildcards * and ? can be applied anywhere_
- User credential support: _None required_
- API request limit: _None_
- Thread limit: 1
- Platform support: For those platforms where the community has compiled a GameBase database, several dozen platforms do have a GameBase database. Some examples: Commodore Machines (VC-20,C64,Plus/4,Amiga), Sinclair Spectrum ("Speccy"), ...
- Media support: _`cover`, `screenshot`_
- Example use: `Skyscraper -p zxspectrum -s gamebase`

A GameBase DB is a community driven effort to collect game information of the
common game releases for a platform, but also more importantly for Homebrew and
Indie released games. It is a great source to find much information about the
games and other media in one place, which is otherwise cluttered over the
internet. Skyscraper only uses the game information, but a GameBase DB also
contains information and files of the platform's former magazines and short
manuals for example. The usual GameBase DB Frontend is Windows based and a
database is in Microsoft Access (`*.mdb`) format. Binary data is held in
subfolders (e.g. Screenshots, Cover) on the filesystem.

Read the setup and config description of this module [here](CONFIGINI.md#gamebasefile).
