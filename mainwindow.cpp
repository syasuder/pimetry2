#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <Parser>
#include <QTimer>
#include <QPainter>
#include <QImage>
#include <QFont>
#include <QPalette>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent) :
#ifndef FRAMELESS
    QMainWindow(parent),
#else
    QMainWindow(parent, Qt::Window | Qt::FramelessWindowHint),
#endif
    ui(new Ui::MainWindow),
    udpSocket(new QUdpSocket(this)),
    sentinel(0)
{
    ui->setupUi(this);

    udpSocket->bind(port, QUdpSocket::ShareAddress);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(procData()));

    ui->tachoWidget->setLcd(ui->lcdSpeed);
    ui->tachoWidget->setRpm(ui->lcdRpm);

#ifdef FRAMELESS
    // 背景描画設定
#ifdef Q_WS_X11
    if(QX11Info::isCompositingManagerRunning()){
        setAttribute(Qt::WA_TranslucentBackground, true);
        qDebug() << "setAttribute on X11";
        // \note xcompmgr を起動した状態でのみここは実行される
    }
#else
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    // フレームレスだとマウスでつかめないので細工する
    // http://www.qtforum.org/article/14404/dragging-qt-framelesswindowhint-windows.html?s=783748020660541269c95faa21a9ae5dc06cdb04#post59041
    setMouseTracking( true );
#endif
    // ウィジェットの色
    QWidgetList widgets = {
        ui->lcdSpeed,
        ui->lcdRpm,
        ui->lcdLoad,
        ui->lcdThrottle,
    };
    foreach (QWidget* w, widgets) {
        QPalette pal =  w->palette();
        pal.setColor(QPalette::Foreground, QColor(255, 128, 16, 192));     // オレンジ
        w->setPalette(pal);
    }

    // OFFLINE表示
    QPalette pal =  ui->offlineWidget->palette();
    pal.setColor(QPalette::Foreground, QColor(222, 0, 0, 192));     // 赤文字
    ui->offlineWidget->setPalette(pal);

    QTimer::singleShot(250, this, SLOT(tick()));  // ハードコード
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::procData()
{
    QByteArray data;
    data.resize(udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(data.data(), data.size());

//    qDebug() << "data:" << data;
    do {
        // パケットが経路上で分割統合されることがあるので、ごにょごにょする
        // パケット構造は\x7eと\x7dでjson文字列を挟んだもの
        buff.append(data);

        if (buff.length() < 1) {
            break;      // ポカヨケ
        }
        QByteArray payload;
        // 先頭文字まで読み飛ばす
        int start = buff.indexOf(header);
        if (start == -1) {
            break;      // ポカヨケ
        }
        buff = buff.mid(start);     // 先頭にゴミがあるとここで破棄
        payload = buff;
        // 終端文字を探す
        int end = payload.indexOf(trailer);
        if (end == -1) {
            break;      // ポカヨケ
        }
        payload = payload.left(end+1);
        buff.chop(end+1);

        // header,trailerを除去
        payload = payload.mid(1);
        payload.chop(1);
//        qDebug() << "payload:" << payload;
        QJson::Parser parser;
        bool ok;
        QVariantMap info = parser.parse (payload, &ok).toMap();
        qDebug() << "info:" << info;
        // info: QMap(("eload", QVariant(double, 0) ) ( "pos" ,  QVariant(double, 0) ) ( "rpm" ,  QVariant(double, 0) ) ( "speed" ,  QVariant(qulonglong, 0) ) )
//        qDebug() << "rpm:" << info["rpm"].toInt();
//        qDebug() << "speed:" << info["speed"].toInt();

        // ウィジェットに反映
        ui->lcdRpm->display(info["rpm"].toInt());
        ui->lcdSpeed->display(info["speed"].toInt());
        ui->lcdLoad->display(info["eload"].toInt());
        ui->lcdThrottle->display(info["pos"].toInt());

        sentinel = 0;       // データが来たのでリセット
    } while(0);

}

void MainWindow::tick()
{
    sentinel++;

    if (sentinel > 20 && ((sentinel % 4) != 0)) { // 0.25*20秒間データがこないとき点滅
        ui->offlineWidget->show();
    } else {
        ui->offlineWidget->hide();
    }

    QTimer::singleShot(250, this, SLOT(tick()));  // ハードコード
}


void MainWindow::mousePressEvent( QMouseEvent *e )
{
    clickPos = e->pos();
}

void MainWindow::mouseMoveEvent( QMouseEvent *e )
{
    move( e->globalPos() - QPoint(width()/2, height()/2) );
}
