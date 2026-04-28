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

    //Функция для обновления списка городов
    function updateCityList() {
        var cities = {
            "Russia": ["Select City", "Moscow", "Saint Petersburg", "Novosibirsk", "Kazan", "Yekaterinburg", "Voronezh"],
            "USA": ["Select City", "New York", "Los Angeles", "Chicago", "Houston", "Miami"],
            "Germany": ["Select City", "Berlin", "Munich", "Hamburg", "Cologne", "Frankfurt"],
            "France": ["Select City", "Paris", "Marseille", "Lyon", "Toulouse", "Nice"],
            "UK": ["Select City", "London", "Manchester", "Birmingham", "Liverpool", "Edinburgh"],
            "Japan": ["Select City", "Tokyo", "Osaka", "Kyoto", "Yokohama", "Nagoya"]
        }

        var selectedCountry = countrySelect.currentText
        var cityList = cities[selectedCountry] || ["Moscow"]
        citySelect.model = cityList
        citySelect.currentIndex = 0
    }

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

                onClicked: {
                    controller.clearData()// Вызываем метод контроллера
               }
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

            //Выбор страны и города

            Rectangle {
                height: 35
                width: 300
                color: "#3d3d3d"
                radius: 5

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    ComboBox {
                        id: countrySelect
                        model: ["▼ Select Country", "Russia", "USA", "Germany", "France", "UK", "Japan"]
                        currentIndex: 0
                        font.pixelSize: 12
                        implicitWidth: 145

                        //Стиль фона
                        background: Rectangle {
                            color: "#e8e9ef"
                            radius: 3
                            border.color: "#c0c0c0"
                            border.width: 1
                        }

                        //Стиль текста
                        contentItem: Text {
                            text: countrySelect.currentText
                            color: currentIndex === 0 ? "#666666" : "#2c3e50"
                            font.bold: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignHCenter
                            leftPadding: 8

                            }

                        //Стиль стрелки
                        indicator: Canvas {
                            id: canvas
                            x: countrySelect.width - width - 10
                            y: countrySelect.height / 2 - height / 2
                            width: 12
                            height: 8
                            contextType: "2d"
                            onPaint: {
                                context.reset()
                                context.moveTo(0, 0)
                                context.lineTo(width, 0)
                                context.lineTo(width / 2, height)
                                context.fillStyle = "#666666"
                                context.fill()
                            }

                        }

                        // Стиль всплывающего списка
                        popup: Popup {
                            y: countrySelect.height
                            width: countrySelect.width
                            implicitHeight: contentItem.implicitHeight
                            padding: 1

                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#c0c0c0"
                                border.width: 1
                                radius: 3
                            }

                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: countrySelect.popup.visible ? countrySelect.delegateModel : null
                                currentIndex: countrySelect.heightlightedIndex

                                delegate: ItemDelegate {
                                    width: countrySelect.width
                                    height: 30
                                    highlighted: ListView.isCurrentItem

                                    contentItem: Text {
                                        text: modelData
                                        color: highlighted ? "#ffffff" : "#2c3e50"
                                        font.pixelSize: 12
                                        font.bold: highlighted ? true : false
                                        horizontalAlignment: Text.AlignLeft
                                        verticalAlignment: Text.AlignHCenter
                                        leftPadding: 8

                                    }

                                    background: Rectangle {
                                        color: highlighted ? "#4CAF50" : "#f5f5f5"
                                    }
                                }

                            }

                        }
                        onCurrentTextChanged: {
                            if (currentIndex > 0) {
                            updateCityList()

                    }

                }
            }
                ComboBox {
                    id:citySelect
                    model: ["▼ Select City", "Moscow", "Saint Petersburg", "Novosibirsk", "Kazan", "Voronezh"]
                    currentIndex: 0
                    font.pixelSize: 12
                    implicitWidth: 145
                    font.bold: true

                    // Стиль фона
                    background: Rectangle {
                        color: "#e8e9ef"
                        radius: 3
                        border.color: "#c0c0c0"
                        border.width: 1
                    }

                    //Стиль текста
                    contentItem: Text {
                        text: citySelect.currentText
                        color: currentIndex === 0 ? "#666666" : "#2c3e50"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignHCenter
                        leftPadding: 8
                    }

                    // Стиль стрелки
                    indicator: Canvas {
                        id: cityCanvas
                        x: citySelect.width - width - 10
                        y: citySelect.height / 2 - height / 2
                        width: 12
                        height: 8
                        contextType: "2d"
                        onPaint: {
                            context.reset()
                            context.moveTo(0, 0)
                            context.lineTo(width, 0)
                            context.lineTo(width / 2, height)
                            context.fillStyle = "#666666"
                            context.fill()
                        }

                    }

                    // Стиль всплывающего окна
                    popup: Popup {
                        y: citySelect.height
                        width: citySelect.width
                        implicitHeight: contentItem.implicitHeight
                        padding: 1

                        background: Rectangle {
                            color: "#ffffff"
                            border.color: "#c0c0c0"
                            border.width: 1
                            radius: 3
                        }

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: citySelect.popup.visible ? citySelect.delegateModel : null
                            currentIndex: citySelect.highlightedIndex

                            delegate: ItemDelegate {
                                width: citySelect.width
                                height: 30
                                highlighted: ListView.isCurrentItem

                                contentItem: Text {
                                    text: modelData
                                    color: highlighted ? "ffffff" : "#2c3e50"
                                    font.pixelSize: 12
                                    font.bold: highlighted ? true : false
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                }

                                background: Rectangle {
                                    color: highlighted ? "#4CAF50" : "f5f5f5"
                                }
                            }
                        }
                    }

                    onCurrentTextChanged: {
                        //Обновляем погоду при смене города
                        if (currentIndex > 0 && controller.isWeatherRunning) {
                            controller.stopWeather()
                            controller.startWeather()

                        }
                    }
                }
            }
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
            color: "#e8e9ef" //Светлый фон
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label { text: "Filter by type:"

                        color: "#2c3e50" //Темный текст
                        font.bold: true
                        font.pixelSize: 12
                }
                ComboBox {
                    id: typeFilter
                    model: ["All", "temperature", "pressure", "humidity"]
                    currentIndex: 0
                    font.pixelSize: 12
                    font.bold: true
                    implicitWidth: 120

                    background: Rectangle {
                        color: "#e8e8ef"
                        radius: 4
                        border.color: "#c0c0c0"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: typeFilter.currentText
                        color: currentIndex === 0 ? "#666666" : "#2c3e50"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignHCenter

                    }

                    indicator: Canvas {
                        id: filterCanvas
                        x: typeFilter.width - width - 10
                        y: typeFilter.height / 2 - height / 2
                        width: 12
                        height: 8
                        contextType: "2d"
                        onPaint: {
                            context.reset()
                            context.moveTo(0, 0)
                            context.lineTo(width, 0)
                            context.lineTo(width / 2, height)
                            context.fillStyle = "#666666"
                            context.fill()
                        }

                    }

                    popup: Popup {
                    y: typeFilter.height
                    width: typeFilter.width
                    implicitHeight: contentItem.implicitHeight
                    padding: 1

                    background: Rectangle {
                        color: "#ffffff"
                        border.color: "#c0c0c0"
                        border.width: 1
                        radius: 4
                    }

                    contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: typeFilter.popup.visible ? typeFilter.delegateModel : null
                    currentIndex: typeFilter.highlightedIndex

                    delegate: ItemDelegate {
                        width: typeFilter.width
                        height: 30
                        highlighted: ListView.isCurrentItem

                        contentItem: Text {
                            text: modelData
                            color: highlighted ? "#ffffff" : "#2c3e50"
                            font.bold: highlighted ? true : false
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                        }

                        background: Rectangle {
                            color: highlighted ? "#4CAF50" : "#f5f5f5"
                        }
                    }
                }
            }

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
            function onClearGraphRequested() {
                //Очищаем все серии графика
                tempSeries.clear()
                pressSeries.clear()
                humSeries.clear()

                //Сбрасываем оси
                axisX.min = 0
                axisX.max = 60
                axisY.min = 0
                axisY.max = 110

                console.log("Graph cleared")
            }

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



