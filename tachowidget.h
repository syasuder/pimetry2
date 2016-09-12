#ifndef PATTERNWIDGET_H
#define PATTERNWIDGET_H

#include <QWidget>
#include <QPair>
#include <QtCore/qmath.h>
#include <QImage>
#include <QPixmap>
#include <QLCDNumber>

namespace Ui {
class TachoWidget;
}

class TachoWidget : public QWidget
{
    Q_OBJECT


public:
    explicit TachoWidget(QWidget *parent = 0);
    ~TachoWidget();

    static const int angleRange = 240;      //!< メーターの角度範囲[degree]
    static const int angleOffset = 150;     //!< メーターの開始角度(左端)[degree]
    static const int angleDivision = 80;    //!< メーラーの目盛線の数(1目盛り100[rpm]

    void setLcd(QLCDNumber* lcd) {
        this->lcd = lcd;
    }
    void setRpm(QLCDNumber* rpm) {
        this->rpm = rpm;
    }

    static const int tachoWidth = 400;
    static const int tachoHeight = 280;
    static const int tachoStart = -30;
    static const int tachoEnd = 240;

protected:
    void paintEvent(QPaintEvent *event);

    void draw(QPainter *painter);
    void drawScale(QPainter *painter);
    void drawTacho(QPainter *painter);
    void drawSpeed(QPainter *painter);

public slots:
    void tick();

private:
public slots:

private:
    Ui::TachoWidget *ui;

    QImage backgroud;               //!< 背景画像

    QTransform trans;               //!< 共通の変換行列

    QLCDNumber* lcd;
    QLCDNumber* rpm;

    int rpmNow;                     //!< RPMの現在指示値
    int rpmTarget;                  //!< RPMの本来指示値

    qreal M1;                       //!< 前ステップの操作量
    qreal err1;                     //!< 前ステップの偏差
    qreal err2;                     //!< 前々ステップの偏差

};

#endif // PATTERNWIDGET_H
