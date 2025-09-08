## The official Skyscraper Frequently Asked Questions

A quick note before I let you loose on the questions:

-   You can get a list of all available command-line options by running `Skyscraper --help`
-   For a more thorough description of the command-line options see the [command line options documentation](CLIHELP.md)
-   You can read all about the available configuration settings in the [`config.ini` documentation](CONFIGINI.md)
-   If you are using Skyscraper with RetroPie search their [forum](https://retropie.org.uk/forum/) or raise a question there if you are puzzled
-   If still can not wrap your head around something, please file an issue. 

And now for the questions (with answers)...

??? Question "I ran Skyscraper and it scraped a lot of artwork and game data, but no data is showing up in my gamelists. What's going on?"

    You probably forgot to generate the gamelist. Skyscraper has two modes; resource gathering mode and gamelist generation mode. First you gather data into Skyscraper's resource cache by scraping the platform with any of the supported scraping modules (eg. `Skyscraper -p snes -s thegamesdb`). When you feel like you have gathered all the resources that you need, you then generate the gamelist by simply leaving out the `-s <MODULE>` option (eg. `Skyscraper -p snes`).
    Read more about the [resource cache](CACHE.md).

??? Question "Skyscraper keeps quitting on me with errors about request limits or similar. What's going on?"

    Almost all of the scraping modules, such as ScreenScraper and TheGamesDb, have restrictions applied to avoid users overloading their servers. These restrictions are set by them, not Skyscraper. Some modules even require you to log in to use them. Read more about the restrictions and how to deal with them in the [scraping modules](SCRAPINGMODULES.md) documentation. If you've created a user at ScreenScraper and want to use your credentials with Skyscraper, just add the following section to `/home/<USER>/.skyscraper/config.ini`. Type it _exactly_ like this, but with your own user and pass.
    ```
    [screenscraper]
    userCreds="USER:PASS"
    ```

??? Question "Skyscraper by default generates a composited screenshot that combines both boxart, screenshot and wheel. I just want it to export the raw artwork, how do I do that?"

    Skyscraper includes a powerful artwork compositor you can set up any way you like. Read more about the [artwork features](ARTWORK.md). If you just want it to export the raw unmodified screenshot and other pieces of artwork, you can use the `/home/<USER>/.skyscraper/artwork.xml.example2` and copy it to `/home/<USER>/.skyscraper/artwork.xml`. Remember to regenerate the gamelists after you've done this to make use of the new artwork configuration. This can be done simply by running Skyscraper with `Skyscraper -p PLATFORM`. This will regenerate the gamelist for that platform using this new artwork configuration.

??? Question "I've edited the artwork.xml or some other part of Skyscraper's gamelist specific configurations, but the games still show up the same inside my frontend. Why is that?"

    Whenever you make any gamelist specific changes to Skyscraper's configurations, you need to regenerate the gamelists for your chosen frontend afterwards for the changes to take effect. This can be done simply by running Skyscraper with `Skyscraper -p PLATFORM`. This will regenerate the gamelist for that platform for the default EmulationStation frontend using your new configuration.
    If you wish to export for one of the other supported frontends, you need to specify this. Read more about the [frontends and their specifics](FRONTENDS.md).

??? Question "I used Skyscraper to generate EmulationStation gamelists for one or more platforms, but when I restarted the system the gamelist hadn't changed. Why is that?"

    Did you remember to quit EmulationStation before running Skyscraper? EmulationStation writes its current gamelists back to disk when it is closed. If you run Skyscraper while EmulationStation is also running, it will overwrite any gamelists you just generated. You need to quit EmulationStation before generating the gamelists with Skyscraper to avoid this. You can quit EmulationStation by pressing F4 on a connected keyboard, or by selecting quit from the menus.

??? Question "I want to set up different options for different platforms. It's really tiresome setting it all on command-line. Can this be done easier?"

    YES! Absolutely. In addition to taking options from the command-line, Skyscraper also reads the `/home/<USER>/.skyscraper/config.ini` file, which is where you should set everything up. This file allows you to configure things both globally, per-platform, per-frontend and per-scraping module. Read more about the available [configuration options](CONFIGINI.md) and option priorities.

??? Question "When I try to scrape data or generate gamelists I get all sorts of write permission errors and missing configurations. What's going on?"

    You are probably running, or at some point ran, Skyscraper with `sudo Skyscraper` instead of just `Skyscraper`. Doing so will result in all sorts of weird behaviour as Skyscraper was then run as the `root` user instead of the ordinary user. Depending on what command-line parameters you ran Skyscraper with, this will have caused permission issues for your files in `/home/<USER>/RetroPie/roms` subfolders and might also have caused permission issues with the `/home/<USER>/.skyscraper` folder.  
    This is a common problem for new Linux users who are used to working with Windows. In Linux you rarely need Administrator privileges. Running a command with `sudo` in front of it will run it as the root / Administrator user. And any folders or files being generated while that command runs, will then be owned by root. When you run software as the normal user afterwards and it tries to write to those folders or files, it will fail.  
    You need to reset those permissions back to be owned by your ordinary user. If you are running RetroPie, the following commands might fix your problem (RUN AT YOUR OWN RISK!):
    ```bash
    $ sudo chown -R pi:pi /home/pi/.skyscraper
    $ sudo chown -R pi:pi /home/pi/RetroPie/roms
    ```
    The above commands _must_ be run with `sudo`, as we need root permission to reset the folders and files (hence the warning). But in general, never run anything with `sudo` unless you are specifically told to do so.

??? Question "I generated a game list for the Pegasus frontend. When I fire up Pegasus the data is there, but it doesn't look very good. Why is that?"

    Skyscraper contains a powerful artwork compositor that can customize the exported screenshot, boxart etc. any way you like. This is done through the `/home/<USER>/.skyscraper/artwork.xml` file. The default configuration for this file is made to look good when exporting for the default frontend EmulationStation. When you use a different frontend, you need to customize this file a bit to make it look good. For Pegasus I suggest something like this:
    ```xml
    <?xml version="1.0" encoding="UTF-8"?>
    <artwork>
      <output type="screenshot" width="640"/>
      <output type="cover" width="640" height="480">
        <layer resource="cover" height="480" align="center" valign="middle">
          <gamebox side="wheel" rotate="90"/>
        </layer>
      </output>
      <output type="wheel" height="200"/>
    </artwork>
    ```
    Read more about how to set up a [custom `artwork.xml` file](ARTWORK.md).

??? Question "I've changed my `artwork.xml` configuration and regenerated the game list, but it still looks the same in the frontend. Why is that?"

    Did you perhaps edit `/home/<USER>/skysource/artwork.xml` instead of `/home/<USER>/.skyscraper/artwork.xml`? The latter is the one you need to edit.  
    Read more about how to set up a [custom `artwork.xml` file](ARTWORK.md).

??? Question "Skyscraper keeps overwriting my edits in a `gamelist.xml`. Is there a way to keep my changes in the gamelist file?"

    This is the supposed modus operandi of Skyscraper. A gamelist file should not be edited manually when using Skyscraper, except for the preserved elements which vary from frontend to frontend, review the [frontend documentation](FRONTENDS.md) for details.  
    Skyscraper offers these options: To make your changes permanent between each Skyscraper run use either [`--cache edit`](CLIHELP.md#-cache-editnewtype) and follow the dialogues or use the [import scraping module](IMPORT.md) if your desired changes are more complex.

??? Question "I have a custom game file extension and when using screenscraper as scraping module the games for a system/platform are not found. Is there a solution to this?"

    Yes, from Skyscraper 3.17.5 onwards. The cause is that screenscraper is the only scraping module which by default passes the extension to the server during the scraping information along with other information (checksums, filesize).  
    When scraping a set of games use the [`--addext`](CLIHELP.md#-addext-extension) option, i.e. `--addext '*.<customext>'`, that way Skyscraper will explicitly query only for the basename (=filename without file extension) of the game file. Let's assume you are using `*.desktop` as custom game file extension, then the command would be `Skyscraper -s screenscraper -p <platform> --addext '*.desktop'`. You may also use the config file [counterpart](CONFIGINI.md#addextensions). You may add `--verbosity 3` to investigate what search term is presented to the screenscraper server.  
    However, when querying for a single game with [`--query`](CLIHELP.md#-query-string) you don't have to specify your custom extension.
