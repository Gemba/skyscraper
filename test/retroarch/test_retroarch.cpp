#include "retroarch.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

class TestRetroArch : public QObject {
    Q_OBJECT

private:
    Settings settings;
    RetroArch *frontend;

private slots:
    void initTestCase() {
        frontend = new RetroArch();
        frontend->setConfig(&settings);
    }

    void cleanupTestCase() { delete frontend; }

    // Test platform output name resolution through public interface
    void testGetPlatformOutputName() {
        // Test fallback behavior for unknown platform
        settings.platform = "UnknownPlatform12345";
        QString result = frontend->getPlatformOutputName();
        QCOMPARE(result, "UnknownPlatform12345"); // Falls back to platform name

        // Test with known platform (depends on platforms_idmap.csv)
        settings.platform = "snes";
        result = frontend->getPlatformOutputName();
        QVERIFY(!result.isEmpty()); // Should resolve to something from CSV
    }

    // Test loading existing playlist files for incremental updates
    void testLoadOldGameList() {
        // Test valid playlist
        bool result = frontend->loadOldGameList("playlists/test.lpl");
        QVERIFY(result);

        // Verify entries were loaded by checking the oldEntries list size
        // We'll access this indirectly through other tests or add a getter if
        // needed

        // Test non-existent file
        QCOMPARE(frontend->loadOldGameList("/nonexistent/file.lpl"), false);

        // Test invalid JSON (create temp file with bad content)
        QFile badFile("playlists/invalid.txt");
        QVERIFY(badFile.open(QIODevice::WriteOnly));
        QTextStream out(&badFile);
        out << "not json at all";
        badFile.close();
        QCOMPARE(frontend->loadOldGameList("playlists/invalid.txt"), false);

        // Test valid JSON but wrong structure (no "items" array)
        QFile weirdFile("playlists/weird.json");
        QVERIFY(weirdFile.open(QIODevice::WriteOnly));
        QTextStream out2(&weirdFile);
        out2 << "{\"version\": \"1.5\", \"data\": []}";
        weirdFile.close();
        // This should return true (it's valid JSON with an object root)
        // but won't load any entries since there's no "items" array
        result = frontend->loadOldGameList("playlists/weird.json");
        QVERIFY(result);

        // Cleanup temp files
        QFile::remove("playlists/invalid.txt");
        QFile::remove("playlists/weird.json");
    }

    // Test the main playlist generation functionality through public interface
    void testAssembleList() {
        RetroArch localFrontend; // Fresh instance without old entries
        Settings localSettings;
        localFrontend.setConfig(&localSettings);

        // Create test game entries
        QList<GameEntry> entries;

        GameEntry entry1;
        entry1.path = "/roms/snes/Mega Man.sfc";
        entry1.title = "Mega Man";
        entry1.baseName = "Mega Man";
        entry1.absoluteFilePath = "/roms/snes/Mega Man.sfc";
        entries.append(entry1);

        GameEntry entry2;
        entry2.path = "/roms/snes/Zelda.zip";
        entry2.title = "The Legend of Zelda";
        entry2.baseName = "Zelda";
        entry2.absoluteFilePath = "/roms/snes/Zelda.zip";
        entries.append(entry2);

        // Set platform for db_name resolution
        localSettings.platform = "snes";

        // Assemble the list
        QString output;
        localFrontend.assembleList(output, entries);

        // Parse and verify JSON structure
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        QVERIFY(doc.isObject());
        QJsonObject root = doc.object();

        // Check version (should be 1.5)
        QCOMPARE(root.value("version").toString(), "1.5");

        // Check default_core_path and default_core_name defaults
        QCOMPARE(root.value("default_core_path").toString(), "DETECT");
        QCOMPARE(root.value("default_core_name").toString(), "DETECT");

        // Check display modes
        QCOMPARE(root.value("label_display_mode").toInt(), 0);
        QCOMPARE(root.value("left_display_mode").toInt(), 0);
        QCOMPARE(root.value("right_display_mode").toInt(), 0);
        QCOMPARE(root.value("thumbnail_match_mode").toInt(), 0);
        QCOMPARE(root.value("sort_mode").toInt(), 0);

        // Check items array
        QJsonArray items = root.value("items").toArray();
        QCOMPARE(items.size(), 2);

        // Verify first item structure
        QJsonObject firstItem = items.at(0).toObject();
        QCOMPARE(firstItem.value("path").toString(), "/roms/snes/Mega Man.sfc");
        QCOMPARE(firstItem.value("label").toString(), "Mega Man");
        QCOMPARE(firstItem.value("core_path").toString(), "DETECT");
        QCOMPARE(firstItem.value("core_name").toString(), "DETECT");
        QCOMPARE(firstItem.value("crc32").toString(), "DETECT");
        QVERIFY(firstItem.value("db_name").toString().endsWith(".lpl"));

        // Verify second item structure
        QJsonObject secondItem = items.at(1).toObject();
        QCOMPARE(secondItem.value("path").toString(), "/roms/snes/Zelda.zip");
        QCOMPARE(secondItem.value("label").toString(), "The Legend of Zelda");
    }

