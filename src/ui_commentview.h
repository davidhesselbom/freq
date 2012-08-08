/********************************************************************************
** Form generated from reading UI file 'commentview.ui'
**
** Created: Wed 8. Aug 18:17:25 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMENTVIEW_H
#define UI_COMMENTVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CommentView
{
public:
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *CommentView)
    {
        if (CommentView->objectName().isEmpty())
            CommentView->setObjectName(QString::fromUtf8("CommentView"));
        CommentView->resize(204, 130);
        CommentView->setWindowOpacity(0.8);
        verticalLayout = new QVBoxLayout(CommentView);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, 2, 4, 0);
        textEdit = new QTextEdit(CommentView);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy);
        textEdit->setMinimumSize(QSize(50, 25));
        QFont font;
        font.setPointSize(12);
        textEdit->setFont(font);
        textEdit->setFrameShape(QFrame::NoFrame);
        textEdit->setFrameShadow(QFrame::Sunken);
        textEdit->setLineWidth(1);
        textEdit->setMidLineWidth(0);

        verticalLayout->addWidget(textEdit);

        verticalSpacer = new QSpacerItem(0, 13, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(CommentView);

        QMetaObject::connectSlotsByName(CommentView);
    } // setupUi

    void retranslateUi(QWidget *CommentView)
    {
        CommentView->setWindowTitle(QApplication::translate("CommentView", "Form", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CommentView: public Ui_CommentView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMENTVIEW_H
