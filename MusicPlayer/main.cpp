#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 设置应用程序图标（使用资源路径）
    a.setWindowIcon(QIcon(":/icon.jpg"));  // 注意前缀 :/ 表示资源路径
    MainWindow w;
    w.show();
    return a.exec();
}
