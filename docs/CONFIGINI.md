## config.ini Options

Look below to find a thorough description of all options and sections available in the `/home/<USER>/.skyscraper/config.ini` configuration file. These options will then be applied whenever running Skyscraper with the `Skyscraper` command.

Please take note that almost all of these options are set at a useful default as documented here (and can therefore be left out) and should only be set if your use case requires it.

!!! warning

    Some users seem to have a habbit of editing the `/home/<USER>/skysource/config.ini` file. _This is not the correct file!_ Please read the first paragraph. :)

Options can be set on four levels; either `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]` or `[<SCRAPER>]`.

Settings in the `[main]` section will always be set regardless of selected platform, frontend and module.

`<PLATFORM>` can be any of the supported platforms (check list with `--help` under the `-p` option), in which case the settings will only be applied while scraping that particular platform.

`<FRONTEND>` can be any of the supported frontends (check list with `--help` under the `-f` option), in which case the settings will only be applied while scraping for that particular frontend.

`<SCRAPER>` can be any of the supported scraping modules (check list with `--help` under the `-s` option), in which case the settings will only be applied while scraping with that particular scraping module.

Each section can have overlapping parameters. In case where a certain option exists in several sections they are prioritized as scraping module first, then frontend, then platform and lastly main. Any commandline option which relates to an configuration setting in `config.ini` has highest precedence, regardless of the other four levels respective sections.

You can find an example config file at `/home/<USER>/.skyscraper/config.ini.example`. This file contains all available options. Just copy the file to `config.ini` and uncomment and edit the ones you wish to use by removing the `#` or `;` in front of the variables. Remember to also uncomment the section the option relates to such as `[main]` or `[amiga]`.

!!! note

    You can set a custom configuration file with the `-c <FILENAME>` command line option. Read more about all available command line options [here](CLIHELP.md).

**Example `config.ini`**:

```ini
[main]
inputFolder="/path/to/your/roms"
cacheFolder="/path/to/your/desired/db/cache/base/folder"
gameListBackup="true"
videos="true"
unattend="true"
verbosity="1"
lang="ja"
region="jp"
cacheMarquees="false"

[amiga]
inputFolder="/specific/path/just/for/amiga/roms"
cacheFolder="/specific/path/just/for/amiga/cache"
minMatch="50"
maxLength="200"

[pegasus]
artworkXml="artwork-pegasus.xml"
maxLength="10000"

[screenscraper]
userCreds="USER:PASSWORD"

```

### Index of Options with Applicable Sections

This is an alphabetical index of all configuration options including the sections they may be applied to.

