import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Переиспользуемая "продакшн" таблица событий (Время/Тип/Значение/Единица/
// Детали). Используется дважды в main.qml — для погоды и для биржи, каждая
// со своей моделью (controller.dataModel / controller.tradingDataModel),
// чтобы данные не смешивались между вкладками.
Rectangle {
    id: dataTableRoot
    property alias model: tableView.model
    property string emptyText: qsTr("No data yet")

    color: "#1e1e1e"
    border.color: "#3d3d3d"
    border.width: 1
    radius: 4
    clip: true

    function typeColor(t) {
        switch (t) {
        case "temperature": return "#ff6b6b"
        case "pressure": return "#64b5f6"
        case "humidity": return "#66bb6a"
        case "price": return "#ffb74d"
        default: return "#9e9e9e"
        }
    }

    ListView {
        id: tableView
        anchors.fill: parent
        anchors.margins: 1
        clip: true

        header: Rectangle {
            width: tableView.width
            height: 36
            color: "#252525"

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#3d3d3d"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 12

                Label { text: qsTr("TIME"); Layout.preferredWidth: 160; color: "#8a8a8a"; font.pixelSize: 10; font.letterSpacing: 1; font.bold: true }
                Label { text: qsTr("TYPE"); Layout.preferredWidth: 90; color: "#8a8a8a"; font.pixelSize: 10; font.letterSpacing: 1; font.bold: true }
                Label { text: qsTr("VALUE"); Layout.preferredWidth: 90; color: "#8a8a8a"; font.pixelSize: 10; font.letterSpacing: 1; font.bold: true; horizontalAlignment: Text.AlignRight }
                Label { text: qsTr("UNIT"); Layout.preferredWidth: 60; color: "#8a8a8a"; font.pixelSize: 10; font.letterSpacing: 1; font.bold: true }
                Label { text: qsTr("DETAILS"); Layout.fillWidth: true; color: "#8a8a8a"; font.pixelSize: 10; font.letterSpacing: 1; font.bold: true }
            }
        }

        delegate: Rectangle {
            width: tableView.width
            height: 34
            color: rowMouse.containsMouse ? "#2f2f2f" : (index % 2 === 0 ? "#212121" : "#252525")
            Behavior on color { ColorAnimation { duration: 100 } }

            MouseArea {
                id: rowMouse
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 12

                Label {
                    Layout.preferredWidth: 160
                    text: model.timestamp || ""
                    color: "#b0b0b0"
                    font.family: "Consolas"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.preferredWidth: 90
                    Layout.alignment: Qt.AlignVCenter
                    implicitHeight: typeBadgeText.implicitHeight + 6
                    radius: height / 2
                    color: Qt.rgba(1, 1, 1, 0.06)
                    border.color: dataTableRoot.typeColor(model.type)
                    border.width: 1
                    visible: !!model.type

                    Text {
                        id: typeBadgeText
                        anchors.centerIn: parent
                        text: model.type || ""
                        color: dataTableRoot.typeColor(model.type)
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Label {
                    Layout.preferredWidth: 90
                    text: model.value !== undefined ? model.value.toFixed(2) : "0.00"
                    color: "#e8e8e8"
                    font.family: "Consolas"
                    font.pixelSize: 12
                    font.bold: true
                    horizontalAlignment: Text.AlignRight
                }

                Label {
                    Layout.preferredWidth: 60
                    text: model.unit || ""
                    color: "#8a8a8a"
                    font.pixelSize: 11
                }

                Label {
                    Layout.fillWidth: true
                    text: model.string || ""
                    color: "#6b6b6b"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }

        Column {
            anchors.centerIn: parent
            visible: tableView.count === 0
            spacing: 6

            Text {
                text: "—"
                color: "#444444"
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: dataTableRoot.emptyText
                color: "#5a5a5a"
                font.pixelSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
