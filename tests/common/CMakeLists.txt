add_library(test_common INTERFACE)

target_include_directories(test_common 
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

add_library(test_common_impl
    STATIC
    test_utils.cpp
    mock_utils.cpp
    benchmark_utils.cpp
)


target_link_libraries(test_common 
    INTERFACE
    gtest
    gmock
)

install(FILES
    test_utils.hpp
    mock_utils.hpp
    benchmark_utils.hpp
    fixtures/sample_data.hpp
    fixtures/test_configs.hpp
    DESTINATION include/test_common
)