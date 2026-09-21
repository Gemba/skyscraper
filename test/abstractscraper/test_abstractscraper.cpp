#include "abstractscraper.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTest>

class TestAbstractScraper : public QObject {
    Q_OBJECT

private:
    Settings settings;
    AbstractScraper *scraper;

    void match(QFileInfo &info, QList<QString> &expected) {
        qDebug() << "[*] From file:" << info.fileName()
                 << "(no regionPrios='' configured) ...";
        scraper->detectRegionFromFilename(info);
        QCOMPARE(scraper->getRegionPrios().size(), expected.size());
        QCOMPARE(scraper->getRegionPrios(), expected);
        qDebug() << "[+] ... passed, got:" << expected;
    }

    QStringList
    setupExpectedRegionPrios(const QStringList &configuredRegionPrios,
                             QString firstRegion) {
        QStringList ret = configuredRegionPrios;
        if (int idx = ret.lastIndexOf(firstRegion); idx > -1) {
            ret.removeAt(idx);
        }
        ret.prepend(firstRegion);
        return ret;
    }

    void matchRegion(QString fn, QStringList regionPriosExp) {
        QFileInfo game(fn);
        qDebug() << "[*] Configured region prios:" << settings.regionPrios;
        qDebug() << "[*] From file:" << game.fileName() << "...";
        qDebug() << "    Expected: " << regionPriosExp;
        scraper->detectRegionFromFilename(game);
        qDebug() << "    Actual:   " << scraper->getRegionPrios();
        QCOMPARE(scraper->getRegionPrios(), regionPriosExp);
        qDebug() << "[+] ... passed";
    }

private slots:
    void initTestCase(){};

    void testDetectRegionsFromFilename1() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (j).zip");
        QList<QString> regionPriosExp;
        regionPriosExp.append("jp");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename2() {
        // "world" shall not be detected as it is not in parenthesis
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (j) world.zip");
        QList<QString> regionPriosExp;
        regionPriosExp.append("jp");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename3() {
        // "world" shall not be detected as it is not in parenthesis
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (france) wOrLD (j).zip");
        QList<QString> regionPriosExp;
        // expected order by match in in filename as no regionPrioList is given
        regionPriosExp.append("fr");
        regionPriosExp.append("jp");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename4() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (Usa) (u).zip");
        QList<QString> regionPriosExp;
        regionPriosExp.prepend("us");
        match(info, regionPriosExp);
        info = QFileInfo("Gametitle (Usa).zip");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename5() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (e) (u).zip");
        QList<QString> regionPriosExp = QStringList({"eu", "us"});
        match(info, regionPriosExp);
    }

    void testRegionsFromFilenameOptionInline() {
        // "br" surplus
        settings.regionPrios = QStringList({"eu", "br", "us", "jp"});
        settings.regionFromFilename = "inline";
        scraper = new AbstractScraper(&settings, NULL);

        QList<QString> regionPriosExp = QStringList({"us", "jp", "eu", "br"});
        matchRegion("Game A (Japan, USA).zip", regionPriosExp);
        matchRegion("Game A' (us, jp).zip", regionPriosExp);

        regionPriosExp = QStringList({"eu", "us", "br", "jp"});
        matchRegion("Game B (USA, Europe).zip", regionPriosExp);

        // "wor" should be last as there is no match in regionPrios
        // FIXME comment
        regionPriosExp = QStringList({"eu", "us", "wor", "br", "jp"});
        matchRegion("Game C (USA, World, Europe).zip", regionPriosExp);

        settings.regionPrios = QStringList({"jp", "eu"});
        regionPriosExp = QStringList({"eu", "us", "jp"});
        matchRegion("Game D (UE).zip", regionPriosExp);
        settings.regionPrios = QStringList({"eu"});
        regionPriosExp = QStringList({"eu", "jp", "us"});
        matchRegion("Game D' (JUE).zip", regionPriosExp);

        settings.regionPrios = QStringList({"eu"});
        regionPriosExp = QStringList({"us", "eu"});
        matchRegion("Game X (USA, xyz).zip", regionPriosExp);
    }

    void testRegionsFromFilenameOptionFirst() {
        // "br" surplus
        settings.regionPrios = QStringList({"eu", "br", "us", "jp"});
        settings.regionFromFilename = "first";
        scraper = new AbstractScraper(&settings, NULL);

        QList<QString> regionPriosExp = QStringList({"jp", "us", "eu", "br"});
        matchRegion("Game A (Japan, USA).zip", regionPriosExp);
        regionPriosExp = QStringList({"us", "jp", "eu", "br"});
        matchRegion("Game A' (us, jp).zip", regionPriosExp);

        regionPriosExp = QStringList({"us", "eu", "br", "jp"});
        matchRegion("Game B (USA, Europe).zip", regionPriosExp);

        regionPriosExp = QStringList({"us", "wor", "eu", "br", "jp"});
        matchRegion("Game C (USA, World, Europe).zip", regionPriosExp);

        settings.regionPrios = QStringList({"eu"});
        regionPriosExp = QStringList({"us", "eu"});
        matchRegion("Game X (USA, xyz).zip", regionPriosExp);
    }

    void testRegionsFromFilenameOptionInlinePR253() {

        settings.regionPrios = QStringList({"eu", "us", "jp"});
        settings.regionFromFilename = "inline";
        scraper = new AbstractScraper(&settings, NULL);

        QList<QString> regionPriosExp = QStringList({"eu", "us", "br", "jp"});
        matchRegion("Game A' (USA, Europe, Brazil).zip", regionPriosExp);
    }
};

QTEST_MAIN(TestAbstractScraper)
#include "test_abstractscraper.moc"
