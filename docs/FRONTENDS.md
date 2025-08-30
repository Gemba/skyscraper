## Overview

When generating a game list with Skyscraper you have the option of generating it for several different frontends. A frontend is the graphical interface that lists and launches your games.

Setting a frontend when generating a game list is done by setting the `-f <FRONTEND>` command-line parameter as explained [in the commandline documentation](CLIHELP.md#-f-frontend) or by setting it in `/home/<USER>/.skyscraper/config.ini` as explained [config file documentation](CONFIGINI.md#frontend). Use for the `<FRONTEND>` value the frontend name all lowercase and with alphabetical characters only: `emulationstation`, `esde`, `pegasus`, `retrobat`, `attractmode`. Some frontends have further options that are either optional or required. Check the frontend sections below for more information on this.

!!! warning

    Skyscraper will overwrite your game list (obviously). So if you have spend a lot of time hand-crafting metadata in a game list for any frontend, please remember to create a backup before overwriting it with Skyscraper. You can also tell Skyscraper to auto-backup old game lists prior to overwriting them. Read more about the [`gamelistbackup` config option](CONFIGINI.md#gamelistbackup).

When generating a game list for any frontend, Skyscraper will try to preserve certain metadata. Check the frontend sections below for more information on what metadata is preserved per frontend.

### EmulationStation (default)

-   Default game list location: `/home/<USER>/RetroPie/roms/<PLATFORM>`
-   Default game list filename: `gamelist.xml`

This is the default frontend used when generating a game list with Skyscraper. If no frontend is defined on command-line or in `config.ini` it will fall back to generating for EmulationStation. By default the `<USER>` is `pi` on RetroPie setups.

#### Metadata preservation

Skyscraper will preserve the following metadata when re-generating a game list for EmulationStation: `favorite`, `hidden`, `kidgame`, `lastplayed`, `playcount`, `sortname`. Also existing `<folder/>` elements of a gamelist file will be preserved. The user editable sub-XML elements for a folder are listed in the [`Metadata.cpp` of EmulationStation](https://github.com/RetroPie/EmulationStation/blob/01de7618d0d248fa2ff1eacde09a20d9d2af5f10/es-app/src/MetaData.cpp#L30).

!!! warning

    Folder data is not cached by Skyscraper, thus if you delete your `gamelist.xml`, Skyscraper can not restore the edited folder elements from cache.

Automatic addition of folder elements if [`addFolder`](CONFIGINI.md#addfolders) is true:  
If at least one ROM is within a subfolder and this subfolder is not yet part of the `gamelist.xml` file, it will be added with two mandatory subelements:

-  `<path/>` reflects the relative subpath from the system folder and 
-  `<name/>`, which represents the direct parent folder of a ROM by default. However, you may edit this to any name which should be shown in EmulationStation.

!!! example

    Consider this folder structure below `snes`, whereas each lowest folder contains at least one ROM:
    ```
    snes
    └── Retail
        ├── EUR
        ├── JP
        └── USA
    ```
    Skyscraper will generate these <folder/> elements if not present in `gamelist.xml` and only if [`addFolders`](CONFIGINI.md#addfolders) is set true:
    ```xml
    [...]
        <folder>
            <path>./Retail</path>
            <name>Retail</name>
        </folder>
        <folder>
            <path>./Retail/USA</path>
            <name>USA</name>
        </folder>
        <folder>
            <path>./Retail/JP</path>
            <name>JP</name>
        </folder>
        <folder>
            <path>./Retail/EUR</path>
            <name>EUR</name>
        </folder>
    [...]
    ```
    The `Retail` folder is added even if it does not contain a ROM because it is part of the path to the ROMs in the lowest folders.

### EmulationStation Desktop Edition (ES-DE)

This is very similar to the default EmulationStation. This section only
highlights the differences.

-   Default game list location: `/home/<USER>/ES-DE/gamelists/<PLATFORM>`
-   Default game list filename: `gamelist.xml`

ROMs are expected to be in the input folder `/home/<USER>/ROMs/<PLATFORM>` for
every `<PLATFORM>` you scrape.

Media filepaths (screenshots, marquees, videos, aso.) are not explicitly stored
in the gamelist file. This is ES-DE default. However, Skyscraper will put the
media files into the `downloaded_media` folder (e.g.
`~/ES-DE/downloaded_media/<PLATFORM>/screenshots/` for screenshots) from where
ES-DE will pick them up. ES-DE does not support textures currently.

!!! tip

    You may override the default storage folder of ES-DE (`/home/<USER>/ES-DE`)
    on a non-Windows build by setting the environment variable `ESDE_APPDATA_DIR`
    to a different folder as documented in the [ES-DE documentation](https://gitlab.com/es-de/emulationstation-de/-/blob/v3.0.2/INSTALL.md#changing-the-application-data-directory).

All the gory details of ES-DE's gamelist file format you can find in the section
"Gamelist Reference" in the [ES-DE documentation]
(https://gitlab.com/es-de/emulationstation-de/-/blob/master/INSTALL.md?ref_type=heads#gamelistxml).

#### Metadata preservation

Skyscraper will, additionally to EmulationStation preserved metadata, retain the
following metadata when re-generating a game list for ES-DE: `altemulator`,
`broken`, `collectionsortname`, `completed`, `controller`, `hidemetadata`,
`nogamecount`, `nomultiscrape`. Also existing `<folder/>` elements of a gamelist
file will be preserved: For these on top is the `folderlink` element is preserved.

### RetroBat

-   Default game list location: `/home/<USER>/RetroPie/roms/<PLATFORM>`
-   Default game list filename: `gamelist.xml`

This is modeled after EmualtionStation as it uses it with slight differences.

### Attract-Mode

-   Default game list location: `/home/<USER>/.attract/romlists`
-   Default game list filename: `<EMULATOR/PLATFORM>.txt`

Attract-Mode is a bit more abstract when it comes to how it saves its game lists and media. To export for Attract-Mode you need to, in addition to setting the frontend, set `-e <EMULATOR>` on command-line or in `config.ini`. The `<EMULATOR>` is a file that describes the platform / emulator you are generating a game list for. The file contains everything needed to tell Attract-Mode how to launch games for the platform, and even where to find the media files for the games (such as screenshots and videos).

If you are running RetroPie most of the `<EMULATOR>` files will have been auto-generated for you. They are usually named the same as the platform you are generating a game list for. So, if you are generating for `snes` you simply use `Skyscraper -p snes -f attractmode -e snes` and that should work just fine. The `<EMULATOR>` files are usually located at `/home/<USER>/.attract/emulators/`. Check them out if you are curious.

#### Metadata preservation

Skyscraper will preserve the following metadata when re-generating a game list for Attract-Mode: `altromname`, `alttitle`, `buttons`, `cloneof`, `control`, `displaycount`, `displaytype`, `extra`, `rotation`, `status`.

### Pegasus

-   Default game list location: `/home/<USER>/RetroPie/roms/<PLATFORM>`
-   Default game list filename: `metadata.pegasus.txt`

Pegasus is easy and simple to generate a game list for. Simply do `Skyscraper -p <PLATFORM> -f pegasus`. If you want to specify a custom launch command (if you are using RetroPie you don't have to, a default one will be used), you can set it on command-line with `-e "<COMMAND>"` or in `config.ini` with:

```ini
[pegasus]
launch="<COMMAND>"
```

You need to add the individual platform rom directories to Pegasus (if they are not already defined) before any of them will show up! Start the Pegasus frontend, press ESC on the keyboard and choose _Settings_ -> _Set game directories_. Simply point it to each individual platform sub-directory. For RetroPie you should have a path for each platform (eg. `/home/<USER>/RetroPie/roms/snes`, `/home/<USER>/RetroPie/roms/megadrive` etc.).

!!! info

    If you are generating game lists for Pegasus, it is highly recommended to disable third-party game list data sources! Otherwise you will have a mish-mash or different sources showing up in Pegasus. Start the Pegasus frontend, press ESC on the keyboard and choose _Settings_ -> _Enable/disable data sources_ and disable everything in that submenu.  
    Then reload the game lists or restart Pegasus, and all of the platforms should show up with media and game information generated by Skyscraper.

#### Metadata preservation

Skyscraper will preserve any metadata key-value pairs added to the header and / or individual game list entries.
