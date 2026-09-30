import QtQuick 2.6
import Sailfish.Silica 1.0

ListItem {
    id: scheduleDelegate

    property int hour: 0
    property string showName: ""
    property string slug: ""
    property string subtitle: ""
    property bool isRepeat: false
    property bool isDito: false
    property bool isLive: false
    property bool continuesFromAbove: false
    property bool nextIsDito: false

    contentHeight: contentRow.height + Theme.paddingMedium

    // Live highlight background
    Rectangle {
        anchors.fill: parent
        visible: scheduleDelegate.isLive
        color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
    }

    // Live indicator bar on left
    Rectangle {
        width: 3
        anchors {
            top: parent.top
            bottom: parent.bottom
            topMargin: Theme.paddingExtraSmall
            bottomMargin: Theme.paddingExtraSmall
        }
        color: Theme.highlightColor
        visible: scheduleDelegate.isLive
    }

    // Connector line for multi-hour shows
    Rectangle {
        width: 3
        anchors {
            top: parent.top
            topMargin: 0
        }
        height: Theme.paddingMedium
        color: Theme.rgba(Theme.highlightColor, 0.25)
        visible: scheduleDelegate.continuesFromAbove
    }

    Row {
        id: contentRow
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        spacing: Theme.paddingMedium

        // Hour column
        Label {
            width: 50
            anchors.verticalCenter: parent.verticalCenter
            text: {
                var h = scheduleDelegate.hour
                return (h < 10 ? "0" : "") + h + ":00"
            }
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignRight
        }

        // Thin vertical separator
        Rectangle {
            width: 1
            height: contentColumn.height
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.rgba(Theme.primaryColor, 0.1)
        }

        // Show info column
        Column {
            id: contentColumn
            width: parent.width - 50 - 1 - Theme.paddingMedium * 3
            spacing: Theme.paddingExtraSmall

            Row {
                spacing: Theme.paddingSmall
                width: parent.width

                Label {
                    id: nameLabel
                    width: parent.width - badgeLoader.width - Theme.paddingSmall
                           - (scheduleDelegate.isLive ? liveLabel.width + livePlayBtn.width + Theme.paddingSmall * 2 : 0)
                    anchors.verticalCenter: parent.verticalCenter
                    text: scheduleDelegate.showName
                    color: scheduleDelegate.isLive
                           ? Theme.highlightColor
                           : (scheduleDelegate.isDito
                              ? Theme.secondaryColor
                              : (scheduleDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor))
                    font.pixelSize: Theme.fontSizeSmall
                    font.bold: scheduleDelegate.isLive
                    truncationMode: TruncationMode.Fade
                    opacity: scheduleDelegate.isDito ? 0.6 : 1.0
                }

                Loader {
                    id: badgeLoader
                    active: scheduleDelegate.isRepeat && !scheduleDelegate.isLive
                    anchors.verticalCenter: parent.verticalCenter
                    sourceComponent: Label {
                        text: qsTr("(Rpt.)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeExtraSmall
                    }
                }

                Label {
                    id: liveLabel
                    visible: scheduleDelegate.isLive
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("LIVE")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    font.bold: true
                }

                Item {
                    id: livePlayBtn
                    visible: scheduleDelegate.isLive
                    anchors.verticalCenter: parent.verticalCenter
                    width: Theme.itemSizeSmall
                    height: Theme.itemSizeSmall

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (radioXCore.livestreamPlaying)
                                radioXCore.stopPlayback()
                            else
                                radioXCore.openLivestream()
                        }
                        Image {
                            anchors.centerIn: parent
                            width: Theme.iconSizeMedium
                            height: Theme.iconSizeMedium
                            source: radioXCore.livestreamPlaying
                                    ? "image://theme/icon-m-stop"
                                    : "image://theme/icon-m-play"
                            opacity: parent.pressed ? 0.4 : 1.0
                        }
                    }
                }
            }

            Label {
                width: parent.width
                visible: text.length > 0
                text: scheduleDelegate.subtitle
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                truncationMode: TruncationMode.Fade
                opacity: scheduleDelegate.isDito ? 0.5 : 0.8
            }
        }
    }

    // Bottom separator
    Rectangle {
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            bottom: parent.bottom
        }
        height: 1
        visible: !scheduleDelegate.nextIsDito
        color: Theme.rgba(Theme.primaryColor, 0.05)
    }

    onClicked: {
        if (scheduleDelegate.slug.length > 0) {
            pageStack.push(Qt.resolvedUrl("../pages/ShowDetailPage.qml"), { slug: scheduleDelegate.slug, showName: scheduleDelegate.showName })
        }
    }
}