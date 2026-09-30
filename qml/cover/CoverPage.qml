import QtQuick 2.6
import QtMultimedia 5.0
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
            text: radioXCore.playbackUrl.length > 0
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
            iconSource: radioXCore.playbackUrl.length > 0
                        ? "image://theme/icon-cover-pause"
                        : "image://theme/icon-cover-play"
            onTriggered: {
                if (radioXCore.playbackUrl.length > 0) {
                    if (audioPlayer.playbackState === Audio.PlayingState)
                        audioPlayer.pause()
                    else
                        audioPlayer.play()
                } else {
                    radioXCore.openLivestream()
                }
            }
        }
    }
}