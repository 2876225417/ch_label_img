
#include <QApplication>
#include <window/mainwindow.h>


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    auto mainwindow = new MainWindow{};

    mainwindow->show();

    return app.exec();
}
