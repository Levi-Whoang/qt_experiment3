cmake_minimum_required(VERSION 3.16)

project(experiment3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        idatabase.cpp
        idatabase.h
        sexdelegate.cpp
        sexdelegate.h
        loginview.cpp
        loginview.h
        welcomeview.cpp
        welcomeview.h
        patientview.cpp
        patientview.h
        patienteditview.cpp
        patienteditview.h
        managementviews.cpp
        managementviews.h
)

qt_add_executable(experiment3
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(experiment3 PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

# 自动将 QMYSQL 插件与 MySQL 客户端运行库复制到可执行文件目录，确保在 Qt Creator 中直接运行可用
add_custom_command(TARGET experiment3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/mysql_runtime"
        "$<TARGET_FILE_DIR:experiment3>"
    COMMENT "Deploying QMYSQL driver and MySQL client runtime DLLs"
)

set_target_properties(experiment3 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS experiment3
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(experiment3)
