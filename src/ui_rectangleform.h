/********************************************************************************
** Form generated from reading UI file 'rectangleform.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECTANGLEFORM_H
#define UI_RECTANGLEFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

namespace Tools {
namespace Selections {

class Ui_RectangleForm
{
public:
    QGridLayout *gridLayout;
    QDoubleSpinBox *spinBoxStartTime;
    QDoubleSpinBox *spinBoxStopTime;
    QLabel *label_5;
    QLabel *label_6;
    QDoubleSpinBox *spinBoxStartFrequency;
    QDoubleSpinBox *spinBoxStopFrequency;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *Tools__Selections__RectangleForm)
    {
        if (Tools__Selections__RectangleForm->objectName().isEmpty())
            Tools__Selections__RectangleForm->setObjectName(QString::fromUtf8("Tools__Selections__RectangleForm"));
        Tools__Selections__RectangleForm->resize(516, 379);
        gridLayout = new QGridLayout(Tools__Selections__RectangleForm);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        spinBoxStartTime = new QDoubleSpinBox(Tools__Selections__RectangleForm);
        spinBoxStartTime->setObjectName(QString::fromUtf8("spinBoxStartTime"));
        spinBoxStartTime->setDecimals(5);
        spinBoxStartTime->setMaximum(2.14748e+09);

        gridLayout->addWidget(spinBoxStartTime, 2, 1, 1, 1);

        spinBoxStopTime = new QDoubleSpinBox(Tools__Selections__RectangleForm);
        spinBoxStopTime->setObjectName(QString::fromUtf8("spinBoxStopTime"));
        spinBoxStopTime->setDecimals(5);
        spinBoxStopTime->setMaximum(2.14748e+09);

        gridLayout->addWidget(spinBoxStopTime, 4, 1, 1, 1);

        label_5 = new QLabel(Tools__Selections__RectangleForm);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 1, 1, 1, 1);

        label_6 = new QLabel(Tools__Selections__RectangleForm);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 1, 2, 1, 1);

        spinBoxStartFrequency = new QDoubleSpinBox(Tools__Selections__RectangleForm);
        spinBoxStartFrequency->setObjectName(QString::fromUtf8("spinBoxStartFrequency"));
        spinBoxStartFrequency->setDecimals(4);

        gridLayout->addWidget(spinBoxStartFrequency, 2, 2, 1, 1);

        spinBoxStopFrequency = new QDoubleSpinBox(Tools__Selections__RectangleForm);
        spinBoxStopFrequency->setObjectName(QString::fromUtf8("spinBoxStopFrequency"));
        spinBoxStopFrequency->setDecimals(4);

        gridLayout->addWidget(spinBoxStopFrequency, 4, 2, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 5, 1, 1, 1);


        retranslateUi(Tools__Selections__RectangleForm);

        QMetaObject::connectSlotsByName(Tools__Selections__RectangleForm);
    } // setupUi

    void retranslateUi(QWidget *Tools__Selections__RectangleForm)
    {
        Tools__Selections__RectangleForm->setWindowTitle(QApplication::translate("Tools::Selections::RectangleForm", "Selection", 0, QApplication::UnicodeUTF8));
        spinBoxStartTime->setSuffix(QApplication::translate("Tools::Selections::RectangleForm", " s", 0, QApplication::UnicodeUTF8));
        spinBoxStopTime->setSuffix(QApplication::translate("Tools::Selections::RectangleForm", " s", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Tools::Selections::RectangleForm", "Time (s)", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Tools::Selections::RectangleForm", "Frequency (Hz)", 0, QApplication::UnicodeUTF8));
        spinBoxStartFrequency->setSuffix(QApplication::translate("Tools::Selections::RectangleForm", " hz", 0, QApplication::UnicodeUTF8));
        spinBoxStopFrequency->setSuffix(QApplication::translate("Tools::Selections::RectangleForm", " hz", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Selections
} // namespace Tools

namespace Tools {
namespace Selections {
namespace Ui {
    class RectangleForm: public Ui_RectangleForm {};
} // namespace Ui
} // namespace Selections
} // namespace Tools

#endif // UI_RECTANGLEFORM_H
