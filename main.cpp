#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>

/// ログファイル名
static QString logpath;

/// qDebug()をカスタマイズ
void messageHandler(QtMsgType type, const char *msg)
{
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%2::Debug: %1").arg(msg).arg(timestamp);
        break;
    case QtWarningMsg:
        txt = QString("%2::Warning: %1").arg(msg).arg(timestamp);
        break;
    case QtCriticalMsg:
        txt = QString("%2::Critical: %1").arg(msg).arg(timestamp);
        break;
    case QtFatalMsg:
        txt = QString("%2::Fatal: %1").arg(msg).arg(timestamp);
        abort();
    }
    QFile outFile(logpath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // ログ
    QString filename = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QDir home = QDir::home();
    logpath = home.filePath("pimetry2" + filename + ".log");
    qInstallMsgHandler(messageHandler);

    w.show();

    return a.exec();
}
