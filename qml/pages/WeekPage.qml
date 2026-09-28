import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: weekPage

    readonly property var dayLabels: [qsTr("Mo"), qsTr("Di"), qsTr("Mi"), qsTr("Do"), qsTr("Fr"), qsTr("Sa"), qsTr("So")]

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: headerColumn.height + weekGrid.height
        contentWidth: Math.max(parent.width, weekGrid.implicitWidth)

        PullDownMenu {
            MenuItem {
                text: qsTr("Next Week")
                onClicked: {
                    radioXCore.programModel.nextWeek()
                    radioXCore.refresh()
                }
            }
            MenuItem {
                text: qsTr("Current Week")
                onClicked: {
                    radioXCore.programModel.resetWeek()
                    radioXCore.refresh()
                }
            }
            MenuItem {
                text: qsTr("Previous Week")
                onClicked: {
                    radioXCore.programModel.prevWeek()
                    radioXCore.refresh()
                }
            }
        }

        Column {
            id: headerColumn
            width: parent.width

            PageHeader {
                title: qsTr("Weekly Schedule")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: radioXCore.programModel.weekLabel
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Item {
            id: weekGrid
            anchors.top: headerColumn.bottom
            anchors.topMargin: Theme.paddingMedium
            width: Math.max(parent.width, gridRow.implicitWidth + Theme.horizontalPageMargin * 2)
            height: gridRow.implicitHeight + Theme.paddingSmall

            // Column headers
            Row {
                id: columnHeaders
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                spacing: 1

                // Spacer for hour column
                Item { width: 50; height: Theme.itemSizeSmall }

                Repeater {
                    model: weekPage.dayLabels

                    delegate: Item {
                        width: 140
                        height: Theme.itemSizeSmall

                        Label {
                            anchors.centerIn: parent
                            text: modelData
                            color: Theme.highlightColor
                            font.pixelSize: Theme.fontSizeSmall
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }

            Column {
                id: gridRow
                anchors.top: columnHeaders.bottom
                anchors.left: columnHeaders.left
                spacing: 1

                Repeater {
                    model: radioXCore.programModel

                    delegate: Row {
                        id: hourRow
                        spacing: 1

                        property int rowHour: model.hour
                        property var day0Data: model.day0
                        property var day1Data: model.day1
                        property var day2Data: model.day2
                        property var day3Data: model.day3
                        property var day4Data: model.day4
                        property var day5Data: model.day5
                        property var day6Data: model.day6

                        function dayData(dayIdx) {
                            switch (dayIdx) {
                            case 0: return day0Data
                            case 1: return day1Data
                            case 2: return day2Data
                            case 3: return day3Data
                            case 4: return day4Data
                            case 5: return day5Data
                            case 6: return day6Data
                            }
                            return null
                        }

                        Label {
                            width: 50
                            height: Theme.itemSizeExtraSmall
                            verticalAlignment: Text.AlignVCenter
                            text: {
                                var h = hourRow.rowHour
                                return h < 10 ? "0" + h : "" + h
                            }
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                            horizontalAlignment: Text.AlignRight
                        }

                        Repeater {
                            model: 7

                            delegate: Item {
                                width: 140
                                height: Theme.itemSizeExtraSmall

                                property var cellData: hourRow.dayData(index)

                                Rectangle {
                                    anchors.fill: parent
                                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
                                }

                                Label {
                                    anchors.fill: parent
                                    anchors.margins: Theme.paddingSmall
                                    verticalAlignment: Text.AlignVCenter
                                    text: cellData ? cellData.showName : ""
                                    color: {
                                        if (cellData && cellData.isDito) return Theme.secondaryColor
                                        return Theme.primaryColor
                                    }
                                    font.pixelSize: Theme.fontSizeExtraSmall
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                    maximumLineCount: 2
                                }
                            }
                        }
                    }
                }
            }
        }

        HorizontalScrollDecorator {}
        VerticalScrollDecorator {}
    }
}