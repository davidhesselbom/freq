/********************************************************************************
** Form generated from reading UI file 'splashscreen.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPLASHSCREEN_H
#define UI_SPLASHSCREEN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QVBoxLayout>

namespace Tools {

class Ui_SplashScreen
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelSplash;
    QProgressBar *progressBar;
    QLabel *labelText;

    void setupUi(QDialog *Tools__SplashScreen)
    {
        if (Tools__SplashScreen->objectName().isEmpty())
            Tools__SplashScreen->setObjectName(QString::fromUtf8("Tools__SplashScreen"));
        Tools__SplashScreen->setWindowModality(Qt::ApplicationModal);
        Tools__SplashScreen->resize(720, 259);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Tools__SplashScreen->sizePolicy().hasHeightForWidth());
        Tools__SplashScreen->setSizePolicy(sizePolicy);
        Tools__SplashScreen->setModal(true);
        verticalLayout = new QVBoxLayout(Tools__SplashScreen);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        labelSplash = new QLabel(Tools__SplashScreen);
        labelSplash->setObjectName(QString::fromUtf8("labelSplash"));
        labelSplash->setPixmap(QPixmap(QString::fromUtf8(":/splash/splash/brickwall.jpg")));
        labelSplash->setScaledContents(false);

        verticalLayout->addWidget(labelSplash);

        progressBar = new QProgressBar(Tools__SplashScreen);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMaximum(300);
        progressBar->setValue(72);
        progressBar->setTextVisible(false);

        verticalLayout->addWidget(progressBar);

        labelText = new QLabel(Tools__SplashScreen);
        labelText->setObjectName(QString::fromUtf8("labelText"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelText->sizePolicy().hasHeightForWidth());
        labelText->setSizePolicy(sizePolicy1);
        labelText->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(labelText);


        retranslateUi(Tools__SplashScreen);

        QMetaObject::connectSlotsByName(Tools__SplashScreen);
    } // setupUi

    void retranslateUi(QDialog *Tools__SplashScreen)
    {
        Tools__SplashScreen->setWindowTitle(QApplication::translate("Tools::SplashScreen", "Dialog", 0, QApplication::UnicodeUTF8));
        labelSplash->setText(QString());
        labelText->setText(QApplication::translate("Tools::SplashScreen", "Please wait while Brickwall Audio is taking over the world...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class SplashScreen: public Ui_SplashScreen {};
} // namespace Ui
} // namespace Tools

#endif // UI_SPLASHSCREEN_H
