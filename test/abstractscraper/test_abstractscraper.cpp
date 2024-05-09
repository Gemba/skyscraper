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
        scraper->detectRegionFromFilename(info);
        QCOMPARE(scraper->getRegionPrios().size(), expected.size());
        QCOMPARE(scraper->getRegionPrios(), expected);
    }

private slots:
    void initTestCase() {};

    void testDetectRegionsFromFilename1() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (j).zip");
        QList<QString> regionPriosExp;
        regionPriosExp.append("jp");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename2() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (j) world.zip");
        QList<QString> regionPriosExp;
        regionPriosExp.append("jp");
        regionPriosExp.append("wor");
        match(info, regionPriosExp);
    }

    void testDetectRegionsFromFilename3() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (france) wOrLD (j).zip");
        QList<QString> regionPriosExp;
        regionPriosExp.append("fr");
        regionPriosExp.append("jp");
        regionPriosExp.append("wor");
        match(info, regionPriosExp);
        qDebug() << info.completeBaseName();
        qDebug() << regionPriosExp;
    }

    void testDetectRegionsFromFilename4() {
        scraper = new AbstractScraper(&settings, NULL);
        QFileInfo info("Gametitle (usa) (u).zip");
        QList<QString> regionPriosExp;
        regionPriosExp.prepend("us");
        match(info, regionPriosExp);
    }
};

QTEST_MAIN(TestAbstractScraper)
#include "test_abstractscraper.moc"
