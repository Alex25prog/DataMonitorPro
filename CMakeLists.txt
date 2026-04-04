cmake_minimum_required(VERSION 3.20)
project(DataMonitorPro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Network Sql WebSockets)

qt_add_executable(DataMonitorPro
    src/main.cpp
    src/core/DataPoint.cpp
    src/core/DataPoint.h
    src/core/DataProcessor.cpp
    src/core/DataProcessor.h
    src/network/WebSocketServer.cpp
    src/network/WebSocketServer.h
    src/network/WeatherFetcher.cpp
    src/network/WeatherFetcher.h
    src/models/DataModel.cpp
    src/models/DataModel.h
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/controllers/MainController.cpp
    src/controllers/MainController.h
    src/export/ReportExporter.h
    src/export/ReportExporter.cpp
)

target_link_libraries(DataMonitorPro
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    Qt6::WebSockets

)

qt_add_qml_module(DataMonitorPro
    URI DataMonitorPro
    VERSION 1.0
    QML_FILES
        qml/main.qml
        SOURCES src/network/WeatherFetcher.h
        SOURCES src/network/WeatherFetcher.cpp
)

target_compile_definitions(DataMonitorPro
    PRIVATE
    QT_QUICKCONTROLS2_CONFIG=material
)