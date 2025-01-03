#include "strtools.h"

#include <QStringBuilder>
#include <QTest>

class TestStrTools : public QObject {
    Q_OBJECT

private:
    QMap<QString, QPair<QString, QString>> tidyDescData;
    QMap<QString, QString> xmlMap;
    QMap<QString, QString> ageMap;
    QMap<QString, QString> playerMap;
    QMap<QString, QString> rdMap;

private slots:
    void initTestCase() {

        tidyDescData = {
            {"do not multbang",
             QPair<QString, QString>(
                 "I am sentence one!!  I am sentence two.\nI "
                 "am sentence three!!!!!!!!!",
                 "I am sentence one!! I am sentence two.\nI am "
                 "sentence three!!!!!!!!!")},
            {"itemize bullet",
             QPair<QString, QString>("bla bla\n  ●  item 1\n* item2\nfoo "
                                     "bar baz\n* item 1b\n●  item "
                                     "2b",
                                     "bla bla\n- item 1\n- item2\nfoo bar "
                                     "baz\n- item 1b\n- item 2b")},
            {"itemize star 2",
             QPair<QString, QString>(
                 "bla bla\n  *  item 1\n* item2\nfoo bar baz\n* "
                 "item 1b\n*  item 2b\n  *  item 3b  \n\n\nbaz",
                 "bla bla\n- item 1\n- item2\nfoo bar baz\n- "
                 "item 1b\n- item 2b\n- item 3b\n\n\nbaz")},
            {"itemize star",
             QPair<QString, QString>("bla bla\n  *  item 1\n* item2\nfoo "
                                     "bar baz\n* item 1b\n*  item "
                                     "2b",
                                     "bla bla\n- item 1\n- item2\nfoo bar "
                                     "baz\n- item 1b\n- item 2b")},
            {"multbang",
             QPair<QString, QString>(
                 "I am sentence one!!  I am sentence two.\nI am sentence "
                 "three!!!!!!!!!",
                 "I am sentence one! I am sentence two.\nI am sentence "
                 "three!")},
            {"multispaces", QPair<QString, QString>("bla bla!    \n\n\nblubb",
                                                    "bla bla!\n\n\nblubb")},
            {"replace ellipsis",
             QPair<QString, QString>("bla …bla  \n  …efefef efefef…\n  \n  ",
                                     "bla ...bla\n...efefef efefef...\n\n")},
            {"sentence spacing",
             QPair<QString, QString>("I am sentence one.  I am sentence "
                                     "two.\nI am sentence three.",
                                     "I am sentence one. I am sentence "
                                     "two.\nI am sentence three.")},
            {"single star 2",
             QPair<QString, QString>("bla bla  \n  *  efefef efefef\nefef",
                                     "bla bla\n*  efefef efefef\nefef")},
            {"single star",
             QPair<QString, QString>("bla bla  \n  *  efefef efefef",
                                     "bla bla\n*  efefef efefef")},
            {"trim lines",
             QPair<QString, QString>("bla bla  \n  efefef efefef\n  \n  ",
                                     "bla bla\nefefef efefef\n\n")},
        };
    }

    void testXmlUnescape() {
        xmlMap = {{"&amp;", "&"},    {"&lt;", "<"},    {"&gt;", ">"},
                  {"&quot;", "\""},  {"&apos;", "'"},  {"&copy;", "(c)"},
                  {"&#32;", " "},    {"&#33;", "!"},   {"&#34;", "\""},
                  {"&#35;", "#"},    {"&#36;", "$"},   {"&#37;", "%"},
                  {"&#38;", "&"},    {"&#39;", "'"},   {"&#40;", "("},
                  {"&#41;", ")"},    {"&#42;", "*"},   {"&#43;", "+"},
                  {"&#44;", ","},    {"&#45;", "-"},   {"&#46;", "."},
                  {"&#47;", "/"},    {"&#032;", " "},  {"&#033;", "!"},
                  {"&#034;", "\""},  {"&#035;", "#"},  {"&#036;", "$"},
                  {"&#037;", "%"},   {"&#038;", "&"},  {"&#039;", "'"},
                  {"&#040;", "("},   {"&#041;", ")"},  {"&#042;", "*"},
                  {"&#043;", "+"},   {"&#044;", ","},  {"&#045;", "-"},
                  {"&#046;", "."},   {"&#047;", "/"},  {"&#160;", " "},
                  {"&#179;", "3"},   {"&#8211;", "-"}, {"&#8217;", "'"},
                  {"&#xF4;", "o"},   {"&#xE3;", "a"},  {"&#xE4;", "ae"},
                  {"&#xE1;", "a"},   {"&#xE9;", "e"},  {"&#xED;", "i"},
                  {"&#x16B;", "uu"}, {"&#x22;", "\""}, {"&#x26;", "&"},
                  {"&#x27;", "'"},   {"&#xB3;", "3"},  {"&#x14D;", "o"}};
        QMapIterator<QString, QString> i(xmlMap);
        while (i.hasNext()) {
            i.next();
            QString xmlIn = i.key();
            QString in = "Pre '" % xmlIn % "' Post";
            QString exp = "Pre '" % i.value() % "' Post";
            QCOMPARE(StrTools::xmlUnescape(in), exp);
        }
        QCOMPARE(StrTools::xmlUnescape("bla &ent; baz"), "bla  baz");
        QCOMPARE(StrTools::xmlUnescape("bla &verylongentity; baz"),
                 "bla &verylongentity; baz");
    }

