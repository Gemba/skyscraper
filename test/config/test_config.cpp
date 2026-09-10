#include "config.h"

#include <QStringBuilder>
#include <QTest>

class TestConfig : public QObject {
    Q_OBJECT

private:
    Config cfg;
    QString applTestDir = QCoreApplication::applicationDirPath();
    QStringList dirsCreated;

private slots:

    void initTestCase() {}

    void testValidateConfigSrcPathMakeInstall() {
        QString sysConf = applTestDir + "/usr/local/etc";
        QString prefix = applTestDir + "/usr/local";
        QString app = applTestDir + "/usr/local/bin";
        // create SYSCONFDIR, this is validated if it exists
        createTestDir(sysConf + "/skyscraper");
        QCOMPARE(cfg.validateConfigSrcPath(sysConf, prefix, app),
                 sysConf + "/skyscraper");
    }

    void testValidateConfigSrcPathAppImage() {
        QString sysConf = "/etc";    // default for AppImage build
        QString prefix = "/usr"; // default for AppImage build
        QString app = applTestDir + "/tmp/mount.xyz" + prefix + "/bin";
        // create SYSCONFDIR inside appimage, this is validated if it exists
        QString exp = applTestDir + "/tmp/mount.xyz" + sysConf + "/skyscraper";
        createTestDir(exp);
        QCOMPARE(cfg.validateConfigSrcPath(sysConf, prefix, app), exp);
    }

    void testValidateConfigSrcPathManualInst() {
        QString sysConf = "/tmp/usr/local/etc";
        QString prefix = "/tmp/usr/local";
        QString app = applTestDir + "/some/where/bin";
        // mimic manual deploy of Skyscraper bin only, do not create SYSCONFDIR
        // and PREFIX
        createTestDir(app);
        QCOMPARE(cfg.validateConfigSrcPath(sysConf, prefix, app), "");
    }

    void cleanup() {
        for (auto const &d : dirsCreated)
            QDir().rmpath(d);
    }

    void createTestDir(QString dirPath) {
        QDir().mkpath(dirPath);
        dirsCreated.append(dirPath);
    }
};

QTEST_MAIN(TestConfig)
#include "test_config.moc"