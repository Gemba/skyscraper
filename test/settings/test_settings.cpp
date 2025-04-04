#include "test_settings.h"

#include "src/settings.h"
#include "src/cli.h"
#include "src/config.h"
#include "src/platform.h"

#include <QCoreApplication>
#include <QDebug>

void TestSettings::testConfigIniPaths() {
    QString currentDir = QDir::currentPath();
    Settings config;
    config.currentDir = currentDir;

    if (!Platform::get().loadConfig()) {
        qWarning() << "*** AIEEE !!!\n";
        exit(1);
    }

    QSettings settings("config_paths.ini", QSettings::IniFormat);
    config.configFile = config.currentDir + "/config_paths.ini";

    Config::initSkyFolders();
    QString platform = "amiga";
    QCommandLineParser *parser = new QCommandLineParser();
    Cli::createParser(parser, platform);

    QStringList params = {"x", "-p", platform};

    parser->parse(params);
    RuntimeCfg *rtConf = new RuntimeCfg(&config, parser);

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    settings.beginGroup("main");
    rtConf->applyConfigIni(RuntimeCfg::CfgType::MAIN, &settings, inputFolderSet,
                           gameListFolderSet, mediaFolderSet);
    settings.endGroup();
    // qDebug() << "Platform: " + config.platform;

    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);

    // test defaults for ES
    QString exp;
    exp = Config::makeAbsolutePath(
        Config::getSkyFolder(Config::SkyFolderType::CACHE), platform);
    QCOMPARE(config.cacheFolder, exp);

    exp = Config::makeAbsolutePath(Config::getSkyFolder(), "artwork.xml");
    QCOMPARE(config.artworkConfig, exp);

    // platform subfolder will be used when present in import/ but not
    // explicitly added like with gameListFolder
    exp = Config::getSkyFolder(Config::SkyFolderType::IMPORT);
    QStringList p = exp.split("/");
    p[2] = "pi";
    exp = p.join("/");
    QCOMPARE(config.importFolder, exp);

    exp = Config::makeAbsolutePath("/home/pi/RetroPie/roms", platform);
    QCOMPARE(config.gameListFolder, exp);

    exp = config.gameListFolder;
    QCOMPARE(config.inputFolder, exp);

    exp = config.gameListFolder;
    QCOMPARE(config.mediaFolder, exp);

    // test relative and absolute paths on CLI
    params.append("-a");
    params.append("../rel/artworks/artwork.xml");
    parser->parse(params);
    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);
    exp = Config::makeAbsolutePath(config.currentDir, params.last());
    QCOMPARE(config.artworkConfig, exp);

    params.removeLast();
    params.append("/abs/artworks/artwork.xml");
    parser->parse(params);
    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);
    exp = params.last();
    QCOMPARE(config.artworkConfig, exp);

    params.append("-g");
    params.append("./rel/gamelists/");
    parser->parse(params);
    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);
    exp = Config::makeAbsolutePath(config.currentDir, params.last());
    QCOMPARE(config.gameListFolder, exp);

    params.removeLast();
    params.append("/abs/gamelists/");
    parser->parse(params);
    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);
    exp = params.last();
    QCOMPARE(config.gameListFolder, exp);

    // test relative paths in config
    // attn: configDir and CWD are the same in this test
    settings.beginGroup(platform);
    rtConf->applyConfigIni(RuntimeCfg::CfgType::PLATFORM, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    settings.endGroup();

    exp = Config::makeAbsolutePath(config.currentDir, "cfgini/rel/gamelist");
    QCOMPARE(config.gameListFolder, exp);

    // Config::makeAbsolutePath is called in skyscraper.cpp:loadConfig()
    exp =
        Config::makeAbsolutePath(config.gameListFolder, "../../rel/inout-roms");
    QCOMPARE(config.gameListFolder % "/" % config.inputFolder, exp);
    exp = Config::makeAbsolutePath(config.gameListFolder, "./rel/media");
    QCOMPARE(config.gameListFolder % config.mediaFolder.replace(".", ""), exp);
}

