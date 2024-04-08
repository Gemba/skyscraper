#include "arcadedb.h"
#include "config.h"
#include "esgamelist.h"
#include "igdb.h"
#include "mobygames.h"
#include "openretro.h"
#include "screenscraper.h"

#include <QDebug>
#include <QTest>

/*
 Tests the getSearchNames() method of the different scrapers.
 WorldOfSpectrum and Import-Scraper and TGDB are covered by Mobygames scraper:
 These use getSearchNames() in the AbstractScraper.

 If testing against v3.10.3 or earlier set the define VER_3_10_3 in the *.pro
 file.
*/
class TestGetSearchNames : public QObject {
    Q_OBJECT

private:
    void match(const QMap<QString, QPair<QString, QStringList>> &tests) {
        QString dbgStr;

        for (auto t : tests.keys()) {
            qInfo() << "Testing" << t << "...";
            QString input = tests.value(t).first;
            QFile f = QFile(input);
            QFileInfo fi(f);
            QStringList exp = tests.value(t).second;
#ifdef VER_3_10_3
            QList<QString> out = scraper->getSearchNames(fi);
#else
            QList<QString> out = scraper->getSearchNames(fi, dbgStr);
#endif
            //            qDebug() << out;

            QCOMPARE(out.size(), exp.size());
            int i = 0;
            for (const auto &e : exp) {
                QCOMPARE(out[i++], e);
            }
            qInfo() << "... done testing" << t;
        }
    }

    Settings settings;
    AbstractScraper *scraper;

private slots:
    void initTestCase() {

        settings.aliasMap = {
            // explictly change first word in the replacement
            // to spot possible errors (MotherX)
            {"Mother 25th Restoration Hack", {"MotherX (Japan)"}},
            {"Dot1.dot2", {"Dot1x Dot2x"}},
            {"Arle's Adventure - Magical Jewels (Japan) (SGB Enhanced) (GB Compatible) [T-En by Jazz v1.0] [n]", {"Arle No Bouken - Mahou No Jewel"}}
        };
    }

