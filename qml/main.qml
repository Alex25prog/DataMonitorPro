import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    id: root
    width: 1400
    height: 800
    visible: true
    title: qsTr("DataMonitor Pro")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Верхняя панель с кнопками
        RowLayout {
            Button {
                text: controller.isServerRunning ? "Stop Server" : "Start Server"
                onClicked: {
                    if (controller.isServerRunning) {
                        controller.stopServer()
                    } else {
                        controller.startServer(8080)
                    }
                }
            }

            Button {
                text: "Clear Data"
                onClicked: controller.dataModel.clear()
            }

            Button {
                text: "Load History (Last 24h)"
                onClicked: {
                    var from = new Date()
                    from.setHours(from.getHours() - 24)
                    controller.loadHistory(from, new Date())
                }
            }
            Button{
                text: "Export CSV"
                onClicked: controller.exportToCSV()
            }

            Button{
                text: "Export PDF"
                onClicked: controller.exportToPDF()
            }

            Button{
                text: controller.isWeatherRunning ? "Stop Weather" : "Start Weather"
                onClicked: {
                    if (controller.isWeatherRunning){
                        controller.stopWeather()

                    }else {
                        controller.startWeather()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
            }

            Label {
                text: controller.isServerRunning ? "● Server Running" : "○ Server Stopped"
                color: controller.isServerRunning ? "#4caf50" : "#f44336"
            }
        }

        // Фильтры
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#808080"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label { text: "Filter by type:"

                        color: "#B22222"
                }
                ComboBox {
                    id: typeFilter
                    model: ["All", "temperature", "pressure", "humidity"]
                    onCurrentTextChanged: {
                        if (currentText === "All") {
                            controller.dataModel.resetFilters()
                        } else {
                            controller.dataModel.setTypeFilter(currentText)
                        }
                    }
                }
            }
        }

        // Основная область: график и таблица
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Vertical

            // График (заглушка, без QtCharts)
            Rectangle {
                SplitView.preferredHeight: 300
                color: "#1e1e1e"
                border.color: "#3d3d3d"

                Label {
                 anchors.centerIn: parent
                 text: "Chart View (Coming Soon)"
                 color: "#808080"

                }

            }

            // Таблица данных
            Rectangle {
                SplitView.fillHeight: true
                color: "#1e1e1e"
                border.color: "#3d3d3d"

                ListView {
                    id: tableView
                    width: parent.width
                    anchors.fill: parent
                    anchors.margins: 5
                    model: controller.dataModel
                    clip: true

                    header: Rectangle {
                        width: tableView.width
                        height: 40
                        color: "#2d2d2d"

                        Row {
                            anchors.fill: parent
                            anchors.margins: 5
                            spacing: 10

                            Rectangle { width: 180; height: 30; color: "#3d3d3d"; radius: 3; Text { text: "Timestamp"; anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 100; height: 30; color: "#3d3d3d"; radius: 3; Text { text: "Type"; anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 100; height: 30; color: "#3d3d3d"; radius: 3; Text { text: "Value"; anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 80; height: 30; color: "#3d3d3d"; radius: 3; Text { text: "Unit"; anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 700; height: 30; color: "#3d3d3d"; radius: 3; Text { text: "Details"; anchors.centerIn: parent; color: "white" } }
                        }
                    }

                    delegate: Rectangle {
                        width: tableView.width
                        height: 35
                        color: index % 2 === 0 ? "#252525" : "#2a2a2a"

                        Row {
                            anchors.fill: parent
                            anchors.margins: 5
                            spacing: 10

                            Text { width: 180; text: model.timestamp || ""; color: "white"; elide: Text.ElideRight; horizontalAlignment: Text.AlignHCenter }
                            Text { width: 100; text: model.type || ""; color: "white"; horizontalAlignment: Text.AlignHCenter }
                            Text { width: 100; text: model.value ? model.value.toFixed(2) : "0.00"; color: "#4caf50"; horizontalAlignment: Text.AlignHCenter }
                            Text { width: 80; text: model.unit || ""; color: "white"; horizontalAlignment: Text.AlignHCenter }
                            Text { width: 700; text: model.string || ""; color: "#808080"; elide: Text.ElideRight; horizontalAlignment: Text.AlignHCenter }
                        }
                    }
                }
            }
        }

        // Нижняя панель статистики
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#808080"
            radius: 3

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                Label { text: " Total points: " + controller.dataModel.count
                color: "#B22222" }
                Label { text: "|" }
                Label { text: "Server: " + (controller.isServerRunning ? "Active" : "Inactive")
                color: "#B22222" }
                Label { text: "|" }
                Label { text: " Database: PostgreSQL"
                color: "#B22222" }
                Label { text: "|"}
                Label {
                    text: "Weather: " + (controller.isWeatherRunning ? "Active" : "Inactive")
                    color: controller.isWeatherRunning ? "#4caf50" : "#f44336"
                }
            }
        }
    }
}