void TestSettings::testConfigIniMain() {
    QString currentDir = QDir::currentPath();
    Settings config;
    if (!Platform::get().loadConfig()) {
        qWarning() << "*** AIEEE !!!\n";
        exit(1);
    }
    QCommandLineParser *parser = new QCommandLineParser();
    Cli::createParser(parser, "amiga");

    const QStringList params = {"x", "-p"
                                     "amiga"};

    parser->parse(params);
    RuntimeCfg *rtConf = new RuntimeCfg(&config, parser);
    QSettings settings("config_test.ini", QSettings::IniFormat);

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    settings.beginGroup("main");
    // legacy
    /*

    rtConf->setMainConfig(&settings, inputFolderSet, gameListFolderSet,
                          mediaFolderSet);
    */
    // refactored
    rtConf->applyConfigIni(RuntimeCfg::CfgType::MAIN, &settings, inputFolderSet,
                           gameListFolderSet, mediaFolderSet);
    QVariant exp;
    exp = settings.value("frontend");
    QCOMPARE(config.frontend, exp);
    exp = settings.value("addExtensions");
    QCOMPARE(config.addExtensions, exp);
    exp = settings.value("artworkXml");
    QCOMPARE(config.artworkConfig, exp);
    exp = settings.value("brackets");
    QCOMPARE(config.brackets, exp);
    exp = settings.value("cacheCovers");
    QCOMPARE(config.cacheCovers, exp);
    exp = settings.value("cacheFolder").toString() + "amiga";
    QCOMPARE(config.cacheFolder, exp);
    exp = settings.value("cacheMarquees");
    QCOMPARE(config.cacheMarquees, exp);
    exp = settings.value("cacheRefresh");
    QCOMPARE(config.refresh, exp);
    exp = settings.value("cacheResize");
    QCOMPARE(config.cacheResize, exp);
    exp = settings.value("cacheScreenshots");
    QCOMPARE(config.cacheScreenshots, exp);
    exp = settings.value("cacheTextures");
    QCOMPARE(config.cacheTextures, exp);
    exp = settings.value("cacheWheels");
    QCOMPARE(config.cacheWheels, exp);
    exp = settings.value("cropBlack");
    QCOMPARE(config.cropBlack, exp);
    if (config.frontend == "attractmode") {
        exp = settings.value("emulator");
        QCOMPARE(config.frontendExtra, exp);
    } else if (config.frontend == "pegasus") {
        exp = settings.value("launch");
        QCOMPARE(config.frontendExtra, exp);
    } else {
        QCOMPARE(config.frontendExtra, "");
    }
    // exp = settings.value("excludeFiles");
    // QCOMPARE(config.excludePattern, exp);
    exp = settings.value("excludeFrom");
    QCOMPARE(config.excludeFrom, exp);
    exp = settings.value("excludePattern");
    QCOMPARE(config.excludePattern, exp);
    exp = settings.value("forceFilename");
    QCOMPARE(config.forceFilename, exp);
    exp = settings.value("gameListBackup");
    QCOMPARE(config.gameListBackup, exp);
    // exp = settings.value("gamelistFolder");
    // QCOMPARE(config.gameListFolder, exp);
    exp = settings.value("gameListFolder");
    QCOMPARE(config.gameListFolder,
             Config::lexicallyNormalPath(exp.toString() + "amiga"));
    QCOMPARE(gameListFolderSet, true);
    exp = settings.value("hints");
    QCOMPARE(config.hints, exp);
    exp = settings.value("importFolder");
    QCOMPARE(config.importFolder, exp);
    // exp = settings.value("includeFiles");
    // QCOMPARE(config.includePattern, exp);
    exp = settings.value("includeFrom");
    QCOMPARE(config.includeFrom, exp);
    exp = settings.value("includePattern");
    QCOMPARE(config.includePattern, exp);
    exp = settings.value("inputFolder");
    QCOMPARE(inputFolderSet, true);
    QCOMPARE(config.inputFolder, exp.toString() + "/amiga");
    exp = settings.value("jpgQuality");
    QCOMPARE(config.jpgQuality, exp);
    exp = settings.value("lang");
    QCOMPARE(config.lang, exp);
    exp = settings.value("langPrios");
    QCOMPARE(config.langPriosStr, exp);
    exp = settings.value("manuals");
    QCOMPARE(config.manuals, exp);
    exp = settings.value("maxFails");
    QCOMPARE(config.maxFails, exp);
    exp = settings.value("maxLength");
    QCOMPARE(config.maxLength, exp);
    exp = settings.value("mediaFolder");
    QCOMPARE(config.mediaFolder, exp.toString() + "/amiga");
    QCOMPARE(mediaFolderSet, true);
    exp = settings.value("minMatch");
    QCOMPARE(config.minMatch, exp);
    exp = settings.value("nameTemplate");
    QCOMPARE(config.nameTemplate, exp);
    exp = settings.value("platform");
    QCOMPARE(config.platform, exp);
    exp = settings.value("pretend");
    QCOMPARE(config.pretend, exp);
    exp = settings.value("region");
    QCOMPARE(config.region, exp);
    exp = settings.value("regionPrios");
    QCOMPARE(config.regionPriosStr, exp);
    exp = settings.value("relativePaths");
    QCOMPARE(config.relativePaths, exp);
    // exp = settings.value("scummIni");
    // QCOMPARE(config.scummIni, exp);
    exp = settings.value("skipped");
    QCOMPARE(config.skipped, exp);
    exp = settings.value("spaceCheck");
    QCOMPARE(config.spaceCheck, exp);
    exp = settings.value("subdirs");
    QCOMPARE(config.subdirs, exp);
    exp = settings.value("symlink");
    QCOMPARE(config.symlink, exp);
    exp = settings.value("theInFront");
    QCOMPARE(config.theInFront, exp);
    exp = settings.value("threads");
    QCOMPARE(config.threads, exp);
    exp = settings.value("tidyDesc");
    QCOMPARE(config.tidyDesc, exp);
    exp = settings.value("unattend");
    QCOMPARE(config.unattend, exp);
    exp = settings.value("unattendSkip");
    QCOMPARE(config.unattendSkip, exp);
    exp = settings.value("unpack");
    QCOMPARE(config.unpack, exp);
    exp = settings.value("verbosity");
    QCOMPARE(config.verbosity, exp);
    exp = settings.value("videoConvertCommand");
    QCOMPARE(config.videoConvertCommand, exp);
    exp = settings.value("videoConvertExtension");
    QCOMPARE(config.videoConvertExtension, exp);
    exp = settings.value("videos");
    QCOMPARE(config.videos, exp);
    exp = settings.value("videoSizeLimit");
    QCOMPARE(config.videoSizeLimit, exp.toInt() * 1000 * 1000);
    settings.endGroup();
}