| Parameter                                                   | `[main]` | `[<PLATFORM>]` | `[<FRONTEND>]` | `[<SCRAPER>]` |
| ----------------------------------------------------------- | :------: | :------------: | :------------: | :-----------: |
| [addExtensions](CONFIGINI.md#addextensions)                 |    Y     |       Y        |                |               |
| [addFolders](CONFIGINI.md#addfolders)                       |          |                |       Y        |               |
| [artworkXml](CONFIGINI.md#artworkxml)                       |    Y     |       Y        |       Y        |               |
| [brackets](CONFIGINI.md#brackets)                           |    Y     |       Y        |       Y        |               |
| [cacheCovers](CONFIGINI.md#cachecovers)                     |    Y     |       Y        |                |       Y       |
| [cacheFolder](CONFIGINI.md#cachefolder)                     |    Y     |       Y        |                |               |
| [cacheMarquees](CONFIGINI.md#cachemarquees)                 |    Y     |       Y        |                |       Y       |
| [cacheRefresh](CONFIGINI.md#cacherefresh)                   |    Y     |                |                |       Y       |
| [cacheResize](CONFIGINI.md#cacheresize)                     |    Y     |       Y        |                |       Y       |
| [cacheScreenshots](CONFIGINI.md#cachescreenshots)           |    Y     |       Y        |                |       Y       |
| [cacheTextures](CONFIGINI.md#cachetextures)                 |    Y     |       Y        |                |       Y       |
| [cacheWheels](CONFIGINI.md#cachewheels)                     |    Y     |       Y        |                |       Y       |
| [cropBlack](CONFIGINI.md#cropblack)                         |    Y     |       Y        |       Y        |               |
| [emulator](CONFIGINI.md#emulator)                           |    Y     |       Y        |       Y        |               |
| [endAt](CONFIGINI.md#endat)                                 |          |       Y        |       Y        |               |
| [excludeFrom](CONFIGINI.md#excludefrom)                     |    Y     |       Y        |                |               |
| [excludePattern](CONFIGINI.md#excludepattern)               |    Y     |       Y        |       Y        |               |
| [extensions](CONFIGINI.md#extensions)                       |          |       Y        |                |               |
| [forceFilename](CONFIGINI.md#forcefilename)                 |    Y     |       Y        |       Y        |               |
| [frontend](CONFIGINI.md#frontend)                           |    Y     |                |                |               |
| [gameListBackup](CONFIGINI.md#gamelistbackup)               |    Y     |                |       Y        |               |
| [gameListFilename](CONFIGINI.md#gamelistfilename)           |    Y     |       Y        |       Y        |               |
| [gameListFolder](CONFIGINI.md#gamelistfolder)               |    Y     |       Y        |       Y        |               |
| [gameListVariants](CONFIGINI.md#gamelistvariants)           |          |                |       Y        |               |
| [hints](CONFIGINI.md#hints)                                 |    Y     |                |                |               |
| [ignoreYearInFilename](CONFIGINI.md#ignoreyearinfilename)   |    Y     |       Y        |                |               |
| [importFolder](CONFIGINI.md#importfolder)                   |    Y     |       Y        |                |               |
| [includeFrom](CONFIGINI.md#includefrom)                     |    Y     |       Y        |                |               |
| [includePattern](CONFIGINI.md#includepattern)               |    Y     |       Y        |       Y        |               |
| [innerBracketsReplace](CONFIGINI.md#innerbracketsreplace)   |    Y     |                |                |               |
| [innerParenthesesReplace](CONFIGINI.md#innerparenthesesreplace) |  Y   |                |                |               |
| [inputFolder](CONFIGINI.md#inputfolder)                     |    Y     |       Y        |                |               |
| [interactive](CONFIGINI.md#interactive)                     |    Y     |       Y        |                |       Y       |
| [jpgQuality](CONFIGINI.md#jpgquality)                       |    Y     |       Y        |                |       Y       |
| [keepDiscInfo](CONFIGINI.md#keepdiscinfo)                   |    Y     |       Y        |                |               |
| [lang](CONFIGINI.md#lang)                                   |    Y     |       Y        |                |               |
| [langPrios](CONFIGINI.md#langprios)                         |    Y     |       Y        |                |               |
| [launch](CONFIGINI.md#launch)                               |    Y     |       Y        |       Y        |               |
| [manuals](CONFIGINI.md#manuals)                             |    Y     |       Y        |                |               |
| [maxFails](CONFIGINI.md#maxfails)                           |    Y     |                |                |               |
| [maxLength](CONFIGINI.md#maxlength)                         |    Y     |       Y        |       Y        |       Y       |
| [mediaFolder](CONFIGINI.md#mediafolder)                     |    Y     |       Y        |       Y        |               |
| [mediaFolderHidden](CONFIGINI.md#mediafolderhidden)         |          |                |       Y        |               |
| [minMatch](CONFIGINI.md#minmatch)                           |    Y     |       Y        |                |       Y       |
| [nameTemplate](CONFIGINI.md#nametemplate)                   |    Y     |       Y        |                |               |
| [onlyMissing](CONFIGINI.md#onlymissing)                     |    Y     |       Y        |                |       Y       |
| [platform](CONFIGINI.md#platform)                           |    Y     |                |                |               |
| [pretend](CONFIGINI.md#pretend)                             |    Y     |       Y        |                |               |
| [region](CONFIGINI.md#region)                               |    Y     |       Y        |                |               |
| [regionPrios](CONFIGINI.md#regionprios)                     |    Y     |       Y        |                |               |
| [relativePaths](CONFIGINI.md#relativepaths)                 |    Y     |       Y        |                |               |
| [scummIni](CONFIGINI.md#scummini)                           |    Y     |                |                |               |
| [skipped](CONFIGINI.md#skipped)                             |    Y     |       Y        |       Y        |               |
| [spaceCheck](CONFIGINI.md#spacecheck)                       |    Y     |                |                |               |
| [startAt](CONFIGINI.md#startat)                             |          |       Y        |       Y        |               |
| [subdirs](CONFIGINI.md#subdirs)                             |    Y     |       Y        |                |               |
| [symlink](CONFIGINI.md#symlink)                             |    Y     |       Y        |       Y        |               |
| [theInFront](CONFIGINI.md#theinfront)                       |    Y     |       Y        |       Y        |               |
| [threads](CONFIGINI.md#threads)                             |    Y     |       Y        |                |       Y       |
| [tidyDesc](CONFIGINI.md#tidydesc)                           |    Y     |       Y        |                |       Y       |
| [unattend](CONFIGINI.md#unattend)                           |    Y     |       Y        |       Y        |       Y       |
| [unattendSkip](CONFIGINI.md#unattendskip)                   |    Y     |       Y        |       Y        |       Y       |
| [unpack](CONFIGINI.md#unpack)                               |    Y     |       Y        |                |               |
| [userCreds](CONFIGINI.md#usercreds)                         |          |                |                |       Y       |
| [verbosity](CONFIGINI.md#verbosity)                         |    Y     |       Y        |       Y        |               |
| [videoConvertCommand](CONFIGINI.md#videoconvertcommand)     |    Y     |                |                |       Y       |
| [videoConvertExtension](CONFIGINI.md#videoconvertextension) |    Y     |                |                |       Y       |
| [videoPreferNormalized](CONFIGINI.md#videoprefernormalized) |          |                |                |       Y       |
| [videos](CONFIGINI.md#videos)                               |    Y     |       Y        |       Y        |       Y       |
| [videoSizeLimit](CONFIGINI.md#videosizelimit)               |    Y     |       Y        |                |       Y       |

#### inputFolder

Sets the rom input folder. By default Skyscraper will look for roms in the `/home/<USER>/RetroPie/roms/<PLATFORM>` folder. If your roms are located in a non-default location, you can set the input path using this option.

!!! note

    If this is set in the `[main]` section it will automatically add `/<PLATFORM>` to the end of the path. If you want better control consider adding it to a `[<PLATFORM>]` section instead where it will be used as is.

Default value: `/home/<USER>/RetroPie/roms/<PLATFORM>`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### gameListFolder

Sets the game list export folder. By default Skyscraper exports the game list to the same directory as the rom input folder. This enables you to change that to a non-default location.

!!! note

    If this is set in the `[main]` or `[<FRONTEND>]` section it will automatically add `/<PLATFORM>` to the end of the path. If you want better control consider adding it to a `[<PLATFORM>]` section instead where it will be used as is.

Default value: `/home/<USER>/RetroPie/roms/<PLATFORM>`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### gameListFilename

Sets the game list export filename. This enables you to change that to a non-default filename.

Default value: depends on [frontend](FRONTENDS.md)
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### gameListBackup

When set to true Skyscraper will create a backup of the existing game list each time it is run in game list generation mode (this mode is enabled by leaving out the `-s MODULE` option). The filename and path of the backup will be equal to the existing game list but have a timestamp appended to the name of the format `-yyyyMMdd-hhmmss` (eg. `gamelist.xml-20200530-115900`).

Default value: `false`  
Allowed in sections: `[main]`, `[<FRONTEND>]`

---

#### mediaFolder

Sets the artwork / media output folder. By default (ie. if the option `mediaFolder` is not explicitly set or pre-defined by the frontend) Skyscraper outputs the composited artwork files to the game list export folder concatenated with `/media`. The `mediaFolder` setting enables you to change that to a non-default location.

Read more about the artwork compositing [here](ARTWORK.md).

!!! note

    If this is set in the `[main]` or `[<FRONTEND>]` section it will automatically add `/<PLATFORM>` to the end of the path. If you want better control consider adding it to a `[<PLATFORM>]` section instead where it will be used as is.

Default value: `/home/<USER>/RetroPie/roms/<PLATFORM>/media`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### mediaFolderHidden

By default Skyscraper uses the `<PLATFORM>/media` folder when generating EmulationStation media. Setting this option will change that to `<PLATFORM>/.media`. Notice the `.` which hides the folder, which can speed up the initial EmulationStation loading sequence when using slow storage such as network file systems.

This option is ignored if you set the media folder manually.

!!! note

    Remember to remove your old `<PLATFORM>/media` folders if you enable this option.

Default value: `false`  
Allowed in sections: Only for frontends `[emulationstation]` or `[retrobat]`

---

#### cacheFolder

Sets a non-default location for the storing and loading of cached game resources. This is what is referred to in the docs as the _resource cache_. By default this folder is set to `/home/<USER>/.skyscraper/cache/<PLATFORM>`. Don't change this unless you have a good reason to (for instance if you want your cache to reside on a USB drive).

!!! note

    If this is set in the `[main]` section (recommended) it will automatically add `/<PLATFORM>` to the end of the path. If you want better control consider adding it to a `[<PLATFORM>]` section instead where it will be used as is.

Default value: `/home/<USER>/.skyscraper/cache/<PLATFORM>`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### cacheResize

By default, to save space, Skyscraper resizes large pieces of artwork before adding them to the resource cache. Setting this option to `"false"` will disable this and save the artwork files with their original resolution. Beware that Skyscraper converts all artwork resources to lossless PNG's when saving them. High resolution images such as covers will take up a lot of space! So set this to `"false"` with caution.

!!! note

    This is not related to the artwork compositing that happens when generating a game list. This is _only_ related to how Skyscraper handles artwork when adding it to the resource cache while gathering data from the scraping modules.

To read about artwork compositing go [here](ARTWORK.md) instead.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cacheRefresh

Skyscraper has a resource cache which works just like the browser cache in Firefox. If you scrape and gather resources for a platform with the same scraping module twice, it will grab the data from the cache instead of hammering the online servers again. This has the advantage in the case where you scrape a rom set twice, only the roms that weren't recognized the first time around will be fetched from the online servers. Everything else will be loaded from the cache.

You can force all data to be refetched from the servers by setting this option to `cacheRefresh="true"`, effectively updating the cached data with new data from the source.

!!! warning

    _Only_ set this option to true if you know data has changed for several roms at the source. Otherwise you are hammering the servers for no reason.

Default value: `false`  
Allowed in sections: `[main]`, `[<SCRAPER>]`

---

#### nameTemplate

By default Skyscraper uses just the title as the game name when generating gamelists for any frontend. You can change this to suit your personal preference by setting this option. It works by replacing certain tokens in the name template with the corresponding data. The valid tokens are:

-   `%t`: The game title as returned by the scraping sources without bracket information (see `%b` and `%B` below)
-   `%f`: The game filename without extension and bracket information (see `%b` and `%B` below)
-   `%b`: The game `()` parentheses information. This information often comes from the filename, but can also come from the scraping source title (eg. `(USA)` or `(en,fr,de)`)
-   `%B`: The game `[]` bracket information. This information often comes from the filename, but can also come from the scraping source title (eg. `[disk 1 of 2]` or `[AGA]`)
-   `%a`: The age restriction as returned by the scraping sources (eg. `16+`)
-   `%d`: The game developer as returned by the scraping sources
-   `%p`: The game publisher as returned by the scraping sources
-   `%r`: The game rating as returned by the scraping sources (`0.0` to `5.0`)
-   `%P`: The number of players as returned by the scraping sources
-   `%D`: The game release date with format `yyyy-mm-dd`

This option also support template groups separated by `;` within the template. The template parser will go over each group. If a group only has empty variables it will not be included in the final game name. So for a template such as `%t;, %P player(s)` where the `%P` is empty because no scraping source has provided the info, it will leave out the `, %P player(s)` part entirely resulting in the title `Game Name`. If this was not separated by `;` the resulting game name would end up being `Game Name, player(s)`.

**Example(s)**

```ini
nameTemplate="%t [%f];, %P player(s)"
```

Will result in: `1945k III [1945kiii], 2 player(s)`

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### tidyDesc

Cleans up some misformatting in scraped description:

1. Heading and trailing spaces are stripped
2. Multiple spaces between sentences are reduced to one space
3. Bulletpoint beginning with \* or ● are replaced with a dash
4. Stylized ellipsis (… Unicode:`&#8230;`) is replaced with three dot characters
5. Multiple exclamation marks are reduced to one, unless for game titles are explicitly typed like that, like 'Super Punch-Out!!'.

!!! quote

    'Multiple exclamation marks,' he went on, shaking his head, 'are a sure sign of a diseased mind.'  
    -- _Eric_ by Terry Pratchett

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### jpgQuality

Sets the default jpg quality when saving image resources to the cache. This will be ignored if `--flags noresize` is set. Default is 95.

All screenshots and any image resource that uses transparency will always be saved as PNG images. In those cases this setting will be ignored as PNG format is lossless.

Default value: `95`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cacheCovers

Enables/disables the caching of the resource type `cover` when scraping with any module. If you never use covers in your artwork configuration, setting this to `"false"` can save you some space.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cacheTextures

Enables/disables the caching of the resource type `cover` when scraping with any module. If you never use covers in your artwork configuration, setting this to `"false"` can save you some space.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cacheScreenshots

Enables/disables the caching of the resource type `screenshot` when scraping with any module. If you never use screenshots in your artwork configuration, setting this to `"false"` can save you some space.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cropBlack

Enables/disables cropping away the black borders around the screenshot resources when compositing the final frontend gamelist artwork.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### cacheWheels

Enables/disables the caching of the resource type `wheel` when scraping with any module. If you never use wheels in your artwork configuration, setting this to `"false"` can save you some space.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### cacheMarquees

Enables/disables the caching of the resource type `marquee` when scraping with any module. If you never use marquees in your artwork configuration, setting this to `"false"` can save you some space.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### importFolder

Sets a non-default folder when scraping using the `-s import` module. By default this is set to `/home/<USER>/.skyscraper/import` and will also look for a `/<PLATFORM>` inside of the chosen folder. Read more about the `-s import` module [here](IMPORT.md).

Default value: `/home/<USER>/.skyscraper/import/<PLATFORM>`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### unpack

Some scraping modules use file checksums to identify the game in their databases. If you've compressed your roms to zip or 7z files yourself, this can pose a problem in getting a good result. You can then try setting this option to `"true"`. Doing so will extract the rom and do the file checksum on the rom itself instead of the compressed file.

!!! info

    Only enable this option if you are having problems getting the roms identified from the compressed files. It slows down the scraping process significantly and should therefore be avoided if possible.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### frontend

Sets the frontend you wish to export a game list for. By default Skyscraper will export an EmulationStation game list, but other frontends are supported as well.

If exporting for the `attractmode` frontend, please also take note of the required `emulator=""` option that goes along with using the `attractmode` frontend.

If exporting for the `pegasus` frontend, please also take note of the optional `launch=""` option that allows you to set the launch parameter used by Pegasus.

Use as value for the frontend option the frontend name in all lowercase and with alphabetical characters only: `emulationstation`, `esde`, `pegasus`, `retrobat`, `attractmode`, aso. Check all supported frontends with '--help' and read a more about the details concerning each of them [here](FRONTENDS.md).

Default value: `emulationstation`  
Allowed in sections: `[main]`

---

#### emulator

This option is _only_ applicable when also setting the `frontend="attractmode"` option. It sets the _emulator_ to be used when generating the `attractmode` game list. On RetroPie the emulator name is mostly the same as the platform.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### launch

This option is _only_ applicable when also setting the `frontend="pegasus"` option. It sets the _launch command_ to be used when generating the `pegasus` game list. This is optional. It defaults to the RetroPie launch options which works on RetroPie.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### videos

By default Skyscraper doesn't scrape and cache video resources because of the significant disk space required to save them. You can enable videos using this option.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`, `[<SCRAPER>]`

---

#### videoSizeLimit

If video scraping is enabled you can set the maximum allowed video file size with this variable. The size is in Megabytes (1.000.000 bytes). If this size is exceeded the video file won't be saved to the cache.

Default value: `100`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### videoConvertCommand

Some scraping modules deliver videos that use a codec or color format that some frontends don't support. In those cases it can be useful to convert the videos before saving them in the Skyscraper resource cache.

This setting allows you to set a command that will be run on each video after it has been downloaded from the selected scraping module. See the examples below for inspiration.

The `%i` and `%o` **are required** and will be replaced with the internally used video input (original) and output (converted) filenames as needed by Skyscraper.

If your command / script always converts to a videofile with a specific extension, you also need to set `videoConvertExtension`.

!!! tip

    Set `--verbosity 3` to route all output from your command / script to the terminal while Skyscraper runs. This will help you ensure everything is working as intended.

The first example below makes use of the excellent `ffmpeg` tool. If you want to use this specific example you need to install `ffmpeg` first. On RetroPie and other Debian-derived distros you can install it with `sudo apt install ffmpeg`. If you want to use a script for the video conversion and run it directly without path, you need to place it in the `/home/<USER>/.skyscraper` folder.

!!! tip

    If you create a script that includes checks on the input video and it sometimes decides not to convert them, you should simply let the script copy the video with `cp %i %o`. The `%i` is a temporary file created internally by Skyscraper. And the `%o` file is the file used by the cache. Both files should exist after a successful conversion. Skyscraper removes the temporary `%i` file automatically.

**Example(s)**

```ini
videoConvertCommand="ffmpeg -i %i -y -pix_fmt yuv420p -t 00:00:10 -c:v libx264 -crf 23 -c:a aac -b:a 64k -vf scale=640:480:force_original_aspect_ratio=decrease,pad=640:480:(ow-iw)/2:(oh-ih)/2,setsar=1 %o"
videoConvertCommand="videoconvert.sh %i %o"
```

Default value: unset  
Allowed in sections: `[main]`, `[<SCRAPER>]`

---

#### videoConvertExtension

If you want to force an extension for the converted video file created by the `videoConvertCommand` command, you need to set this option. The converted file will then automatically have this extension no matter what the input file extension is.

For instance, if a scraping module delivers the file `videofile.avi` and you always want the converted files to be `mp4` files, you simply set this option to `videoConvertExtension="mp4"`.

It is up to you to make sure that the command you provide in `videoConvertCommand` actually converts to a video file using the selected extension. For some conversion tools such as `ffmpeg` this is handled simply by setting the extension of the output file. But for other tools you might have to provide additional options to the `videoConvertCommand` above.

Default value: unset  
Allowed in sections: `[main]`, `[<SCRAPER>]`

---

#### videoPreferNormalized

This option is _only_ applicable when scraping with the `-s screenscraper` module. ScreenScraper offers two versions of some of their videos. A normalized version, which adheres to some defined standard they made, and the originals. If you prefer converting or standardizing the videos yourself (see `videoConvertCommand` above) then you can set this to `false`. If you do so Skyscraper will fetch the original videos from ScreenScraper instead of the normalized ones.

Be aware that the original videos often vary a lot in codec, color format and size. So it is recommended to convert them afterwards using the `videoConvertCommand`.

**Example(s)**

```ini
[screenscraper]
videoPreferNormalized="false"
```

Default value: `true`  
Allowed in sections: Only for scraper `[screenscraper]`

---

#### symlink

Enabling this option is only relevant while also setting the `videos="true"` option. It basically means that Skyscraper will create a link to the cached videos instead of copying them when generating the game list media files. This will save a lot of space, but has the caveat that if you somehow remove the videos from the cache, the links will be broken and the videos then won't show anymore.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### theInFront

Game titles are returned from the scraping sources sometimes as 'The Game' and other times as 'Game, The'. Enabling this option will force Skyscraper to always try and move 'The' to the front of the titles. If it is not enabled, Skyscraper will always try and move it to the end of the title, regardless of how it was originally returned by the scraping sources.

!!! info

    When generating gamelists Skyscraper will still sort the games as if the game titles didn't have 'The' at the beginning.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### brackets

Disable this option to remove any bracket notes when generating the game list. It will disable notes such as `(Europe)` and `[AGA]` completely. This option is only relevant when generating the game list. It makes no difference when gathering data into the resource cache. Default (true) will keep brackets in the game title.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### innerBracketsReplace

Only in use when the option `brackets` is set to `true` for gamelist creation:
This replaces consecutive brackets `][` in the game title with whatever is
defined in this option. This setting has no effect, if there is only one bracket
present in the game filename.  
Use the option `innerParenthesesReplace` for the same effect on round brackets
`)(` (aka. parentheses).

**Example(s)**

Filename: `Oddworld - Abe's Exoddus [NTSC-U] [SLUS-00710].m3u`

- `innerBracketsReplace=""` (unset), gamelist game title output: `Oddworld - Abe's Exoddus [NTSC-U][SLUS-00710]`
- `innerBracketsReplace="] ["`, gamelist game title output (_note the space._): `Oddworld - Abe's Exoddus [NTSC-U] [SLUS-00710]`
- `innerBracketsReplace=","`, gamelist game title output: `Oddworld - Abe's Exoddus [NTSC-U,SLUS-00710]`

Default value: unset  
Allowed in sections: `[main]`

---

#### innerParenthesesReplace

Same as [innerBracketsReplace](#innerbracketsreplace) but for parentheses `)(`
(aka. round brackets).

---

#### keepDiscInfo

Only in use when the option `brackets` is set to `false` for gamelist creation:
If you set `keepDiscInfo="true"`, Skyscraper attempts to retain the "Disc N (of
M)" part in the resulting game title in the gamelist. Currently the term 'disc'
is identified in the filename in English. German, French, Italian.  
Any disc information is first searched in parentheses e.g., (Disc 1 of 4) and then in
brackets e.g., [Disc 1]. Any suffix after the disc number like "of 4" is also
kept.

!!! tip

    If the emulator supports multi disc loading with an `*.m3u` or `*.cue` file you should use it, as it will remove gamelist clutter. Another option is to define a custom game title for each of the filenames with disc information via Skyscraper's [import function](IMPORT.md). The latter option allows you to define any "Disc N of M" display style in your gamelist. 

!!! note

    The option keepDiscInfo is not applicable if you use a [name template](#nametemplate).

**Example(s)**

Filename: `Stupid Invaders v1.001 (2001)(Ubi Soft)(US)(Disc 1 of 2)[!].chd`  
Resulting game title: `Stupid Invaders (Disc 1 of 2)`

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### maxLength

Sets the maximum length of returned game descriptions. This is a convenience option if you feel like game descriptions are too long. By default it is set to 2500.

Default value: `2500`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`, `[<SCRAPER>]`

---

#### minMatch

Some scraping modules are based on a file name or title based search. This option sets the minimum percentage any returned results need to match with in order for it to be accepted. For instance, the game `Wonderboy in Monsterland` might return the title `Wonder Boy in Monster Land` which is clearly a match. But it's not a 100% match. So it needs to be set relatively high, while still ignoring bad matches. By default it is set to 65 which has been tested to be a good middle-ground.

Default value: `65`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### threads

Sets the desired number of parallel threads to be run when scraping. Some modules have maximum allowed threads. If you set this higher than the allowed value, it will be auto-adjusted. By default it is set to 4.

Default value: `4`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### pretend

This option is _only_ relevant when generating a game list (by leaving out the `-s <SCRAPER>` command line option). It disables the game list generator and artwork compositor and only outputs the results of the potential game list generation to the terminal. It is mostly useful when used as a command line flag with `--flags pretend`. It makes little sense to set it here, but you can if you want to.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### unattend

When generating a game list Skyscraper will check if it already exists and ask if you want to overwrite it. And it will also ask if you wish to skip existing game list entries. By enabling this option Skyscraper will _always_ overwrite an existing game list and _never_ skip existing entries, in other words: the game list will be newly created. This is flag useful for example when scripting Skyscraper to avoid the need for user input.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`, `[<SCRAPER>]`

---

#### unattendSkip

When generating a game list Skyscraper will check if it already exists and ask if you want to overwrite it. And it will also ask if you wish to skip existing game list entries. By enabling this option Skyscraper will _always_ overwrite an existing game list and _always_ skip existing entries, in other words: game list entries are added if not present in the gamelist, existing entries are left untouched. This flag is useful for example when scripting Skyscraper to avoid the need for user input.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`, `[<SCRAPER>]`

---

#### interactive

When gathering data from any of the scraping modules many potential entries will be returned. Normally Skyscraper chooses the best entry for you. But should you wish to choose the best entry yourself, you can enable this option. Skyscraper will then list the returned entries and let you choose which one is the best one. It is recommended to use the command line flag `--flags interactive` instead in the (hopefully) rare cases where this mode is necessary.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### forceFilename

Enable this option to force Skyscraper to use the file name (excluding extension) instead of the cached scraping module titles when generating a game list.

!!! tip

    If you set `forceFilename="true"` and your filenames contain bracket notes such as `(this)` or `[that]` at the end, these will be combined with whatever bracket notes are at the end of the titles returned from the sources. This can cause some confusion. For instance, if you have the filename `Gran Turismo 2 (USA) (Arcade Mode)` and the cached title is `Gran Turismo 2 (Arcade Mode)`, then the gamelist name will become `Gran Turismo 2 (Arcade Mode)(USA)(Arcade Mode)`. You can disable them altogether with the `brackets="false"` option.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### verbosity

Sets how verbose Skyscraper should be when running. Default level is 0. The higher the value, the more info Skyscraper will output to the terminal while running. Anything higher than 3 does not further increase the verbosity of the output.

Default value: `0`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### skipped

Only has an effect if a rom has no resources attached to it in the cache. If false (default) the rom will be left out when generating a game list file. However, it will still show up in the frontend (at least it does for EmulationStation), but it won't exist in the game list file. If unsure leave it to false, unless you need generic gamelist entries, consisting of `<path/>` and `<name/>`, for some reason. In other words: If skipped is set to false and no entry for a rom is found in the cache _nothing_ will be added to the gamelist for that rom. If skipped is true and no entry for a rom is found in the cache a _generic_ entry will be added to the gamelist file for that rom. If a rom has data in Skyscraper's cache then this flag has no effect.

!!! note

    When skipped is set true, any generic added game entry is not eligible for formatting with a [nameTemplate](CONFIGINI.md#nametemplate).

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### maxFails

Not all scraping modules support all platforms. This means that you can potentially start a scraping run with a module and a platform that is incompatible. This will hammer the servers for potentially hundreds of roms but provide 0 results for any of them. To avoid this Skyscraper has a builtin limit for initially allowed failed rom lookups. If this is reached it will quit. Setting this option allows you to set this limit yourself, but not above a maximum of 200. The default limit is 42. Don't change this unless you have a very good reason to do so.

Default value: `42`  
Allowed in sections: `[main]`

---

#### region

Adds the specified region to the top of the existing default internal region priority list. Only one region is supported with this configuration. To overwrite the internal default region priority list entirely check [here](CONFIGINI.md#regionprios). Read more about how regions are handled in general [here](REGIONS.md).

!!! info

    Setting the region will overwrite any region [auto-detected](REGIONS.md#region-auto-detection) from the file name.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`

**Example(s)**

```ini
[main]
region="de"
```

or

```ini
[snes]
region="de"
```

---

#### regionPrios

Completely overwrites the internal region priority list inside of Skyscraper. Multiple regions can be configured here separated by commas. Read more about how regions are handled in general [here](REGIONS.md).

!!! info

    Any region [auto-detected](REGIONS.md#region-auto-detection) from the file name will still be added to the top of this list.

Default value: `eu, us, ss, uk, wor, jp, au, ame, de, cus, cn, kr, asi, br, sp, fr, gr, it, no, dk, nz, nl, pl, ru, se, tw, ca`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

**Example(s)**

```ini
[main]
regionPrios="de,us,jp"
```

or

```ini
[megadrive]
regionPrios="de,us,jp"
```

---

#### lang

Adds the specified language to the top of the existing default internal language priority list. Only one language is supported with this configuration. Read more about how languages are handles in general [here](LANGUAGES.md).

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`

**Example(s)**

```ini
[main]
lang="it"
```

or

```ini
[snes]
lang="it"
```

---

#### langPrios

Completely overwrites the internal language priority list inside of Skyscraper. Multiple languages can be configured here separated by commas. Read more about how languages are handles in general [here](LANGUAGES.md).

Default value: `en, de, fr, es`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

**Example(s)**

```ini
[main]
langPrios="it,en"
```

or

```ini
[megadrive]
langPrios="it,en"
```

---

#### artworkXml

Sets a non-default xml file to use when setting up the artwork compositing. By default Skyscraper uses the file `/home/<USER>/.skyscraper/artwork.xml`. If you provide a relative filepath it will be expanded to `/home/<USER>/.skyscraper/<artworkXml>`, respective to `$XDG_CONFIG_HOME/skyscraper/<artworkXml>`, if you use Skyscraper in [XDG](XDG.md) mode.  
Read more about the artwork.xml format and customization options [here](ARTWORK.md).

!!! tip

    It can be _very_ useful to set this in any platform section or frontend section where you want a specific artwork setup / look.

Default value: `artwork.xml`  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### relativePaths

Enabling this forces the rom and any media paths inside the game list to be relative to the rom input folder. Currently only relevant when generating an EmulationStation, a Retrobat or a Pegasus game list (see also [frontend](#frontend) option).

!!! info

    This setting has no effect when the frontend is set to attractmode.

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### extensions

Completely overwrites the rom extensions Skyscraper allows for the chosen platform. Use a space between each extension (e.g., `*.abc *.def`).

!!! info

    If you feel like you are using a file extension that ought to be supported by default, please report it so it can be added in a later version of Skyscraper.

Default value: unset. If set it ignores `"formats": [ ... ]` in `peas.json` file  
Allowed in sections: `[<PLATFORM>]`

---

#### addExtensions

Adds the rom extensions to the ones that are already supported by the platform, use a space between each extension.

!!! info

    If you feel like you are using a file extension that ought to be supported by default, please report it so it can be added in a later version of Skyscraper.

Default value: `*.zip *.7z *.ml` (if and only if `extensions` parameter is unset)  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### hints

If set to false it disables the "Did you know" hints when running Skyscraper.

Default value: `true`  
Allowed in sections: `[main]`

---

#### subdirs

By default Skyscraper will include roms located in subfolders. By disabling this option Skyscraper will only scrape the roms located directly in the input folder. See `inputFolder="<PATH>"` further up to read more about the rom input folder.

Default value: `true`  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### onlyMissing

This flag, when set true, will only scrape game information for ROMs that do not have any data in Skyscraper's cache. The cache information of a ROM that has a cache hit will not be updated or extended when 'only missing' is true. A use case could be that you want to exclude already scraped games from a scraping process and you are using a scraping module which has a limit on how many ROMs may be scraped: With 'only missing' enabled the ROMs with cache data are subtracted from all ROMs to be scraped for a platform, thus the remaining ROM count may be below the scrape module's limit.

!!! tip

    Please consider using the command line flag option `--flags onlymissing` instead. It makes little sense to set this permanently in your configuration unless you _always_ want to use the cached data for already cached game entries. Also when 'only missing' is set and the cache has any data for a game entry, the refresh option has no effect. But you still can use the 'only missing' in the config file if you absolutely want to.

Default value: false  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<SCRAPER>]`

---

#### startAt

If you only ever wish to gather data for a subset of your roms from the scraping modules you can use this option to set the starting rom. It will then scrape alphabetically from that rom and onwards.

!!! tip

    Please consider using the command line option `--startat <FILENAME>` instead. It makes little sense to set this permanently in your configuration unless you _always_ want it to only ever scrape from a certain file and onward. But you can if you absolutely want to.

Default value: unset  
Allowed in sections: `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### endAt

If you only ever wish to gather data for a subset of your roms from the scraping modules you can use this option to set the rom to end at. It will then scrape alphabetically until it reaches this rom, then stop.

!!! tip

    Please consider using the command line option `--endat <FILENAME>` instead. It makes little sense to set this permanently in your configuration unless you _always_ want it to only ever scrape up until it reaches a certain file. But you can if you absolutely want to.

Default value: unset  
Allowed in sections: `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### includePattern

Per platform Skyscraper have default file extensions that it will accept. This option allows you to only include certain files within that scope. The pattern is a simple asterisk type pattern. In cases where you need to match for a comma you need to escape it as `\,`.

!!! note

    You might also want to check out the file extension options and the the 'includeFrom' option.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### excludePattern

Per platform Skyscraper have default file extensions that it will accept. This option allows you to exclude certain files within that scope. The pattern is a simple asterisk type pattern. In cases where you need to match for a comma you need to escape it as `\,`.

!!! note

    You might also want to check out the file extension options and the 'excludeFrom' option.

!!! tip

    If you create a file named `.skyscraperignore` within any subfolder of the input dir, all files from that directory will be ignored by Skyscraper. Additionally, if you put an empty file `.skyscraperignoretree` in a folder, all files in that directory and all subdirectories are ignored.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`, `[<FRONTEND>]`

---

#### includeFrom

Tells Skyscraper to only include the files listed in FILENAME. One filename per line (with FULL path, eg. '/home/pi/RetroPie/roms/snes/subdir/somefile.zip').

This file can be generated with the '--cache report:missing' option or made manually.

!!! note

    You might also want to check out the 'includePattern' option.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### excludeFrom

Tells Skyscraper to exclude the files listed in FILENAME. One filename per line (with FULL path, eg. '/home/pi/RetroPie/roms/snes/subdir/somefile.zip').

This file can be generated with the '--cache report:missing' option or made manually.

!!! note

    You might also want to check out the 'excludePattern' option.

!!! tip

    If you create a file named '.skyscraperignore' within any subfolder of the input dir, all files from that directory will be ignored by Skyscraper. Additionally, if you put an empty file `.skyscraperignoretree` in a folder, all files in that directory and all subdirectories are ignored.

Default value: unset  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### userCreds

Some scraping modules require a `key` or `user:password` to work. You can create a `[<SCRAPER>]` section and add a `userCreds="user:pass"` or `userCreds="key"` line beneath it. Doing this will always use these credentials when scraping from the module in question. Check the scraping module overview to see the specific requirements for each module [here](SCRAPINGMODULES.md).

**Example(s)**

```ini
[screenscraper]
userCreds="user:password"
```

or

```ini
[igdb]
userCreds="key"
```

Default value: unset  
Allowed in sections: `[<SCRAPER>]`

---

#### spaceCheck

Skyscraper will continuously check if you are running low on disk space. If you go below 200 MB in either the game list export folder or the resource cache folder, it will quit to make sure your system doesn't become unstable. Some types of file systems provide a faulty result to Skyscraper when it comes to these checks and thus it can be necessary to disable it altogether. You can use this option to do just that.

Default value: `true`  
Allowed in sections: `[main]`

---

#### scummIni

Allows you to set a non-default path of the `scummvm.ini` file. This file is used whenever scraping the `scummvm` platform. It converts the shortname such as `monkey2` to the more search-friendly name `Monkey Island 2: LeChuck's Revenge` whenever using one of the file name search based scraping modules.

Default value: If `~/.scummvmrc` has precedence over `/opt/retropie/configs/scummvm/scummvm.ini`  
Allowed in sections: `[main]`

---

#### platform

Allows you to set a platform, which is applied when no command line switch `-p` is given.

Default value: unset  
Allowed in sections: `[main]`

---

#### addFolders

If you arrange your ROMs in directories below a platform directory, this flag
comes into play. The [gamelist
specification](https://github.com/RetroPie/EmulationStation/blob/master/GAMELISTS.md#folder)
allows you to define also metadata for directories. This metadata is held in
`<folder/>` elements in the gamelist XML file. In minimum a `<folder/>` has a
`<path/>` and a `<name/>` inner element. If no `<name/>` is given, the last
subdirectory of the path element is taken as name. Which additionally
subelements are interpreted and rendered is dependent on the frontend, for
example EmulationStation for RetroPie supports [these
elements](https://github.com/RetroPie/EmulationStation/blob/95ba1582356cf51734a6505525bc8c67a072d16d/es-app/src/MetaData.cpp#L37-L50)
to be used inside a folder element.

If this flag set to true, Skyscraper will create generic folder elements in the
gamelist file, containing path and name for each directory found. If false, no
folder elements will be created for the directories with ROM(s) ([example](FRONTENDS.md#metadata-preservation)).

In both cases the existing `<folder/>` elements of a gamelist will be preserved.
However, folder data is not cached by Skyscraper, which means if you delete your
`gamelist.xml` Skyscraper can not restore the edited folder elements.

!!! bug "EmulationStation 2.11.2rp and earlier"

    EmulationStation 2.11.2 (RetroPie) and earlier have a bug that adds a `<folder/>` element everytime you edit and save metadata for the _same_ folder within EmulationStation. The last edit will be the latest folder element in the gamelist file, i.e. it adds a `<folder/>` element at the end. Skyscraper in contrast expects in maximum only one `<folder/>` element per each path. If you regenerate a gamelist with Skyscraper, you will lose the latest edit of the folder metadata. This bug is [described here](https://retropie.org.uk/forum/post/295367) and is fixed in the next release of EmulationStation respective is fixed already in the EmulationStation-dev version.

Default value: false  
Allowed in sections: Only for frontends `[emulationstation]`, `[esde]` or `[retrobat]`

---

#### manuals

By default Skyscraper doesn't scrape and cache game manuals resources because not all scraping sites provide this data and also only some frontends support PDF display of these game manuals. If enabled Skyscraper will collect game manuals for the scraping modules that provide this data. For frontend ES-DE no further option must be set to enable the output of the PDF manuals to the appropriate folder. For other EmulationStation forks see also option [gameListVariants](CONFIGINI.md#gamelistvariants).

Default value: false  
Allowed in sections: `[main]`, `[<PLATFORM>]`

---

#### gameListVariants

This is a comma separated list of options for the different gamelist variants
used by the various EmulationStation forks. Currently only `enable-manuals` is
evaluated as variant: It generates `<manual/>` entries in the gamelist for the
game manuals scraped or found in the cache, if also the `manuals` configuration
option is enabled. However, the `gameListVariants` option is not not needed for
the ES-DE frontend to output game manuals.

**Example(s)**

```ini
[emulationstation]
gameListVariants="enable-manuals"
```

Default value: unset  
Allowed in sections: Only for frontend `[emulationstation]`

---

#### ignoreYearInFilename

During scraping, if and only if a year information is identified in parentheses
in the ROM filename, this information is compared against the release year in
the scraper database. If the years are different the game information from the
scraper database is discarded. If no year information is in the ROM filename any
match for the game from the scraper is accepted. To allow a less strict
comparision without having to remove or adjust the year information from the
filename, you can set this option to true.

**Example(s)**

Filename: `Statix (1995)(Black Legend)[h TRSi - HLM].zip`  
Release year from scrape source: 1994  
`ignoreYearInFilename=true`: Scraper match is accepted  
`ignoreYearInFilename=false`: Scraper match is discarded, may end up in result _Game 'Statix (1995)(Black Legend)[h TRSi - HLM]' not found :(_

Default value: `false`  
Allowed in sections: `[main]`, `[<PLATFORM>]`