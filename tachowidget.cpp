#include "tachowidget.h"
#include "ui_tachowidget.h"
#include <QPainter>
#include <QImage>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QTimer>
#include <QTransform>
#include <QFont>

TachoWidget::TachoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TachoWidget),
    lcd(NULL),
    rpmNow(0),
    rpmTarget(0),
    M1(0.0f),
    err1(0.0f),
    err2(0.0f)
{
    ui->setupUi(this);

    // 共通の変換行列 横につぶしてかつせん断変形
    QVector<QPointF> quad = {QPointF(0.2, 0.0), QPointF(1.2, 0.0), QPointF(1.0, 1.0), QPointF(0.0, 1.0) };
    trans.squareToQuad(quad, trans);
    trans.scale(1.0, 0.7);

    QTimer::singleShot(500, this, SLOT(tick()));  // ハードコード
}

TachoWidget::~TachoWidget()
{
    delete ui;
}

void TachoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    // 背景描画
    painter.drawImage(QPoint(0,0), backgroud);
    draw(&painter);
}

void TachoWidget::draw(QPainter *painter)
{
    QMatrix m;
    QSize size = this->size();
    m.translate( size.width() / 2.0f, size.height() / 2.0f);
    m.scale(size.width()  / 480.0f, size.height()  / 300.0f);
    painter->setMatrix(m);

    drawScale(painter);
    drawSpeed(painter);
    drawTacho(painter);
}

void TachoWidget::drawScale(QPainter *painter)
{
    QSize size = this->size();
//    painter->drawPie( -tachoWidth/2,-tachoHeight/2, tachoWidth, tachoHeight, tachoStart*16, tachoEnd*16);

    QPointF pointCOld;
    for (int i=0; i<=angleDivision; i++) {
        qreal angle = angleRange * i / (qreal)angleDivision + angleOffset;

        int lenA = 160;
        int lenB = 180;
        int lenC = 110;
        int lenD = 210;

        if ((i % 10) == 0) {
            lenA = lenC;
            lenB = 200;
        } else if ((i % 5) == 0) {
            lenA = lenC;
            lenB = 190;
        }

        QTransform transScale = trans;
        transScale.rotate(angle);

        QPointF identA(lenA, 0);
        QPointF pointA = transScale.map(identA);
        QPointF identB(lenB, 0);
        QPointF pointB = transScale.map(identB);
        QPointF identC(lenC, 0);
        QPointF pointC = transScale.map(identC);
        QPointF identD(lenD, 0);
        QPointF pointD = transScale.map(identD);
//        painter->setPen(QPen(Qt::black, 2.5));
        painter->setPen(QPen(QBrush(QColor(0, 0, 0, 128)), 2.5));
        painter->drawLine(pointA, pointB);
        if(i > 0) {
//            painter->setPen(QPen(Qt::black, 1.5));
            painter->setPen(QPen(QBrush(QColor(0, 0, 0, 128)), 1.5));
            painter->drawLine(pointC, pointCOld);
        }
        pointCOld = pointC;

        // 文字盤 1000ごと
        if ((i % 10) == 0) {
            QFont font = painter->font() ;
            font.setPointSize(16);
            painter->setFont(font);
            pointD.setY(pointD.y() + 10);
            painter->drawText(pointD, QString().setNum(i/10));
        }
    }
}

void TachoWidget::drawTacho(QPainter *painter)
{
    if(rpm == NULL) {
        return;
    }

//    int rpmInt = rpm->intValue();     // そのまま表示するとつまらない
    int rpmInt = (int)rpmNow;

    qreal angle = angleRange * rpmInt / (qreal)angleDivision / 100.0f  + angleOffset;
    int lenA = -20;
    int lenB = 195;

    QTransform transScale = trans;
    transScale.rotate(angle);
    QPointF identA(lenA, 0);
    QPointF pointA = transScale.map(identA);
    QPointF identB(lenB, 0);
    QPointF pointB = transScale.map(identB);
//    painter->setPen(QPen(Qt::red, 5.0));
    painter->setPen(QPen(QBrush(QColor(255, 0, 0, 192)), 5.0, Qt::SolidLine,Qt::RoundCap));
    painter->drawLine(pointA, pointB);  // 赤い棒

    painter->setPen(QPen(Qt::black, 2.0));
    painter->setBrush(Qt::white);
    painter->drawEllipse(QPoint(0,0), 3, 3);

}

void TachoWidget::drawSpeed(QPainter *painter)
{
    if(lcd == NULL) {
        return;
    }
    // http://dnaga392.hatenablog.com/entry/2015/02/05/213437
    QPixmap pix(lcd->size());
#ifdef FRAMELESS
    lcd->render(&pix,QPoint(), QRegion(), 0);
#else
    lcd->render(&pix,QPoint(), QRegion());
#endif
    QTransform skew;
    QVector<QPointF> quad = {QPointF(0.4, 0.0), QPointF(1.4, 0.0), QPointF(1.0, 1.0), QPointF(0.0, 1.0) };
    skew.squareToQuad(quad, skew);
    QImage image = pix.toImage();
    QSize size = image.size();
    painter->drawImage(-size.width()/2, -size.height(), image.transformed(skew));
}

void TachoWidget::tick()
{
    // 定数 ハードコード
    qreal delta = 1;    // 静止検出
    qreal Kp = 0.1f;    // PIDパラメータ
    qreal Ki = 0.005f;
    qreal Kd = 0.05f;    // 微分を大きくするとオーバーシュート気味になり「それっぽい」
    qreal MMax = 3000.0; // 操作量の最大値(針の最大速度)
    qreal rpmMax = 250.0f + angleDivision * 100.0f; // 針が動ける範囲
    qreal rpmMin = - 250.0f;

    // RPMの針をそれっぽく動かす
    rpmTarget = rpm->intValue();           // OBDからのRPMの値が目標値
    qreal err = rpmTarget - rpmNow;        // 現在の偏差
    if (err < delta) {
        rpmNow = rpmTarget;                 // デルタ値より小さいので一致とみなす
    }
    qreal Mdelta = Kp * (err - err1) + Ki * err + Kd * (err - err1*2 + err2);
    qreal M = M1 + Mdelta;      // 操作量
    M = qMin(M, MMax);
    rpmNow += M;        // 現在指示値更新
    rpmNow = qMin((qreal)rpmNow, rpmMax);      // 機械的な制限ぽいもの
    rpmNow = qMax((qreal)rpmNow, rpmMin);

    // ステップ更新
    M1 = M;
    err2 = err1;
    err1 = err1;

    update();
    QTimer::singleShot(100, this, SLOT(tick()));  // ハードコード
}

