#ifndef LAUNCH_H
#define LAUNCH_H


#include "qtils/logger.hpp"
#include <array>
#include <core/async_logger.h>

#include <QString>

namespace labelimg::utils{

template <typename T = void>
void app_launch() {
    using namespace labelimg::core::logger;


#if defined (PROJECT_VERSION_MAJOR) 
    auto VERSION_MAJOR = PROJECT_VERSION_MAJOR;
#else
    auto VERSION_MAJOR = "UNKNOWN";
#endif

#if defined (PROJECT_VERSION_MINOR) 
    auto VERSION_MINOR = PROJECT_VERSION_MINOR;
#else
    auto VERSION_MINOR = "UNKNOWN";
#endif

#if defined (PROJECT_VERSION_PATCH) 
    auto VERSION_PATCH = PROJECT_VERSION_PATCH;
#else
    auto VERSION_PATCH = "UNKNOWN";
#endif

#endif


    auto* line_0("██████╗ ██████╗  ██████╗ ██╗    ██╗ ██████╗  ██████╗  ██████╗ ");
    auto* line_1("██╔══██╗██╔══██╗██╔═══██╗██║    ██║██╔═══██╗██╔═══██╗██╔═══██╗");
    auto* line_2("██████╔╝██████╔╝██║   ██║██║ █╗ ██║██║   ██║██║   ██║██║   ██║");
    auto* line_3("██╔═══╝ ██╔═══╝ ██║▄▄ ██║██║███╗██║██║▄▄ ██║██║▄▄ ██║██║▄▄ ██║");
    auto* line_4("██║     ██║     ╚██████╔╝╚███╔███╔╝╚██████╔╝╚██████╔╝╚██████╔╝");
    auto* line_5("╚═╝     ╚═╝      ╚══▀▀═╝  ╚══╝╚══╝  ╚══▀▀═╝  ╚══▀▀═╝  ╚══▀▀═╝ ");
    
    async_log << "\t\t" << "=================================================================================" << '\n'
              << "\t\t" << "|\t" << "\t\t\t\t\t\t\t"  << "\t\t|" << '\n'
              << "\t\t" << "|\t" << "\t\t\t\t\t\t\t"  << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_0 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_1 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_2 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_3 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_4 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << line_5 << "\t\t|" << '\n'
              << "\t\t" << "|\t" << "\t\t\t\t\t\t\t"  << "\t\t|" << '\n'
              << "\t\t" << "|\t" << "\t\t\t\t\t\t\t"  << "\t\t|" << '\n'
              << "\t\t" << "|\t" << "                        Project: Labelimg                     " << "\t\t|" << '\n'
              << "\t\t" << "|\t" << "                        Version: " << VERSION_MAJOR << "." 
                                                                          << VERSION_MINOR << "." 
                                                                          << VERSION_PATCH << "\t\t\t\t\t|"      << '\n'
              << "\t\t\t" << "=================================================================================" << '\n'
              << "\n";
}
} // namespace labelimg::utils
#endif
