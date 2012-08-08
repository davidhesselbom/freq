/********************************************************************************
** Form generated from reading UI file 'transforminfoform.ui'
**
** Created: Wed 8. Aug 18:17:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRANSFORMINFOFORM_H
#define UI_TRANSFORMINFOFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TransformInfoForm
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tableWidget;
    QGridLayout *gridLayout;
    QLineEdit *binResolutionEdit;
    QLabel *binResolutionLabel;
    QLineEdit *maxHzEdit;
    QLineEdit *minHzEdit;
    QLabel *minHzLabel;
    QLabel *maxHzLabel;
    QLabel *sampleRateLabel;
    QLineEdit *sampleRateEdit;
    QLabel *windowSizeLabel;
    QLineEdit *windowSizeEdit;
    QLabel *windowTypeLabel;
    QComboBox *windowTypeComboBox;
    QLabel *overlapLabel;
    QLineEdit *overlapEdit;
    QLabel *averagingLabel;
    QLineEdit *averagingEdit;
    QLabel *normalizationLabel;
    QComboBox *normalizationComboBox;

    void setupUi(QWidget *TransformInfoForm)
    {
        if (TransformInfoForm->objectName().isEmpty())
            TransformInfoForm->setObjectName(QString::fromUtf8("TransformInfoForm"));
        TransformInfoForm->resize(400, 300);
        verticalLayout = new QVBoxLayout(TransformInfoForm);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tableWidget = new QTableWidget(TransformInfoForm);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));

        verticalLayout->addWidget(tableWidget);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        binResolutionEdit = new QLineEdit(TransformInfoForm);
        binResolutionEdit->setObjectName(QString::fromUtf8("binResolutionEdit"));

        gridLayout->addWidget(binResolutionEdit, 2, 1, 1, 1);

        binResolutionLabel = new QLabel(TransformInfoForm);
        binResolutionLabel->setObjectName(QString::fromUtf8("binResolutionLabel"));

        gridLayout->addWidget(binResolutionLabel, 2, 0, 1, 1);

        maxHzEdit = new QLineEdit(TransformInfoForm);
        maxHzEdit->setObjectName(QString::fromUtf8("maxHzEdit"));

        gridLayout->addWidget(maxHzEdit, 1, 1, 1, 1);

        minHzEdit = new QLineEdit(TransformInfoForm);
        minHzEdit->setObjectName(QString::fromUtf8("minHzEdit"));

        gridLayout->addWidget(minHzEdit, 0, 1, 1, 1);

        minHzLabel = new QLabel(TransformInfoForm);
        minHzLabel->setObjectName(QString::fromUtf8("minHzLabel"));

        gridLayout->addWidget(minHzLabel, 0, 0, 1, 1);

        maxHzLabel = new QLabel(TransformInfoForm);
        maxHzLabel->setObjectName(QString::fromUtf8("maxHzLabel"));

        gridLayout->addWidget(maxHzLabel, 1, 0, 1, 1);

        sampleRateLabel = new QLabel(TransformInfoForm);
        sampleRateLabel->setObjectName(QString::fromUtf8("sampleRateLabel"));

        gridLayout->addWidget(sampleRateLabel, 3, 0, 1, 1);

        sampleRateEdit = new QLineEdit(TransformInfoForm);
        sampleRateEdit->setObjectName(QString::fromUtf8("sampleRateEdit"));

        gridLayout->addWidget(sampleRateEdit, 3, 1, 1, 1);

        windowSizeLabel = new QLabel(TransformInfoForm);
        windowSizeLabel->setObjectName(QString::fromUtf8("windowSizeLabel"));

        gridLayout->addWidget(windowSizeLabel, 4, 0, 1, 1);

        windowSizeEdit = new QLineEdit(TransformInfoForm);
        windowSizeEdit->setObjectName(QString::fromUtf8("windowSizeEdit"));

        gridLayout->addWidget(windowSizeEdit, 4, 1, 1, 1);

        windowTypeLabel = new QLabel(TransformInfoForm);
        windowTypeLabel->setObjectName(QString::fromUtf8("windowTypeLabel"));

        gridLayout->addWidget(windowTypeLabel, 7, 0, 1, 1);

        windowTypeComboBox = new QComboBox(TransformInfoForm);
        windowTypeComboBox->setObjectName(QString::fromUtf8("windowTypeComboBox"));

        gridLayout->addWidget(windowTypeComboBox, 7, 1, 1, 1);

        overlapLabel = new QLabel(TransformInfoForm);
        overlapLabel->setObjectName(QString::fromUtf8("overlapLabel"));

        gridLayout->addWidget(overlapLabel, 8, 0, 1, 1);

        overlapEdit = new QLineEdit(TransformInfoForm);
        overlapEdit->setObjectName(QString::fromUtf8("overlapEdit"));

        gridLayout->addWidget(overlapEdit, 8, 1, 1, 1);

        averagingLabel = new QLabel(TransformInfoForm);
        averagingLabel->setObjectName(QString::fromUtf8("averagingLabel"));

        gridLayout->addWidget(averagingLabel, 5, 0, 1, 1);

        averagingEdit = new QLineEdit(TransformInfoForm);
        averagingEdit->setObjectName(QString::fromUtf8("averagingEdit"));

        gridLayout->addWidget(averagingEdit, 5, 1, 1, 1);

        normalizationLabel = new QLabel(TransformInfoForm);
        normalizationLabel->setObjectName(QString::fromUtf8("normalizationLabel"));

        gridLayout->addWidget(normalizationLabel, 6, 0, 1, 1);

        normalizationComboBox = new QComboBox(TransformInfoForm);
        normalizationComboBox->setObjectName(QString::fromUtf8("normalizationComboBox"));

        gridLayout->addWidget(normalizationComboBox, 6, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(TransformInfoForm);

        QMetaObject::connectSlotsByName(TransformInfoForm);
    } // setupUi

    void retranslateUi(QWidget *TransformInfoForm)
    {
        TransformInfoForm->setWindowTitle(QApplication::translate("TransformInfoForm", "Form", 0, QApplication::UnicodeUTF8));
        binResolutionLabel->setText(QApplication::translate("TransformInfoForm", "Hz/bin", 0, QApplication::UnicodeUTF8));
        minHzLabel->setText(QApplication::translate("TransformInfoForm", "Min hz", 0, QApplication::UnicodeUTF8));
        maxHzLabel->setText(QApplication::translate("TransformInfoForm", "Max hz", 0, QApplication::UnicodeUTF8));
        sampleRateLabel->setText(QApplication::translate("TransformInfoForm", "Sample rate", 0, QApplication::UnicodeUTF8));
        windowSizeLabel->setText(QApplication::translate("TransformInfoForm", "Window size", 0, QApplication::UnicodeUTF8));
        windowTypeLabel->setText(QApplication::translate("TransformInfoForm", "Window type", 0, QApplication::UnicodeUTF8));
        overlapLabel->setText(QApplication::translate("TransformInfoForm", "Overlap", 0, QApplication::UnicodeUTF8));
        averagingLabel->setText(QApplication::translate("TransformInfoForm", "Averaging", 0, QApplication::UnicodeUTF8));
        normalizationLabel->setText(QApplication::translate("TransformInfoForm", "Normalization", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TransformInfoForm: public Ui_TransformInfoForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRANSFORMINFOFORM_H
