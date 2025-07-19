include_guard(GLOBAL)

# 是否在项目源码中显示详细信息

option(DEPRECATED_INFO "Enable deprecated info in project" ON)

if (DEPRECATED_INFO)
    add_compile_definitions(_DEPRECATED_INFO_=1)
endif()