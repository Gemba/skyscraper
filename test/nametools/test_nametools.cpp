#include "nametools.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTest>

class TestNameTools : public QObject {
    Q_OBJECT

private:
    QMap<QString, QString> romanArabicMap;

private slots:

    void initTestCase() {
        romanArabicMap = {
            {"1", "I"},     {"2", "II"},     {"3", "III"},  {"4", "IV"},
            {"5", "V"},     {"6", "VI"},     {"7", "VII"},  {"8", "VIII"},
            {"9", "IX"},    {"10", "X"},     {"11", "XI"},  {"12", "XII"},
            {"13", "XIII"}, {"14", "XIV"},   {"15", "XV"},  {"16", "XVI"},
            {"17", "XVII"}, {"18", "XVIII"}, {"19", "XIX"}, {"20", "XX"}};
    }

    void testConvertToRomanNumeral() {
        QString bn = "Yadda yadda _";
        QMapIterator<QString, QString> i(romanArabicMap);

        while (i.hasNext()) {
            i.next();
            QString base = QString(bn).replace("_", i.key());
            QString exp = QString(bn).replace("_", i.value());
            QCOMPARE(NameTools::convertToRomanNumeral(base), exp);
        }
        QCOMPARE(NameTools::convertToRomanNumeral("Blarf 25"), "Blarf 25");
    }

    void testConvertToArabicNumeral() {
        QString bn = "Yadda yadda _ Yabbadabba";
        QMapIterator<QString, QString> i(romanArabicMap);

        while (i.hasNext()) {
            i.next();
            QString base = QString(bn).replace("_", i.value());
            QString exp = QString(bn).replace("_", i.key());
            QCOMPARE(NameTools::convertToArabicNumeral(base), exp);
        }
        QCOMPARE(NameTools::convertToArabicNumeral("Blarf XXV"), "Blarf XXV");
    }

    void testGetNumeral() {
        QString bn = "Yadda yadda _: Some text";
        QMapIterator<QString, QString> i(romanArabicMap);

        while (i.hasNext()) {
            i.next();
            QString base = QString(bn).replace("_", i.value());
            QCOMPARE(NameTools::getNumeral(base), i.key().toInt());
        }
        QCOMPARE(NameTools::getNumeral("Blarf XXV"), 1);

        i.toFront();
        while (i.hasNext()) {
            i.next();
            QString base = QString(bn).replace("_", i.key());
            QCOMPARE(NameTools::getNumeral(base), i.key().toInt());
        }
        QCOMPARE(NameTools::getNumeral("Blarf 0"), 1);
        QCOMPARE(NameTools::getNumeral("Blarf -1"), 1);
    }

    void testGetSqrNotes() {
        QCOMPARE(NameTools::getSqrNotes(
                     "Blarf _Aga_ text _Cd32_Ntsc texttext [any] end"),
                 "[any][AGA][CD32][NTSC]");
        QCOMPARE(NameTools::getSqrNotes(
                     "Blarf _Aga_ text _Cd32 [Aga] texttext [any] end"),
                 "[Aga][any][AGA][CD32]");
        QCOMPARE(NameTools::getSqrNotes("Blarf end"), "");
    }

    void testGetParNotes() {
        QCOMPARE(NameTools::getParNotes("Texttext 4Disk (2024) _Dk_"),
                 "(2024)(4 Disk)(Dk)");
    }

    void testGetUniqueNote() {
        QCOMPARE(NameTools::getUniqueNotes("[aa][bb][AA]", '['), "[aa][bb]");
        QCOMPARE(NameTools::getUniqueNotes("(ee)(FF)(ff)(DD)", '('),
                 "(ee)(FF)(DD)");
        QCOMPARE(NameTools::getUniqueNotes("", '('), "");
        QCOMPARE(NameTools::getUniqueNotes("(2024)", '('), "(2024)");
    }

    void testGetScummName() {
        QString ini = "./scummvm.ini";

        QMap<QString, QPair<QString, QString>> tests = {
            {"scummvm tentacle",
             QPair<QString, QString>(
                 "tentacle", "Day of the Tentacle (Floppy/DOS/English)")},
            {"scummvm dontexists",
             QPair<QString, QString>("dontexists", "dontexists")},
            {"scummvm dig", QPair<QString, QString>("dig", "The Dig")},
            {"scummvm use game id from ROM file",
             QPair<QString, QString>("lba", "Little Big Adventure")},
            {"scummvm no valid gameid",
             QPair<QString, QString>("Some Mighty Adventure",
                                     "Some Mighty Adventure")},
            {"scummvm no gameid",
             QPair<QString, QString>("No game id in romfile",
                                     "No game id in romfile")}

        };

        QFile romfile("rom_samples/The Dig (1995).scummvm");

        for (auto t : tests.keys()) {
            if (t == "scummvm no gameid") {
                QFile romfile("rom_samples/Some Mighty Adventure.svm");
            } else if (t == "scummvm no gameid") {
                QFile romfile("rom_samples/No game id in romfile.svm");
            }
            QString baseName = tests.value(t).first;
            QString exp = tests.value(t).second;
            QString out =
                NameTools::getScummName(QFileInfo(romfile), baseName, ini);
            QCOMPARE(out, exp);
        }
    }
};

QTEST_MAIN(TestNameTools)
#include "test_nametools.moc"
