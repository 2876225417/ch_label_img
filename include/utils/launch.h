#ifndef LAUNCH_H
#define LAUNCH_H


#include <core/async_logger.h>

#ifdef EXPORT_PROJ_INFO
#include "proj_config.h"
#endif




namespace labelimg::utils{

template <typename T = void>
void app_launch() {
    using namespace labelimg::core::logger;

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
              #ifdef EXPORT_PROJ_INFO
              << "\t\t" << "|\t" << "                        Project: "   << PROJECT_NAME          << "\t\t\t\t|" << '\n'
              << "\t\t" << "|\t" << "                        Version: "   << PROJECT_VERSION_MAJOR << "." 
                                                                          << PROJECT_VERSION_MINOR << "." 
                                                                          << PROJECT_VERSION_PATCH << "\t\t\t\t\t|" << '\n'
              #endif 
              << "\t\t" << "=================================================================================" << '\n'
              << "\n";
}
} // namespace labelimg::utils
#endif