    // Test that special characters in game titles are handled properly
    // (indirectly tests sanitizeForFilename via getTargetFileName)
    void testSpecialCharactersInTitles() {
        RetroArch localFrontend;
        Settings localSettings;
        localFrontend.setConfig(&localSettings);

        QList<GameEntry> entries;

        // Entry with special characters in title that need sanitization for
        // filenames
        GameEntry entry;
        entry.path = "/roms/snes/SpecialGame.sfc";
        entry.title = "Special & Game! With Forbidden/Chars";
        entry.baseName = "SpecialGame";
        entry.absoluteFilePath = "/roms/snes/SpecialGame.sfc";
        entries.append(entry);

        QString output;
        localFrontend.assembleList(output, entries);

        // Verify the playlist was created correctly with special chars in title
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        QJsonObject root = doc.object();
        QJsonArray items = root.value("items").toArray();
        QCOMPARE(items.size(), 1);
        QJsonObject item = items.at(0).toObject();

        // Title should be preserved in the label (JSON handles special chars
        // natively)
        QCOMPARE(item.value("label").toString(),
                 "Special & Game! With Forbidden/Chars");

        // Verify JSON is valid and parseable (indirectly tests that output is
        // well-formed)
        QVERIFY(doc.isObject());
    }

    // Test the playlist filename generation through public interface
    void testGetGameListFileName() {
        Settings localSettings;
        RetroArch localFrontend;
        localFrontend.setConfig(&localSettings);

        // Default case - uses platform output name + .lpl extension
        localSettings.platform = "snes";
        localSettings.gameListFilename.clear();
        QString result = localFrontend.getGameListFileName();
        QVERIFY(result.endsWith(".lpl"));

        // Custom filename from config
        localSettings.gameListFilename = "custom_playlist.lpl";
        QCOMPARE(localFrontend.getGameListFileName(), "custom_playlist.lpl");

        // Another custom format test
        localSettings.gameListFilename = "my_games.json";
        QCOMPARE(localFrontend.getGameListFileName(), "my_games.json");
    }

    // Test all the folder path getter functions through public interface
    void testFolderPaths() {
        Settings localSettings;
        RetroArch localFrontend;
        localFrontend.setConfig(&localSettings);

        localSettings.platform = "snes";
        localSettings.mediaFolder = "/opt/retropie/configs/all/assets";

        // Test media folder - this returns the hardcoded thumbnail path
        QCOMPARE(localFrontend.getMediaFolder(),
                 "/opt/retropie/configs/all/retroarch/thumbnails");

        // Test covers folder (Named_Boxarts subfolder of config->mediaFolder)
        QCOMPARE(localFrontend.getCoversFolder(),
                 "/opt/retropie/configs/all/assets/Named_Boxarts");

        // Test screenshots folder (Named_Snaps subfolder)
        QCOMPARE(localFrontend.getScreenshotsFolder(),
                 "/opt/retropie/configs/all/assets/Named_Snaps");

        // Test marquees/wheels folder (Named_Logos subfolder)
        QCOMPARE(localFrontend.getMarqueesFolder(),
                 "/opt/retropie/configs/all/assets/Named_Logos");
        QCOMPARE(localFrontend.getWheelsFolder(),
                 "/opt/retropie/configs/all/assets/Named_Logos");

        // Test with different media folder
        localSettings.mediaFolder = "/home/pi/RetroPie/downloaded_media";
        QCOMPARE(localFrontend.getCoversFolder(),
                 "/home/pi/RetroPie/downloaded_media/Named_Boxarts");
    }

    // Test that the correct media types are supported through public interface
    void testSupportedMedia() {
        GameEntry::Types supported = frontend->supportedMedia();

        // Should support COVER, SCREENSHOT, and WHEEL
        QVERIFY(supported & GameEntry::COVER);
        QVERIFY(supported & GameEntry::SCREENSHOT);
        QVERIFY(supported & GameEntry::WHEEL);

        // Should NOT support VIDEO (not yet implemented)
        QVERIFY(!(supported & GameEntry::VIDEO));

        // Should NOT support MANUAL, TEXTURE, FANART, BACKCOVER
        QVERIFY(!(supported & GameEntry::MANUAL));
        QVERIFY(!(supported & GameEntry::TEXTURE));
        QVERIFY(!(supported & GameEntry::FANART));
        QVERIFY(!(supported & GameEntry::BACKCOVER));
    }

