
#include <QApplication>
#include <window/mainwindow.h>

auto main(int argc, char* argv[]) -> int {
    QApplication app(argc, argv);
    
    auto mainwindow = new MainWindow{};

    mainwindow->show();


    return app.exec();
}
