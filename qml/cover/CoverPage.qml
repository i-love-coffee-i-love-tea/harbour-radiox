import QtQuick 2.6
import Sailfish.Silica 1.0

CoverBackground {
    id: coverPage

    Column {
        anchors.centerIn: parent
        spacing: Theme.paddingMedium

        Label {
            text: qsTr("radio x")
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeLarge
            color: Theme.highlightColor
        }

        Label {
            text: "FM 91,8"
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.primaryColor
        }

        Label {
            text: app.currentPlaybackUrl.length > 0
                  ? qsTr("Playing…")
                  : qsTr("Stopped")
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }

    CoverActionList {
        id: coverActions

        CoverAction {
            iconSource: app.currentPlaybackUrl.length > 0
                        ? "image://theme/icon-cover-pause"
                        : "image://theme/icon-cover-play"
            onTriggered: {
                if (app.currentPlaybackUrl.length > 0) {
                    app.currentPlaybackUrl = ""
                } else {
                    radioXCore.openLivestream()
                }
            }
        }
    }
}