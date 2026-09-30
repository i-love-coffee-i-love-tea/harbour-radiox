import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtMultimedia 5.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: mainPage

    property int todayIndex: {
        var d = new Date().getDay()
        return d === 0 ? 6 : d - 1
    }
    property int selectedDay: todayIndex
    property int currentHour: new Date().getHours()
    property bool isRecording: radioXCore.playbackTitle.length > 0

    Timer {
        interval: 60000
        running: true
        repeat: true
        onTriggered: mainPage.currentHour = new Date().getHours()
    }

    function formatTime(ms) {
        var totalSec = Math.floor(ms / 1000)
        var h = Math.floor(totalSec / 3600)
        var m = Math.floor((totalSec % 3600) / 60)
        var s = totalSec % 60
        var pad = function(n) { return n < 10 ? "0" + n : "" + n }
        return h > 0 ? h + ":" + pad(m) + ":" + pad(s) : m + ":" + pad(s)
    }

    SlideshowView {
        id: slideshow
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: dayBar.top
        }
        model: 7
        currentIndex: mainPage.selectedDay
        onCurrentIndexChanged: mainPage.selectedDay = currentIndex
        itemWidth: width
        itemHeight: height

        delegate: SilicaListView {
            id: dayList
            width: slideshow.itemWidth
            height: slideshow.itemHeight
            model: radioXCore.programModel

            property int dayIndex: index

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

                // Playback indicator
                Rectangle {
                    width: parent.width
                    height: playbackColumn.height + Theme.paddingSmall * 2
                    visible: radioXCore.playbackUrl.length > 0
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.15)

                    Column {
                        id: playbackColumn
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: Theme.paddingSmall

                        RowLayout {
                            width: parent.width
                            spacing: Theme.paddingMedium

                            Label {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                text: {
                                    var state = audioPlayer.playbackState === Audio.PlayingState
                                                ? qsTr("Playing") : qsTr("Paused")
                                    if (mainPage.isRecording) {
                                        var title = radioXCore.playbackTitle
                                        return (title.length > 0 ? title : qsTr("Recording"))
                                               + " \u2014 " + state
                                    }
                                    return qsTr("radio x Live") + " \u2014 " + state
                                }
                                color: Theme.highlightColor
                                font.pixelSize: Theme.fontSizeSmall
                                truncationMode: TruncationMode.Fade
                            }

                            Item {
                                Layout.preferredWidth: Theme.itemSizeSmall
                                Layout.preferredHeight: Theme.itemSizeSmall
                                Layout.alignment: Qt.AlignVCenter
                                visible: mainPage.isRecording

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (audioPlayer.playbackState === Audio.PlayingState)
                                            audioPlayer.pause()
                                        else
                                            audioPlayer.play()
                                    }
                                    Image {
                                        anchors.centerIn: parent
                                        width: Theme.iconSizeMedium
                                        height: Theme.iconSizeMedium
                                        source: audioPlayer.playbackState === Audio.PlayingState
                                                ? "image://theme/icon-m-pause"
                                                : "image://theme/icon-m-play"
                                        opacity: parent.pressed ? 0.4 : 1.0
                                    }
                                }
                            }

                            Item {
                                Layout.preferredWidth: Theme.itemSizeSmall
                                Layout.preferredHeight: Theme.itemSizeSmall
                                Layout.alignment: Qt.AlignVCenter

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: radioXCore.stopPlayback()
                                    Image {
                                        anchors.centerIn: parent
                                        width: Theme.iconSizeMedium
                                        height: Theme.iconSizeMedium
                                        source: "image://theme/icon-m-stop"
                                        opacity: parent.pressed ? 0.4 : 1.0
                                    }
                                }
                            }
                        }

                        // Position bar for recordings
                        Row {
                            width: parent.width
                            visible: mainPage.isRecording
                            spacing: Theme.paddingSmall

                            Label {
                                id: positionLabel
                                anchors.verticalCenter: parent.verticalCenter
                                text: formatTime(audioPlayer.position)
                                color: Theme.secondaryColor
                                font.pixelSize: Theme.fontSizeExtraSmall
                            }

                            Slider {
                                id: positionSlider
                                width: parent.width - positionLabel.width - durationLabel.width - Theme.paddingSmall * 2
                                anchors.verticalCenter: parent.verticalCenter
                                minimumValue: 0
                                maximumValue: audioPlayer.duration
                                value: audioPlayer.position
                                stepSize: 1000
                                onReleased: audioPlayer.seek(value)

                                Binding on value {
                                    when: !positionSlider.pressed
                                    value: audioPlayer.position
                                }
                            }

                            Label {
                                id: durationLabel
                                anchors.verticalCenter: parent.verticalCenter
                                text: formatTime(audioPlayer.duration)
                                color: Theme.secondaryColor
                                font.pixelSize: Theme.fontSizeExtraSmall
                            }
                        }
                    }
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: {
                        var raw = radioXCore.programModel.weekLabel
                        if (raw.length === 0) return ""
                        var m = raw.match(/KW\s*(\d+)\s*\/\s*(\d+)/)
                        if (m) return qsTr("Program schedule (Week %1, %2)").arg(m[1]).arg(m[2])
                        return raw
                    }
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

                Item {
                    width: parent.width
                    height: Theme.paddingLarge
                }

                // Empty state
                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Pull down to refresh")
                    visible: dayList.count === 0 && radioXCore.errorMessage.length === 0
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }

            delegate: ScheduleDelegate {
                property var dayData: model['day' + dayList.dayIndex]
                showName: dayData ? dayData.showName : ""
                slug: dayData ? dayData.slug : ""
                subtitle: dayData ? dayData.subtitle : ""
                isDito: dayData ? dayData.isDito : false
                isRepeat: dayData ? dayData.isRepeat : false
                hour: model.hour
                isLive: dayList.dayIndex === mainPage.todayIndex
                         && model.hour === mainPage.currentHour
                         && !dayData.isDito
                continuesFromAbove: {
                    if (index <= 0 || !dayData || dayData.isDito) return false
                    var prevItem = dayList.itemAtIndex(index - 1)
                    if (!prevItem || !prevItem.dayData) return false
                    return prevItem.dayData.showName === dayData.showName
                           && !prevItem.dayData.isDito
                }
            }

            VerticalScrollDecorator {}
        }
    }

    Rectangle {
        id: dayBar
        width: parent.width
        height: dayRow.height + Theme.paddingMedium * 2
        anchors.bottom: parent.bottom
        color: Theme.rgba(Theme.highlightBackgroundColor, 0.06)

        Row {
            id: dayRow
            anchors.centerIn: parent
            spacing: Theme.paddingSmall

            Repeater {
                model: radioXCore.programModel.dayLabels.length >= 7
                       ? radioXCore.programModel.dayLabels
                       : [qsTr("Mo"), qsTr("Di"), qsTr("Mi"), qsTr("Do"), qsTr("Fr"), qsTr("Sa"), qsTr("So")]

                delegate: Item {
                    property string rawLabel: modelData
                    property string dayName: rawLabel.replace(/\s*\d{2}\.\d{2}\.?\s*/, "")
                    property string dateStr: {
                        var m = rawLabel.match(/(\d{2}\.\d{2})\.?/)
                        return m ? m[1] : ""
                    }

                    width: Math.max(dayCol.implicitWidth + Theme.paddingMedium * 2, 48)
                    height: dayCol.implicitHeight + Theme.paddingMedium * 2

                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.paddingSmall
                        color: mainPage.selectedDay === index
                               ? Theme.rgba(Theme.highlightColor, 0.2)
                               : (dayMouse.pressed
                                  ? Theme.rgba(Theme.highlightBackgroundColor, 0.15)
                                  : "transparent")
                        border.width: mainPage.selectedDay === index ? 1 : 0
                        border.color: Theme.rgba(Theme.highlightColor, 0.4)
                    }

                    Column {
                        id: dayCol
                        anchors.centerIn: parent
                        spacing: 2

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dayName
                            color: mainPage.selectedDay === index
                                   ? Theme.highlightColor
                                   : Theme.primaryColor
                            font.pixelSize: Theme.fontSizeSmall
                            font.bold: mainPage.selectedDay === index
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: dateStr.length > 0
                            text: dateStr
                            color: mainPage.selectedDay === index
                                   ? Theme.highlightColor
                                   : Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeTiny
                        }
                    }

                    MouseArea {
                        id: dayMouse
                        anchors.fill: parent
                        onClicked: mainPage.selectedDay = index
                    }
                }
            }
        }
    }
}