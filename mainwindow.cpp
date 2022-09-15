#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sendRequest("CYCJ83039873");
//    sendRequest("37081047295817");
//    sendRequest("30371740184712910");
//    sendRequest("57201847194719104");
//    sendRequest("72917549188371371");
//    sendRequest("37082622221129901");

    sendJSON();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendRequest(const QString &id)
{
    //1.json格式传输报文
    QJsonObject json;
    QJsonDocument jsonDoc;

    //打包json数据
    json["id"] = id;

    jsonDoc.setObject(json);
    QByteArray dataArray = jsonDoc.toJson(QJsonDocument::Compact);
    qDebug()<< "jsondata组为："<<dataArray;

    //2.构造URL&&请求头
    QString urlStr = "https://ellipse.xmu.edu.cn/xspine/query";
    qDebug()<<"url地址为:"<<urlStr;
    url = urlStr;
    //构造请求头
    req.setUrl(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json;charset=UTF-8"));
    //req.setRawHeader("Connection", "Keep-Alive");

    //3.post数据
    manager = new QNetworkAccessManager(this);
    reply = manager->post(req,dataArray);
    QEventLoop eventLoop;
    connect(manager,SIGNAL(finished(QNetworkReply*)),&eventLoop,SLOT(quit()));
    connect(reply,&QNetworkReply::finished,this,&MainWindow::replyFinished);
    eventLoop.exec();

}

void MainWindow::sendJSON()
{

    QString send_url = "https://ellipse.xmu.edu.cn/xspine/send";
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setUrl(QUrl(send_url.toUtf8()));
    QVariantMap map;
    map.insert("id","CYCJ83039873");
    QVariantList sdata;
    QFile file("D:/QTDemo/ScoliosisPC/number.txt");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file.atEnd())
        {

            QByteArray line = file.readLine();
            QString str(line);
            sdata << str;
        }

    }
//    sdata<<4.84<<-2.05<<2.01<<2.31<<-0.87<<-1.07<<-3.90<<-1.40<<-4.68
//        <<1.82<<-4.94<<-1.03<<-3.71<<1.36<<-4.56<<2.17<<4.95<<0.15<<0.79<<-4.14
//       <<3.82<<1.02<<4.35<<2.50<<1.14<<-0.96<<1.50<<-0.01<<-2.46<<2.04<<1.53
//      <<3.98<<-3.60<<2.34<<-0.79<<-3.08<<2.36<<-2.47<<4.27<<2.94<<-4.42<<4.21<<4.38<<-4.07<<-4.49;
    map.insert("sdata",sdata);
    auto data = QJsonDocument::fromVariant(map).toJson();
    qDebug()<<"data:"<<QString(data);
    auto reply = send_manager.post(request,data);
    connect(reply,&QNetworkReply::finished,this,[=](){
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();
        qDebug()<<"status:"<<status;
        if(reply->error() != QNetworkReply::NoError){
            QString msg = QString(reply->readAll());
            qDebug()<<"reply->errorString():"<<reply->errorString();
            qDebug()<<"reply->error():"<<reply->error();
            qDebug()<<"msg:"<<msg;
            qDebug()<<"reply:"<<reply<<"reply.url:"<<reply->url().toString();
            if(msg.isEmpty()) msg = reply->errorString();
            qDebug()<<QString("HTTP[%1]:[%2]")
                      .arg(status)
                      .arg(msg);
            //ui->textEdit->setText(msg);
            reply->deleteLater();
            return;
        }
        QByteArray result = reply->readAll();
        QString str = QString(result);
        reply->deleteLater();
        qDebug()<<str;
        //ui->textEdit->setText(str);
    });
}


void MainWindow::replyFinished()
{
    // <1>判断有没有错误
        if (reply->error()){
            qDebug() << "error" << endl;
            qDebug()<<reply->errorString();
            reply->deleteLater();
            return;
        }

        // <2>检测网页返回状态码，常见是200，404等，200为成功
        int statusCode  = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "statusCode:" << statusCode << "请求成功" << endl;

        // <3>判断是否需要重定向
        if (statusCode >= 200 && statusCode <300){
            // ok

            // 准备读数据
            QTextCodec *codec = QTextCodec::codecForName("utf8");
            QString all = codec->toUnicode(reply->readAll());
            qDebug() << "接收到的数据" <<all;
            //分割
            QStringList list = all.split(QRegExp("[/. \t|]"));
                QString splitstr;
                for(int i=0;i<list.size();i++){
                    splitstr = list[i];
                    qDebug() << splitstr<<endl;
                }

            //显示在mainwindow 测试，将服务器返回值打印至label上，服务器用的是mongoose模拟
            //QLabel *label = new QLabel(this);
            ui->label->setText(all);
            //label->move(200,200);
            ui->label->setVisible(true);

            // 数据读取完成之后，清除reply
            reply->deleteLater();
            reply = nullptr;

        } else if (statusCode >=300 && statusCode <400){
            // redirect

            // 获取重定向信息
            const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            // 检测是否需要重定向，如果不需要则读数据
            if (!redirectionTarget.isNull()) {
                const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());

                reply->deleteLater();
                reply = nullptr;

                //startRequest(redirectedUrl);
                qDebug()<< "http redirect to " << redirectedUrl.toString();
                return;
            }
        }
}

