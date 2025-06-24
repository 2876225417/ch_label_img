
#include "widget/message_box.h"
#include <QApplication>
#include <window/mainwindow.h>

#include <qtils/logger.hpp>


auto main(int argc, char* argv[]) -> int {
    QApplication app(argc, argv);
    
    auto mainwindow = new MainWindow{};
    mainwindow->show();

    //MessageBox::show_info("Test Info");

    return app.exec();
}
