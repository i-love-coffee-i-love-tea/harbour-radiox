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

    contentHeight: contentRow.height + Theme.paddingSmall

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

        // Show info column
        Column {
            width: parent.width - 50 - Theme.paddingMedium
            spacing: Theme.paddingSmall

            Row {
                spacing: Theme.paddingSmall
                width: parent.width

                Label {
                    id: nameLabel
                    width: parent.width - badgeLoader.width - Theme.paddingSmall
                    anchors.verticalCenter: parent.verticalCenter
                    text: scheduleDelegate.showName
                    color: scheduleDelegate.isDito
                           ? Theme.secondaryColor
                           : (scheduleDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor)
                    font.pixelSize: Theme.fontSizeSmall
                    truncationMode: TruncationMode.Fade
                    opacity: scheduleDelegate.isDito ? 0.6 : 1.0
                }

                Loader {
                    id: badgeLoader
                    active: scheduleDelegate.isRepeat
                    anchors.verticalCenter: parent.verticalCenter
                    sourceComponent: Label {
                        text: "(Wdh.)"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeExtraSmall
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

    onClicked: {
        if (scheduleDelegate.slug.length > 0) {
            pageStack.push(Qt.resolvedUrl("../pages/ShowDetailPage.qml"), { slug: scheduleDelegate.slug, showName: scheduleDelegate.showName })
        }
    }
}