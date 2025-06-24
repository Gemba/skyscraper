## Path Logic in Skyscraper

This page describes how Skyscraper processes the different paths and especially
how the absolute path is calculated when a you provide a relative path.

Do not get confused by the lenghty flow diagram below. It covers game list folder,
input folder and media folder handling. You wiil notice that input folder and
media folder are processed in the same manner.

<figure markdown style="width:100%">
  <img style="width:100%" src="../resources/path_handling_flow.svg"/>
  <figcaption>Path processing flowchart</figcaption>
</figure>

Ah, good you made it to the end of the flow.

Pay attention to the connectors _A-A_, _B-B_ and _C-C_ in the diagram.

_Normalization_ means any surplus `./` and `../` are removed from the path.
However, at this stage it is not verified if the filepath does exists. Also, any
symbolic links are not resolved.

Remember the precedence of [CLI and configuration
options](CONFIGINI.md#configini-options) when you read through this document.

Now, let's see how Skyscraper handles relative path configuration options.

### Computing the Absolute Path from ...

a relative path provided. The next subsections are summarizing the absolute path
calculation of the different path and file options.

#### ... Current Working Directory

The current working directory (CWD) is the directory from where you run
Skyscraper. The absolute path is computed as `<CWD>/<parameter-value>` for these
CLI parameters and their values:

`-a <artwork.xml>`  
`-c <configfile>`  
`-g <gamelistfolder>`  
`-d <cachefolder>`

This means the first part of the flow diagram (resolution of Gamelist folder)
also applies to `-a` and `-d` options and their configfile counterparts.

In contrast, when you use one of the three parameters
(`<artwork.xml>`,`<gamelistfolder>` and `<cachefolder>`) in a configuration
INI-file the absolute path it determined from the absolute path of the config
file. See [below](#an-option-in-a-configuration-ini-file).

#### ... Skyscraper Built-in Config Directory

This is usually `/home/<USER>/.skyscraper/` (=_base_). With [XDG](XDG.md) it is
slightly different. The files provided with these options

`--excludeFrom <excludes.txt>`  
`--includeFrom <includes.txt>`

are searched by concatenating the _base_ and for example the exclude file. If
not found, Skyscraper tries to access it with the current working directory as
_base_. If it is not found in any of these locations Skyscraper end with an
error message.

#### ... the Gamelist Folder

If you define a Gamelist folder either via `-g` or via `gameListFolder=`
(INI-file) and are using a frontend for EmulationStation (or any other frontend,
which is not Pegasus) then the input folder must provided absolute and can not
be relative. The media folder, if relative, is then assumed to be relative to
the input folder.

However, if you selected the Pegasus frontend then the input folder may be
relative. The input folder and media folder, when relative, are then interpreted
by Skyscraper to be relative to the game list folder.

This is also depicted in the diagram above.

#### ... Input Folder (ROM-/gamefile-path)

The files provided with these options

`--startAt <ROM-or-game-file-A>`  
`--endAt <ROM-or-game-file-B>`

are first searched in the current working directory. If not found, Skyscraper
tries to access them in the input folder. If they are not found at all
Skyscraper silently assumes that `--startAt` respective `--endAt` are not set.

#### ... an Option in a Configuration INI File

When you have set one of the four following parameters in the default
configuration file (`/home/<USER>/.skyscraper/config.ini`) or in a custom config
file defined with `-c <configfile>` the path is calculated from the absolute
path of the location of the config INI-file.

```ini
artworkXml=<artwork.xml>
gameListFolder=<gamelistfolder>
cacheFolder=cachefolder>
importFolder=<importfolder>
```

Remember, that the path calculation for the paramters `artworkXml`,
`gameListFolder` and `cacheFolder` differs when using their CLI
counterparts, see [CLI options](#current-working-directory) above.

!!! note

    The `importFolder=` parameter has no counterpart on the command line.