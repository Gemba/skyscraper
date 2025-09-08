## Overview

Some game information comes in several languages. Skyscraper provides several ways of configuring these for your convenience. It is important to understand the distinction between game region and language. Setting a language is a user-preferred thing and will only affect the game descriptions and tags (genres). The remaining game data is tied to the region instead (artwork (screenshots, marquees, ...) and, in some cases, the game name).

### Scraping Modules that Support Languages

-   screenscraper (game description and tags / genres)

Below follows a general list of supported languages. Not all languages are supported by all modules (the supporting module is noted inside the parenthesis for each language).

When configuring languages be sure to use the short-names as shown (eg. 'no' for Norwegian).

## List of Supported Languages

-   cz: Czech (screenscraper)
-   da: Danish (screenscraper)
-   de: German (screenscraper)
-   en: English (screenscraper)
-   es: Spanish (screenscraper)
-   fi: Finnish (screenscraper)
-   fr: French (screenscraper)
-   hu: Hungarian (screenscraper)
-   it: Italian (screenscraper)
-   ja: Japanese (screenscraper)
-   ko: Korean (screenscraper)
-   nl: Dutch (screenscraper)
-   no: Norwegian (screenscraper)
-   pl: Polish (screenscraper)
-   pt: Portuguese (screenscraper)
-   ru: Russian (screenscraper)
-   sk: Slovakian (screenscraper)
-   sv: Swedish (screenscraper)
-   tr: Turkish (screenscraper)
-   zh: Chinese (screenscraper)

### Default Language Precedence

Skyscraper's default internal language priority list is as follows. Topmost language has highest priority and so forth.

-   Auto-detected language or language set with `--lang LANGUAGE` or `lang="LANGUAGE"`
-   en
-   de
-   fr
-   es

## Configuring Language

It is possible to manipulate the default internal language priority list. You can either do this on command-line or through `/home/<USER>/.skyscraper/config.ini`. It is recommended to set it in `config.ini` for a permanent setup.

### Permanent setting in config.ini

Read [this](CONFIGINI.md#lang) and [this](CONFIGINI.md#langprios).

### Command Line

Read [this](CLIHELP.md#-lang-code).
