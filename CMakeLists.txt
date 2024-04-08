cmake_minimum_required(VERSION 3.20.2)

project(CLAP-Restart-Test)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  clap
  GIT_REPOSITORY https://github.com/free-audio/clap.git
  GIT_TAG 1.2.0
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.0
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.90.4
)

FetchContent_Declare(
  clap_imgui_support
  GIT_REPOSITORY https://github.com/free-audio/clap-imgui-support.git
)

FetchContent_MakeAvailable(fmt clap imgui)

set(IMGUI_SOURCE_DIR ${imgui_SOURCE_DIR})

FetchContent_MakeAvailable(clap_imgui_support)

add_library(note-logger MODULE
        note-logger-plugin.cpp
    )
target_link_libraries(note-logger clap fmt imgui-clap-support)

if(APPLE)
    set_target_properties(note-logger PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION clap
            MACOSX_BUNDLE_GUI_IDENTIFIER org.mccann.clap-restart-test.note-logger
            MACOSX_BUNDLE_BUNDLE_NAME note-logger
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/note-logger.plist.in
            )
    target_compile_definitions(note-logger PRIVATE IS_MAC=1)
elseif(UNIX)
    target_compile_definitions(note-logger PRIVATE IS_LINUX=1)
    set_target_properties(note-logger PROPERTIES SUFFIX ".clap" PREFIX "")
else()
    target_compile_definitions(note-logger PRIVATE IS_WIN=1)
    set_target_properties(note-logger PROPERTIES SUFFIX ".clap" PREFIX "")
endif()