    void testConformAges() {
        ageMap = {{"0 (ohne Altersbeschränkung)", "1"},
                  {"U", "1"},
                  {"E", "1"},
                  {"E - Everyone", "1"},
                  {"Everyone", "1"},
                  {"GA", "1"},
                  {"EC", "3"},
                  {"Early Childhood", "3"},
                  {"3+", "3"},
                  {"G", "3"},
                  {"KA", "6"},
                  {"Kids to Adults", "6"},
                  {"G8+", "8"},
                  {"E10+", "10"},
                  {"E10+ - Everyone 10+", "10"},
                  {"Everyone 10+", "10"},
                  {"11+", "11"},
                  {"12+", "11"},
                  {"MA-13", "13"},
                  {"T", "13"},
                  {"T - Teen", "13"},
                  {"Teen", "13"},
                  {"M", "15"},
                  {"M15+", "15"},
                  {"MA 15+", "15"},
                  {"MA15+", "15"},
                  {"PG", "15"},
                  {"15+", "15"},
                  {"MA-17", "17"},
                  //{"M","17"}, /* dupe */
                  {"18+", "18"},
                  {"R18+", "18"},
                  {"18 (keine Jugendfreigabe)", "18"},
                  {"A", "18"},
                  {"AO", "18"},
                  {"AO - Adults Only", "18"},
                  {"Adults Only", "18"},
                  {"M - Mature", "18"},
                  {"Mature", "18"}};
        QMapIterator<QString, QString> i(ageMap);
        while (i.hasNext()) {
            i.next();
            QCOMPARE(StrTools::conformAges(i.key()), i.value());
        }
        QCOMPARE(StrTools::conformAges("23"), "23");
        QCOMPARE(StrTools::conformAges(""), "");
    }

    void testConformPlayers() {
        playerMap = {
            {"1 Player", "1"}, {"1 Only", "1"},   {"single player", "1"},
            {"1 or 2", "2"},   {"1-33", "33"},    {"0-5", "5"},
            {"2 - 44", "44"},  {"3 - 7", "7"},    {"4 -99", "99"},
            {"5 -9", "9"},     {"1 to 66", "66"}, {"1 to 8", "8"},
            {"4+", "4"},       {"235", "235"},    {"", ""}};
        QMapIterator<QString, QString> i(playerMap);
        while (i.hasNext()) {
            i.next();
            QCOMPARE(StrTools::conformPlayers(i.key()), i.value());
        }
    }

    void testConformReleaseDate() {
        rdMap = {{"1972", "19720101"},
                 {"1972-11", "19721101"},
                 {"1972-11-29", "19721129"},
                 {"11/29/1972", "19721129"},
                 {"1972-Nov-29", "19721129"},
                 {"Nov, 1972", "19721101"},
                 {"Nov 29, 1972", "19721129"},
                 {"19721129", "19721129"},
                 {"19721129T204933", "19721129"},
                 {"200?", "200?"},
                 {"20??", "20??"},
                 {"?", ""},
                 {"N/A", ""},
                 {"", ""}};
        QMapIterator<QString, QString> i(rdMap);
        while (i.hasNext()) {
            i.next();
            qDebug() << "Input: " << i.key();
            QCOMPARE(StrTools::conformReleaseDate(i.key()), i.value());
        }
    }

    void testTidyDesc() {
        QMapIterator<QString, QPair<QString, QString>> i(tidyDescData);
        while (i.hasNext()) {
            i.next();
            QString testName = i.key();
            QString input = i.value().first;
            QString exp = i.value().second;
            QString out = StrTools::tidyText(
                input, testName == "do not multbang" ? true : false);
            QCOMPARE(out, exp);
        }
    }
};

QTEST_MAIN(TestStrTools)
#include "test_strtools.moc"