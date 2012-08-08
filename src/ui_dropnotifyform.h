/********************************************************************************
** Form generated from reading UI file 'dropnotifyform.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DROPNOTIFYFORM_H
#define UI_DROPNOTIFYFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

namespace Tools {

class Ui_DropNotifyForm
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *labelPluginImage;
    QLabel *labelInfoText;
    QPushButton *pushButtonReadMore;
    QPushButton *pushButtonClose;

    void setupUi(QWidget *Tools__DropNotifyForm)
    {
        if (Tools__DropNotifyForm->objectName().isEmpty())
            Tools__DropNotifyForm->setObjectName(QString::fromUtf8("Tools__DropNotifyForm"));
        Tools__DropNotifyForm->resize(496, 39);
        horizontalLayout = new QHBoxLayout(Tools__DropNotifyForm);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 4, -1, 4);
        labelPluginImage = new QLabel(Tools__DropNotifyForm);
        labelPluginImage->setObjectName(QString::fromUtf8("labelPluginImage"));
        labelPluginImage->setMaximumSize(QSize(33, 31));
        labelPluginImage->setPixmap(QPixmap(QString::fromUtf8(":/icons/icons/plugin.png")));
        labelPluginImage->setScaledContents(false);

        horizontalLayout->addWidget(labelPluginImage);

        labelInfoText = new QLabel(Tools__DropNotifyForm);
        labelInfoText->setObjectName(QString::fromUtf8("labelInfoText"));
        labelInfoText->setWordWrap(true);

        horizontalLayout->addWidget(labelInfoText);

        pushButtonReadMore = new QPushButton(Tools__DropNotifyForm);
        pushButtonReadMore->setObjectName(QString::fromUtf8("pushButtonReadMore"));
        pushButtonReadMore->setMaximumSize(QSize(90, 16777215));

        horizontalLayout->addWidget(pushButtonReadMore);

        pushButtonClose = new QPushButton(Tools__DropNotifyForm);
        pushButtonClose->setObjectName(QString::fromUtf8("pushButtonClose"));
        pushButtonClose->setMaximumSize(QSize(20, 16777215));

        horizontalLayout->addWidget(pushButtonClose);


        retranslateUi(Tools__DropNotifyForm);

        QMetaObject::connectSlotsByName(Tools__DropNotifyForm);
    } // setupUi

    void retranslateUi(QWidget *Tools__DropNotifyForm)
    {
        Tools__DropNotifyForm->setWindowTitle(QApplication::translate("Tools::DropNotifyForm", "Form", 0, QApplication::UnicodeUTF8));
        labelPluginImage->setText(QString());
        labelInfoText->setText(QApplication::translate("Tools::DropNotifyForm", "labelInfoText", 0, QApplication::UnicodeUTF8));
        pushButtonReadMore->setText(QApplication::translate("Tools::DropNotifyForm", "Read more", 0, QApplication::UnicodeUTF8));
        pushButtonClose->setText(QApplication::translate("Tools::DropNotifyForm", "x", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class DropNotifyForm: public Ui_DropNotifyForm {};
} // namespace Ui
} // namespace Tools

#endif // UI_DROPNOTIFYFORM_H
