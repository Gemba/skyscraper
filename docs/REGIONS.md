## Overview

Some game information and / or game media is region-based. Skyscraper provides several ways of configuring these for your convenience. But most importantly; it supports region auto-detection directly from the file names. Read on for more information about how regions are handled by Skyscraper.

## Scraping modules that support regions

-   screenscraper (pretty much all game information and media)
-   igdb (only release date)
-   mobygames (only cover artwork)

Below follows a general list of supported regions. Please note that not all regions are supported by all modules (support is noted inside the parenthesis for each country or region).

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

Skyscraper will try to auto-detect the region from the file name. It will look for designations such as `(Europe)` or `(e)` and set the region accordingly. This currently works for the following regions and / or countries:

-   asi, au, br, ca, cn
-   de, dk, eu, fr, it
-   jp, kr, nl, se, sp
-   tw, us, wor

So if your files are named like `Game Name (Europe).zip`, there's no need to configure regions manually. Skyscraper will recognize `Europe` and add it to the top of the internal region priority list. If info or media isn't found for the auto-detected region, it will move down the list and check the next region on the list until it finds one that has data for the requested resource.

### Default Region Prioritization

Skyscraper's default internal region priority list is as follows. Topmost region has highest priority and so forth.

-   User-specified region set with `--region <REGION>` (command line) or `region="<REGION>"` (config.ini)
-   If no user-specified region is set, the [auto-detected](REGIONS.md#region-auto-detection) region will be added here
-   Then this list is processed in order: eu, us, ss (Screenscraper specific), uk, wor, jp, au, ame, de, cus, cn, kr, asi, br, sp, fr, gr, it, no, dk, nz, nl, pl, ru, se, tw, ca

## Configuring Region Manually

If you insist, of course you can configure the region manually. You can either do this on command-line or through `/home/<USER>/.skyscraper/config.ini`. It is recommended to set it in `config.ini` for a permanent setup.

### config.ini

Read about the [`region`](CONFIGINI.md#region) and [`regionprios`](CONFIGINI.md#regionprios) setting.

### Command line

Read about the [`--region`](CLIHELP.md#-region-code) option.
