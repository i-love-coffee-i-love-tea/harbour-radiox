import QtQuick 2.6
import Sailfish.Silica 1.0

ListItem {
    id: sendetippDelegate

    property string title: ""
    property string showName: ""
    property string showSlug: ""
    property string dateTime: ""
    property string description: ""
    property string imageUrl: ""

    contentHeight: contentColumn.height + Theme.paddingMedium

    Column {
        id: contentColumn
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        spacing: Theme.paddingSmall

        Label {
            width: parent.width
            text: sendetippDelegate.title
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            wrapMode: Text.WordWrap
        }

        Row {
            width: parent.width
            spacing: Theme.paddingSmall

            Label {
                text: sendetippDelegate.showName
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }

            Label {
                text: "·"
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }

            Label {
                text: sendetippDelegate.dateTime
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        Label {
            width: parent.width
            visible: text.length > 0
            text: {
                var desc = sendetippDelegate.description
                if (desc.length > 150)
                    return desc.substring(0, 150) + "…"
                return desc
            }
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    onClicked: {
        if (sendetippDelegate.showSlug.length > 0) {
            pageStack.push(Qt.resolvedUrl("../pages/ShowDetailPage.qml"), {
                slug: sendetippDelegate.showSlug,
                showName: sendetippDelegate.showName
            })
        }
    }
}