void TestSettings::testConfigIniPlatform() {
    QString currentDir = QDir::currentPath();
    Settings config;
    if (!Platform::get().loadConfig()) {
        qWarning() << "*** AIEEE !!!\n";
        exit(1);
    }
    QCommandLineParser *parser = new QCommandLineParser();
    Cli::createParser(parser, "amiga");

    const QStringList params = {"x", "-p"
                                     "amiga"};

    parser->parse(params);
    RuntimeCfg *rtConf = new RuntimeCfg(&config, parser);
    QSettings settings("config_test.ini", QSettings::IniFormat);

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    settings.beginGroup("main");
    config.frontend = settings.value("frontend").toString();
    settings.endGroup();

    settings.beginGroup("amiga");
    // legacy

    // rtConf->setPlatformConfig(&settings, inputFolderSet, gameListFolderSet,
    //                       mediaFolderSet);

    // refactored
    rtConf->applyConfigIni(RuntimeCfg::CfgType::PLATFORM, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    QVariant exp;

    if (config.frontend == "attractmode") {
        exp = settings.value("emulator");
        QCOMPARE(config.frontendExtra, exp);
    } else if (config.frontend == "pegasus") {
        exp = settings.value("launch");
        QCOMPARE(config.frontendExtra, exp);
    } else {
        QCOMPARE(config.frontendExtra, "");
    }
    exp = settings.value("addExtensions");
    QCOMPARE(config.addExtensions, exp);
    exp = settings.value("artworkXml");
    QCOMPARE(config.artworkConfig, exp);
    exp = settings.value("brackets");
    QCOMPARE(config.brackets, exp);
    exp = settings.value("cacheCovers");
    QCOMPARE(config.cacheCovers, exp);
    exp = settings.value("cacheFolder");
    QCOMPARE(config.cacheFolder, exp);
    exp = settings.value("cacheMarquees");
    QCOMPARE(config.cacheMarquees, exp);
    exp = settings.value("cacheResize");
    QCOMPARE(config.cacheResize, exp);
    exp = settings.value("cacheScreenshots");
    QCOMPARE(config.cacheScreenshots, exp);
    exp = settings.value("cacheTextures");
    QCOMPARE(config.cacheTextures, exp);
    exp = settings.value("cacheWheels");
    QCOMPARE(config.cacheWheels, exp);
    exp = settings.value("cropBlack");
    QCOMPARE(config.cropBlack, exp);
    exp = settings.value("endAt");
    QCOMPARE(config.endAt, exp);
    exp = settings.value("excludeFrom");
    QCOMPARE(config.excludeFrom, exp);
    exp = settings.value("excludePattern");
    QCOMPARE(config.excludePattern, exp);
    exp = settings.value("extensions");
    QCOMPARE(config.extensions, exp);
    exp = settings.value("forceFilename");
    QCOMPARE(config.forceFilename, exp);
    exp = settings.value("gameListFolder");
    QCOMPARE(config.gameListFolder,
             Config::lexicallyNormalPath(exp.toString()));
    exp = settings.value("importFolder");
    QCOMPARE(config.importFolder, exp);
    exp = settings.value("includeFrom");
    QCOMPARE(config.includeFrom, exp);
    exp = settings.value("includePattern");
    QCOMPARE(config.includePattern, exp);
    exp = settings.value("inputFolder");
    QCOMPARE(config.inputFolder, exp);
    exp = settings.value("interactive");
    QCOMPARE(config.interactive, exp);
    exp = settings.value("jpgQuality");
    QCOMPARE(config.jpgQuality, exp);
    exp = settings.value("lang");
    QCOMPARE(config.lang, exp);
    exp = settings.value("langPrios");
    QCOMPARE(config.langPriosStr, exp);
    exp = settings.value("manuals");
    QCOMPARE(config.manuals, exp);
    exp = settings.value("maxLength");
    QCOMPARE(config.maxLength, exp);
    exp = settings.value("mediaFolder");
    QCOMPARE(config.mediaFolder, exp);
    exp = settings.value("minMatch");
    QCOMPARE(config.minMatch, exp);
    exp = settings.value("nameTemplate");
    QCOMPARE(config.nameTemplate, exp);
    exp = settings.value("pretend");
    QCOMPARE(config.pretend, exp);
    exp = settings.value("region");
    QCOMPARE(config.region, exp);
    exp = settings.value("regionPrios");
    QCOMPARE(config.regionPriosStr, exp);
    exp = settings.value("relativePaths");
    QCOMPARE(config.relativePaths, exp);
    exp = settings.value("skipped");
    QCOMPARE(config.skipped, exp);
    exp = settings.value("startAt");
    QCOMPARE(config.startAt, exp);
    exp = settings.value("subdirs");
    QCOMPARE(config.subdirs, exp);
    exp = settings.value("symlink");
    QCOMPARE(config.symlink, exp);
    exp = settings.value("theInFront");
    QCOMPARE(config.theInFront, exp);
    exp = settings.value("threads");
    QCOMPARE(config.threads, exp);
    exp = settings.value("tidyDesc");
    QCOMPARE(config.tidyDesc, exp);
    exp = settings.value("unattend");
    QCOMPARE(config.unattend, exp);
    exp = settings.value("unattendSkip");
    QCOMPARE(config.unattendSkip, exp);
    exp = settings.value("unpack");
    QCOMPARE(config.unpack, exp);
    exp = settings.value("verbosity");
    QCOMPARE(config.verbosity, exp);
    exp = settings.value("videos");
    QCOMPARE(config.videos, exp);
    exp = settings.value("videoSizeLimit");
    QCOMPARE(config.videoSizeLimit, exp.toInt() * 1000 * 1000);

    settings.endGroup();
}

void TestSettings::testConfigIniFrontend() {
    QString currentDir = QDir::currentPath();
    Settings config;
    if (!Platform::get().loadConfig()) {
        qWarning() << "*** AIEEE !!!\n";
        exit(1);
    }
    QCommandLineParser *parser = new QCommandLineParser();
    Cli::createParser(parser, "amiga");

    const QStringList params = {"x", "-p"
                                     "amiga"};

    parser->parse(params);
    RuntimeCfg *rtConf = new RuntimeCfg(&config, parser);
    QSettings settings("config_test.ini", QSettings::IniFormat);

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    settings.beginGroup("main");
    config.frontend = settings.value("frontend").toString();
    settings.endGroup();
    settings.beginGroup("pegasus");
    // legacy

    // rtConf->setPlatformConfig(&settings, inputFolderSet, gameListFolderSet,
    //                       mediaFolderSet);

    // refactored
    rtConf->applyConfigIni(RuntimeCfg::CfgType::FRONTEND, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    QVariant exp;

    qDebug() << "From test, frontend: " << config.frontend;
    if (config.frontend == "attractmode") {
        exp = settings.value("emulator");
        QCOMPARE(config.frontendExtra, exp);
    } else if (config.frontend == "pegasus") {
        exp = settings.value("launch");
        QCOMPARE(config.frontendExtra, exp);
    } else {
        QCOMPARE(config.frontendExtra, "");
    }
    exp = settings.value("artworkXml");
    QCOMPARE(config.artworkConfig, exp);
    exp = settings.value("brackets");
    QCOMPARE(config.brackets, exp);
    exp = settings.value("cropBlack");
    QCOMPARE(config.cropBlack, exp);
    exp = settings.value("endAt");
    QCOMPARE(config.endAt, exp);
    exp = settings.value("excludePattern");
    QCOMPARE(config.excludePattern, exp);
    exp = settings.value("forceFilename");
    QCOMPARE(config.forceFilename, exp);
    exp = settings.value("gameListBackup");
    QCOMPARE(config.gameListBackup, exp);
    exp = settings.value("gameListFolder");
    // #88
    QCOMPARE(config.gameListFolder, exp.toString() + "/amiga");
    exp = settings.value("includePattern");
    QCOMPARE(config.includePattern, exp);
    exp = settings.value("maxLength");
    QCOMPARE(config.maxLength, exp);
    exp = settings.value("mediaFolder");
    // #88
    QCOMPARE(config.mediaFolder, exp.toString() + "/amiga");
    exp = settings.value("mediaFolderHidden");
    QCOMPARE(config.mediaFolderHidden, exp.toBool());
    exp = settings.value("skipped");
    QCOMPARE(config.skipped, exp);
    exp = settings.value("startAt");
    QCOMPARE(config.startAt, exp);
    exp = settings.value("symlink");
    QCOMPARE(config.symlink, exp);
    exp = settings.value("theInFront");
    QCOMPARE(config.theInFront, exp);
    exp = settings.value("unattend");
    QCOMPARE(config.unattend, exp);
    exp = settings.value("unattendSkip");
    QCOMPARE(config.unattendSkip, exp);
    exp = settings.value("verbosity");
    QCOMPARE(config.verbosity, exp);
    exp = settings.value("videos");
    QCOMPARE(config.videos, exp);

    settings.endGroup();
}

void TestSettings::testConfigIniScraper() {
    QString currentDir = QDir::currentPath();
    Settings config;
    if (!Platform::get().loadConfig()) {
        qWarning() << "*** AIEEE !!!\n";
        exit(1);
    }
    QCommandLineParser *parser = new QCommandLineParser();
    Cli::createParser(parser, "amiga");

    const QStringList params = {"x", "-p"
                                     "amiga"};

    parser->parse(params);
    RuntimeCfg *rtConf = new RuntimeCfg(&config, parser);
    QSettings settings("config_test.ini", QSettings::IniFormat);

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    settings.beginGroup("main");
    config.frontend = settings.value("frontend").toString();
    config.scraper = "screenscraper";
    settings.endGroup();
    settings.beginGroup("screenscraper");
    // legacy

    // rtConf->setPlatformConfig(&settings, inputFolderSet, gameListFolderSet,
    //                       mediaFolderSet);

    // refactored
    rtConf->applyConfigIni(RuntimeCfg::CfgType::SCRAPER, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    QVariant exp;

    qDebug() << "From test, frontend: " << config.frontend;
    if (config.frontend == "attractmode") {
        exp = settings.value("emulator");
        QCOMPARE(config.frontendExtra, exp);
    } else if (config.frontend == "pegasus") {
        exp = settings.value("launch");
        QCOMPARE(config.frontendExtra, exp);
    } else {
        QCOMPARE(config.frontendExtra, "");
    }

    exp = settings.value("cacheCovers");
    QCOMPARE(config.cacheCovers, exp);
    exp = settings.value("cacheMarquees");
    QCOMPARE(config.cacheMarquees, exp);
    exp = settings.value("cacheRefresh");
    QCOMPARE(config.refresh, exp);
    exp = settings.value("cacheResize");
    QCOMPARE(config.cacheResize, exp);
    exp = settings.value("cacheScreenshots");
    QCOMPARE(config.cacheScreenshots, exp);
    exp = settings.value("cacheTextures");
    QCOMPARE(config.cacheTextures, exp);
    exp = settings.value("cacheWheels");
    QCOMPARE(config.cacheWheels, exp);
    exp = settings.value("interactive");
    QCOMPARE(config.interactive, exp);
    exp = settings.value("jpgQuality");
    QCOMPARE(config.jpgQuality, exp);
    exp = settings.value("maxLength");
    QCOMPARE(config.maxLength, exp);
    QCOMPARE(config.minMatch, 65);
    QCOMPARE(config.minMatchSet, false);
    exp = settings.value("threads");
    QCOMPARE(config.threads, exp);
    exp = settings.value("tidyDesc");
    QCOMPARE(config.tidyDesc, exp);
    exp = settings.value("unattend");
    QCOMPARE(config.unattend, exp);
    exp = settings.value("unattendSkip");
    QCOMPARE(config.unattendSkip, exp);
    exp = settings.value("userCreds");
    QCOMPARE(config.userCreds, exp);
    exp = settings.value("videoConvertCommand");
    QCOMPARE(config.videoConvertCommand, exp);
    exp = settings.value("videoConvertExtension");
    QCOMPARE(config.videoConvertExtension, exp);
    exp = settings.value("videoPreferNormalized");
    QCOMPARE(config.videoPreferNormalized, exp);
    exp = settings.value("videos");
    QCOMPARE(config.videos, exp);
    exp = settings.value("videoSizeLimit");
    QCOMPARE(config.videoSizeLimit, exp.toInt() * 1000 * 1000);

    settings.endGroup();
}

QTEST_MAIN(TestSettings)
#include "moc_settings.cpp"
