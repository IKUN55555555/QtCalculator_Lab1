#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // 支持命令行参数打开文件：TextEditor.exe <文件名>
    if (a.arguments().size() > 1) {
        w.loadFileFromCommandLine(a.arguments().at(1));
    }

    return a.exec();
}
