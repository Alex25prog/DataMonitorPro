cmake_minimum_required(VERSION 3.20)
project(DataMonitorPro)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/MP)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Network Sql WebSockets Pdf Widgets
Charts ChartsQml LinguistTools)

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
    src/core/SecretManager.h
    src/network/ExchangeClient.cpp
    src/network/ExchangeClient.h
    src/models/CandleData.h
    src/models/CandleModel.cpp
    src/models/CandleModel.h
    src/chart/TradingChartManager.h
    src/chart/TradingChartManager.cpp

)

target_link_libraries(DataMonitorPro
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    Qt6::WebSockets
    Qt6::Pdf
    #Qt6::Graphs
    Qt6::Widgets
    Qt6::Charts
    Qt6::ChartsQml


)

qt_import_qml_plugins(DataMonitorPro)

# Установка политик Qt для подавления предупреждений
qt_policy(SET QTP0001 NEW) # Для QML resourse prefix
qt_policy(SET QTP0004 NEW) # Для qmldir files

qt_add_qml_module(DataMonitorPro
    URI DataMonitorPro
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES
        qml/main.qml
        qml/DataTableView.qml
    DEPENDENCIES
        QtCharts
        SOURCES src/core/LanguageManager.h
        SOURCES src/core/LanguageManager.h
        SOURCES src/core/LanguageManager.cpp src/core/LanguageManager.h
        SOURCES src/network/ExchangeClient.h
        SOURCES src/network/ExchangeClient.cpp
        SOURCES src/models/CandleData.h
        SOURCES src/models/CandleModel.h
        SOURCES src/models/CandleModel.cpp
        SOURCES src/chart/TradingChartManager.h
        SOURCES src/chart/TradingChartManager.cpp
        #QML_FILES qml/DataTableView.qml

)

# Настройка системы переводов
set(TS_FILES
    "translations/app_ru.ts"
)

qt_add_translations(DataMonitorPro
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/translations"
)

target_compile_definitions(DataMonitorPro
    PRIVATE
    QT_QUICKCONTROLS2_CONFIG=material
)

# Явное указание путей для статического анализатора Qt Creator
set(QML_IMPORT_PATH ${Qt6_INSTALL_PREFIX}/qml CACHE STRING "QML Import Path" FORCE)