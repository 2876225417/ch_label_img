
include(${CMAKE_CURRENT_LIST_DIR}/PrettyPrint.cmake)

function(show_qt_info)
    set(options DETAILED)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    if (NOT Qt6_FOUND AND NOT Qt5_FOUND)
        pretty_message(OPTION "Qt not found, skipping Qt into display")
        return()
    endif()

    pretty_message(STATUS "==============================================")
    pretty_message(STATUS "Qt Configuration Information")
    pretty_message(STATUS "==============================================")

    if (Qt6_FOUND)
        set(QT_VERSION_MAJOR "6")
        set(QT_PREFIX "Qt6")
    elseif(Qt5_FOUND)
        set(QT_VERSION_MAJOR "5")
        set(QT_PREFIX "Qt5")
    endif()

    if (TARGET ${QT_PREFIX}::Core)
        get_target_property(QT_VERSION ${QT_PREFIX}::Core VERSION)
        pretty_message(INFO "Qt Version: ${QT_VERSION}")
        pretty_message(INFO "Qt Major Version: ${QT_VERSION_MAJOR}")
    endif()

    if (DEFINED ${QT_PREFIX}_DIR)
        pretty_message(INFO "Qt Directory: ${${QT_PREFIX}_DIR}")
    endif()

    if (TARGET ${QT_PREFIX}::Core)
        get_target_property(QT_CORE_LOCATION ${QT_PREFIX}::Core LOCATION)
        if (QT_CORE_LOCATION)
            get_filename_component(QT_INSTALL_PREFIX "${QT_CORE_LOCATION}" DIRECTORY)
            get_filename_component(QT_INSTALL_PREFIX "${QT_INSTALL_PREFIX}" DIRECTORY)
            pretty_message(INFO "Qt Install Prefix: ${QT_INSTALL_PREFIX}")
        endif()
    endif()

    pretty_message(INFO "Qt Components Found:")
    _show_qt_compoents(${QT_PREFIX})

    if (ARG_DETAILED)
        pretty_message(INFO "")
        pretty_message(INFO "Detailed Qt Information:")
        _show_qt_detailed_info(${QT_PREFIX})
    endif()

    pretty_message(STATUS "==============================================")
endfunction()

function(_show_qt_compoents QT_PREFIX)
    set(QT_COMMON_COMPONENTS
        Core
        Gui
        Widgets
        Network
        Sql
        Xml
        Concurrent
        PrintSupport
        OpenGL
        Svg
        Test
        Designer
        Help
        LinguistTools
        UiTools
        Quick
        QuickWidgets
        Qml
        QmlModels
        WebEngineCore
        WebEngineWidgets
        Charts
        DataVisualization
        3DCore
        3DRender
        3DInput
        3DLogic
        3DExtras
        SerialPort
        Multimedia
        MultimediaWidgets
        Positioning
        Location
        Sensors
        WebSockets
        WebChannel
        Bluetooth
        Nfc
    )

    set(FOUND_COMPONENTS "")
    set(MISSING_COMPONENTS "")
    
    foreach(component ${QT_COMMON_COMPONENTS})
        if(TARGET ${QT_PREFIX}::${component})
            list(APPEND FOUND_COMPONENTS ${component})
        else()
            list(APPEND MISSING_COMPONENTS ${component})
        endif()
    endforeach()

    foreach(component ${FOUND_COMPONENTS})
        if (TARGET ${QT_PREFIX}::${component})
            get_target_property(comp_version ${QT_PREFIX}::${component} VERSION)
            if (comp_version)
                pretty_message(SUCCESS "  ✓ ${component} (${comp_version})")
            else()
                pretty_message(SUCCESS "  ✓ ${component}")
            endif()
        endif()
    endforeach()
    
    list(LENGTH MISSING_COMPONENTS MISSING_COUNT)
    if(MISSING_COUNT GREATER 0)
        pretty_message(DEBUG "Components not found: ${MISSING_COUNT}")
        foreach(component ${MISSING_COMPONENTS})
            pretty_message(DEBUG "  ○ ${component}")
        endforeach()
    endif()
endfunction()

function(_show_qt_detailed_info QT_PREFIX)
    pretty_message(INFO "Qt Build Configuration:")
    
    if (DEFINED QT_IS_STATIC)
        if (QT_IS_STATIC)
            pretty_message(INFO "  Build Type: Static")
        else()
            pretty_message(INFO "  Build Type: Shared")
        endif()
    endif()

    pretty_message(INFO "Qt Tools: ")
    set(QT_TOOLS moc rcc uic qmake lupdate lrelease)
    foreach(tool ${QT_TOOLS})
        string(TOUPPER ${tool} TOOL_UPPER)
        if (DEFINED QT_${TOOL_UPPER}_EXECUTABLE OR DEFINED ${QT_PREFIX}_${TOOL_UPPER}_EXECUTABLE)
            if (DEFINED ${QT_PREFIX}_${TOOL_UPPER}_EXECUTABLE)
                set(tool_path ${${QT_PREFIX}_${TOOL_UPPER}_EXECUTABLE})
            else()
                set(tool_path ${QT_${TOOL_UPPER}_EXECUTABLE})
            endif()

            if (EXISTS ${tool_path})
                pretty_message(SUCCESS "  ✓ ${tool}: ${tool_path}")
            else()
                pretty_message(OPTION  "  ? ${tool}: ${tool_path} (not found)")
            endif()
        endif()
    endforeach()

    if (DEFINED QT_PLUGINS_DIR OR DEFINED ${QT_PREFIX}_PLUGINS_DIR)
        if (DEFINED ${QT_PREFIX}_PLUGINS_DIR)
            pretty_message(INFO     "Qt Plugins Directory: ${${QT_PREFIX}_PLUGINS_DIR}")
        else()
            pretty_message(OPTIONAL "Qt Plugins Directory: ${QT_PLUGINS_DIR}")
        endif()
    endif()
    
    


endfunction()
