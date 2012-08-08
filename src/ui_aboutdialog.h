/********************************************************************************
** Form generated from reading UI file 'aboutdialog.ui'
**
** Created: Wed 8. Aug 18:17:25 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QLabel *labelVersion;
    QLabel *labelTimestamp;
    QLabel *label_5;
    QLabel *label_6;
    QFrame *line;
    QLabel *labelSystem;
    QTextEdit *textEdit;
    QLabel *labelLicense;
    QLabel *label_2;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(519, 503);
        buttonBox = new QDialogButtonBox(AboutDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 460, 471, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);
        label = new QLabel(AboutDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 50, 101, 101));
        label->setPixmap(QPixmap(QString::fromUtf8(":/icons/icons/sonicawe.png")));
        labelVersion = new QLabel(AboutDialog);
        labelVersion->setObjectName(QString::fromUtf8("labelVersion"));
        labelVersion->setGeometry(QRect(130, 15, 361, 61));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        labelVersion->setFont(font);
        labelVersion->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        labelVersion->setWordWrap(true);
        labelTimestamp = new QLabel(AboutDialog);
        labelTimestamp->setObjectName(QString::fromUtf8("labelTimestamp"));
        labelTimestamp->setGeometry(QRect(130, 191, 381, 21));
        labelTimestamp->setWordWrap(true);
        label_5 = new QLabel(AboutDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(130, 215, 381, 20));
        label_6 = new QLabel(AboutDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(130, 236, 361, 71));
        label_6->setWordWrap(true);
        line = new QFrame(AboutDialog);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(130, 300, 371, 21));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        labelSystem = new QLabel(AboutDialog);
        labelSystem->setObjectName(QString::fromUtf8("labelSystem"));
        labelSystem->setGeometry(QRect(130, 330, 371, 161));
        textEdit = new QTextEdit(AboutDialog);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(127, 80, 380, 111));
        textEdit->setFrameShape(QFrame::NoFrame);
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textEdit->setUndoRedoEnabled(false);
        textEdit->setReadOnly(true);
        textEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        labelLicense = new QLabel(AboutDialog);
        labelLicense->setObjectName(QString::fromUtf8("labelLicense"));
        labelLicense->setGeometry(QRect(130, 44, 361, 61));
        labelLicense->setFont(font);
        labelLicense->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        labelLicense->setWordWrap(true);
        label_2 = new QLabel(AboutDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(0, 140, 121, 61));
        labelSystem->raise();
        buttonBox->raise();
        label->raise();
        labelVersion->raise();
        labelTimestamp->raise();
        label_5->raise();
        label_6->raise();
        line->raise();
        textEdit->raise();
        labelLicense->raise();
        label_2->raise();

        retranslateUi(AboutDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AboutDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AboutDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Sonic AWE", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        labelVersion->setText(QApplication::translate("AboutDialog", "Sonic AWE version name", 0, QApplication::UnicodeUTF8));
        labelTimestamp->setText(QApplication::translate("AboutDialog", "Build on DATE at TIME from revision REVISION.", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("AboutDialog", "Copyright 2009-2011 MuchDifferent. All rights reserved.", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("AboutDialog", "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.", 0, QApplication::UnicodeUTF8));
        labelSystem->setText(QApplication::translate("AboutDialog", "System information", 0, QApplication::UnicodeUTF8));
        textEdit->setHtml(QApplication::translate("AboutDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<table border=\"0\" style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">\n"
"<tr>\n"
"<td style=\"border: none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Sonic AWE is built using the following libraries (license within parenthesis where required); <a href=\"http://qt.nokia.com/\"><span style=\" text-decoration: underline; color:#0000ff;\">Qt4</span></a> (<a href=\"http://www.gnu.org/licenses/lgpl-2.1.html\"><span style=\" text-decoration: underline; color:#0000ff;\">LGPL 2.1</span></a>), <a href=\"http://www.nvidia.com/object/cuda_home.html\"><span st"
                        "yle=\" text-decoration: underline; color:#0000ff;\">Cuda</span></a>, <a href=\"http://www.mega-nerd.com/libsndfile/\"><span style=\" text-decoration: underline; color:#0000ff;\">libsndfile</span></a> (<a href=\"http://www.gnu.org/copyleft/lesser.html\"><span style=\" text-decoration: underline; color:#0000ff;\">LGPL</span></a>), <a href=\"http://www.boost.org/\"><span style=\" text-decoration: underline; color:#0000ff;\">boost</span></a>, <a href=\"http://www.portaudio.com/\"><span style=\" text-decoration: underline; color:#0000ff;\">portaudio</span></a> (<a href=\"http://www.portaudio.com/license.html\"><span style=\" text-decoration: underline; color:#0000ff;\">license</span></a>), <a href=\"http://freeglut.sourceforge.net/\"><span style=\" text-decoration: underline; color:#0000ff;\">freeglut</span></a> (<a href=\"http://www.xfree86.org/3.3.6/COPYRIGHT2.html#3\"><span style=\" text-decoration: underline; color:#0000ff;\">X Consortium</span></a>), <a href=\"http://glew.sourceforge.net/\"><span style=\" text"
                        "-decoration: underline; color:#0000ff;\">glew</span></a> (<a href=\"http://glew.sourceforge.net/glew.txt\"><span style=\" text-decoration: underline; color:#0000ff;\">BSD style</span></a>), <a href=\"http://www.hdfgroup.org/HDF5/\"><span style=\" text-decoration: underline; color:#0000ff;\">hdf5</span></a> (<a href=\"http://www.hdfgroup.org/HDF5/doc/Copyright.html\"><span style=\" text-decoration: underline; color:#0000ff;\">license</span></a>), <a href=\"http://www.zlib.net/\"><span style=\" text-decoration: underline; color:#0000ff;\">zlib</span></a> and <a href=\"http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html\"><span style=\" text-decoration: underline; color:#0000ff;\">oourafft</span></a>. Also see <a href=\"file:///usr/share/sonicawe/license/license.txt\"><span style=\" text-decoration: underline; color:#0000ff;\">license.txt</span></a> included in the release of Sonic AWE.</p></td></tr></table></body></html>", 0, QApplication::UnicodeUTF8));
        labelLicense->setText(QApplication::translate("AboutDialog", "Sonic AWE version name", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AboutDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<table border=\"0\" style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">\n"
"<tr>\n"
"<td style=\"border: none;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">SONIC AWE</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt;\">Visualization based</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text"
                        "-indent:0px;\"><span style=\" font-size:9pt;\">signal analysis</span></p></td></tr></table></body></html>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
