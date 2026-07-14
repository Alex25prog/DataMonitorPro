import QtQuick
import QtCharts
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root

    width: 1400
    height: 800
    visible: true
    title: qsTr("DataMonitor Pro")
    background: Rectangle { color: "#1e1e1e" }

    // Для сохранения состояния перевода
    property string currentLocale: "en"

    // Переменные для переключения графиков
    property bool showWeatherGraph: true
    property bool showTradingGraph: false

    property bool updatingAxes: false

    // Используем только обновление осей

    function updateAxes() {
       if (updatingAxes) {
          console.log("updateAxes: already running, skipping...")
          return
    }
       updatingAxes = true

            if (!candleModel || candleModel.count === 0) {
                updatingAxes = false
                console.log("No candles for axes update")
                return
            }

            var minPrice = Infinity
            var maxPrice = -Infinity
            var firstTime = null
            var lastTime = null

            for (var i = 0; i < candleModel.count; i++) {
                var c = candleModel.get(i)
                if (!c) continue

                if (c.low < minPrice) minPrice = c.low
                if (c.high > maxPrice) maxPrice = c.high
                if (i === 0) firstTime = c.openTime
                if (i === candleModel.count - 1) lastTime = c.closeTime
            }

            if (minPrice === Infinity || maxPrice === -Infinity) {
                updatingAxes = false
                return
            }

            // Надежная конвертация времени для оси Х
            // Используем Number(), чтобы гарантировать правильный тип для new Date()
            var safeFirstTime = Number(firstTime)
            var safeLastTime = Number(lastTime)


            axisX.min = new Date(safeFirstTime)
            axisX.max = new Date(safeLastTime + 3600000)

            var margin = (maxPrice - minPrice) * 0.1
            if (margin === 0) margin = 10

            axisY.min = minPrice - margin
            axisY.max = maxPrice + margin

            console.log("Axes updated! Min:", axisY.min, "Max:", axisY.max)

            updatingAxes = false
        }

    // Обработчик смены языка
    onCurrentLocaleChanged: {
        // Запоминает текущие значения перед перерисовкой
        var savedCountryIndex = countrySelect.currentIndex;
        var savedCityValue = citySelect.currentValue;

        // Принудительно обновляем список стран, чтобы сработал qsTr()
        var countryModel = []
        if (currentLocale === "ru") {
            countryModel = [
                    { text: "▼ Выберите страну", value: 0 },
                    { text: "Россия", value: 1 },
                    { text: "США", value: 2 },
                    { text: "Германия", value: 3 },
                    { text: "Франция", value: 4 },
                    { text: "Великобритания", value: 5 },
                    { text: "Япония", value: 6 }
                 ];
         } else {


           countryModel = [
                    { text: "▼ Select Country", value: 0 },
                    { text: "Russia", value: 1 },
                    { text: "USA", value: 2 },
                    { text: "Germany", value: 3 },
                    { text: "France", value: 4 },
                    { text: "UK", value: 5 },
                    { text: "Japan", value: 6 }
                ];
        }
        countrySelect.model = countryModel;
        // Возврящаем выбранную страну на место
        countrySelect.currentIndex = savedCountryIndex;

        // Принудительно обновляем отображение
        //countrySelect.currentText = countrySelect.model[savedCountryIndex].text;

        // Перерисовываем города и восстанавливаем выбранный
        updateCityList(savedCityValue);
    }

    // Функция для обновления списка городов
    function updateCityList(savedCityValue = "") {
        var cityModel = [];
        var isRussian = (currentLocale === "ru");

        // Базовый элемент "Выберите город"
        cityModel.push({
            text: isRussian ? "▼ Выберите город" : "▼ Select City",
            value: ""
        });

        switch (countrySelect.currentIndex) {
            case 1: // Russia
                cityModel.push(
                    { text: isRussian ? "Москва" : "Moscow", value: "Moscow" },
                    { text: isRussian ? "Санкт-Петербург" : "Saint Petersburg", value: "Saint Petersburg" },
                    { text: isRussian ? "Новосибирск" : "Novosibirsk", value: "Novosibirsk" },
                    { text: isRussian ? "Казань" : "Kazan", value: "Kazan" },
                    { text: isRussian ? "Екатеринбург" : "Yekaterinburg", value: "Yekaterinburg" },
                    { text: isRussian ? "Воронеж" : "Voronezh", value: "Voronezh" }
                );

                break;
            case 2: // USA
                cityModel.push(
                    //{ text: qsTr("▼ Select City", value: "" },
                    { text: isRussian ? "Нью Йорк" : "New York", value: "New York" },
                    { text: isRussian ? "Лос Анджелес" : "Los Angeles" , value: "Los Angeles" },
                    { text: isRussian ? "Чикаго" : "Chicago" , value: "Chicago" },
                    { text: isRussian ? "Хьюстон" : "Houston" , value: "Houston" },
                    { text: isRussian ? "Майами" : "Miami" , value: "Miami" }
                );
                break;
            case 3: // Germany
                cityModel.push(
                    //{ text: qsTr("▼ Select City"), value: "" },
                    { text: isRussian ? "Берлин" : "Berlin", value: "Berlin" },
                    { text: isRussian ? "Мюнхен" : "Munich", value: "Munich" },
                    { text: isRussian ? "Гамбург" : "Hamburg", value: "Hamburg" },
                    { text: isRussian ? "Кёльн" : "Cologne", value: "Cologne" },
                    { text: isRussian ? "Франкфурт" : "Frankfurt", value: "Frankfurt" }
                );
                break;
            case 4: // France
                cityModel.push(
                    //{ text: qsTr("▼ Select City"), value: "" },
                    { text: isRussian ? "Париж" : "Paris", value: "Paris" },
                    { text: isRussian ? "Марсель" : "Marseille", value: "Marseille" },
                    { text: isRussian ? "Лион" : "Lyon", value: "Lyon" },
                    { text: isRussian ? "Тулуза" : "Toulouse", value: "Toulouse" },
                    { text: isRussian ? "Ницца" : "Nice", value: "Nice" }
                );
                break;
            case 5: // UK
                cityModel.push(
                    //{ text: qsTr("▼ Select City"), value: "" },
                    { text: isRussian ? "Лондон" : "London", value: "London" },
                    { text: isRussian ? "Манчестер" : "Manchester", value: "Manchester" },
                    { text: isRussian ? "Бирмингем" : "Birmingham", value: "Birmingham" },
                    { text: isRussian ? "Ливерпуль" : "Liverpool", value: "Liverpool" },
                    { text: isRussian ? "Эдинбург" : "Edinburgh", value: "Edinburgh" }
                );
                break;
            case 6: // Japan
                cityModel.push(
                    //{ text: qsTr("▼ Select City"), value: "" },
                    { text: isRussian ? "Токио" : "Tokyo", value: "Tokyo" },
                    { text: isRussian ? "Осака" : "Osaka", value: "Osaka" },
                    { text: isRussian ? "Киото" : "Kyoto", value: "Kyoto" },
                    { text: isRussian ? "Иокогама" : "Yokohama", value: "Yokohama" },
                    { text: isRussian ? "Нагоя" : "Nagoya", value: "Nagoya" }
                );
                break;
            default:
                cityModel = [{ text: isRussian ? "▼ Выберите город" : "▼ Select City", value: "" }]
        }


        citySelect.model = cityModel
        // Если нам передали сохраненный город (режим смены языка) ищем и восстанавливаем
        if (savedCityValue !== "") {
            for (var i = 0; i < cityModel.length; i++) {
                if (cityModel[i].value === savedCityValue) {
                    citySelect.currentIndex = i;
                    return; // Успешно восстановили индекс, выходим
                }
            }
        }
        // Если это обычный клик пользователя по новой стране - сбрасываем город
        citySelect.currentIndex = 0
        controller.setCity("")
    }

    // Таймер для защиты от спама
    Timer {
        id: loadDebounceTimer
        interval: 1500
        repeat: false
        onTriggered: {
            loadBtcButton.enabled = true
            loadBtcButton.isLoading = false
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Верхняя панель с кнопками
        RowLayout {
            spacing: 5

            Column {
                Layout.alignment: Qt.AlignVCenter
                spacing: 2

                // Start/Stop Server кнопка
                Button {
                    id: serverButton
                    implicitWidth: Math.max(85, contentItem.implicitWidth + 5)
                    implicitHeight: 45

                    background: Rectangle {
                        color: controller.isServerRunning ? "#2e7d32" : "#1565c0"
                        radius: 15
                        opacity: parent.pressed ? 0.7 : 1.0
                    }

                    contentItem: Text {
                        text: controller.isServerRunning ? qsTr("Stop Server") : qsTr("Start Server")
                        color: "black"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.fill: parent
                    }

                    onClicked: {
                        if (controller.isServerRunning) {
                            controller.stopServer()
                        } else {
                            controller.startServer(8080)
                        }
                    }
                }

                Text {
                    id: serverStatusLabel
                    text: controller.isServerRunning ? qsTr("● Server Running") : qsTr("○ Server Stopped")
                    color: controller.isServerRunning ? "#4caf50" : "#f44336"
                    font.pixelSize: 9
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // Clear Data кнопка
            Button {
                id: clearDataButton
                text: qsTr("Clear Data")
                implicitWidth: contentItem.implicitWidth + 5
                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: clearDataButton.text
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                onClicked: controller.clearData()
            }

            // Load History кнопка
            Button {
                id: loadHistoryButton
                text: qsTr("Load History (Last 24h)")
                implicitWidth: contentItem.implicitWidth + 5
                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: loadHistoryButton.text
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
                id: exportCsvButton
                text: qsTr("Export CSV")
                implicitWidth: 85
                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: exportCsvButton.text
                    color: "black"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: controller.exportToCSV()
            }

            // Export PDF
            Button {
                id: exportPdfButton
                text: qsTr("Export PDF")
                implicitWidth: 85
                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: exportPdfButton.text
                    color: "black"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: controller.exportToPDF()
            }

            // Кнопки переключения графиков
            Button {
                id: weatherToggleButton
                text: qsTr("Weather")
                implicitWidth: 85
                background: Rectangle {
                    color: showWeatherGraph ? "#4caf50" : "#e8e9ef"
                    radius: 8
                }
                contentItem: Text {
                   text: weatherToggleButton.text
                   color: showWeatherGraph ? "white" : "black"
                   font.bold: true
                   horizontalAlignment: Text.Center
                }
                onClicked: {
                    showWeatherGraph = true
                    showTradingGraph = false
                }
            }

            Button {
                id: birzhaToggleButton
                text: qsTr("Birzha")
                implicitWidth: 85
                background: Rectangle {
                    color: showTradingGraph ? "#4caf50" : "#e8e9ef"
                    radius: 8
                }
                contentItem: Text {
                    text: birzhaToggleButton.text
                    color: showTradingGraph ? "white" : "black"
                    font.bold: true
                    horizontalAlignment: Text.Center
                }
                onClicked: {
                    showWeatherGraph = false
                    showTradingGraph = true
                    updateAxes()
                }
            }

            // Кнопка переключения языка
            ComboBox {
                id: langSelector
                model: [
                    { text: "EN", code: "en" },
                    { text: "RU", code: "ru" }
                ]
                textRole: "text"
                valueRole: "code"
                implicitWidth: 80
                implicitHeight: 30
                currentIndex: (currentLocale === "ru") ? 1 : 0

                onActivated: {
                    var selectedCode = currentValue
                    currentLocale = selectedCode
                    languageManager.setLanguage(selectedCode)
                }

                background: Rectangle {
                    color: "#e8e9ef"
                    radius: 8
                    border.color: "#c0c0c0"
                }
            }

            Item {
                Layout.fillWidth: true
                height: 1
            }

            // Выбор страны и города
            Rectangle {
                height: 35
                Layout.preferredWidth: 300 // Фиксированная ширина
                color: "#3d3d3d"
                radius: 5

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    ComboBox {
                        id: countrySelect
                        model: [
                            { text: qsTr("▼ Select Country"), value: 0 },
                            { text: qsTr("Russia"), value: 1 },
                            { text: qsTr("USA"), value: 2 },
                            { text: qsTr("Germany"), value: 3 },
                            { text: qsTr("France"), value: 4 },
                            { text: qsTr("UK"), value: 5 },
                            { text: qsTr("Japan"), value: 6 }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: 0
                        font.pixelSize: 12
                        Layout.fillWidth: true // Растягивается внутри Rectangle
                        Layout.minimumWidth: 120


                        onActivated: {
                            updateCityList(""); // Передаем пустоту, чтобы сбросить город при ручном выборе страны
                        }

                        background: Rectangle {
                            color: "#e8e9ef"
                            radius: 3
                            border.color: "#c0c0c0"
                            border.width: 1
                        }

                        contentItem: Text {
                            // Привязка ширины текста
                            leftPadding: 8 // отступ от края
                            rightPadding: 10
                            text: countrySelect.currentIndex === 0 ? qsTr("▼ Select Country") : countrySelect.currentText
                            color: countrySelect.currentIndex === 0 ? "#666666" : "#2c3e50"
                            font.bold: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignHCenter
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
                                        text: modelData ? modelData.text : ""
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

                    ComboBox {
                        id: citySelect
                        model: [{ text: qsTr("▼ Select City"), value: "" }]
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: 0
                        font.pixelSize: 12
                        Layout.preferredWidth: implicitWidth //130
                        font.bold: true
                        Layout.fillWidth: true // растягивается внутри Rectangle
                        Layout.minimumWidth: 120


                        onActivated: {
                            if (index > 0) {
                                // Берет точное значение напрямую из массива модели
                                controller.setCity(citySelect.model[index].value)
                            } else {
                                controller.setCity("")
                            }
                        }

                        background: Rectangle {
                            color: citySelect.enabled ? "#e8e9ef" : "#cccccc"
                            radius: 3
                            border.color: "#c0c0c0"
                            border.width: 1
                        }

                        contentItem: Text {
                            leftPadding: 8
                            rightPadding: 10
                            text: citySelect.currentIndex === 0 ? qsTr("▼ Select City") : citySelect.currentText
                            color: citySelect.currentIndex === 0 ? "#666666" : "#2c3e50"
                            font.pixelSize: 12
                            font.bold: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignHCenter
                            // Автоматическая ширина текста
                            //implicitWidth: paintedWidth
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
                                        text: modelData ? modelData.text : ""
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
                text: controller.isWeatherRunning ? qsTr("Stop Weather") : qsTr("Start Weather")
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

                    Behavior on color { ColorAnimation { duration: 150 } }
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
                    if (!controller.isCitySelected) return
                    if (controller.isWeatherRunning) {
                        controller.stopWeather()
                    } else {
                        controller.startWeather()
                    }
                }
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
                    text: qsTr("Filter by type:")
                    color: "#2c3e50"
                    font.bold: true
                    font.pixelSize: 12
                }

                ComboBox {
                    id: typeFilter
                    model: [qsTr("All"), qsTr("temperature"), qsTr("pressure"), qsTr("humidity")]
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
                        color: typeFilter.currentIndex === 0 ? "#666666" : "#2c3e50"
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

                    onCurrentIndexChanged: {
                        if (currentIndex === 0) {
                            tempSeries.visible = true
                            pressSeries.visible = true
                            humSeries.visible = true
                            controller.dataModel.resetFilters()
                        } else if (currentIndex === 1) {
                            tempSeries.visible = true
                            pressSeries.visible = false
                            humSeries.visible = false
                            controller.dataModel.setTypeFilter("temperature")
                        } else if (currentIndex === 2) {
                            tempSeries.visible = false
                            pressSeries.visible = true
                            humSeries.visible = false
                            controller.dataModel.setTypeFilter("pressure")
                        } else if (currentIndex === 3) {
                            tempSeries.visible = false
                            pressSeries.visible = false
                            humSeries.visible = true
                            controller.dataModel.setTypeFilter("humidity")
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
                    titleText: qsTr("Point number")
                    min: 0
                    max: 60
                    gridVisible: true
                    gridLineColor: "#404040"
                }
                // @disable-check M300
                ValueAxis {
                    id: weatherAxisY_Temp
                    titleText: qsTr("Temperature (°C)")
                    color: "#ff5050"
                    gridVisible: true
                    gridLineColor: "#404040"
                    min: -30
                    max: 40
                }
                // @disable-check M300
                ValueAxis {
                    id: weatherAxisY_Press
                    titleText: qsTr("Pressure (hPa)")
                    color: "#5090ff"
                    gridVisible: false
                    min: 950
                    max: 1050
                }
                // @disable-check M300
                ValueAxis {
                    id: weatherAxisY_Hum
                    titleText: qsTr("Humidity (%)")
                    color: "#50ff50"
                    gridVisible: false
                    min: 0
                    max: 100
                }

                LineSeries {
                    id: tempSeries
                    name: qsTr("Temperature")
                    color: "#ff5050"
                    width: 2
                    //pointsVisible: true // Точка на графике
                    axisX: weatherAxisX
                    axisY: weatherAxisY_Temp
                }

                LineSeries {
                    id: pressSeries
                    name: qsTr("Pressure")
                    color: "#5090ff"
                    width: 2
                    //pointsVisible: true // Точка на графике
                    axisX: weatherAxisX
                    axisYRight: weatherAxisY_Press
                }

                LineSeries {
                    id: humSeries
                    name: qsTr("Humidity")
                    color: "#50ff50"
                    width: 2
                    //pointsVisible: true // Точка на графике
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
                id: tradingChart
                SplitView.preferredHeight: 300
                Layout.fillWidth: true
                theme: ChartView.ChartThemeDark
                antialiasing: true
                animationOptions: ChartView.NoAnimations // NoAnimation, чтобы избежать assert(error)
                backgroundColor: "#1e1e1e"
                visible: showTradingGraph

                DateTimeAxis {
                    id: axisX
                    format: "hh:mm:ss"
                    titleText: qsTr("Time")
                    gridVisible: true
                    gridLineColor: "#404040"
                    labelsFont.pixelSize: 10
                    titleFont.pixelSize: 12
                }

                // @disable-check M300
                ValueAxis {
                    id: axisY
                    titleText: qsTr("Price")
                    gridVisible: true
                    gridLineColor: "#404040"
                    labelsFont.pixelSize: 10
                    titleFont.pixelSize: 12
                    min: 0
                    max: 100000
                }

                // Серия для свечей - пустая, заполняется из С++ через модель
                CandlestickSeries {
                    id: candlestickSeries
                    name: qsTr("Price")
                    axisX: axisX // Привязка осей
                    axisY: axisY
                    increasingColor: "#26a69a"
                    decreasingColor: "#ef5350"
                    // Защита от схлопывания свечей
                    bodyWidth: 0.7
                    maximumColumnWidth: 30
                    minimumColumnWidth: 5


                    // Отдаем управление серией в С++
                    Component.onCompleted: {
                        if (chartManager) {
                            chartManager.attachSeries(candlestickSeries)
                            console.log("CandlestickSeries attached to chartManager")
                    } else {
                       console.log("ERROR: chartManager is null!")

                   }
                }
             }
                // Кнопки управления с защитой от спама
                Row {
                    spacing: 10 // Отступ между кнопками в пикселях
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 10
                    z: 10

                Button {
                    id: loadBtcButton
                    property bool isLoading: false

                    text: isLoading ? qsTr("Loading...") : qsTr("Load BTCUSD")
                    enabled: !isLoading

                    background: Rectangle {
                        color: loadBtcButton.enabled ? "#1565c0" : "#666666"
                        radius: 5
                        opacity: parent.pressed && loadBtcButton.enabled ? 0.7 : 1.0
                    }

                    contentItem: Text {
                        text: loadBtcButton.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (isLoading) return

                        isLoading = true
                        enabled = false

                        controller.loadCandles("BTCUSDT", 4, 100) // 4 = H1
                        loadDebounceTimer.start()
                }
            }

                Button {
                    id: startRealtimeButton
                    text: qsTr("Start Realtime")
                    enabled: !controller.exchangeClient || !controller.exchangeClient.isRealtimeConnected // Блокировка кнопки от повторного нажатия

                    background: Rectangle {
                        color: "#2e7d32"
                        radius: 5
                        opacity: parent.pressed ? 0.7 : 1.0
                    }

                    contentItem: Text {
                        text: startRealtimeButton.text
                        color: "white"
                        font.bold: true
                    }

                    onClicked:
                        controller.startRealtimeCandles("BTCUSDT", 4)
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
                    anchors.rightMargin: 220
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
                            onClicked: tradingChart.zoomIn()
                            background: Rectangle { color: "#42f5e3"; radius: btnIn.height / 2 }
                        }
                        Button {
                            id: btnIn2
                            text: "-"
                            font.pixelSize: 16
                            font.bold: true
                            onClicked: tradingChart.zoomOut()
                            background: Rectangle { color: "#42f5e3"; radius: btnIn2.height / 2 }
                        }
                        Button {
                            id: btnIn3
                            text: "↺"
                            font.pixelSize: 16
                            font.bold: true
                            onClicked: tradingChart.zoomReset()
                            background: Rectangle { color: "#42f5e3"; radius: btnIn3.height / 2 }
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

                            Rectangle { width: 180; height: 30; color: "#3d3d3d"; radius: 3
                                Text { text: qsTr("Timestamp"); anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 100; height: 30; color: "#3d3d3d"; radius: 3
                                Text { text: qsTr("Type"); anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 100; height: 30; color: "#3d3d3d"; radius: 3
                                Text { text: qsTr("Value"); anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 80; height: 30; color: "#3d3d3d"; radius: 3
                                Text { text: qsTr("Unit"); anchors.centerIn: parent; color: "white" } }
                            Rectangle { width: 700; height: 30; color: "#3d3d3d"; radius: 3
                                Text { text: qsTr("Details"); anchors.centerIn: parent; color: "white" } }
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
                            Text { width: 100; text: model.type ? qsTr(model.type) : ""; color: "white"; horizontalAlignment: Text.AlignHCenter }
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

                Label { text: qsTr("Total points:") + " " + controller.dataModel.count; color: "#B22222" }
                Label { text: "|" }

                Label {
                    id: statusServer
                    text: qsTr("Server:") + " " + (controller.isServerRunning ? qsTr("Active") : qsTr("Inactive"))
                    color: "#B22222"
                }
                Label { text: "|" }
                Label { text: qsTr("Database: PostgreSQL"); color: "#B22222" }
                Label { text: "|" }
                Label {
                    id: statusWeather
                    text: qsTr("Weather:") + " " + (controller.isWeatherRunning ? qsTr("Active") : qsTr("Inactive"))
                    color: controller.isWeatherRunning ? "#4caf50" : "#f44336"
                }
            }
        }
    }

    Connections {
        target: controller

        function onClearGraphRequested() {
            // Очищаем все линии
            tempSeries.clear()
            pressSeries.clear()
            humSeries.clear()
            //candlestickSeries.clear() // Не вызываем это делает С++
            movingAverageSeries.clear()

            // Сбрасываем оси погоды
            weatherAxisX.min = 0
            weatherAxisX.max = 60

            // Сбрасываем оси биржи с помощью объектов Date
            var now = new Date()
            var future = new Date(now.getTime() + 60 * 60 * 1000) // + 1час
            axisX.min = now
            axisX.max = future

            axisY.min = 0
            axisY.max = 100
            console.log("Graph cleared and axes reset successfully")
        }


        function onChartDataReceived(index, value, type) {
            console.log("Chart data", type, index, value)

            // Добавляем все три точки с одним индексом
            //tempSeries.append(index, temperature)
            //pressSeries.append(index, pressure)
            //humSeries.append(index, humidity)

            var series;
            var axisY;
            var delta = 5;

            // Динамическое масштабирование для температуры
            if (type === "temperature") {
                series = tempSeries;
                axisY = weatherAxisY_Temp;
                delta = 5;
            // Динамическое масштабирование для давления
            } else if (type === "pressure") {
                series = pressSeries;
                axisY = weatherAxisY_Press;
                delta = 10;
            // Динамическое масштабирование для влажности
            } else if (type === "humidity") {
                series = humSeries;
                axisY = weatherAxisY_Hum;
                delta = 5;
            } else {
                return;
            }

            // Обновляем ось Y
            if (value < axisY.min) axisY.min = value - delta;
            if (value > axisY.max) axisY.max = value + delta;

            // Добавляем точку
            series.append(index + 1, value)

            console.log("Series count", series.count)
            console.log(
                weatherChart.plotArea.width,
                weatherChart.plotArea.height
            )

                  //Принудительно Обновляем ось X
                   var currentMaxX = Math.max(tempSeries.count, pressSeries.count, humSeries.count);
                  // Устанавливаем видимы диапазон для первой точки\
                   if (currentMaxX <= 1) {
                       weatherAxisX.min = 0;
                       weatherAxisX.max = 60;

                   } else if (currentMaxX > 50) {
                       weatherAxisX.min = currentMaxX - 50;
                       weatherAxisX.max = currentMaxX + 5;
                   } else {
                       weatherAxisX.min = 0;
                       weatherAxisX.max = 60;

            }
                   // Принудительно обновляем график (для QtCharts)
                   weatherChart.update();
        }

        // Только обновление осей, свечи уже обновлены в С++

        function onCandlesUpdated() {
            console.log("=== Signal: candlesUpdated ===")
            // Вызываем метод С++ менеджера
            //chartManager.updateSeries(candleModel)

            // Масштабируем оси, основываясь на данных модели
            updateAxes()
        }
    }

    // Следим за изменением модели, только оси.

   // Connections {
            //target: candleModel

            //function onCountChanged() {
                //console.log("=== CandleModel count changed to:", candleModel.count, "===")
                //if (showTradingGraph) {
                    // Свечи уже обновлены через TradingChartManager в C++
                    // Обновляем только оси
                    //updateAxes()
                //}
            //}
        //}

        // СИГНАЛ ОТ TradingChartManager — ОБНОВЛЕНИЕ ОСЕЙ + ПЕРЕРИСОВКА
        Connections {
            target: chartManager

            function onSeriesUpdated() {
                console.log("=== ChartManager: seriesUpdated ===")
                updateAxes()

                // Заставляем QML перерисовать геометрию серии
                // Это безопасно и не требует доступа к С++ объектам

                candlestickSeries.visible = false
                candlestickSeries.visible = true

                console.log("CandlestickSeries redraw triggered")
            }
        }

        // ОТЛАДКА — ПРОВЕРКА КОНТЕКСТНЫХ СВОЙСТВ
        Component.onCompleted: {
            console.log("=== Component completed ===")
            console.log("controller:", controller)
            console.log("candleModel:", candleModel)
            console.log("chartManager:", chartManager)
            console.log("candlestickSeries:", candlestickSeries)
        }
    }

