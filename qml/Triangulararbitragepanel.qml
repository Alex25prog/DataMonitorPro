import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Компактная панель треугольного арбитража — обе биржи сразу, по одной
// строке на каждую, вместо двух полноразмерных блоков. Пока жёстко на
// 2 биржи (Binance/Bybit) — когда появится реальная третья, тогда и имеет
// смысл переходить на список/менеджер, не раньше (см. обсуждение).
Rectangle {
    id: root

    SplitView.preferredHeight: 76
    SplitView.minimumHeight: 64
    Layout.fillWidth: true
    color: "#1a1a1a"
    border.color: "#2d2d2d"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 4

        Label {
            text: qsTr("Triangular Arbitrage") + " (BTC/ETH/USDT)"
            color: "#e0e0e0"
            font.bold: true
            font.pixelSize: 13
        }

        // Строка Binance
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Binance"
                color: "#FFFF00"
                font.pixelSize: 11
                Layout.preferredWidth: 55
            }

            Button {
                id: binanceArbToggle
                text: controller.triangularArbitrage.isRunning ? qsTr("Stop") : qsTr("Start")
                implicitHeight: 22
                implicitWidth: 58
                verticalPadding: 0
                clip: true


                background: Rectangle {
                    color: controller.triangularArbitrage.isRunning ? "#c62828" : "#2e7d32"
                    radius: 4
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: binanceArbToggle.text
                    color: "white"
                    font.pixelSize: 10
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (controller.triangularArbitrage.isRunning) {
                        controller.triangularArbitrage.stop()
                    } else {
                        controller.triangularArbitrage.start()
                    }
                }
            }

            Label {
                text: qsTr("Forward") + ": " + controller.triangularArbitrage.forwardProfitPercent.toFixed(3) + "%"
                font.pixelSize: 11
                font.bold: true
                color: controller.triangularArbitrage.forwardProfitPercent > controller.triangularArbitrage.feePercent
                       ? "#4caf50" : "#8a8a8a"
            }

            Label {
                text: qsTr("Reverse") + ": " + controller.triangularArbitrage.reverseProfitPercent.toFixed(3) + "%"
                font.pixelSize: 11
                font.bold: true
                color: controller.triangularArbitrage.reverseProfitPercent > controller.triangularArbitrage.feePercent
                       ? "#4caf50" : "#8a8a8a"
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("Updated") + ": " + (controller.triangularArbitrage.lastUpdateTime || "--")
                font.pixelSize: 10
                color: "#FFFF00"
            }
        }

        // Строка Bybit
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Bybit"
                color: "#FFFF00"
                font.pixelSize: 11
                Layout.preferredWidth: 55
            }

            Button {
                id: bybitArbToggle
                text: controller.bybitTriangularArbitrage.isRunning ? qsTr("Stop") : qsTr("Start")
                implicitHeight: 22
                implicitWidth: 58
                verticalPadding: 0
                clip: true

                background: Rectangle {
                    color: controller.bybitTriangularArbitrage.isRunning ? "#c62828" : "#2e7d32"
                    radius: 4
                    opacity: parent.pressed ? 0.7 : 1.0
                }
                contentItem: Text {
                    text: bybitArbToggle.text
                    color: "white"
                    font.pixelSize: 10
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (controller.bybitTriangularArbitrage.isRunning) {
                        controller.bybitTriangularArbitrage.stop()
                    } else {
                        controller.bybitTriangularArbitrage.start()
                    }
                }
            }

            Label {
                text: qsTr("Forward") + ": " + controller.bybitTriangularArbitrage.forwardProfitPercent.toFixed(3) + "%"
                font.pixelSize: 11
                font.bold: true
                color: controller.bybitTriangularArbitrage.forwardProfitPercent > controller.bybitTriangularArbitrage.feePercent
                       ? "#4caf50" : "#8a8a8a"
            }

            Label {
                text: qsTr("Reverse") + ": " + controller.bybitTriangularArbitrage.reverseProfitPercent.toFixed(3) + "%"
                font.pixelSize: 11
                font.bold: true
                color: controller.bybitTriangularArbitrage.reverseProfitPercent > controller.bybitTriangularArbitrage.feePercent
                       ? "#4caf50" : "#8a8a8a"
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("Updated") + ": " + (controller.bybitTriangularArbitrage.lastUpdateTime || "--")
                font.pixelSize: 10
                color: "#FFFF00"
            }
        }
    }
}
