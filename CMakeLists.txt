cmake_minimum_required(VERSION 3.20)
project(DataMonitorPro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Network Sql)

qt_add_executable(DataMonitorPro
    src/main.cpp
    src/core/DataPoint.cpp
    src/core/DataProcessor.cpp
    src/models/DataModel.cpp
    src/network/DataServer.cpp
    src/database/DatabaseManager.cpp
    src/controllers/MainController.cpp
)

target_link_libraries(DataMonitorPro
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
)

qt_add_qml_module(DataMonitorPro
    URI DataMonitorPro
    VERSION 1.0
    QML_FILES
        qml/main.qml
)

target_compile_definitions(DataMonitorPro
    PRIVATE
    QT_QUICKCONTROLS2_CONFIG=material
)