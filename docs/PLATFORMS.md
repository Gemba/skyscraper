## Supported Platforms

Get a list of supported platforms with `Skyscraper --help`. The set of supported
platforms can be adapted to your needs by configuration files.

### File One: Platformnames, -aliases and Gamefile Extensions

After the initial work from torresflo @ GitHub it was possible to add new
platforms by editing the `platforms.json` file. However, since version 3.9.0 of
Skyscraper this file is replaced by `peas.json`. in the same folder. The
filename is an acronym for *p*latforms, *e*xtensions/formats, *a*liases and
*s*crapers. However, the scrapers list has been removed with 3.13.0 as it did
not have any use.

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
        ]
    },
    [...]
```

- `<key>`: the key (`megadrive` in the example) reflects the platform/folder
  name (or more generally the handle), usually provided with `-p` on the command
  line.
- `aliases`: these aliases can be used to allow additional hits for a search.
  The use of aliases is handy especially for scraping websites which do _not_
  provide a web API. Currently Screenscraper, Mobygames or The Games DB are
  providing an web API. See also section about
  [`platforms_idmap.csv`](#exact-platform-mapping) and [Updating the
  files](#updating-peasjson-and-platforms_idmapcsv) below.
- `formats`: set of ROM file extensions which will be included in scraping if
  a ROM file is not provided explicitly via command line.

Since Skyscraper 3.13.0 you should maintain local changes to the `peas.json`
in a separate file called `peas_local.json` alongside to the `peas.json`. The
format is identical to the `peas.json`. Since Skyscraper 3.15 you can also
maintain local changes to the `platforms_idmap.csv` in a separate file with a
`_local` postfix. 

!!! tip "The order of precedence" 

    Any platform entry from `peas_local.json` with the same platform name as in
    `peas.json` overwrites the one from `peas.json`. The same applies for the
    platforms ID-mapping (see next section).

!!! tip

    If you need a specific folder name for a platform (on your setup or due to an
    EmulationStation theme) use a symbolic link (for example `megadrive` (=folder)
    and `genesis` (=symlink) on RetroPie setups or another example: `plus4`
    (=folder) and `c16` (=symlink)) instead of duplicating the platform in the JSON
    file.

### File Two: Exact platform mapping

The second file is used for to instruct scrapers to lookup games by the numeric
platform identifier the scraping site uses for queries. The file
`platforms_idmap.csv` defines exact platform id for the web APIs of
Screenscraper, MobyGames and the GamesDB. It is a CSV file which maps the
platform handle (e.g. `megadrive`) to the respective platform id of the scraping
site (selected with the CLI option `-s`):

```csv
folder,screenscraper_id,mobygames_id,tgdb_id
[...]
megadrive,1,16,36
[...]
```

You can display the number with their platform name on each of the three
scraping sites in a more readable format with the script
`peas_and_idmap_verify.py`. Find the script sibling to the Skyscraper
executable. Below is a part of the output (ScrS refers to the Screenscraper
site):

```
[...]
    ├── megadrive
    │   ├── ScrS    1: Megadrive
    │   ├── Moby   16: Genesis
    │   └── TGDB   36: Sega Mega Drive
[...]
```

## How to Change Platform Aliases or Gamefile Extensions

Follow instructions in this section if you only want to overwrite platform alias
names or gamefile extensions.

1. If you don't have a `peas_local.json` file: Create the file `peas_local.json`
   sibling to `peas.json`. Enter in this file an empty JSON object `{}` (=just
   curly braces).
2. Create a new platform block in `peas_local.json` inside these curly braces
   (you created in step 1) by copying an existing platform block from the
   `peas.json` and adapt to your needs, but keep the platform name.

Example: Copy this excerpt from `peas.json`...

```json linenums="1" hl_lines="2 20"
    [...]
    "psx": {            <-- begin of platform block
        "aliases": [
            "playstation",
            "sony playstation"
        ],
        "formats": [
            "*.cbn",
            "*.chd",
            "*.cue",
            "*.img",
            "*.iso",
            "*.m3u",
            "*.mdf",
            "*.pbp",
            "*.toc",
            "*.z",
            "*.znx"
        ]
    },                  <-- end of platform block
    [...]
