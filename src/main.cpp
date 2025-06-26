
#include "stacked_page/label_img.h"
#include "utils/launch.h"
#include "widget/message_box.h"
#include <QApplication>
#include <array>
#include <cstdlib>
#include <window/mainwindow.h>

#include <qtils/logger.hpp>



auto main(int argc, char* argv[]) -> int {
    QApplication app(argc, argv);

    labelimg::utils::app_launch();



    auto mainwindow = new MainWindow{};
    mainwindow->show();

    return app.exec();
}
