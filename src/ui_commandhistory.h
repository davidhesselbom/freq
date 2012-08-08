/********************************************************************************
** Form generated from reading UI file 'commandhistory.ui'
**
** Created: Wed 8. Aug 18:17:25 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMANDHISTORY_H
#define UI_COMMANDHISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QWidget>

namespace Tools {
namespace Commands {

class Ui_CommandHistory
{
public:
    QGridLayout *gridLayout;
    QListWidget *listWidget;

    void setupUi(QWidget *Tools__Commands__CommandHistory)
    {
        if (Tools__Commands__CommandHistory->objectName().isEmpty())
            Tools__Commands__CommandHistory->setObjectName(QString::fromUtf8("Tools__Commands__CommandHistory"));
        Tools__Commands__CommandHistory->resize(400, 300);
        gridLayout = new QGridLayout(Tools__Commands__CommandHistory);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        listWidget = new QListWidget(Tools__Commands__CommandHistory);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        gridLayout->addWidget(listWidget, 0, 0, 1, 1);


        retranslateUi(Tools__Commands__CommandHistory);

        QMetaObject::connectSlotsByName(Tools__Commands__CommandHistory);
    } // setupUi

    void retranslateUi(QWidget *Tools__Commands__CommandHistory)
    {
        Tools__Commands__CommandHistory->setWindowTitle(QApplication::translate("Tools::Commands::CommandHistory", "History", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Commands
} // namespace Tools

namespace Tools {
namespace Commands {
namespace Ui {
    class CommandHistory: public Ui_CommandHistory {};
} // namespace Ui
} // namespace Commands
} // namespace Tools

#endif // UI_COMMANDHISTORY_H
