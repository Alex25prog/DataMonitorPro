import QtQuick
import QtCharts
import QtQuick.Controls
import QtQuick.Layouts


ApplicationWindow {
    id: root
    width: 1400
    height: 800
    visible: true
    title: qsTr("DataMonitor Pro")
    background: Rectangle { color: "#1e1e1e" }

    // Переменные для переключения графиков
    property bool showWeatherGraph: true
    property bool showTradingGraph: false

    // Функция для обновления списка городов
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
        var cityList = cities[selectedCountry]

        if (cityList) {
            citySelect.model = cityList
            citySelect.currentIndex = 0
            controller.setCity("")
        } else {
            citySelect.model = ["Select City"]
            citySelect.currentIndex = 0
            controller.setCity("")
        }
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

                background: Rectangle {
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

            // Clear Data кнопка
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
                    controller.clearData()
                }
            }

            // Load History кнопка
            Button {
                text: "Load History (Last 24h)"
                background: Rectangle {
                    color: "#e8e9ef"
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

            // Export CSV
            Button {
                text: "Export CSV"
                background: Rectangle {
                    color: "#e8e9ef"
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

            // Export PDF
            Button {
                text: "Export PDF"
                background: Rectangle {
                    color: "#e8e9ef"
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

            // Выбор страны и города
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

                        onCurrentTextChanged: {
                            if (currentIndex > 0) {
                                updateCityList()
                            } else {
                                citySelect.model = ["Select City"]
                                citySelect.currentIndex = 0
                                controller.setCity("")
                            }
                        }

                        background: Rectangle {
                            color: "#e8e9ef"
                            radius: 3
                            border.color: "#c0c0c0"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: countrySelect.currentText
                            color: currentIndex === 0 ? "#666666" : "#2c3e50"
                            font.bold: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignHCenter
                            leftPadding: 8
                        }

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
                                currentIndex: countrySelect.highlightedIndex

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
                    }
                    // Кнопки переключения графиков
                    Button {
                        text: "Weather"
                        background: Rectangle {
                            color: showWeatherGraph ? "#4caf50" : "#e8e9f"
                            radius: 8
                        }
                        contentItem: Text {
                           text: "Weather"
                           color: showWeatherGraph ? "white" : "black"
                           font.bold: true
                        }

                        onClicked: {
                            showWeatherGraph = true
                            showTradingGraph = false
                        }
                    }
                    // Кнопка переключения биржи
                    Button {
                        text: "Birzha"
                        background: Rectangle {
                            color: showTradingGraph ? "#4caf50" : "#e8e9ef"
                            radius: 8
                        }
                        contentItem: Text {
                            text: "Birzha"
                            color: showTradingGraph ? "white" : "black"
                            font.bold: true
                        }
                        onClicked: {
                            showWeatherGraph = false
                            showTradingGraph = true
                        }
                    }

                    ComboBox {
                        id: citySelect
                        model: ["▼ Select City"]
                        currentIndex: 0
                        enabled: true

                        onCurrentTextChanged: {
                            if (currentIndex > 0 && currentText !== "Select City" && currentText !== "▼ Select City") {
                                controller.setCity(currentText)
                            } else {
                                controller.setCity("")
                            }
                        }

                        font.pixelSize: 12
                        implicitWidth: 145
                        font.bold: true

                        background: Rectangle {
                            color: citySelect.enabled ? "#e8e9ef" : "#cccccc"
                            radius: 3
                            border.color: "#c0c0c0"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: citySelect.currentText
                            color: currentIndex === 0 ? "#666666" : "#2c3e50"
                            font.pixelSize: 12
                            font.bold: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignHCenter
                            leftPadding: 8
                        }

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
                                        color: highlighted ? "#ffffff" : "#2c3e50"
                                        font.pixelSize: 12
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
                    }
                }
            }

            // Кнопка погоды
            Button {
                id: weatherButton
                text: controller.isWeatherRunning ? "Stop Weather" : "Start Weather"
                enabled: controller.isCitySelected

                background: Rectangle {
                    color: {
                        if (!enabled) return "#999999"
                        return controller.isWeatherRunning ? "#2e7d32" : "#c62828"
                    }
                    radius: 15
                    border.width: 1
                    border.color: {
                        if (!enabled) return "#666666"
                        return controller.isWeatherRunning ? "#4caf50" : "#ef5053"
                    }
                    opacity: parent.pressed && enabled ? 0.7 : 1.0

                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }

                contentItem: Text {
                    text: weatherButton.text
                    color: enabled ? "black" : "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.pixelSize: 12
                }

                onClicked: {
                    if (!controller.isCitySelected) {
                        console.log("No city selected")
                        return
                    }
                    if (controller.isWeatherRunning) {
                        controller.stopWeather()
                    } else {
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
            color: "#e8e9ef"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Filter by type:"
                    color: "#2c3e50"
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
                        color: "#e8e9ef"
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
                                    font.pixelSize: 12
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

            // График погоды
            ChartView {
                id: weatherChart
                SplitView.preferredHeight: 300
                Layout.fillWidth: true
                theme: ChartView.ChartThemeDark
                antialiasing: true
                animationOptions: ChartView.SeriesAnimations
                backgroundColor: "#1e1e1e"
                visible: showWeatherGraph

                  // @disable-check M300
                 ValueAxis {
                    id: weatherAxisX
                    titleText: "Point number"
                    min: 0
                    max: 60
                    gridVisible: true
                    gridLineColor: "#404040"
                }
                 // @disable-check M300
                 ValueAxis {
                    id: weatherAxisY_Temp
                    titleText: "Temperature (°C)"
                    color: "#ff5050"
                    gridVisible: true
                    gridLineColor: "#404040"
                    min: -30
                    max: 40
                }
                 // @disable-check M300
                 ValueAxis {
                    id: weatherAxisY_Press
                    titleText: "Pressure (hPa)"
                    color: "#5090ff"
                    gridVisible: false
                    min: 950
                    max: 1050

                 }
                // @disable-check M300
                 ValueAxis {
                    id: weatherAxisY_Hum
                    titleText: "Humidity (%)"
                    color: "#50ff50"
                    gridVisible: false
                    min: 0
                    max: 100


                }

                LineSeries {
                    id: tempSeries
                    name: "Temperature"
                    color: "#ff5050"
                    width: 2
                    axisX: weatherAxisX
                    axisY: weatherAxisY_Temp
                }

                LineSeries {
                    id: pressSeries
                    name: "Pressure"
                    color: "#5090ff"
                    width: 2
                    axisX: weatherAxisX
                    axisYRight: weatherAxisY_Press
                }

                LineSeries {
                    id: humSeries
                    name: "Humidity"
                    color: "#50ff50"
                    width: 2
                    axisX: weatherAxisX
                    axisYRight: weatherAxisY_Hum

                }

                legend {
                    visible: true
                    alignment: Qt.AlignTop
                    labelColor: "white"
                    color: "#1e1e1e"
                }
            }


            // График биржи
            ChartView {
                id: chartView
                SplitView.preferredHeight: 300
                Layout.fillWidth: true
                theme: ChartView.ChartThemeDark
                antialiasing: true
                animationOptions: ChartView.SeriesAnimations
                backgroundColor: "#1e1e1e"
                visible: showTradingGraph

                DateTimeAxis {
                    id: axisX
                    format: "hh.mm.ss"
                    titleText: "Time"
                    gridVisible: true
                    gridLineColor: "#404040"
                    labelsFont.pixelSize: 10
                    titleFont.pixelSize: 12
                }
                // @disable-check M300
                ValueAxis {
                    id: axisY
                    titleText: "Price"
                    gridVisible: true
                    gridLineColor: "#404040"
                    labelsFont.pixelSize: 10
                    titleFont.pixelSize: 12
                }

                CandlestickSeries {
                    id: candlestickSeries
                    name: "Price"
                    increasingColor: "#26a69a"
                    decreasingColor: "#ef5350"
                    bodyWidth: 0.7
                    maximumColumnWidth: 30
                    minimumColumnWidth: 5

                    onClicked: {
                        console.log("Candle clicked:", timestamp, "Open:", open, "High:", high, "Low:", low, "Close:", close)
                    }

                    onHovered: {
                        if (hovered) {
                            console.log("Hovering over candle:", timestamp)
                        }
                    }
                }

                LineSeries {
                    id: movingAverageSeries
                    name: "MA(20)"
                    color: "#ff9800"
                    width: 2
                    visible: false
                }

                legend {
                    visible: true
                    alignment: Qt.AlignTop
                    labelColor: "white"
                    color: "#1e1e1e"

                }

                // Кнопки управления масштабом
                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: 10
                    anchors.rightMargin: 220 // отступ с права
                    anchors.margins: 10
                    radius: 5
                    z: 10

                    RowLayout {
                        anchors.fill: parent
                        spacing: 5

                        Button {
                            id: btnIn
                            text: "+"
                            font.pixelSize: 16
                            font.bold: true
                            onClicked: {
                                chartView.zoomIn()
                            }

                            background: Rectangle {
                                color: "#42f5e3"
                                radius: btnIn.height / 2
                            }
                        }
                        Button {
                            id: btnIn2
                            text: "-"
                            font.pixelSize: 16
                            font.bold: true
                            onClicked: {
                                chartView.zoomOut()

                            }

                            background: Rectangle {
                                color: "#42f5e3"
                                radius: btnIn2.height / 2
                            }
                        }
                        Button {
                            id: btnIn3
                            text: "↺"
                            font.pixelSize: 16
                            font.bold: true
                            onClicked: {
                                chartView.zoomReset()
                            }

                            background: Rectangle {
                                color: "#42f5e3"
                                radius: btnIn2.height / 2
                            }
                        }
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

                Label { text: " Total points: " + controller.dataModel.count; color: "#B22222" }
                Label { text: "|" }
                Label { text: "Server: " + (controller.isServerRunning ? "Active" : "Inactive"); color: "#B22222" }
                Label { text: "|" }
                Label { text: " Database: PostgreSQL"; color: "#B22222" }
                Label { text: "|" }
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
            candlestickSeries.clear()
            movingAverageSeries.clear()
            axisX.min = 0
            axisX.max = 60
            axisY.min = 0
            axisY.max = 100
            console.log("Graph cleared")
        }

        function onCandleDataReceived(open, high, low, close, timestamp) {
            console.log("Candle received:", timestamp, open, high, low, close)
            var dateTime = new Date(timestamp)
            candlestickSeries.append(dateTime, open, high, low, close)

            if (high > axisY.max) axisY.max = high + (high * 0.05)
            if (low < axisY.min) axisY.min = low - (low * 0.05)
        }

        function onChartDataReceived(index, value, type) {
            // Для совместимости с погодой - пока не используется
            console.log("Chart data received:", type, index, value)

            if (type === "temperature") {
                console.log("ADDING TEMP POINT:", index,  value)
                tempSeries.append(index, value)
                if (value < weatherAxisY_Temp.min) weatherAxisY_Temp.min = value - 5
                if (value > weatherAxisY_Temp.max) weatherAxisY_Temp.max = value + 5
               } else if (type === "pressure") {
                pressSeries.append(index, value)
                if (value < weatherAxisY_Press.min) weatherAxisY_Press.min = value - 10
                if (value > weatherAxisY_Press.max) weatherAxisY_Press.max = value + 10
               } else if (type === "humidity") {
                humSeries.append(index, value)
                if (value < weatherAxisY_Hum.min) weatherAxisY_Hum.min = value - 5
                if (value > weatherAxisY_Hum.max) weatherAxisY_Hum.max = value + 5
            }

            // Масштабирование оси X
            if (index > 50) {
                weatherAxisX.min = index - 50
                weatherAxisX.max = index + 5
            } else {
                weatherAxisX.min = 0
                weatherAxisX.max = 60
            }
        }
    }
}