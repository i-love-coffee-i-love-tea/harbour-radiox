import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

Page {
    id: mainPage

    property int selectedDay: {
        var d = new Date().getDay()
        return d === 0 ? 6 : d - 1 // JS: 0=Sun → index 6, 1=Mon → 0
    }

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: radioXCore.programModel

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: qsTr("Sendetipps")
                onClicked: pageStack.push(Qt.resolvedUrl("SendetippsPage.qml"))
            }
            MenuItem {
                text: qsTr("Recordings")
                onClicked: pageStack.push(Qt.resolvedUrl("RecordingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Week Overview")
                onClicked: pageStack.push(Qt.resolvedUrl("WeekPage.qml"))
            }
            MenuItem {
                text: qsTr("Livestream")
                onClicked: radioXCore.openLivestream()
            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: radioXCore.refresh()
            }
        }

        header: Column {
            width: parent.width

            PageHeader {
                title: qsTr("radio x")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: radioXCore.programModel.weekLabel
                visible: text.length > 0
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
            }

            // Error banner
            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: radioXCore.errorMessage
                visible: text.length > 0
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            // Debug info
            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: radioXCore.lastInfo + " | rows: " + radioXCore.programModel.count
                visible: radioXCore.lastInfo.length > 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                horizontalAlignment: Text.AlignHCenter
            }

            // Empty state
            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Pull down to refresh")
                visible: listView.count === 0 && radioXCore.errorMessage.length === 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Item {
                width: parent.width
                height: dayRow.height + Theme.paddingSmall
                visible: listView.count > 0

                Row {
                    id: dayRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingSmall

                    Repeater {
                        model: [qsTr("Mo"), qsTr("Di"), qsTr("Mi"), qsTr("Do"), qsTr("Fr"), qsTr("Sa"), qsTr("So")]

                        delegate: Item {
                            width: Math.max(dayLabel.implicitWidth + Theme.paddingMedium * 2, 60)
                            height: dayLabel.implicitHeight + Theme.paddingSmall * 2

                            Rectangle {
                                anchors.fill: parent
                                radius: 4
                                color: mainPage.selectedDay === index
                                       ? Theme.highlightColor
                                       : "transparent"
                                opacity: mainPage.selectedDay === index ? 0.3 : 0
                            }

                            Label {
                                id: dayLabel
                                anchors.centerIn: parent
                                text: modelData
                                color: mainPage.selectedDay === index
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
                                font.pixelSize: Theme.fontSizeSmall
                                font.bold: mainPage.selectedDay === index
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: mainPage.selectedDay = index
                            }
                        }
                    }
                }
            }
        }

        delegate: ScheduleDelegate {
            showName: {
                var dayData = model['day' + mainPage.selectedDay]
                return dayData ? dayData.showName : ""
            }
            slug: {
                var dayData = model['day' + mainPage.selectedDay]
                return dayData ? dayData.slug : ""
            }
            subtitle: {
                var dayData = model['day' + mainPage.selectedDay]
                return dayData ? dayData.subtitle : ""
            }
            isDito: {
                var dayData = model['day' + mainPage.selectedDay]
                return dayData ? dayData.isDito : false
            }
            isRepeat: {
                var dayData = model['day' + mainPage.selectedDay]
                return dayData ? dayData.isRepeat : false
            }
            hour: model.hour
        }

        VerticalScrollDecorator {}
    }
}