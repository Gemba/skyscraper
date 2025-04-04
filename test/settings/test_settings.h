
#include <QtTest/QtTest>

class TestSettings : public QObject {
    Q_OBJECT
public:
    TestSettings() {}
    ~TestSettings() {}
private slots:
    void testConfigIniMain();
    void testConfigIniPlatform();
    void testConfigIniFrontend();
    void testConfigIniScraper();
    void testConfigIniPaths();
};