    // Test frontend capabilities flags through public interface
    void testFrontendCapabilities() {
        // Can skip existing entries for incremental updates
        QVERIFY(frontend->canSkip());
    }

    // Test empty game list handling through public interface
    void testEmptyGameList() {
        RetroArch localFrontend;
        Settings localSettings;
        localFrontend.setConfig(&localSettings);

        QList<GameEntry> entries; // Empty list
        localSettings.platform = "snes";

        QString output;
        localFrontend.assembleList(output, entries);

        // assembleList has early return for empty lists - no output is produced
        // This prevents creating empty playlist files when there are no games
        QVERIFY(output.isEmpty());
    }

    // Test frontendExtra parsing for core path info (contains /) through public
    // interface
    void testFrontendExtraCorePath() {
        RetroArch localFrontend;
        Settings localSettings;
        localFrontend.setConfig(&localSettings);

        QList<GameEntry> entries;
        GameEntry entry;
        entry.path = "/roms/test.rom";
        entry.title = "Test Game";
        entry.baseName = "test";
        entry.absoluteFilePath = "/roms/test.rom";
        entries.append(entry);

        // Test frontendExtra with core path info (contains /)
        localSettings.frontendExtra = "/path/to/core;mycore";
        localSettings.platform = "testplatform";

        QString output;
        localFrontend.assembleList(output, entries);

        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        QJsonObject root = doc.object();

        // When frontendExtra contains /, it's parsed as core path info
        // The first part becomes default_core_path, second (after ;) becomes
        // default_core_name
        QCOMPARE(root.value("default_core_path").toString(), "/path/to/core");
        QCOMPARE(root.value("default_core_name").toString(), "mycore");

        // db_name is determined by platform mapping, not frontendExtra
        QJsonArray items = root.value("items").toArray();
        QVERIFY(items.size() > 0);
        QVERIFY(items.at(0).toObject().value("db_name").toString().endsWith(
            ".lpl"));
    }

    // Test that JSON output is valid with various game title formats
    void testGameTitleFormats() {
        RetroArch localFrontend;
        Settings localSettings;
        localFrontend.setConfig(&localSettings);
        localSettings.platform = "snes";

        QList<GameEntry> entries;

        // Game with quotes in title
        GameEntry entry1;
        entry1.path = "/roms/snes/GameOne.sfc";
        entry1.title = "Game's \"Title\"";
        entry1.baseName = "GameOne";
        entry1.absoluteFilePath = "/roms/snes/GameOne.sfc";
        entries.append(entry1);

        // Game with newlines (unusual but possible)
        GameEntry entry2;
        entry2.path = "/roms/snes/GameTwo.sfc";
        entry2.title = "Game Two\nSubtitle";
        entry2.baseName = "GameTwo";
        entry2.absoluteFilePath = "/roms/snes/GameTwo.sfc";
        entries.append(entry2);

        // Game with backslash in path
        GameEntry entry3;
        entry3.path = "/roms/snes/Path\\Test.sfc";
        entry3.title = "Backslash Path Test";
        entry3.baseName = "Path\\Test";
        entry3.absoluteFilePath = "/roms/snes/Path\\Test.sfc";
        entries.append(entry3);

        QString output;
        localFrontend.assembleList(output, entries);

        // Verify all JSON is valid and can be parsed back
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        QVERIFY(doc.isObject());

        QJsonObject root = doc.object();
        QJsonArray items = root.value("items").toArray();
        QCOMPARE(items.size(), 3);

        // Verify titles are preserved correctly (JSON escaping handles them)
        QCOMPARE(items.at(0).toObject().value("label").toString(),
                 "Game's \"Title\"");
        QCOMPARE(items.at(1).toObject().value("label").toString(),
                 "Game Two\nSubtitle");
        QCOMPARE(items.at(2).toObject().value("label").toString(),
                 "Backslash Path Test");
    }

    // Test input folder path through public interface
    void testInputFolder() {
        Settings localSettings;
        RetroArch localFrontend;
        localFrontend.setConfig(&localSettings);

        localSettings.platform = "snes";

        // Input folder should be ~/RetroPie/roms/<platform>
        QString inputPath = localFrontend.getInputFolder();
        QVERIFY(inputPath.contains("/RetroPie/roms/snes"));
    }

    // Test game list folder path through public interface
    void testGameListFolder() {
        Settings localSettings;
        RetroArch localFrontend;
        localFrontend.setConfig(&localSettings);

        // Game list folder should be the standard RetroArch playlist location
        QCOMPARE(localFrontend.getGameListFolder(),
                 "/opt/retropie/configs/all/retroarch/playlists");
    }
};

QTEST_MAIN(TestRetroArch)
#include "test_retroarch.moc"
