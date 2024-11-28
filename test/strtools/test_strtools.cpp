#include "strtools.h"

#include <QTest>

class TestNameTools : public QObject {
    Q_OBJECT

private:
    QMap<QString, QPair<QString, QString>> tidyDescData;

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

QTEST_MAIN(StrTools)
#include "test_strtools.moc"