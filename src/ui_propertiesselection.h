/********************************************************************************
** Form generated from reading UI file 'propertiesselection.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTIESSELECTION_H
#define UI_PROPERTIESSELECTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

namespace Saweui {

class Ui_PropertiesSelection
{
public:
    QAction *actionCutoffSelection;
    QAction *actionPeakSelection;
    QAction *actionRectangleSelection;
    QAction *actionPolygonSelection;
    QAction *actionSplineSelection;
    QAction *actionActivateSelection;
    QHBoxLayout *horizontalLayout;
    QLabel *label;

    void setupUi(QWidget *Saweui__PropertiesSelection)
    {
        if (Saweui__PropertiesSelection->objectName().isEmpty())
            Saweui__PropertiesSelection->setObjectName(QString::fromUtf8("Saweui__PropertiesSelection"));
        Saweui__PropertiesSelection->resize(202, 43);
        actionCutoffSelection = new QAction(Saweui__PropertiesSelection);
        actionCutoffSelection->setObjectName(QString::fromUtf8("actionCutoffSelection"));
        actionCutoffSelection->setCheckable(true);
        actionCutoffSelection->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/icon-cutoff-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCutoffSelection->setIcon(icon);
        actionPeakSelection = new QAction(Saweui__PropertiesSelection);
        actionPeakSelection->setObjectName(QString::fromUtf8("actionPeakSelection"));
        actionPeakSelection->setCheckable(true);
        actionPeakSelection->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/icon-peak-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPeakSelection->setIcon(icon1);
        actionRectangleSelection = new QAction(Saweui__PropertiesSelection);
        actionRectangleSelection->setObjectName(QString::fromUtf8("actionRectangleSelection"));
        actionRectangleSelection->setCheckable(true);
        actionRectangleSelection->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/icon-rectangle-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRectangleSelection->setIcon(icon2);
        actionPolygonSelection = new QAction(Saweui__PropertiesSelection);
        actionPolygonSelection->setObjectName(QString::fromUtf8("actionPolygonSelection"));
        actionPolygonSelection->setCheckable(true);
        actionPolygonSelection->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/icon-quad-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPolygonSelection->setIcon(icon3);
        actionSplineSelection = new QAction(Saweui__PropertiesSelection);
        actionSplineSelection->setObjectName(QString::fromUtf8("actionSplineSelection"));
        actionSplineSelection->setCheckable(true);
        actionSplineSelection->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/icon-spline-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSplineSelection->setIcon(icon4);
        actionActivateSelection = new QAction(Saweui__PropertiesSelection);
        actionActivateSelection->setObjectName(QString::fromUtf8("actionActivateSelection"));
        actionActivateSelection->setCheckable(true);
        actionActivateSelection->setChecked(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/icons/icon-circle-selection.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon5.addFile(QString::fromUtf8(":/icons/icons/icon_select.png"), QSize(), QIcon::Normal, QIcon::On);
        actionActivateSelection->setIcon(icon5);
        actionActivateSelection->setIconVisibleInMenu(false);
        horizontalLayout = new QHBoxLayout(Saweui__PropertiesSelection);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(Saweui__PropertiesSelection);
        label->setObjectName(QString::fromUtf8("label"));
        label->setEnabled(false);
        label->setWordWrap(true);

        horizontalLayout->addWidget(label);


        retranslateUi(Saweui__PropertiesSelection);

        QMetaObject::connectSlotsByName(Saweui__PropertiesSelection);
    } // setupUi

    void retranslateUi(QWidget *Saweui__PropertiesSelection)
    {
        Saweui__PropertiesSelection->setWindowTitle(QApplication::translate("Saweui::PropertiesSelection", "Form", 0, QApplication::UnicodeUTF8));
        actionCutoffSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Cutoff selection", 0, QApplication::UnicodeUTF8));
        actionPeakSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Peak selection", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionPeakSelection->setToolTip(QApplication::translate("Saweui::PropertiesSelection", "Peak selection", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionRectangleSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Rectangle selection", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionRectangleSelection->setToolTip(QApplication::translate("Saweui::PropertiesSelection", "Rectangle selection", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionPolygonSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Polygon selection", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionPolygonSelection->setToolTip(QApplication::translate("Saweui::PropertiesSelection", "Polygon selection", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSplineSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Spline selection", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSplineSelection->setToolTip(QApplication::translate("Saweui::PropertiesSelection", "Spline selection", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionActivateSelection->setText(QApplication::translate("Saweui::PropertiesSelection", "Activate Selection", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionActivateSelection->setToolTip(QApplication::translate("Saweui::PropertiesSelection", "Activate selection [S]", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionActivateSelection->setShortcut(QApplication::translate("Saweui::PropertiesSelection", "S", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Saweui::PropertiesSelection", "Placeholder for tool settings", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Saweui

namespace Saweui {
namespace Ui {
    class PropertiesSelection: public Ui_PropertiesSelection {};
} // namespace Ui
} // namespace Saweui

#endif // UI_PROPERTIESSELECTION_H
