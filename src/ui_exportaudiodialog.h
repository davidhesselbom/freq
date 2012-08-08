/********************************************************************************
** Form generated from reading UI file 'exportaudiodialog.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORTAUDIODIALOG_H
#define UI_EXPORTAUDIODIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>

QT_BEGIN_NAMESPACE

class Ui_ExportAudioDialog
{
public:
    QDialogButtonBox *buttonBoxAbort;
    QLabel *labelExporting;
    QDialogButtonBox *buttonBoxOk;
    QProgressBar *progressBar;
    QCheckBox *checkBoxNormalize;

    void setupUi(QDialog *ExportAudioDialog)
    {
        if (ExportAudioDialog->objectName().isEmpty())
            ExportAudioDialog->setObjectName(QString::fromUtf8("ExportAudioDialog"));
        ExportAudioDialog->resize(400, 148);
        buttonBoxAbort = new QDialogButtonBox(ExportAudioDialog);
        buttonBoxAbort->setObjectName(QString::fromUtf8("buttonBoxAbort"));
        buttonBoxAbort->setGeometry(QRect(190, 110, 101, 31));
        buttonBoxAbort->setOrientation(Qt::Horizontal);
        buttonBoxAbort->setStandardButtons(QDialogButtonBox::Abort);
        labelExporting = new QLabel(ExportAudioDialog);
        labelExporting->setObjectName(QString::fromUtf8("labelExporting"));
        labelExporting->setGeometry(QRect(10, 0, 381, 81));
        labelExporting->setWordWrap(true);
        buttonBoxOk = new QDialogButtonBox(ExportAudioDialog);
        buttonBoxOk->setObjectName(QString::fromUtf8("buttonBoxOk"));
        buttonBoxOk->setEnabled(false);
        buttonBoxOk->setGeometry(QRect(300, 110, 91, 31));
        buttonBoxOk->setStandardButtons(QDialogButtonBox::Ok);
        progressBar = new QProgressBar(ExportAudioDialog);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(20, 80, 371, 23));
        progressBar->setValue(0);
        checkBoxNormalize = new QCheckBox(ExportAudioDialog);
        checkBoxNormalize->setObjectName(QString::fromUtf8("checkBoxNormalize"));
        checkBoxNormalize->setGeometry(QRect(18, 116, 101, 20));

        retranslateUi(ExportAudioDialog);
        QObject::connect(buttonBoxAbort, SIGNAL(accepted()), ExportAudioDialog, SLOT(accept()));
        QObject::connect(buttonBoxAbort, SIGNAL(rejected()), ExportAudioDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ExportAudioDialog);
    } // setupUi

    void retranslateUi(QDialog *ExportAudioDialog)
    {
        ExportAudioDialog->setWindowTitle(QApplication::translate("ExportAudioDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        labelExporting->setText(QApplication::translate("ExportAudioDialog", "Exporting... please wait", 0, QApplication::UnicodeUTF8));
        checkBoxNormalize->setText(QApplication::translate("ExportAudioDialog", "Normalize", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ExportAudioDialog: public Ui_ExportAudioDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORTAUDIODIALOG_H
