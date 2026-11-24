#include "mainwindow.h"

#include <QApplication>
#include <QStyleHints>
#include <QSqlDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 统一使用浅色配色方案，避免 Windows 深色模式下 QTableView / 表单文字变白导致不可见
    a.setStyle("Fusion");
    a.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    qDebug() << "Qt支持的数据库驱动：" << QSqlDatabase::drivers();

    MainWindow w;
    w.show();

    return a.exec();
}
