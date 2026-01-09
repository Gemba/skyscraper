#include "pegasus.h"

#include <QDebug>
#include <QTest>

class TestPegasus : public QObject {
    Q_OBJECT

private:
    Settings settings;
    Pegasus *frontend;

private slots:
    void initTestCase(){};

    void testReplaceColon() {
        frontend = new Pegasus();
        QString in = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                     "sed do eiusmod tempor incididunt ut labore et dolore "
                     "magna aliqua: Ut enim ad minim veniam.";
        QString exp = QString(in).replace(":", QChar(0xa789));
        frontend->replaceColon(in, "Didelum");
        QCOMPARE(in, exp);
    }
};

QTEST_MAIN(TestPegasus)
#include "test_pegasus.moc"
