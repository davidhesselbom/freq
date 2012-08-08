/********************************************************************************
** Form generated from reading UI file 'sendfeedback.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SENDFEEDBACK_H
#define UI_SENDFEEDBACK_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

namespace Tools {

class Ui_SendFeedback
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    QTextEdit *textEditMessage;
    QLineEdit *lineEditEmail;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEditAttachFile;
    QPushButton *pushButtonBrowse;
    QLabel *label_4;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Tools__SendFeedback)
    {
        if (Tools__SendFeedback->objectName().isEmpty())
            Tools__SendFeedback->setObjectName(QString::fromUtf8("Tools__SendFeedback"));
        Tools__SendFeedback->resize(538, 361);
        gridLayout = new QGridLayout(Tools__SendFeedback);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        label = new QLabel(Tools__SendFeedback);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        gridLayout->addWidget(label, 1, 1, 1, 1);

        label_2 = new QLabel(Tools__SendFeedback);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(true);

        gridLayout->addWidget(label_2, 3, 1, 1, 1);

        textEditMessage = new QTextEdit(Tools__SendFeedback);
        textEditMessage->setObjectName(QString::fromUtf8("textEditMessage"));
        textEditMessage->setAcceptRichText(false);

        gridLayout->addWidget(textEditMessage, 5, 1, 1, 2);

        lineEditEmail = new QLineEdit(Tools__SendFeedback);
        lineEditEmail->setObjectName(QString::fromUtf8("lineEditEmail"));

        gridLayout->addWidget(lineEditEmail, 2, 1, 1, 1);

        label_3 = new QLabel(Tools__SendFeedback);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        gridLayout->addWidget(label_3, 0, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lineEditAttachFile = new QLineEdit(Tools__SendFeedback);
        lineEditAttachFile->setObjectName(QString::fromUtf8("lineEditAttachFile"));

        horizontalLayout->addWidget(lineEditAttachFile);

        pushButtonBrowse = new QPushButton(Tools__SendFeedback);
        pushButtonBrowse->setObjectName(QString::fromUtf8("pushButtonBrowse"));

        horizontalLayout->addWidget(pushButtonBrowse);


        gridLayout->addLayout(horizontalLayout, 8, 1, 1, 1);

        label_4 = new QLabel(Tools__SendFeedback);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setWordWrap(true);

        gridLayout->addWidget(label_4, 6, 1, 1, 1);

        buttonBox = new QDialogButtonBox(Tools__SendFeedback);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 9, 1, 1, 2);


        retranslateUi(Tools__SendFeedback);
        QObject::connect(buttonBox, SIGNAL(accepted()), Tools__SendFeedback, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Tools__SendFeedback, SLOT(reject()));

        QMetaObject::connectSlotsByName(Tools__SendFeedback);
    } // setupUi

    void retranslateUi(QDialog *Tools__SendFeedback)
    {
        Tools__SendFeedback->setWindowTitle(QApplication::translate("Tools::SendFeedback", "Send feedback", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Tools::SendFeedback", "Email address:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Tools::SendFeedback", "Please leave your message here:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Tools::SendFeedback", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<table border=\"0\" style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">\n"
"<tr>\n"
"<td style=\"border: none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">You are very welcome to send feedback or report bugs with this form. You can also report bugs directly at <a href=\"http://bugs.muchdifferent.com\"><span style=\" text-decoration: underline; color:#0000ff;\">bugs.muchdifferent.com</span></a>.</p></td></tr></table></body></html>", 0, QApplication::UnicodeUTF8));
        pushButtonBrowse->setText(QApplication::translate("Tools::SendFeedback", "&Browse", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Tools::SendFeedback", "Attach file (Sonic AWE logs will be included automatically):", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class SendFeedback: public Ui_SendFeedback {};
} // namespace Ui
} // namespace Tools

#endif // UI_SENDFEEDBACK_H
