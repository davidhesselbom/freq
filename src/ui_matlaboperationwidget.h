/********************************************************************************
** Form generated from reading UI file 'matlaboperationwidget.ui'
**
** Created: Wed 8. Aug 18:17:24 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MATLABOPERATIONWIDGET_H
#define UI_MATLABOPERATIONWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

namespace Tools {

class Ui_MatlabOperationWidget
{
public:
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout_3;
    QGroupBox *settingsBox;
    QGridLayout *gridLayout_2;
    QLabel *labelChunkSize;
    QLabel *labelArgumentDescription;
    QLabel *labelRedundantSamples;
    QSpinBox *redundant;
    QPushButton *pushButtonRestoreChanges;
    QSpinBox *chunksize;
    QLabel *labelChunkSizeInfo;
    QCheckBox *computeInOrder;
    QLineEdit *arguments;
    QLabel *labelRedundantSamplesInfo;
    QLabel *labelInOrderInfo;
    QGridLayout *gridLayout_4;
    QLabel *label;
    QLabel *labelEmptyForTerminal;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *scriptname;
    QPushButton *browseButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonRestartScript;
    QPushButton *pushButtonShowOutput;

    void setupUi(QWidget *Tools__MatlabOperationWidget)
    {
        if (Tools__MatlabOperationWidget->objectName().isEmpty())
            Tools__MatlabOperationWidget->setObjectName(QString::fromUtf8("Tools__MatlabOperationWidget"));
        Tools__MatlabOperationWidget->resize(452, 488);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Tools__MatlabOperationWidget->sizePolicy().hasHeightForWidth());
        Tools__MatlabOperationWidget->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(Tools__MatlabOperationWidget);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        scrollArea = new QScrollArea(Tools__MatlabOperationWidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 429, 564));
        gridLayout_3 = new QGridLayout(scrollAreaWidgetContents);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        settingsBox = new QGroupBox(scrollAreaWidgetContents);
        settingsBox->setObjectName(QString::fromUtf8("settingsBox"));
        settingsBox->setFlat(false);
        settingsBox->setCheckable(true);
        gridLayout_2 = new QGridLayout(settingsBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        labelChunkSize = new QLabel(settingsBox);
        labelChunkSize->setObjectName(QString::fromUtf8("labelChunkSize"));

        gridLayout_2->addWidget(labelChunkSize, 0, 0, 1, 1);

        labelArgumentDescription = new QLabel(settingsBox);
        labelArgumentDescription->setObjectName(QString::fromUtf8("labelArgumentDescription"));
        labelArgumentDescription->setWordWrap(true);

        gridLayout_2->addWidget(labelArgumentDescription, 4, 0, 1, 1);

        labelRedundantSamples = new QLabel(settingsBox);
        labelRedundantSamples->setObjectName(QString::fromUtf8("labelRedundantSamples"));

        gridLayout_2->addWidget(labelRedundantSamples, 5, 0, 1, 1);

        redundant = new QSpinBox(settingsBox);
        redundant->setObjectName(QString::fromUtf8("redundant"));
        redundant->setMaximum(10000000);
        redundant->setSingleStep(1000);

        gridLayout_2->addWidget(redundant, 5, 1, 1, 1);

        pushButtonRestoreChanges = new QPushButton(settingsBox);
        pushButtonRestoreChanges->setObjectName(QString::fromUtf8("pushButtonRestoreChanges"));

        gridLayout_2->addWidget(pushButtonRestoreChanges, 7, 0, 1, 1);

        chunksize = new QSpinBox(settingsBox);
        chunksize->setObjectName(QString::fromUtf8("chunksize"));
        chunksize->setMinimum(-1);
        chunksize->setMaximum(10000000);
        chunksize->setSingleStep(1000);

        gridLayout_2->addWidget(chunksize, 0, 1, 1, 1);

        labelChunkSizeInfo = new QLabel(settingsBox);
        labelChunkSizeInfo->setObjectName(QString::fromUtf8("labelChunkSizeInfo"));
        labelChunkSizeInfo->setWordWrap(true);

        gridLayout_2->addWidget(labelChunkSizeInfo, 1, 1, 1, 1);

        computeInOrder = new QCheckBox(settingsBox);
        computeInOrder->setObjectName(QString::fromUtf8("computeInOrder"));

        gridLayout_2->addWidget(computeInOrder, 2, 1, 1, 1);

        arguments = new QLineEdit(settingsBox);
        arguments->setObjectName(QString::fromUtf8("arguments"));

        gridLayout_2->addWidget(arguments, 4, 1, 1, 1);

        labelRedundantSamplesInfo = new QLabel(settingsBox);
        labelRedundantSamplesInfo->setObjectName(QString::fromUtf8("labelRedundantSamplesInfo"));
        labelRedundantSamplesInfo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        labelRedundantSamplesInfo->setWordWrap(true);

        gridLayout_2->addWidget(labelRedundantSamplesInfo, 6, 1, 1, 1);

        labelInOrderInfo = new QLabel(settingsBox);
        labelInOrderInfo->setObjectName(QString::fromUtf8("labelInOrderInfo"));
        labelInOrderInfo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        labelInOrderInfo->setWordWrap(true);

        gridLayout_2->addWidget(labelInOrderInfo, 3, 1, 1, 1);


        gridLayout_3->addWidget(settingsBox, 3, 0, 1, 5);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label = new QLabel(scrollAreaWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_4->addWidget(label, 0, 0, 1, 1);

        labelEmptyForTerminal = new QLabel(scrollAreaWidgetContents);
        labelEmptyForTerminal->setObjectName(QString::fromUtf8("labelEmptyForTerminal"));
        labelEmptyForTerminal->setWordWrap(true);

        gridLayout_4->addWidget(labelEmptyForTerminal, 1, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        scriptname = new QLineEdit(scrollAreaWidgetContents);
        scriptname->setObjectName(QString::fromUtf8("scriptname"));

        horizontalLayout_2->addWidget(scriptname);

        browseButton = new QPushButton(scrollAreaWidgetContents);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));

        horizontalLayout_2->addWidget(browseButton);


        gridLayout_4->addLayout(horizontalLayout_2, 0, 1, 1, 1);


        gridLayout_3->addLayout(gridLayout_4, 1, 1, 1, 1);

        verticalSpacer = new QSpacerItem(351, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer, 6, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButtonRestartScript = new QPushButton(scrollAreaWidgetContents);
        pushButtonRestartScript->setObjectName(QString::fromUtf8("pushButtonRestartScript"));

        horizontalLayout->addWidget(pushButtonRestartScript);

        pushButtonShowOutput = new QPushButton(scrollAreaWidgetContents);
        pushButtonShowOutput->setObjectName(QString::fromUtf8("pushButtonShowOutput"));
        pushButtonShowOutput->setCheckable(true);
        pushButtonShowOutput->setChecked(false);

        horizontalLayout->addWidget(pushButtonShowOutput);


        gridLayout_3->addLayout(horizontalLayout, 7, 1, 1, 1);

        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 0, 1, 1);


        retranslateUi(Tools__MatlabOperationWidget);

        QMetaObject::connectSlotsByName(Tools__MatlabOperationWidget);
    } // setupUi

    void retranslateUi(QWidget *Tools__MatlabOperationWidget)
    {
        Tools__MatlabOperationWidget->setWindowTitle(QApplication::translate("Tools::MatlabOperationWidget", "Form", 0, QApplication::UnicodeUTF8));
        settingsBox->setTitle(QApplication::translate("Tools::MatlabOperationWidget", "Script settings", 0, QApplication::UnicodeUTF8));
        labelChunkSize->setText(QApplication::translate("Tools::MatlabOperationWidget", "Chunk size", 0, QApplication::UnicodeUTF8));
        labelArgumentDescription->setText(QApplication::translate("Tools::MatlabOperationWidget", "Arguments", 0, QApplication::UnicodeUTF8));
        labelRedundantSamples->setText(QApplication::translate("Tools::MatlabOperationWidget", "Overlap", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonRestoreChanges->setToolTip(QApplication::translate("Tools::MatlabOperationWidget", "Changes has been made, click here to restore the changes.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonRestoreChanges->setText(QApplication::translate("Tools::MatlabOperationWidget", "Restore changes", 0, QApplication::UnicodeUTF8));
        labelChunkSizeInfo->setText(QApplication::translate("Tools::MatlabOperationWidget", "Number of samples per chunk. If set to '0' Sonic AWE will decide. Set it to '-1' to filter the entire signal in one big chunk.", 0, QApplication::UnicodeUTF8));
        computeInOrder->setText(QApplication::translate("Tools::MatlabOperationWidget", "Compute chunks in order", 0, QApplication::UnicodeUTF8));
        labelRedundantSamplesInfo->setText(QApplication::translate("Tools::MatlabOperationWidget", "Number of overlapping samples per chunk. This number of samples is included on both sides of each chunk.", 0, QApplication::UnicodeUTF8));
        labelInOrderInfo->setText(QApplication::translate("Tools::MatlabOperationWidget", "If unset Sonic AWE will decide in which order to compute chunks.", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Tools::MatlabOperationWidget", "Script", 0, QApplication::UnicodeUTF8));
        labelEmptyForTerminal->setText(QApplication::translate("Tools::MatlabOperationWidget", "Leave empty to open a terminal instead of running a script.", 0, QApplication::UnicodeUTF8));
        browseButton->setText(QApplication::translate("Tools::MatlabOperationWidget", "Browse", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonRestartScript->setToolTip(QApplication::translate("Tools::MatlabOperationWidget", "Changes has been made, click here to reload the script with these settings.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonRestartScript->setText(QApplication::translate("Tools::MatlabOperationWidget", "Restart script", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonShowOutput->setToolTip(QApplication::translate("Tools::MatlabOperationWidget", "Changes has been made, click here to restore the changes.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonShowOutput->setText(QApplication::translate("Tools::MatlabOperationWidget", "Show output", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace Tools

namespace Tools {
namespace Ui {
    class MatlabOperationWidget: public Ui_MatlabOperationWidget {};
} // namespace Ui
} // namespace Tools

#endif // UI_MATLABOPERATIONWIDGET_H
