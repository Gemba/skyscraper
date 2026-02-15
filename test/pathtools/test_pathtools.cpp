#include "pathtools.h"
#include "config.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTest>

class TestPathTools : public QObject {
    Q_OBJECT

private:
    QMap<QString, QString> romanArabicMap;

private slots:

    void initTestCase() {}

    void testConcatPath() {
        QString actual = PathTools::concatPath("tmp/cache/", "/covers");
        QCOMPARE(actual, "tmp/cache/covers");
        actual = PathTools::concatPath("tmp/cache", "/wheels");
        QCOMPARE(actual, "tmp/cache/wheels");
        actual = PathTools::concatPath("/tmp/cache", "../wheels");
        QCOMPARE(actual, "/tmp/cache/../wheels");
        actual = PathTools::concatPath("../yadda/yadda", ".");
        QCOMPARE(actual, "../yadda/yadda");
        actual = PathTools::concatPath("aaa", ".bbb");
        QCOMPARE(actual, "aaa/.bbb");
    }

    void testOnlyAbs() {
        QCOMPARE(PathTools::makeAbsolutePath("/home/pi/RetroPie/roms", "/tmp"),
                 "/tmp");
        QCOMPARE(PathTools::makeAbsolutePath("/path/to/cwd", "config.ini"),
                 "/path/to/cwd/config.ini");
        // rationale for returning "": ease of use when subpath is empty
        // -> complete value is empty -> do not output in/for frontend
        QCOMPARE(PathTools::makeAbsolutePath("/blarf/blubb////", ""),
                 "");
        // ES writes ~/bla in some circumstance instead of absolute path /home/foo/bla
        // --> return unchanged        
        QCOMPARE(PathTools::makeAbsolutePath("/home/pi/RetroPie/roms", "~/.emulationstation/downloaded_media/apple2/videos/loderunr.mp4"),
                 "~/.emulationstation/downloaded_media/apple2/videos/loderunr.mp4");
    }

    void testAbsWithRel() {
        QString actual =
            PathTools::makeAbsolutePath("/home/pi/RetroPie/roms", "./amiga");
        QCOMPARE(actual, "/home/pi/RetroPie/roms/amiga");

        actual =
            PathTools::makeAbsolutePath("/home/pi/RetroPie/roms/", ".///amiga");
        QCOMPARE(actual, "/home/pi/RetroPie/roms/amiga");

        actual = PathTools::makeAbsolutePath("/path/to/pegasus", "../roms/snes");
        QCOMPARE(actual, "/path/to/pegasus/../roms/snes");

        actual = PathTools::makeAbsolutePath("/yadda", ".");
        QCOMPARE(actual, "/yadda");

        actual = PathTools::makeAbsolutePath("/yadda/meh///", "wuff");
        QCOMPARE(actual, "/yadda/meh/wuff");
    }

    void testlexicalRel() {
        QString actual = PathTools::lexicallyRelativePath(
            "/home/pi/RetroPie/roms", "/home/pi/RetroPie/roms/amiga");
        QCOMPARE(actual, "amiga");
        actual = PathTools::lexicallyRelativePath(
            "/home/pi/RetroPie/roms/amiga", "/home/pi/RetroPie/roms/amiga/screen.png");
        QCOMPARE(actual, "screen.png");
        // inputFolder is not the default
        actual = PathTools::lexicallyRelativePath(
            "/home/pi/inputfolder/some/where/else/amiga", "/home/pi/RetroPie/roms/amiga");
        QCOMPARE(actual, "../../../../../RetroPie/roms/amiga");
        actual = PathTools::lexicallyRelativePath("/path/to/pegasus/",
                                               "/path/to/pegasus/../roms/snes");
        QCOMPARE(actual, "../roms/snes");
        actual =
            PathTools::lexicallyRelativePath("/path/to/pegasus/", "../roms/snes");
        QCOMPARE(actual, "");
        actual = PathTools::lexicallyRelativePath("relative/path/to/pegasus/",
                                               "roms/fba");
        QCOMPARE(actual, "../../../../roms/fba");
        actual = PathTools::lexicallyRelativePath("/path/to/pegasus/",
                                               "/path/to/roms/ports");
        QCOMPARE(actual, "../roms/ports");
        actual = PathTools::lexicallyRelativePath("/path/to/pegasus/",
                                               "/other/path/to/roms/apple2");
        QCOMPARE(actual, "../../../other/path/to/roms/apple2");
    }
};

QTEST_MAIN(TestPathTools)
#include "test_pathtools.moc"
