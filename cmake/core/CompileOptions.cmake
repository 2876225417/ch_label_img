include_guard(GLOBAL)

set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
)

set(DEBUG_COMPILE_FLAGS
    -g3
    -O0
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)

set(DEBUG_LINK_FLAGS
    -fsanitize=address,undefined
)

set(DEBUG_DEFINITIONS
    DEBUG_MODE
)

set(RELEASE_COMPILE_FLAGS
    -O3
    -flto
    -march=native
)

set(RELEASE_LINK_FLAGS
    -flto
)

set(RELEASE_DEFINITIONS
    NDEBUG_MODE
)

set(RELWITHDEBINFO_COMPILE_FLAGS
    -O2
    -g
)

set(RELWITHDEBINFO_DEFINITIONS
    NDEBUG_MODE
)

function(target_apply_options TARGET_NAME)
    # Unnecessary
    target_compile_features(${TARGET_NAME} INTERFACE cxx_std_23)
    
    target_compile_options(${TARGET_NAME} PRIVATE ${WARNING_FLAGS})

    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:${DEBUG_COMPILE_FLAGS}>)
    target_link_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:${DEBUG_LINK_FLAGS}>)
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>)

    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>:${RELEASE_COMPILE_FLAGS}>)
    target_link_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>:${RELEASE_LINK_FLAGS}>)
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>:${RELEASE_DEFINITIONS}>)

    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:RelWithDebInfo>:${RELWITHDEBINFO_COMPILE_FLAGS}>)
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:RelWithDebInfo>:${RELWITHDEBINFO_DEFINITIONS}>)
endfunction()

