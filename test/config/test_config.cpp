#include "config.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTest>

class TestConfig : public QObject {
    Q_OBJECT

private:
    QMap<QString, QString> romanArabicMap;

private slots:

    void initTestCase() {}

    void testConcatPath() {
        QString actual = Config::concatPath("tmp/cache/", "/covers");
        QCOMPARE(actual, "tmp/cache/covers");
        actual = Config::concatPath("tmp/cache", "/wheels");
        QCOMPARE(actual, "tmp/cache/wheels");
        actual = Config::concatPath("/tmp/cache", "../wheels");
        QCOMPARE(actual, "/tmp/cache/../wheels");
        actual = Config::concatPath("../yadda/yadda", ".");
        QCOMPARE(actual, "../yadda/yadda");
        actual = Config::concatPath("aaa", ".bbb");
        QCOMPARE(actual, "aaa/.bbb");
    }

    void testOnlyAbs() {
        QCOMPARE(Config::makeAbsolutePath("/home/pi/RetroPie/roms", "/tmp"),
                 "/tmp");
        QCOMPARE(Config::makeAbsolutePath("/path/to/cwd", "config.ini"),
                 "/path/to/cwd/config.ini");
        // rationale for returning "": ease of use when subpath is empty
        // -> complete value is empty -> do not output in/for frontend
        QCOMPARE(Config::makeAbsolutePath("/blarf/blubb////", ""),
                 "");
    }

    void testAbsWithRel() {
        QString actual =
            Config::makeAbsolutePath("/home/pi/RetroPie/roms", "./amiga");
        QCOMPARE(actual, "/home/pi/RetroPie/roms/amiga");

        actual =
            Config::makeAbsolutePath("/home/pi/RetroPie/roms/", ".///amiga");
        QCOMPARE(actual, "/home/pi/RetroPie/roms/amiga");

        actual = Config::makeAbsolutePath("/path/to/pegasus", "../roms/snes");
        QCOMPARE(actual, "/path/to/pegasus/../roms/snes");

        actual = Config::makeAbsolutePath("/yadda", ".");
        QCOMPARE(actual, "/yadda");

        actual = Config::makeAbsolutePath("/yadda/meh///", "wuff");
        QCOMPARE(actual, "/yadda/meh/wuff");
    }

    void testlexicalRel() {
        QString actual = Config::lexicallyRelativePath(
            "/home/pi/RetroPie/roms", "/home/pi/RetroPie/roms/amiga");
        QCOMPARE(actual, "amiga");
        actual = Config::lexicallyRelativePath("/path/to/pegasus/",
                                               "/path/to/pegasus/../roms/snes");
        QCOMPARE(actual, "../roms/snes");
        actual =
            Config::lexicallyRelativePath("/path/to/pegasus/", "../roms/snes");
        QCOMPARE(actual, "");
        actual = Config::lexicallyRelativePath("relative/path/to/pegasus/",
                                               "roms/fba");
        QCOMPARE(actual, "../../../../roms/fba");
        actual = Config::lexicallyRelativePath("/path/to/pegasus/",
                                               "/path/to/roms/ports");
        QCOMPARE(actual, "../roms/ports");
        actual = Config::lexicallyRelativePath("/path/to/pegasus/",
                                               "/other/path/to/roms/apple2");
        QCOMPARE(actual, "../../../other/path/to/roms/apple2");
    }
};

QTEST_MAIN(TestConfig)
#include "test_config.moc"