```

...into `peas_local.json` and add `*.bin` as recognized extension. 

```json linenums="1" hl_lines="8 21"
{
    "psx": {
        "aliases": [
            "playstation",
            "sony playstation"
        ],
        "formats": [
            "*.bin",    <-- added extenstion
            "*.cbn",
            "*.chd",
            "*.cue",
            "*.img",
            "*.iso",
            "*.m3u",
            "*.mdf",
            "*.pbp",
            "*.toc",
            "*.z",
            "*.znx"
        ]
    }                   <-- comma needed here if not last in platform list
}
```

If you have multiple platforms defined in your local file make sure the platform
blocks are separated by a comma `,`.


!!! tip

    On RetroPie you may also have to edit `~/.emulationstation/es_systems.cfg` and
    add `.bin` and `.BIN`.

## How to Add Platforms For Scraping

Outline:

1. Create a file `peas_local.json` sibling to `peas.json`. Enter in this file an
   empty `{}` JSON object.
2. Create a new platform block in `peas_local.json` inside the outer (empty)
   block created before, or copy an existing block and adapt to your needs. For
   RetroPie your chosen `<platform_name>` must match the folder in
   `~/RetroPie/roms/<platform_name>`.
3. Use `<platform_name>` also in `platforms_idmap_local.csv`. If you need to
   create an `platforms_idmap_local.csv` put in the column names
   `folder,screenscraper_id,mobygames_id,tgdb_id` (i.e. the first line of
   `platforms_idmap.csv`) . See also below for details of this CSV-file.
4. If you use RetroPie do add the platform/system also to your `es_systems.cfg`
   as documented in the 
   [RetroPie documentation](https://retropie.org.uk/docs/Add-a-New-System-in-EmulationStation/).

There is also a an verbatim example, you may skip the next section initially and
can continue with the [hands-on example](PLATFORMS.md#sample-usecase-adding-platform-satellaview).

### Updating `peas_local.json` and `platforms_idmap_local.csv`

These two files are ment to be locally edited and extended for additional
platforms. Whenever you add a new platform block to the `peas_local.json` do also
lookup the corresponding platform ids and add them to `platforms_idmap_local.csv` for
the scraping sites via a web-API.

**From Skyscraper 3.15 onwards** creating/editing the `peas_local.json` and
`platforms_idmap_local.csv` is the preferred way. In any case both files (e.g.
`peas.json` and `peas_local.json`) will be evalutated but the `_local`
configuration has precedence over the distributed `peas.json`. The same rule
applies to `platforms_idmap.csv` and `platforms_idmap_local.csv`.  
If you have made local changes before Skyscraper 3.15 to either `peas.json` or
`platforms_idmap.csv` Skyscraper will inform you with an warning. Read the
section [Transferring Local Platform
Changes](PLATFORMS.md#transferring-local-platform-changes) on how to transfer
your changes to the corresponding `*_local.*` files.

To find the platform ids for Screenscraper, Mobygames and The Games DB, please
consult the files `screenscraper_platforms.json`, `mobygames_platforms.json` and
`tgdb_platforms.json` which are located sibling to your `config.ini` of the
Skyscraper installation. If you can not identify an ID in these files use `-1`
as value in the CSV. If you add `-1` to CSV, the `aliases` from peas are tried
to find a match upon scraping. Edits in `screenscraper_platforms.json`,
`mobygames_platforms.json` and `tgdb_platforms.json` are not needed. Moreover,
they will be overwritten with each Skyscraper update as these files are only a
reference for finding the id values for the `platforms_idmap[_local].csv`.

For those scraping sites without a web-API or without exact ID match do use the
platform name which is used on the respective scraping source site and put it
into the in the `aliases` list in the `peas_local.json` for the respective
platform/system at `<platform_name>`. The `<platform_name>` is identical to the
folder on your filesystem where you keep your games.

!!! example "Use of Aliases"

    The platforms ScummVM and Steam do not have an exact match on Mobygames, however you may scrape successfully for ScummVM and Steam games if you use 'PC', 'DOS', 'Windows', 'Linux' or similar as `"aliases": ...` in the `"scummvm": ...` or `"steam": ...` section of `peas.json`. Usually you find the platforms a game runs on if you lookup the game manually on the scraping website.

### Sample Usecase: Adding Platform _Satellaview_

Let the platform/systemname be `satellaview`. You may read about this SNES
enhancing peripheral [in Wikipedia](https://en.wikipedia.org/wiki/Satellaview).

#### Step 1: Add a Section to `peas_local.json`

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

#### Step 2: Lookup Values for _Satellaview_ and Update `platforms_idmap.csv`

To fill in the values into the CSV file do consult the
[`screenscraper_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/screenscraper_platforms.json),
[`mobygames_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/mobygames_platforms.json)
and
[`tgdb_platforms.json`](https://github.com/Gemba/skyscraper/blob/master/tgdb_platforms.json)
files. Try to find the ID for the platform in these files. For `satellaview`
only Screenscraper has an exact match:
[`107`](https://github.com/Gemba/skyscraper/blob/4aff22586d848f9974d2464d5372b8986a0e64c0/screenscraper_platforms.json#L1179-L1180).
Use `-1` as ID when no exact match is provided for the scraping site. Whenever
an `-1` is encountered Skyscraper tries the `"aliases":` from `peas.json` (and
from `peas_local.json` if present) to find scraping data.

Add this information to your `platforms_idmap_local.csv`:
```csv
satellaview,107,-1,-1
```

#### Step 3: Create the System in RetroPie/EmulationStation and Populate the ROM Folder

This part should be added to your `~/.emulationstation/es_systems.cfg`. See the
[RetroPie
documentation](https://retropie.org.uk/docs/Add-a-New-System-in-EmulationStation/)
for additional information on this.

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
  However, Skyscraper uses case insensitive file extension mapping. The
  extensions `.7z` and `.zip` are added automagically by Skyscraper, thus the
  `"formats":` list is usually shorter than the EmulationStation `<extension/>`
  list.
- Line 9: If your theme doesn't support Satellaview, you can also use `snes` as
  <theme> value.

!!! note

    If you run a different frontend than EmulationStation, consult the
    documentation for your frontend on how to add additional systems.

#### Step 4: Happy Scraping

1. Scrape and generate the `satellaview/gamelist.xml` as in the [introductive
   use case](USECASE.md) using `Skyscraper -p satellaview -s screenscraper` and
   then `Skyscraper -p satellaview`
2. Restart EmulationStation, respective trigger reload of the gamelist in your
   frontend.
3. Smile :)

!!! success "Kudos"

    Thanks to retrobit @ GitHub for contributing this usecase.

### Transferring Local Platform Changes

This section describes how to transfer your changes from `peas.json` and
`platforms_idmap.csv` to `*_local.*` files with the same format. If you never
changed the first two files, you can safely ignore this section.  
Introduction: Whenever there is an update and maybe upstream changes to `peas.json` and
`platforms_idmap.csv` Skyscraper will place the upstream/distribution files as
`peas.json.rp-dist` and `platforms_idmap.csv.rp-dist`.  
Before Skyscraper 3.15 you had to manually transfer updates from
`peas.json.rp-dist` and `platforms_idmap.csv.rp-dist` to the actual files. With
Skyscraper 3.15 onwards there is a semi-automated approach.

!!! note Non-RetroPie Users

    If you are using Skyscraper in a non-RetroPie
    setup these files will have the suffix `.dist`. The manual will use
    `.rp-dist` as synonym for both.

#### Step 1: Transfer Platform Information (`peas`) to Local File

Install Python Deepdiff: `sudo apt install python3-deepdiff`. Then navigate to
`/opt/retropie/supplementary/skyscraper/` and find the script
`deepdiff_peas_jsonfiles.py`.

!!! tip Non-RetroPie Users

    If you are using Skyscraper without RetroPie
    context the you can find the Python script in the source-tree below
    the `supplementary/scraperdata` folder.

The script expects at least two parameters:

1. `<source_peas.json>`: The pristiine/baseline file with all platform
   information (most likely `peas.json.rp-dist`)
2. `<dest_peas.json>`: The file with your current local changes (most likely
   `peas.json`)
3. Optionally `<outfile_peas.json>`: Once you have reviewed the changes provide
   this file to store the platform "diff" between the pristine file and your
   changes

**Example(s)**

Create a diff on the console:
```bash
python3 deepdiff_peas_jsonfiles.py peas.json.rp-dist peas.json
```

Review the diff, then run:
```bash
python3 deepdiff_peas_jsonfiles.py peas.json.rp-dist peas.json peas_local.json
```

Backup your `peas.json` if needed and when satisfied with the content of
`peas_local.json` move `peas.json.rp-dist` to `peas.json`.

#### Step 2: Transfer Platform Scraper IDs (`platforms_idmap`) to Local File

The logic is the same as before and provides an output of the lines you have
changed in `platforms_idmap.csv` in relation to the baseline
`platforms_idmap.csv.rp-dist`.

!!! warning

    If you already have an existing `platforms_idmap_local.csv` make a
    backup as the following command will create a new one.
    You may also alter the file redirect to a different file to avoid the
    file to be overwritten.

Navigate to the folder with the `platforms_idmap.csv`. Then run:

```bash
[[ -e platforms_idmap_local.csv ]] && mv platforms_idmap_local.csv platforms_idmap_local.csv.backup
diff \
  --new-line-format="%L" \
  --old-line-format="" \
  --unchanged-line-format="" \
  platforms_idmap.csv.rp-dist platforms_idmap.csv > platforms_idmap_local.csv
```

Also add the column header (folder, screenscraper_id, mobygames_id, tgdb_id) to
the `platforms_idmap_local.csv` file, for example with:

```bash
echo folder,screenscraper_id,mobygames_id,tgdb_id | \
cat - platforms_idmap_local.csv > tmp_piggy.csv && \
mv tmp_piggy.csv platforms_idmap_local.csv
```

Backup your `platforms_idmap.csv` if needed and when satisfied with the content
of your `platforms_idmap_local.csv` move `platforms_idmap.csv.rp-dist` to
`platforms_idmap.csv`.

#### Finish

From now on upstream changes will be placed in `peas.json` and
`platforms_idmap.csv` and your local additions or changes are kept in the
`*_local.*` counterparts. However, remember when platforms are defined in both
files the `*_local.*` platform configuration wins.

#### One More Thing...

If you have changes which would be beneficial for the community, feel free to
file an issue with the proposed additions/changes or table it in the [RetroPie
Forum/Skyscraper Thread](https://retropie.org.uk/forum/topic/34588). Thank you!

### Migrating `platforms.json` and `screenscraper.json`

This section is only applicable if you update from Skyscraper 3.7.7-2.

!!! tip

    If you neither edited `platforms.json` nor `screenscraper.json` or do not
    have these files in the Skyscraper config folder (sibling to the
    `config.ini`) you can safely ignore this section.

If you have local changes in these files you can transfer your local
changes to the new files. The new `peas.json` is the `platforms.json` but in a
more compact form which has less visual overhead and also allows faster parsing.
The `platforms_idmap.csv` maps the former `screenscraper.json` platform ids plus
the platform ids of two more web API sites.

Use the script `convert_platforms_json.py` (sibling to the Skyscraper
executable) to convert the `platforms.json` to a `peas_mine.json` file which you
can then diff to the `peas.json` and transfer your changes to
`peas_local.json`.

Use the script `check_screenscraper_json_to_idmap.py` to identify differences
from your `screenscraper.json`. Then use the three `<scraper>_platforms.json`
files to identify the matching platform ids to be entered in
`platforms_idmap_local.csv`. Use `-1` in this file, if there is no matching platform id.

### Summary of Changes in the Config Files

Filenames shown ~~strikethrough~~ are superseded.  
Filenames shown _italic_ are user editable.

| <div style="width:140px">File</div> | Introduced with Version | Notes (version)                                                                                                                                                                                       |
| ----------------------------------- | ----------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ~~`mobygames.json`~~                | 3.8.0                   | superseded by `mobygames_platforms.json` (v3.9.0); not to be edited; IDs are used in `platforms_idmap.csv`                                                                                            |
| ~~`platforms.json`~~                | 3.7.7-2 (@detain)       | superseded by _`peas.json`_ (3.9.0) and _`peas_local.json`_ (3.13.0); do edit the latter to add/change platforms; these files use a leaner format than the initially used `platforms.json`            |
| _`platforms_idmap.csv`_             | 3.9.0                   | maps the platform names (handles) from _`peas.json`_ / _`peas_local.json`_ to exact platform IDs used in scrapers MobyGames, Screenscraper or TGDB; do edit to add new platforms                      |
| _`peas.json`_                       | 3.9.0                   | maps platform names (read: ROM folder names) to extensions and aliases for that platform                                                                                                              |
| _`peas_local.json`_                 | 3.13.0                  | same as usage as `peas.json`, the `_local.json` file will not be altered by Skyscraper updates. Entries in this file have higher precedence than the distribution file `peas.json`                    |
| _`platforms_idmap_local.csv`_       | 3.15.2                  | same as usage as `platforms_idmap.csv`, the `_local.csv` file will not be altered by Skyscraper updates. Entries in this file have higher precedence than the distribution file `platforms_idmap.csv` |
| ~~`screenscraper.json`~~            | 3.7.7-2 (@detain)       | IDs formerly used in here are part of `platforms_idmap.csv` (3.9.0); superseded by `screenscraper_platforms.json` (3.9.0) which is not to be edited                                                   |
| `tgdb_developers.json`              | 2.5.3 (@muldjord)       | API mapping of 'Developers'; Uses leaner format as before (3.9.0); not to be edited                                                                                                                   |
| `tgdb_genres.json`                  | 3.9.0                   | API mapping of 'Genres' (3.9.0); not to be edited                                                                                                                                                     |
| `tgdb_platforms.json`               | 3.9.0                   | API mapping of 'Platforms' (3.9.0); not to be edited; IDs are used in `platforms_idmap.csv`                                                                                                           |
| `tgdb_publishers.json`              | 2.5.3 (@muldjord)       | API mapping of 'Publishers'; Uses leaner format as before (3.9.0); not to be edited                                                                                                                   |