    void testArcadeDB() {
        scraper = new ArcadeDB(&settings, NULL);
        QMap<QString, QPair<QString, QStringList>> tests_adb = {
#ifndef VER_3_10_3
            {"ArcadeDB, with aliasMap match",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"MotherX (Japan)"})},
            {"ArcadeDB, with aliasMap match (completeBaseName)",
             QPair<QString, QStringList>(
                 "./rom_samples/Dot1.dot2.zip",
                 {"Dot1x Dot2x"})},
#endif
            {"ArcadeDB, w/o  aliasMap match",
             QPair<QString, QStringList>("./rom_samples/1942.zip", {"1942"})},

        };

        match(tests_adb);
    }

    void testESGamelist() {
        QMap<QString, QPair<QString, QStringList>> tests_esgl = {
            {"ESgl, with aliasMap match (aliasMap should be ignored)",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"Mother 25th Restoration Hack.zip"})},
            {"ESgl, w/o  aliasMap match",
             QPair<QString, QStringList>("./rom_samples/1942.zip",
                                         {"1942.zip"})},

        };

        scraper = new ESGameList(&settings, NULL);
        match(tests_esgl);
    }

    void testOpenRetro() {
        scraper = new OpenRetro(&settings, NULL);
        QMap<QString, QPair<QString, QStringList>> tests_matchone = {
            {"OpenRetro, with aliasMap match",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"/browse?q=motherx"})},
            {"OpenRetro, with ROM hack and no aliasMap entry",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack_Hack.zip",
                 {"/browse?q=mother+25th"})},
            {"OpenRetro, w/o  aliasMap match 1 word",
             QPair<QString, QStringList>("./rom_samples/1942.zip",
                                         {"/browse?q=1942"})},
            {"OpenRetro, w/o  aliasMap match 2+ words",
             QPair<QString, QStringList>("./rom_samples/abclimax.zip",
                                         {"/browse?q=after+burner"})},

        };

        settings.platform = "fba";
        settings.mameMap = {{"1942", "1942 (Revision B)"},
                            {"abclimax", "After Burner Climax (Export)"}

        };
        match(tests_matchone);
    }

    void testOpenRetroMatchMany() {
        scraper = new OpenRetro(&settings, NULL);
        settings.platform = "amiga";
        settings.whdLoadMap = {
            {"BillsTomatoGame_v2.0_1748",
             {"Bill's Tomato Game", "307c1c7f-1f31-5b7d-ac71-57a9f035679f"}},
            {"Burntime_v1.2_AGA_2213",
             {"Burntime [AGA]", "930a2652-50e0-5b89-8076-3ec478f0e6ad"}},

        };

        QMap<QString, QPair<QString, QStringList>> tests_matchmany = {
            {"OpenRetro, with amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/BillsTomatoGame_v2.0_1748.lha",
                 {"/game/307c1c7f-1f31-5b7d-ac71-57a9f035679f",
                  "/browse?q=bill%27s+tomato"})},
            {"OpenRetro, w/o amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/Some game-here V1.0.lha",
                 {"/browse?q=some+game-here", "/browse?q=some+game"})},
            {"OpenRetro, with amiga whdl and aga match",
             QPair<QString, QStringList>(
                 "./rom_samples/Burntime_v1.2_AGA_2213.lha",
                 {"/game/930a2652-50e0-5b89-8076-3ec478f0e6ad",
                  "/browse?q=burntime+aga", "/browse?q=burntime"})},

        };

        match(tests_matchmany);

        QMap<QString, QPair<QString, QStringList>> tests_scummvm = {
            {"OpenRetro, with scummvm.ini match",
             QPair<QString, QStringList>("./rom_samples/tentacle.svm",
                                         {"/browse?q=day+of"})},
            {"OpenRetro, w/o scummvm.ini match",
             QPair<QString, QStringList>(
                 "./rom_samples/the yabba dabba doo.svm",
                 {"/browse?q=yabba+dabba"})},

        };

        settings.platform = "scummvm";
        settings.scummIni =
            QCoreApplication::applicationDirPath() + "/scummvm.ini";

        match(tests_scummvm);
    }

    void testIgdb() {
        scraper = new Igdb(&settings, NULL);
        settings.platform = "amiga";
        settings.whdLoadMap = {
            {"BillsTomatoGame_v2.0_1748",
             {"Bill's Tomato Game", "307c1c7f-1f31-5b7d-ac71-57a9f035679f"}},
            {"Burntime_v1.2_AGA_2213",
             {"Burntime [AGA]", "930a2652-50e0-5b89-8076-3ec478f0e6ad"}},

        };

        settings.aliasMap.insert("Mother.25th.Restoration.Hack", "Dot.dot.dot");

        QMap<QString, QPair<QString, QStringList>> tests_matchmany = {
            {"Igdb, with amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/BillsTomatoGame_v2.0_1748.lha",
                 {"Bill's Tomato Game"})},
            {"Igdb, w/o amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/Some game-here V99.33.lha",
                 {"Some game-here V99.33"})},
            {"Igdb, with amiga whdl and aga match",
             QPair<QString, QStringList>(
                 "./rom_samples/Burntime_v1.2_AGA_2213.lha", {"Burntime"})},
            {"Igdb, with aliasMap match",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"MotherX"})},
            {"Igdb, with aliasMap match, ROM with dots",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother.25th.Restoration.Hack.zip",
                 {"Dot.dot.dot"})},
            {"Igdb, with ROM hack and no aliasMap entry",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack_Hack.zip",
                 {"Mother 25th Restoration Hack_Hack"})},

        };

        match(tests_matchmany);

        QMap<QString, QPair<QString, QStringList>> tests_scummvm = {
            {"Igdb, with scummvm.ini match",
             QPair<QString, QStringList>("./rom_samples/tentacle.svm",
                                         {"Day of the Tentacle"})},
            {"Igdb, w/o scummvm.ini match",
             QPair<QString, QStringList>(
                 "./rom_samples/the yabba dabba doo.svm",
                 {"the yabba dabba doo"})},

        };

        settings.platform = "scummvm";
        settings.scummIni =
            QCoreApplication::applicationDirPath() + "/scummvm.ini";

        match(tests_scummvm);
    }

    void testMobyGames() {
        scraper = new MobyGames(&settings, NULL);
        settings.platform = "amiga";
        settings.whdLoadMap = {
            {"BillsTomatoGame_v2.0_1748",
             {"Bill's Tomato Game", "307c1c7f-1f31-5b7d-ac71-57a9f035679f"}},
            {"Burntime_v1.2_AGA_2213",
             {"Burntime [AGA]", "930a2652-50e0-5b89-8076-3ec478f0e6ad"}},

        };

        QMap<QString, QPair<QString, QStringList>> tests_matchmany = {
            {"Mobygames, with amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/BillsTomatoGame_v2.0_1748.lha",
                 {"bill%27s+tomato+game"})},
            {"Mobygames, w/o amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/Some game-here V99.33.lha",
                 {"some+game-here+v99.33"})},
            {"Mobygames, with amiga whdl and aga match",
             QPair<QString, QStringList>(
                 "./rom_samples/Burntime_v1.2_AGA_2213.lha", {"burntime"})},
            {"Mobygames, with aliasMap match",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"motherx"})},
            {"Mobygames, with ROM hack and no aliasMap entry",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack_Hack.zip",
                 {"mother+25th+restoration+hack+hack"})},

        };

        match(tests_matchmany);

        QMap<QString, QPair<QString, QStringList>> tests_scummvm = {
            {"Mobygames, with scummvm.ini match",
             QPair<QString, QStringList>("./rom_samples/tentacle.svm",
                                         {"day+of+the+tentacle"})},
            {"Mobygames, w/o scummvm.ini match",
             QPair<QString, QStringList>(
                 "./rom_samples/the yabba dabba doo.svm", {"yabba+dabba+doo"})},

        };

        settings.platform = "scummvm";
        settings.scummIni =
            QCoreApplication::applicationDirPath() + "/scummvm.ini";

        match(tests_scummvm);
    }
    void testScreenscraper() {
        scraper = new ScreenScraper(&settings, NULL);
        settings.platform = "mame-libretro";
        QStringList zaxxon_expected = {
            "crc=BB2E0146", "md5=852605F01A3E2D21FBAF35FCAB385B94",
            "sha1=05F32B2286ECE2F30CB1EFBC2B4C7C2AEF2F9662",
            "romnom=zaxxon.zip", "romtaille=32768"};
        QMap<QString, QPair<QString, QStringList>> tests = {
#ifndef VER_3_10_3
            {"Screenscraper, with aliasMap match",
             QPair<QString, QStringList>(
                 "./rom_samples/Mother 25th Restoration Hack.zip",
                 {"romnom=MotherX%20(Japan)"})},
            {"Screenscraper, with aliasMap match (completeBaseName)",
             QPair<QString, QStringList>(
                 "./rom_samples/Arle's Adventure - Magical Jewels (Japan) (SGB Enhanced) (GB Compatible) [T-En by Jazz v1.0] [n].zip",
                 {"romnom=Arle%20No%20Bouken%20-%20Mahou%20No%20Jewel"})},
#endif
            {"Screenscraper, with arcade match",
             QPair<QString, QStringList>("./rom_samples/zaxxon.zip",
                                         {zaxxon_expected.join("&")})},
            {"Screenscraper, 0 byte ROM",
             QPair<QString, QStringList>("./rom_samples/empty (0 bytes).zip",
                                         {"romnom=empty%20(0%20bytes).zip"})},
            {"Screenscraper, with amiga whdl match",
             QPair<QString, QStringList>(
                 "./rom_samples/BillsTomatoGame_v2.0_1748.lha",
                 {"romnom=BillsTomatoGame_v2.0_1748.lha"})},

        };
        settings.mameMap.insert("zaxxon.zip", "Should not break the test.zip");

        match(tests);
    }
};

QTEST_MAIN(TestGetSearchNames)
#include "test_getsearchnames.moc"
