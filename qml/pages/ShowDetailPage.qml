import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: showDetailPage

    property string slug: ""
    property string showName: ""

    Component.onCompleted: radioXCore.fetchShowDetail(slug)

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("View on radiox.de")
                onClicked: Qt.openUrlExternally("https://www.radiox.de/sendungen/" + showDetailPage.slug)
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: radioXCore.showDetail.title || showDetailPage.showName || qsTr("Show Details")
            }

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                size: BusyIndicatorSize.Medium
                running: radioXCore.loadingShowDetail
                visible: running
            }

            Image {
                id: showImage
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: implicitHeight > 0 ? width * (implicitHeight / implicitWidth) : 0
                anchors.horizontalCenter: parent.horizontalCenter
                visible: status === Image.Ready
                source: radioXCore.showDetail.imageUrl || ""
                fillMode: Image.Stretch
                autoTransform: true
                asynchronous: true
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                visible: text.length > 0
                text: radioXCore.showDetail.description || ""
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
                lineHeight: 1.15
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !radioXCore.loadingShowDetail
                         && (radioXCore.showDetail.description || "").length === 0
                text: qsTr("No details available for this show.")
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
        }

        VerticalScrollDecorator {}
    }
}