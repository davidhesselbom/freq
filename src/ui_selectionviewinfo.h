/********************************************************************************
** Form generated from reading UI file 'selectionviewinfo.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTIONVIEWINFO_H
#define UI_SELECTIONVIEWINFO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

namespace Tools {

class Ui_SelectionViewInfo
{
public:
    QAction *actionSelection_Info;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    QTextEdit *textEdit;

    void setupUi(QDockWidget *Tools__SelectionViewInfo)
    {
        if (Tools__SelectionViewInfo->objectName().isEmpty())
            Tools__SelectionViewInfo->setObjectName(QString::fromUtf8("Tools__SelectionViewInfo"));
        Tools__SelectionViewInfo->resize(400, 300);
        actionSelection_Info = new QAction(Tools__SelectionViewInfo);
        actionSelection_Info->setObjectName(QString::fromUtf8("actionSelection_Info"));
        actionSelection_Info->setCheckable(true);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout = new QGridLayout(dockWidgetContents);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textEdit = new QTextEdit(dockWidgetContents);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        gridLayout->addWidget(textEdit, 0, 0, 1, 1);

        Tools__SelectionViewInfo->setWidget(dockWidgetContents);

        retranslateUi(Tools__SelectionViewInfo);

        QMetaObject::connectSlotsByName(Tools__SelectionViewInfo);
    } // setupUi

    void retranslateUi(QDockWidget *Tools__SelectionViewInfo)
    {
        Tools__SelectionViewInfo->setWindowTitle(QApplication::translate("Tools::SelectionViewInfo", "Selection Info", 0, QApplication::UnicodeUTF8));
        actionSelection_Info->setText(QApplication::translate("Tools::SelectionViewInfo", "Selection &Info", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSelection_Info->setToolTip(QApplication::translate("Tools::SelectionViewInfo", "Show information about the current selection", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSelection_Info->setShortcut(QApplication::translate("Tools::SelectionViewInfo", "Ctrl+Shift+I", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class SelectionViewInfo: public Ui_SelectionViewInfo {};
} // namespace Ui
} // namespace Tools

#endif // UI_SELECTIONVIEWINFO_H
