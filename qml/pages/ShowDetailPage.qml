import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: showDetailPage

    property string slug: ""
    property string showName: ""

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: showDetailPage.showName.length > 0
                       ? showDetailPage.showName
                       : qsTr("Show Details")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Slug: ") + showDetailPage.slug
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            BackgroundItem {
                width: parent.width
                Label {
                    anchors.centerIn: parent
                    text: qsTr("View on radiox.de")
                    color: parent.highlighted ? Theme.highlightColor : Theme.highlightColor
                    font.underline: true
                }
                onClicked: Qt.openUrlExternally("https://www.radiox.de/sendeplan/" + showDetailPage.slug)
            }
        }
    }
}