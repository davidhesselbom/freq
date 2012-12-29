#include "sendfeedback.h"
#include "ui_sendfeedback.h"

#include "support/buildhttppost.h"

#include "sawe/application.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "sawe/reader.h"
#include <TaskTimer.h>

#include <QNetworkAccessManager>
#include <QDir>
#include <QSettings>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>

namespace Tools {

SendFeedback::
        SendFeedback(::Ui::SaweMainWindow *mainwindow) :
    QDialog(mainwindow),
    ui(new Ui::SendFeedback),
    targetUrl("http://feedback.sonicawe.com/sendfeedback.php")
{
    ui->setupUi(this);

    ::Ui::MainWindow* mui = mainwindow->getItems();
    connect(mui->actionReport_a_bug, SIGNAL(triggered()), SLOT(open()));

    ui->lineEditEmail->setText( Sawe::Reader::name.c_str() );

    connect(ui->pushButtonBrowse, SIGNAL(clicked()), SLOT(browse()));
}


SendFeedback::
        ~SendFeedback()
{
    delete ui;
}


void SendFeedback::
        browse()
{
    QString filename = QFileDialog::getOpenFileName(0, "Find attachment");
    ui->lineEditAttachFile->setText(filename);
}


void SendFeedback::
        accept()
{
    this->setEnabled( false );

    sendLogFiles(
            ui->lineEditEmail->text(),
            ui->textEditMessage->toPlainText(),
            ui->lineEditAttachFile->text() );
}


void SendFeedback::
        sendLogFiles(QString email, QString message, QString extraFile)
{
    Support::BuildHttpPost postdata;

    QString logdir = Sawe::Application::log_directory();
    unsigned count = 0;
    QFileInfoList filesToSend = QDir(logdir).entryInfoList();
    if (QFile::exists(extraFile))
        filesToSend.append(extraFile);

    QString omittedMessage;
    qint64 uploadLimit = 7 << 20;
    foreach(QFileInfo f, filesToSend)
    {
        if (f.size() > uploadLimit)
        {
            omittedMessage += QString("File %1 (%2) was omitted\n")
                              .arg(f.fileName())
                              .arg(DataStorageVoid::getMemorySizeText(f.size()).c_str());
            continue;
        }
        if (postdata.addFile(f))
            count++;
    }

    if (!omittedMessage.isEmpty())
    {
        message = omittedMessage + "\n" + message;
        QMessageBox::information(
                dynamic_cast<QWidget*>(parent()), "Some files were to large", omittedMessage);
    }

    postdata.addKeyValue( "email", email );
    postdata.addKeyValue( "message", message );
    postdata.addKeyValue( "value", QSettings().value("value").toString() );

    QByteArray feedbackdata = postdata;

    unsigned N = feedbackdata.size();
    TaskInfo ti("SendFeedback sends %s in %u files",
             DataStorageVoid::getMemorySizeText(N).c_str(),
             count);

    manager.reset( new QNetworkAccessManager(this) );
    connect(manager.data(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    postdata.send( manager.data(), targetUrl );
}


void SendFeedback::
        replyFinished(QNetworkReply* reply)
{
    QString s = reply->readAll();
    TaskInfo ti("SendFeedback reply");
    TaskInfo("%s", s.replace("\\r\\n","\n").replace("\r","").toStdString().c_str());
    if (QNetworkReply::NoError != reply->error())
    {
        TaskInfo("SendFeedback error=%s (code %d)",
                 QNetworkReply::NoError == reply->error()?"no error":reply->errorString().toStdString().c_str(),
                 (int)reply->error());
    }

    if (QNetworkReply::NoError != reply->error())
        QMessageBox::warning(dynamic_cast<QWidget*>(parent()), "Could not send feedback", reply->errorString() + "\n" + s);
    else if (s.contains("sorry", Qt::CaseInsensitive) ||
             s.contains("error", Qt::CaseInsensitive) ||
             s.contains("fatal", Qt::CaseInsensitive) ||
             s.contains("fail", Qt::CaseInsensitive) ||
             s.contains("html", Qt::CaseInsensitive) ||
             !s.contains("sendfeedback finished", Qt::CaseInsensitive))
    {
        QMessageBox::warning(dynamic_cast<QWidget*>(parent()), "Could not send feedback", s);
    }
    else
    {
        QDialog::accept();
        QMessageBox::information(dynamic_cast<QWidget*>(parent()), "Feedback", "Your input has been sent. Thank you!");
        ui->textEditMessage->setText ("");
        ui->lineEditAttachFile->setText ("");
    }

    setEnabled( true );
}


} // namespace Tools
