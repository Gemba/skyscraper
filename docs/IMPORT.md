## Custom Data Import

The following describes how to import your own custom textual, artwork and / or video data into the resource cache for later use when generating game lists (enable game list generation mode simply by leaving out the `-s` command line option).

!!! tip 
  
    For any path in the following description, you can also add a platform subfolder. Skyscraper will auto-detect this and use it instead of the base import folder. For instance, you can create the folder `/home/<USER>/.skyscraper/import/amiga` and it will use that as base instead of `/home/<USER>/.skyscraper/import/` when importing for the `amiga` platform.

!!! info

    Be sure to also check the `--cache edit` option [here](CLIHELP.md#--cache-editnewtype).

### Images, Videos and Game Manuals

To import videos or images into the resource cache, use the following procedure:

-   Name your image or video file with the _exact_ base name of the rom you wish to connect it to. Example: `Bubble Bobble.nes` will import images with a filename of `Bubble Bobble.jpg` or `Bubble Bobble.png` or other well-known image formats. As long as the base name is an _exact_ match. Same goes for video files. I recommend only making use of well-known video formats since Skyscraper imports them directly without conversion (unless you convert them as described [here](CONFIGINI.md#videoconvertcommand)), so they need to be supported directly by the frontend you plan to use.
-   Game manuals are expected to use PDF format and have the extension `.pdf`. The base name must match the ROM file, thus the game manual of the example is `Bubble Bobble.pdf`.
-   Place all of your images, videos or game manuals in the `/home/<USER>/.skyscraper/import/<PLATFORM>/screenshots`, `covers`, `wheels`, `marquees`, `videos` or `manuals` folders.
-   Now run Skyscraper with `Skyscraper -p <PLATFORM> -s import`. If you named your files correctly, they will now be imported. Look for the green 'YES' in the output at the rom(s) you've placed files for. This will tell you if it succeeded or not.
-   The data is now imported into the resource cache. To make use of if read [here](#how-to-actually-use-the-data).

#### Special Note for Videos

If you are importing videos, you also need to add the command line flag `--flags videos` for this to work. Videos aren't imported or scraped by default, since it is considered a huge disk space hog. So keep this in mind if you want to import videos into the cache. If you plan on always using videos, consider adding this option to the [config.ini](CONFIGINI.md) instead.

### Textual Data (Publisher, Players, Rating, ...)

Skyscraper also allows you to import textual data for any rom you have in your collection. All you need to do is to prepare files for each rom with an _exactly_ matching base name. For instance `Bubble Bobble.nes` should have a file called `Bubble Bobble.txt` or `Bubble Bobble.xml` or whatever suffix you want to use. The suffix is not important. What _is_ important is that you place all of these raw data files into the `/home/<USER>/.skyscraper/import/textual` folder. And then you need to make a definitions file so Skyscraper has a recipe for these files.

#### Textual data definitions file

In order for Skyscraper to understand your textual data files, it needs a recipe. Or a definition of your format if you like. The format is completely up to you. The file must be placed at `/home/<USER>/.skyscraper/import/definitions.dat`. Here follows a few examples with a matching data file for comparison:

**Example 1**

Definitions file: `/home/<USER>/.skyscraper/import/definitions.dat`

```xml
<game>
  <title>###TITLE###</title>
  <description>###DESCRIPTION###</description>
  <developer>###DEVELOPER###</developer>
  <publisher>###PUBLISHER###</publisher>
  <players>###PLAYERS###</players>
  <rating>###RATING###</rating>
  <ages>###AGES###</ages>
  <genre>###TAGS###</genre>
  <releasedate>###RELEASEDATE###</releasedate>
</game>
```

Import file: `/home/<USER>/.skyscraper/import/textual/<EXACT ROM BASE NAME>.xml`

```xml
<game>
  <title>The Game Title</title>
  <description>This game is about yada, yada yada.
Yes, the closing element may be on a different line.   
  </description>
  <developer>The game developer</developer>
  <publisher>The game publisher</publisher>
  <players>Maximum number of players supported (example '4')</players>
  <rating>The game star rating 0-5 (example '3.5') or use the percent range 0.1 ... 1.0</rating>
  <ages>Minimum age restriction as integer (example '16')</ages>
  <genre>Genres, Comma-separated</genre>
  <releasedate>The game release date (example '1985-06-01')</releasedate>
</game>

```

!!! tip

    As with Skyscraper 3.9.2 onwards you are no longer required to have the same indention or whitespace (hard tabs, spaces) as in the XML-like `definitions.dat`. Matching is solely done via XML elements (e.g. `<title>`), which saves you headache in identifiying whitespace mismatch between the XML template and your XML import file. Any heading or trailing whitespace of the element value will be chopped off. If you use a custom XML-like `definitions.dat`: Use it as in the example definition shown above. In your import file assure to use also closing elements (`</title>`) and the same caSiNg in the elements between template and import file, i.e. provide a valid XML file.

**Example 2**

Definitions file: `/home/<USER>/.skyscraper/import/definitions.dat`

```
Title      : ###TITLE###
Description: ###DESCRIPTION###
Developer  : ###DEVELOPER###
Publisher  : ###PUBLISHER###
Players    : ###PLAYERS###
Rating     : ###RATING###
Rating     : ###AGES###
Genre      : ###TAGS###
Date       : ###RELEASEDATE###
```

Import file: `/home/<USER>/.skyscraper/import/textual/<EXACT ROM BASE NAME>.txt`

```
Title      : Game Title
Description: This game is about yada, yada yada.
Developer  : The game developer
Publisher  : The game publisher
Players    : Maximum number of players supported (example '4')
Rating     : The game rating 0-5 (example '3.5')
Ages       : Minimum age restriction as integer (example '16')
Genre      : Genres, Comma-separated
Date       : The game release date (example '1985-06-01')
```

!!! warning 

    Make sure any line matches _exactly_ with the line in the recipe **including white-space characters such as newline characters, spaces and tabs!** Otherwise Skyscraper won't recognize it. Especially for newline characters this can be difficult to debug. Newline characters appear the same in an editor so you won't notice the problem until you load it up in a hex editor.

#### List of known tags

From the examples above you'll notice the `###SOMETHING###` tags. This is what Skyscraper recognizes your data from. The supported tags are:

-   `###TITLE###`
-   `###DESCRIPTION###`
-   `###DEVELOPER###`
-   `###PUBLISHER###`
-   `###PLAYERS###`
-   `###AGES###`
-   `###RATING###`
-   `###TAGS###`
-   `###RELEASEDATE###`

#### Resource formats

For `###PLAYERS###`, `###AGES###`, `###RELEASEDATE###` and `###RATING###` a certain format is required.

-   Players must contain just the maximum number of players as an integer such as '4'.
-   Ages must be an integer between 1 and 18 (for instance "16" means it is suitable from ages 16 and up).
-   Release date must be of one of the following formats:

    -   `yyyy`
    -   `yyyy-MM`
    -   `yyyy-MM-dd` (full ISO 8601 date)
    -   `yyyy-MMM-dd` (MMM is Jan, Feb and so on...)
    -   `MM/dd/yyyy`
    -   `MMM, yyyy` (MMM is Jan, Feb and so on...)
    -   `MMM dd, yyyy` (MMM is Jan, Feb and so on...)

-   Rating can be either a number between 0 and 5, use as fraction only .5
    ("Star rating scale"). Examples: 3 or 4.5, but not 4.25 or 1.0. The value
    will be divided by five to match the scale of the rating in a gamelist.
    Alternatively, a value between 0.0 and 1.0 ("Percent scale") can be used.
    This value will not be transformed. If you want to use 0.5 with the percent
    scale, provide it as .5 (note the missing zero) or as 0.50 (add redundant
    zeros). Also, do add a fraction of zero explicitly when you want to denote a
    rating of 1.0 (100%). Without this trailing fraction it will be interpreted
    as if it would be of the star rating scale (0 to 5), thus it will be rated
    20% for given integer of 1.

### How to actually use the data?

When you've imported all of your data into the resource cache, you can make use of it by enabling Skyscrapers _game list generation_ mode simply by leaving out the `-s` command line option (eg. `Skyscraper -p <PLATFORM>`). The game list generator will then make use of your imported data. If you don't know what the resource cache is, read more about it [here](CACHE.md).
