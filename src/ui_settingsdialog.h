/********************************************************************************
** Form generated from reading UI file 'settingsdialog.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

namespace Tools {

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLineEdit *lineEditMatlab;
    QPushButton *pushButtonMatlab;
    QLineEdit *lineEditOctave;
    QPushButton *pushButtonOctave;
    QRadioButton *radioButtonOctave;
    QRadioButton *radioButtonMatlab;
    QLabel *label;
    QComboBox *comboBoxAudioIn;
    QLabel *label_2;
    QComboBox *comboBoxAudioOut;
    QLabel *label_3;
    QLineEdit *lineEditLogFiles;
    QSlider *horizontalSliderResolution;
    QLabel *label_4;
    QLabel *label_5;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Tools__SettingsDialog)
    {
        if (Tools__SettingsDialog->objectName().isEmpty())
            Tools__SettingsDialog->setObjectName(QString::fromUtf8("Tools__SettingsDialog"));
        Tools__SettingsDialog->resize(400, 243);
        verticalLayout = new QVBoxLayout(Tools__SettingsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lineEditMatlab = new QLineEdit(Tools__SettingsDialog);
        lineEditMatlab->setObjectName(QString::fromUtf8("lineEditMatlab"));

        gridLayout->addWidget(lineEditMatlab, 0, 1, 1, 1);

        pushButtonMatlab = new QPushButton(Tools__SettingsDialog);
        pushButtonMatlab->setObjectName(QString::fromUtf8("pushButtonMatlab"));

        gridLayout->addWidget(pushButtonMatlab, 0, 2, 1, 1);

        lineEditOctave = new QLineEdit(Tools__SettingsDialog);
        lineEditOctave->setObjectName(QString::fromUtf8("lineEditOctave"));

        gridLayout->addWidget(lineEditOctave, 2, 1, 1, 1);

        pushButtonOctave = new QPushButton(Tools__SettingsDialog);
        pushButtonOctave->setObjectName(QString::fromUtf8("pushButtonOctave"));

        gridLayout->addWidget(pushButtonOctave, 2, 2, 1, 1);

        radioButtonOctave = new QRadioButton(Tools__SettingsDialog);
        radioButtonOctave->setObjectName(QString::fromUtf8("radioButtonOctave"));

        gridLayout->addWidget(radioButtonOctave, 2, 0, 1, 1);

        radioButtonMatlab = new QRadioButton(Tools__SettingsDialog);
        radioButtonMatlab->setObjectName(QString::fromUtf8("radioButtonMatlab"));

        gridLayout->addWidget(radioButtonMatlab, 0, 0, 1, 1);

        label = new QLabel(Tools__SettingsDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 3, 0, 1, 1);

        comboBoxAudioIn = new QComboBox(Tools__SettingsDialog);
        comboBoxAudioIn->setObjectName(QString::fromUtf8("comboBoxAudioIn"));

        gridLayout->addWidget(comboBoxAudioIn, 3, 1, 1, 1);

        label_2 = new QLabel(Tools__SettingsDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 4, 0, 1, 1);

        comboBoxAudioOut = new QComboBox(Tools__SettingsDialog);
        comboBoxAudioOut->setObjectName(QString::fromUtf8("comboBoxAudioOut"));

        gridLayout->addWidget(comboBoxAudioOut, 4, 1, 1, 1);

        label_3 = new QLabel(Tools__SettingsDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 5, 0, 1, 1);

        lineEditLogFiles = new QLineEdit(Tools__SettingsDialog);
        lineEditLogFiles->setObjectName(QString::fromUtf8("lineEditLogFiles"));
        lineEditLogFiles->setReadOnly(true);

        gridLayout->addWidget(lineEditLogFiles, 5, 1, 1, 1);

        horizontalSliderResolution = new QSlider(Tools__SettingsDialog);
        horizontalSliderResolution->setObjectName(QString::fromUtf8("horizontalSliderResolution"));
        horizontalSliderResolution->setMaximum(1000);
        horizontalSliderResolution->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSliderResolution, 6, 1, 1, 1);

        label_4 = new QLabel(Tools__SettingsDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 6, 0, 1, 1);

        label_5 = new QLabel(Tools__SettingsDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 6, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(Tools__SettingsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::RestoreDefaults);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(Tools__SettingsDialog);

        QMetaObject::connectSlotsByName(Tools__SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *Tools__SettingsDialog)
    {
        Tools__SettingsDialog->setWindowTitle(QApplication::translate("Tools::SettingsDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        pushButtonMatlab->setText(QApplication::translate("Tools::SettingsDialog", "Browse", 0, QApplication::UnicodeUTF8));
        pushButtonOctave->setText(QApplication::translate("Tools::SettingsDialog", "Browse", 0, QApplication::UnicodeUTF8));
        radioButtonOctave->setText(QApplication::translate("Tools::SettingsDialog", "Octave", 0, QApplication::UnicodeUTF8));
        radioButtonMatlab->setText(QApplication::translate("Tools::SettingsDialog", "Matlab", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Tools::SettingsDialog", "Recording", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Tools::SettingsDialog", "Playback", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Tools::SettingsDialog", "Log files", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Tools::SettingsDialog", "Low detail", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Tools::SettingsDialog", "High detail", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui
} // namespace Tools

#endif // UI_SETTINGSDIALOG_H
