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
            text: {
                if (radioXCore.playbackUrl.length === 0)
                    return qsTr("Stopped")
                if (audioPlayer.playbackState !== Audio.PlayingState)
                    return qsTr("Stopped")
                return radioXCore.playbackTitle.length > 0
                        ? qsTr("Playing…")
                        : qsTr("Live")
            }
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }

    CoverActionList {
        id: coverActions

        CoverAction {
            iconSource: audioPlayer.playbackState === Audio.PlayingState
                        ? "image://theme/icon-cover-pause"
                        : "image://theme/icon-cover-play"
            onTriggered: {
                if (audioPlayer.playbackState === Audio.PlayingState) {
                    if (radioXCore.playbackTitle.length > 0)
                        audioPlayer.pause()
                    else
                        radioXCore.stopPlayback()
                } else if (radioXCore.playbackUrl.length > 0) {
                    audioPlayer.play()
                } else {
                    radioXCore.openLivestream()
                }
            }
        }
    }
}