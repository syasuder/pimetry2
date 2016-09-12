#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static const int port = 5001;
    static const char header = 0xe7;
    static const char trailer = 0xd7;

public slots:
    void procData();
    void tick();

protected:
    void mousePressEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);

private:
    Ui::MainWindow *ui;
    QPoint clickPos;

    int sentinel;            //!< OBDからデータが来ない期間をカウントする

    QByteArray buff;
    QUdpSocket * udpSocket;

};

#endif // MAINWINDOW_H
