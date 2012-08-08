/********************************************************************************
** Form generated from reading UI file 'propertiesstroke.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTIESSTROKE_H
#define UI_PROPERTIESSTROKE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

namespace Saweui {

class Ui_PropertiesStroke
{
public:
    QHBoxLayout *horizontalLayout;
    QToolButton *toolButton;
    QLabel *label;
    QToolButton *toolButton_2;

    void setupUi(QWidget *Saweui__PropertiesStroke)
    {
        if (Saweui__PropertiesStroke->objectName().isEmpty())
            Saweui__PropertiesStroke->setObjectName(QString::fromUtf8("Saweui__PropertiesStroke"));
        Saweui__PropertiesStroke->resize(400, 43);
        horizontalLayout = new QHBoxLayout(Saweui__PropertiesStroke);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        toolButton = new QToolButton(Saweui__PropertiesStroke);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));

        horizontalLayout->addWidget(toolButton);

        label = new QLabel(Saweui__PropertiesStroke);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        toolButton_2 = new QToolButton(Saweui__PropertiesStroke);
        toolButton_2->setObjectName(QString::fromUtf8("toolButton_2"));

        horizontalLayout->addWidget(toolButton_2);


        retranslateUi(Saweui__PropertiesStroke);

        QMetaObject::connectSlotsByName(Saweui__PropertiesStroke);
    } // setupUi

    void retranslateUi(QWidget *Saweui__PropertiesStroke)
    {
        Saweui__PropertiesStroke->setWindowTitle(QApplication::translate("Saweui::PropertiesStroke", "Form", 0, QApplication::UnicodeUTF8));
        toolButton->setText(QApplication::translate("Saweui::PropertiesStroke", "...", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Saweui::PropertiesStroke", "Settings for strokes, such as opacity and width", 0, QApplication::UnicodeUTF8));
        toolButton_2->setText(QApplication::translate("Saweui::PropertiesStroke", "...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Saweui

namespace Saweui {
namespace Ui {
    class PropertiesStroke: public Ui_PropertiesStroke {};
} // namespace Ui
} // namespace Saweui

#endif // UI_PROPERTIESSTROKE_H
