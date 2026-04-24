import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

ApplicationWindow {
    id: root
    width: 1400
    height: 800
    visible: true
    title: qsTr("DataMonitor Pro")
    background: Rectangle { color: "#1e1e1e" }//Задание принудительного цвета

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10


        // Верхняя панель с кнопками
        RowLayout {
            // Start/Stop Server кнопка
            Button {
                id: serverButton
                text: controller.isServerRunning ? "Stop Server" : "Start Server"

                background:  Rectangle {
                    color: controller.isServerRunning ? "#2e7d32" : "#1565c0"
                    radius: 15
                    opacity: parent.pressed ? 0.7 : 1.0
                }

                contentItem: Text {
                    text: serverButton.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignHCenter
                    font.bold: true
                }

                onClicked: {
                    if (controller.isServerRunning) {
                        controller.stopServer()
                    } else {
                        controller.startServer(8080)
                    }
                }
            }
            //Clear Data кнопка

            Button {
                text: "Clear Data"
                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: "Clear Data"
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }

                onClicked: controller.dataModel.clear()
            }
            //Load History кнопка

            Button {
                text: "Load History (Last 24h)"
                background: Rectangle {
                    color: "#e8e9ef"//grey
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: "Load History (Last 24h)"
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignHCenter
                    font.bold: true
                }

                onClicked: {
                    var from = new Date()
                    from.setHours(from.getHours() - 24)
                    controller.loadHistory(from, new Date())
                }
            }
            //Кнопка Export
            Button{
                text: "Export CSV"
                background: Rectangle {
                    color: "#e8e9ef"//grey
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0

                }
                contentItem: Text {
                    text: "Export CSV"
                    color: "black"
                    font.bold: true
                }

                onClicked: controller.exportToCSV()
            }
            //Кнопка Export в PDF

            Button{
                text: "Export PDF"
                background: Rectangle {
                    color: "#e8e9ef" //grey
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0

                }
                contentItem: Text {

                    text: "Export PDF"
                    color: "black"
                    font.bold: true
                }

                onClicked: controller.exportToPDF()
            }
            //Кнопка парсинга погоды

            Button{
                id: weatherButton
                text: controller.isWeatherRunning ? "Stop Weather" : "Start Weather"

                //Стиль кнопки "Прямоугольник"
                background: Rectangle {
                    color: controller.isWeatherRunning ? "#2e7d32" : "#c62828" //темно-зеленый/темно-красный
                    radius: 15//В пикселях
                    border.width: 1//толщина контура
                    border.color: controller.isWeatherRunning ? "#4caf50" : "#ef5053"
                    opacity: parent.pressed ? 0.7 : 1.0


                     //Анимация при наведении
                    Behavior on color {

                        ColorAnimation { duration: 150 }
                        }
                    }
                contentItem: Text {
                    text: weatherButton.text
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.pixelSize: 12
                }

                onClicked: {
                    if (controller.isWeatherRunning){
                        controller.stopWeather()

                    }else {
                        controller.startWeather()
                    }
                }
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

            // График
            GraphsView {
                id: graphsView
                SplitView.preferredHeight: 300
                Layout.fillWidth: true


                //Настрока темы и внешнего вида
                theme: GraphsTheme {
                   backgroundColor: "transparent" //Прозрачный фон всей области
                   plotAreaBackgroundColor: "#1e1e1e" //темный фон только для графика
                   labelTextColor: "white"
          }

                //определяем кастомный компонент для точек
                Component {
                    id: customPointDelegate
                    Rectangle {
                        //объявляем свойства, чтобы принять их от графика
                        property bool pointSelected: false
                        property color pointColor: "blue"
                        property real pointValueX: 0
                        property real pointValueY: 0

                        //Настраиваем внешний вид
                        width: pointSelected ? 10 : 6
                        height: width
                        radius: width / 2 //Делаем точку круглой
                        color: pointSelected ? "orange" : pointColor
                        border.color: pointSelected ? "red" : "transparent"
                        border.width: 2

                    }
                }

                //Отвечает за точки
                LineSeries{
                    id: tempSeries
                    name: "Temperature"
                    color: "#ff5050"
                    width: 2
                    pointDelegate: customPointDelegate

                   }
                LineSeries {
                    id: pressSeries
                    name: "Pressure"
                    color: "#5090ff"
                    width: 2
                    pointDelegate: customPointDelegate
                }
                LineSeries {
                    id: humSeries
                    name: "Humidity"
                    color: "#50ff50"
                    width: 2
                    pointDelegate: customPointDelegate
                }
                ValueAxis {
                    id: axisX
                    titleText: "Point number"
                    min: 0
                    max: 60 // Показываем последние 60 точек
                }
                ValueAxis {
                    id: axisY
                    titleText: "Value (normalized)"
                    min: 0
                    max: 110
                }

                Component.onCompleted: {
                    graphsView.axisX = axisX;
                    graphsView.axisY = axisY;
                }

            }

            //Легенда
            Row {
                id: customLegend
                anchors.top: graphsView.top     // Привязываем к верху графика
                anchors.right: graphsView.right // Привязываем к правому краю графика
                anchors.margins: 10             // отступ от края
                spacing: 20                     // Расстояние между элементами
                z: graphsView.z + 1             // Чтобы легенда была поверх графика

                // Элемент 1: Температура
                Row {
                    spacing: 5
                    Rectangle {
                        width: 15
                        height: 15
                        radius: 2
                        color: "#ff6666" // Тот же цвет, что у tempSeries
                    }
                    Text {
                        text: "Temperature"
                        color: "white"
                        font.pixelSize: 12
                    }
                }

                //Элемент 2: Давление
                Row {
                    spacing: 5
                    Rectangle {
                        width: 15
                        height: 15
                        radius: 2
                        color: "#6666ff" // Тот же цвет, что у pressSeries
                    }
                    Text {
                        text: "Pressure"
                        color: "white"
                        font.pixelSize: 12
                    }
                }
                //Элемент 3: Влажность
                Row {
                    spacing: 5
                    Rectangle {
                        width: 15
                        height: 15
                        radius: 2
                        color: "#66ff66" //Тот же цвет, что у humSeries
                    }
                    Text {
                        text: "Humidity"
                        color: "white"
                        font.pixelSize: 12
                    }
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
            color: "#2d2d2d"
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


        Connections {
            target: controller
            function onChartDataReceived(index, value, type) {
                console.log("QML received:", type, index, value)

                // При первом получении данных добавляем нулевые точки для всех серий
                if (tempSeries.count === 0 && pressSeries.count === 0 && humSeries.count === 0) {
                    tempSeries.append(0, 0);
                    pressSeries.append(0, 0);
                    humSeries.append(0, 0);
                    }

                //Принудительно устанавливаем минимальный индекс для всех серий
                if (type === "temperature") {
                   tempSeries.append(index, value);

                } else if (type === "pressure") {
                    pressSeries.append(index, value);

                } else if (type === "humidity") {
                   humSeries.append(index, value);

             }


            // обновляем оси
            if (value > axisY.max) axisY.max = value + 10;
            if (value < axisY.min && value > -10) axisY.min = value - 10;

            //Показываем последние 50 точек
            if (index > 50) {
                axisX.min = index - 50;
                axisX.max = index + 5;
            } else {
                axisX.min = 0;
                axisX.max = 60;
            }
        }
    }
}



