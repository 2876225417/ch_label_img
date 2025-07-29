
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
#include <core/refl/detail/hash.hpp>
#include "core/refl/detail/hash/hash_algorithms.hpp"


auto main(int argc, char* argv[]) -> int {
    QApplication app(argc, argv);

    labelimg::utils::app_launch();

    labelimg::core::formatter::function::FunctionInfo i;

    //std::cout << labelimg::core::formatter::function::get_caller_info_cpp20();

    constexpr auto hasher = labelimg::core::refl::hash::algorithms::HashComputer<labelimg::core::refl::hash::algorithms::fnv1a_algorithm>{};
    constexpr auto hash1 = hasher("Hello world");
    std::cout << "Hash of hash1(Hello world) by fnv1a: " << hash1 << '\n';

    constexpr auto hash2 = hasher.compute("Test", 4);
    std::cout << "Hash of hash1(Test) by fnv1a: " << hash2 << '\n';
    constexpr auto hash3 = hasher.compute<4>("Tes");
    std::cout << "Hash of hash1(Tes) by fnv1a: " << hash3 << '\n';

    std::cout << "Compute_with_algo for ppqwqqq by fnv1a: " << labelimg::core::refl::hash::algorithms::compute_with_algorithm("ppqwqqq");
    std::cout << "Compile time hash: " << labelimg::core::refl::hash::algorithms::compute_with_algorithm<labelimg::core::refl::hash::algorithms::StringHashAlgo::djb2>("Compile time algo");

    auto mainwindow = new MainWindow{};
    mainwindow->show();

    return app.exec();
}

