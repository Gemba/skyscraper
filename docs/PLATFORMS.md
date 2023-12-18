## Supported Platforms

Get a list of supported platforms with `Skyscraper --help`.

After the initial work from torresflo @ GitHub it is possible to add new
platforms by editing the `platforms.json` file. However, since version 3.9.0 of
Skyscraper this file is replaced by `peas.json` (short for *p*latforms,
*e*xtensions/formats, *a*liases and *s*crapers) in the same folder. The information in
the file is the same as before.

Take this example from the `peas.json` file:

```json
    [...]
    "megadrive": {
        "aliases": [
            "genesis",
            "mega drive",
            "sega mega drive",
            "sega mega drive/genesis"
        ],
        "formats": [
            "*.bin",
            "*.gen",
            "*.md",
            "*.sg",
            "*.smd"
        ],
        "scrapers": [
            "openretro",
            "screenscraper"
        ]
    },
    [...]
```

- `megadrive`: this key reflects the platform/folder name (or more generally the
  handle), usually provided with `-p` on the command line.
- `aliases`: these aliases can be used to allow additional hits for a search.
  The use of aliases is handy especially for scraping websites which do _not_
  provide a web API. Currently Screenscraper, Mobygames or The Games DB are
  providing an web API. See also section about
  [`platforms_idmap.csv`](#exact-platform-mapping) and [Updating the
  files](#updating-peasjson-and-platforms_idmapcsv) below.
- `formats`: set of ROM file extensions which will be included in scraping if
  a ROM file is not provided explicitly via command line.
- `scrapers`: Denotes the possible scrapers if not overridden by `-s` command
  line switch. The `scrapers` segment in the JSON will be deprecated in some
  later versions as it is not used in the codebase.

!!! tip

    If you need a specific folder name for a platform (on your setup or due to an EmulationStation theme) use a symbolic link (see `megadrive` (=folder) and `genesis` (=symlink) for example on RetroPie or `plus4` (=folder) and `c16` (=symlink)) instead of adding a new platform in this JSON file.

### Exact platform mapping

The file `platforms_idmap.csv` defines exact platform id for the web API of the
three before mentioned scraping sites. It is a CSV file which maps the platform
handle (e.g. `megadrive`) to the respective platform id of the scraping sites:

```csv
folder,screenscraper_id,mobygames_id,tgdb_id
[...]
megadrive,1,16,36
[...]
```

You can display the number with their platform name on each of the three
scraping sites with the script `peas_and_idmap_verify.py`. Find the script
sibling to the Skyscraper executable. Below is a part of the output:

```
[...]
    ├── megadrive
    │   ├── ScrS    1: Megadrive
    │   ├── Moby   16: Genesis
    │   └── TGDB   36: Sega Mega Drive
[...]
```

## How to Add Platforms For Scraping

Outline:

1. Create a new platform block in `peas.json`, or copy an existing one and adapt
   to your needs. For RetroPie your chosen `<platform_name>` must match the
   folder in `~/RetroPie/roms/<platform_name>`.
2. Use `<platform_name>` also in `platforms_idmap.csv` (see below for details).
3. If you use RetroPie do add the platform/system also to your `es_systems.cfg` as
   documented
   [here](https://retropie.org.uk/docs/Add-a-New-System-in-EmulationStation/)

There is also a an verbatim example, you may skip the next section initially and
can continue with the [hands-on example](PLATFORMS.md#sample-usecase-adding-platform-satellaview).

### Updating `peas.json` and `platforms_idmap.csv`

These two files are ment to be locally edited and extended for additional
platforms. Whenever you add a new platform block to the `peas.json` do also
lookup the corresponding platform ids and add them to `platforms_idmap.csv` for
the scraping sites with an API.

To find the platform ids for Screenscraper, Mobygames and The Games DB, please
consult the files `screenscraper_platforms.json`, `mobygames_platforms.json` and
`tgdb_platforms.json` which are located sibling to your `config.ini` of the
Skyscraper installation. If you can not identify an id in these files use `-1`
as value in the CSV. If you add `-1` to CSV, the `aliases` from peas are tried
to find a match upon scraping. Edits in `screenscraper_platforms.json`,
`mobygames_platforms.json` and `tgdb_platforms.json` are not needed and moreover
they will be overwritten with each Skyscraper update, as these files are only a
reference for finding the id values for the `platforms_idmap.csv`.

For those scraping sites without an API or without exact id match do use the
platform name which is used on the scraping site and put it into the in the `aliases`
list in the `peas.json` for the respective platform/system at `<platform_name>`.

!!! example Use of Aliases

    The platforms ScummVM or Steam do not have an exact match on Mobygames, however you may scrape successfully for ScummVM and Steam games if you use 'PC', 'DOS', 'Windows', 'Linux' or similar as `"aliases": ...` in the `"scummvm": ...` or `"steam": ...` section of `peas.json`. Usually you find the platform information if you lookup the game manually on the scraping website.

### Sample Usecase: Adding Platform _Satellaview_ 

Let the platform/systemname be `satellaview`. You may read about this SNES
enhancing peripheral [here](https://en.wikipedia.org/wiki/Satellaview).

#### Step 1: Add a Section to `peas.json`

Actually, this is a copy of the [`"snes:
..."`](https://github.com/Gemba/skyscraper/blob/4aff22586d848f9974d2464d5372b8986a0e64c0/peas.json#L1833-L1857)
block, with slight modifications.

```json linenums="1" hl_lines="2 4"
...
"satellaview": {
	"aliases": [
		"snes",
		"nintendo power",
		"snes - super mario world hacks",
		"sufami turbo",
		"super famicom",
		"super nintendo",
		"super nintendo (snes)",
		"super nintendo entertainment system (snes)",
		"super nintendo msu-1"
	],
	"formats": [
		"*.bin",
		"*.fig",
		"*.mgd",
		"*.sfc",
		"*.smc",
		"*.swc"
	]
}
...
```

- Line 2 defines the platform name
- Line 4 adds an alias to SNES as Satellaview is an hardware addon to a SNES
- Note the absence of the `"scrapers":` section. It is not used by Skyscraper.

#### Step 2: Lookup Values for _Satellaview_ and Update `platforms_idmap.csv`

To fill in the values into the CSV file do consult the
[`screenscraper_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/screenscraper_platforms.json),
[`mobygames_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/mobygames_platforms.json)
and
[`tgdb_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/tgdb_platforms.json)
files. Try to find the ID for the platform in these files. For `satellaview`
only Screenscraper has an exact match: [`107`](https://github.com/Gemba/skyscraper/blob/4aff22586d848f9974d2464d5372b8986a0e64c0/screenscraper_platforms.json#L1179-L1180). Use `-1` as ID when no exact
match is provided for the scraping site. Whenever an `-1` is encountered
Skyscraper tries the `"aliases":` from `peas.json` to find scraping data.

Add this information at the end of the CSV file. Why at the end? On rare
occasions the `platforms_idmap.csv` may be updated on a new release. On RetroPie
the installed file is then named `platforms_idmap.csv.rp-dist` (the same
mechanism works for `peas.json`), having changes at the end may be a less
cumbersome manual merge with your local `platforms_idmap.csv`.

Add this information:
```csv
satellaview,107,-1,-1
```

#### Step 3: Create the System in RetroPie/EmulationStation and Populate the ROM Folder

This part should be added to your `~/.emulationstation/es_systems.cfg`. See
[here](https://retropie.org.uk/docs/Add-a-New-System-in-EmulationStation/) for
additional information on this.

!!! warning

    Stop EmulationStation before editing the config file.

```xml linenums="1" hl_lines="3 6 9"
...
<system>
	<name>satellaview</name>
	<fullname>Satellaview</fullname>
	<path>/home/pi/RetroPie/roms/Nintendo - Satellaview</path>
	<extension>.7z .bin .bs .smc .sfc .fig .swc .mgd .zip .7Z .BIN .BS .SMC .SFC .FIG .SWC .MGD .ZIP</extension>
	<command>/opt/retropie/supplementary/runcommand/runcommand.sh 0 _SYS_ snes %ROM%</command>
	<platform>satellaview,snes</platform> <!--Not used by Screenscraper, but by built-in ES scraper and maybe others-->
	<theme>satellaview</theme>
</system>
...
```

- Line 3 defines the platform name, respective the folder name for your ROMs.
  Thus, Skyscraper expects to find ROMs in `/home/pi/RetroPie/roms/satellaview`.
- Line 6 contains the extensions which are recognized by EmulationStation. These
  extensions should be also be present in the `"formats":` block of `peas.json`.
  However, Skyscraper uses case insensitive extension mapping. The
  extensions `.7z` and `.zip` are added automagically by Skyscraper, thus the
  `"formats":` list is usually shorter than the EmulationStation `<extension/>`
  list.
- Line 9: If your theme doesn't support Satellaview, you can also use `snes` as
  <theme> value.

!!! note

    If you run a different frontend than EmulationStation, consult the documentation for your frontend on how to add additional systems.

#### Step 4: Happy Scraping

1. Scrape and generate the `satellaview/gamelist.xml` as in the [introductive
   use case](USECASE.md) using `Skyscraper -p
   satellaview -s screenscraper` and `Skyscraper -p satellaview`
2. Restart EmulationStation, respective trigger reload of the Gamelist in your
   frontend.
3. Smile :)

!!! success "Kudos"

    Thanks to retrobit @ GitHub for contributing this usecase.

### Migrating `platforms.json` and `screenscraper.json`

!!! info

    If you neither edited `platforms.json` nor `screenscraper.json` or do not have these files in the Skyscraper config folder (sibling to the `config.ini`) you can safely ignore this section.

If you have local changes in these files you can transfer your local
changes to the new files. The new `peas.json` is the `platforms.json` but in a
more compact form which has less visual overhead and also allows faster parsing.
The `platforms_idmap.csv` maps the former `screenscraper.json` platform ids plus
the platform ids of two more web API sites.

Use the script `convert_platforms_json.py` (sibling to the Skyscraper
executable) to convert the `platforms.json` to a `peas_mine.json` which you can
then diff to the `peas.json` and transfer your changes to `peas.json`.

Use the script `check_screenscraper_json_to_idmap.py` to identify differences
from your `screenscraper.json`. Then use the three `<scraper>_platforms.json`
files to identify the matching platform ids to be entered in
`platform_idmap.csv`. Use `-1` in this file, if there is no matching platform id.

### Summary of Changes in the Config Files

Filenames shown ~~strikethrough~~ are superseeded.  
Filenames shown _italic_ are user editable.

| <div style="width:140px">File</div> | Introduced with Version | Notes (version)                                                                                                                                      |
| ----------------------------------- | ----------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| ~~`mobygames.json`~~                | 3.8.0                   | superseeded by `mobygames_platforms.json` (v3.9.0); not to be edited; IDs are used in `platforms_idmap.csv`                                          |
| ~~`platforms.json`~~                | 3.7.7-2 (@detain)       | superseeded by _`peas.json`_ (3.9.0); do edit to add new platforms; uses leaner format than `platforms.json`                                         |
| _`platforms_idmap.csv`_             | 3.9.0                   | maps the platform names (handles) from _`peas.json`_ to exact platform IDs used in Moby, Screenscraper or TGDB; do edit to add new platforms         |
| ~~`screenscraper.json`~~            | 3.7.7-2 (@detain)       | IDs formerly used in here are part of `platforms_idmap.csv` (3.9.0); superseeded by `screenscraper_platforms.json` (3.9.0) which is not to be edited |
| `tgdb_developers.json`              | 2.5.3 (@muldjord)       | API mapping of 'Developers'; Uses leaner format as before (3.9.0); not to be edited                                                                  |
| `tgdb_genres.json`                  | 3.9.0                   | API mapping of 'Genres' (3.9.0); not to be edited                                                                                                    |
| `tgdb_platforms.json`               | 3.9.0                   | API mapping of 'Platforms' (3.9.0); not to be edited; IDs are used in `platforms_idmap.csv`                                                          |
| `tgdb_publishers.json`              | 2.5.3 (@muldjord)       | API mapping of 'Publishers'; Uses leaner format as before (3.9.0); not to be edited                                                                  |
