
#include "core/async_logger.h"
#include "core/formatter/function.h"
#include "stacked_page/label_img.h"
#include "utils/launch.h"
#include "widget/message_box.h"
#include <QApplication>
#include <array>
#include <cstdlib>
#include <window/mainwindow.h>
#include <core/asm/hp_timer.hpp>
#include <qtils/logger.hpp>

struct [[meta]] A {

};


auto main(int argc, char* argv[]) -> int {
    QApplication app(argc, argv);

    labelimg::utils::app_launch();

    labelimg::core::formatter::function::FunctionInfo i;

    //std::cout << labelimg::core::formatter::function::get_caller_info_cpp20();



    auto mainwindow = new MainWindow{};
    mainwindow->show();

    return app.exec();
}

