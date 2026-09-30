import QtQuick 2.6
import Sailfish.Silica 1.0

ListItem {
    id: recordingDelegate

    property int recordingId: 0
    property string showName: ""
    property string subtitle: ""
    property string dayHeader: ""
    property var time: ""
    property string playbackUrl: ""

    contentHeight: (dayHeaderLabel.visible ? dayHeaderLabel.height + Theme.paddingSmall : 0) + contentRow.height + Theme.paddingSmall

    // Day header shown for the first recording of each day
    Label {
        id: dayHeaderLabel
        visible: recordingDelegate.dayHeader.length > 0
        anchors {
            top: parent.top
            topMargin: Theme.paddingSmall
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
        text: recordingDelegate.dayHeader
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeMedium
        font.bold: true
    }

    Row {
        id: contentRow
        anchors {
            top: dayHeaderLabel.visible ? dayHeaderLabel.bottom : parent.top
            topMargin: dayHeaderLabel.visible ? Theme.paddingSmall : 0
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
        spacing: Theme.paddingMedium

        // Show info
        Column {
            width: parent.width - playIcon.width - Theme.paddingMedium
            spacing: Theme.paddingSmall

            Label {
                width: parent.width
                text: recordingDelegate.showName
                color: recordingDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }

            Row {
                spacing: Theme.paddingSmall
                width: parent.width

                Label {
                    id: timeLabel
                    visible: recordingDelegate.time !== undefined && recordingDelegate.time !== ""
                    text: {
                        var t = recordingDelegate.time
                        if (t && t.toString) return t.toString()
                        return "" + t
                    }
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }

                Label {
                    width: parent.width - (timeLabel.visible ? timeLabel.width : 0) - Theme.paddingSmall
                    visible: text.length > 0
                    text: recordingDelegate.subtitle
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    truncationMode: TruncationMode.Fade
                }
            }
        }

        // Play icon
        IconButton {
            id: playIcon
            anchors.verticalCenter: parent.verticalCenter
            icon.source: "image://theme/icon-m-play"
            onClicked: {
                radioXCore.playRecording(recordingDelegate.recordingId, recordingDelegate.showName)
            }
        }
    }

    onClicked: {
        if (recordingDelegate.playbackUrl.length > 0) {
            Qt.openUrlExternally("https://www.radiox.de" + recordingDelegate.playbackUrl)
        }
    }
}