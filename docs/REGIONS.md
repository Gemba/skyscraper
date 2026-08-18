## Overview

Some game information and / or game media is region-based (e.g., release date, artwork). Skyscraper provides several ways of configuring these for your convenience. But most importantly; it supports region auto-detection directly from the file names. Read on for more information about how regions are handled by Skyscraper.

## Scraping modules that support regions

-   Screenscraper (pretty much all game information and media)
-   IGDB (only release date)
-   MobyGames (only cover artwork)

Below follows a general list of supported regions. Not all regions are supported by all modules: The supporting modules are checkmarked for each region.

When configuring regions be sure to use the short-names as shown (eg. 'fr' for France).

## List of supported regions

| Region Key | Country/Region     | ScreenScraper | IGDB | MobyGames |
| :--------: | ------------------ | :-----------: | :--: | :-------: |
|    ame     | American continent |       ✓       |      |           |
|    asi     | Asia               |       ✓       |  ✓   |           |
|     au     | Australia          |       ✓       |  ✓   |     ✓     |
|     bg     | Bulgaria           |       ✓       |      |     ✓     |
|     br     | Brazil             |       ✓       |      |     ✓     |
|     ca     | Canada             |       ✓       |      |     ✓     |
|     cl     | Chile              |       ✓       |      |     ✓     |
|     cn     | China              |       ✓       |  ✓   |     ✓     |
|    cus     | Custom             |       ✓       |      |           |
|     cz     | Czech republic     |       ✓       |      |     ✓     |
|     de     | Germany            |       ✓       |      |     ✓     |
|     dk     | Denmark            |       ✓       |      |     ✓     |
|     eu     | Europe             |       ✓       |  ✓   |           |
|     fi     | Finland            |       ✓       |      |     ✓     |
|     fr     | France             |       ✓       |      |     ✓     |
|     gr     | Greece             |       ✓       |      |     ✓     |
|     hu     | Hungary            |       ✓       |      |     ✓     |
|     il     | Israel             |       ✓       |      |     ✓     |
|     it     | Italy              |       ✓       |      |     ✓     |
|     jp     | Japan              |       ✓       |  ✓   |     ✓     |
|     kr     | Korea              |       ✓       |      |     ✓     |
|     kw     | Kuwait             |       ✓       |      |           |
|    mor     | Middle East        |       ✓       |      |           |
|     nl     | Netherlands        |       ✓       |      |     ✓     |
|     no     | Norway             |       ✓       |      |     ✓     |
|     nz     | New Zealand        |       ✓       |  ✓   |     ✓     |
|    oce     | Oceania            |       ✓       |      |           |
|     pe     | Peru               |       ✓       |      |           |
|     pl     | Poland             |       ✓       |      |     ✓     |
|     pt     | Portugal           |       ✓       |      |     ✓     |
|     ru     | Russia             |       ✓       |      |     ✓     |
|     se     | Sweden             |       ✓       |      |     ✓     |
|     sk     | Slovakia           |       ✓       |      |     ✓     |
|     sp     | Spain              |       ✓       |      |     ✓     |
|     ss     | ScreenScraper      |       ✓       |      |           |
|     tr     | Turkey             |       ✓       |      |     ✓     |
|     tw     | Taiwan             |       ✓       |      |     ✓     |
|     uk     | United Kingdom     |       ✓       |      |     ✓     |
|     us     | USA                |       ✓       |  ✓   |     ✓     |
|    wor     | World              |       ✓       |  ✓   |     ✓     |


### Region auto-detection

Skyscraper will try to auto-detect the region from the file name. It will look for designations in parenthesis such as `(Europe)` or `(e)` or combinations like `(USA, Japan)` and set the region priorities accordingly. This currently works for the following regions and / or countries:

-   asi, au, br, ca, cn
-   de, dk, eu, fr, it
-   jp, kr, nl, se, sp
-   tw, us, wor

So if your files are named like `Game Name (Europe).zip`, there's no need to configure regions manually. Skyscraper will recognize `Europe` and verfifies if it is on the region prios list, unless you disabled the region from filename detection (see configuration option [regionFromFilename](CONFIGINI.md#regionfromfilename)). The default behaviour (`"inline"`) is:  
- If a detected region is in the region prios list, then its position in the configured region prios is 'snapped' to the front for finding a scraping match for the game, preserving the order that these regions were already in the region prios list.
- If it is not in the region prios list, then the detected region from the filename is added after the regions detetcted and already in region prios list, but before the remaining regions in the region prios (non-detected regions).
- If you set `regionFromFilename` to `"first"`, then every detected region is prepended to the region list in the order they appear in the filename.
Skyscraper will process the region prios list from begin to end and checks the region on the list until it finds one that has data for the requested resource. Do not configure the region prios too narrow, as you might not find a match for every game in your collection then, always put some fail-safes at the end of the list.

### Default Region Prioritization

Skyscraper's default internal region priority list is as follows. Topmost region has highest priority and so forth.

1.   User-specified region set with `--region <REGION>` (command line) or `region="<REGION>"` (config.ini). The `regionPrios=` setting is not applied in this case.
2.   If no user-specified region is set, the [auto-detected](REGIONS.md#region-auto-detection) region(s) will be added to the beginning of the region prios in the order they appear in the filename ("`first`") or in the order they appear in the region prios list already ("`inline`"). You can also disable region detection from filename at all (`"off"`). See configuration option [regionFromFilename](CONFIGINI.md#regionfromfilename) for examples.
3.   If no changes are made by the region prio switches, then this is the default processing order: eu, us, ss (Screenscraper specific), uk, wor, jp, au, ame, de, cus (Screenscraper specific), cn, kr, asi, br, sp, fr, gr, it, no, dk, nz, nl, pl, ru, se, tw and ca. If you have configured a region prios list, the list will be processed from left to right.

## Configuring Region Manually

If you insist, of course you can configure the region manually. You can either do this on command-line or through `/home/<USER>/.skyscraper/config.ini`. It is recommended to set it in `config.ini` for a permanent setup.

### config.ini

Read about the [`region`](CONFIGINI.md#region) and [`regionprios`](CONFIGINI.md#regionprios) setting.

### Command line

Read about the [`--region`](CLIHELP.md#-region-code) option.
