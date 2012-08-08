/********************************************************************************
** Form generated from reading UI file 'enterlicense.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ENTERLICENSE_H
#define UI_ENTERLICENSE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_EnterLicense
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *EnterLicense)
    {
        if (EnterLicense->objectName().isEmpty())
            EnterLicense->setObjectName(QString::fromUtf8("EnterLicense"));
        EnterLicense->resize(551, 101);
        gridLayout = new QGridLayout(EnterLicense);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(EnterLicense);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEdit = new QLineEdit(EnterLicense);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        gridLayout->addWidget(lineEdit, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(EnterLicense);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(false);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 1);


        retranslateUi(EnterLicense);
        QObject::connect(buttonBox, SIGNAL(accepted()), EnterLicense, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), EnterLicense, SLOT(reject()));

        QMetaObject::connectSlotsByName(EnterLicense);
    } // setupUi

    void retranslateUi(QDialog *EnterLicense)
    {
        EnterLicense->setWindowTitle(QApplication::translate("EnterLicense", "Sonic AWE", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("EnterLicense", "Please enter the license key you recieved from us at muchdifferent.com.", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class EnterLicense: public Ui_EnterLicense {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ENTERLICENSE_H
