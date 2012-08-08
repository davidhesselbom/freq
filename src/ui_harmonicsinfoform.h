/********************************************************************************
** Form generated from reading UI file 'harmonicsinfoform.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HARMONICSINFOFORM_H
#define UI_HARMONICSINFOFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HarmonicsInfoForm
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTableWidget *tableWidget;

    void setupUi(QWidget *HarmonicsInfoForm)
    {
        if (HarmonicsInfoForm->objectName().isEmpty())
            HarmonicsInfoForm->setObjectName(QString::fromUtf8("HarmonicsInfoForm"));
        HarmonicsInfoForm->resize(400, 300);
        verticalLayout = new QVBoxLayout(HarmonicsInfoForm);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(HarmonicsInfoForm);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        verticalLayout->addWidget(label);

        tableWidget = new QTableWidget(HarmonicsInfoForm);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));

        verticalLayout->addWidget(tableWidget);


        retranslateUi(HarmonicsInfoForm);

        QMetaObject::connectSlotsByName(HarmonicsInfoForm);
    } // setupUi

    void retranslateUi(QWidget *HarmonicsInfoForm)
    {
        HarmonicsInfoForm->setWindowTitle(QApplication::translate("HarmonicsInfoForm", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("HarmonicsInfoForm", "Harmonics, select row to center camera. Right click on a marker to delete.", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HarmonicsInfoForm: public Ui_HarmonicsInfoForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HARMONICSINFOFORM_H
