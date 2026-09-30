import QtQuick 2.6
import QtMultimedia 5.0
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: app
    _defaultPageOrientations: Orientation.All

    property bool audioPlaying: audioPlayer.playbackState === Audio.PlayingState

    Audio {
        id: audioPlayer
        autoLoad: true
        onError: {
            console.warn("Audio error:", errorString, error)
        }
        onStatusChanged: console.log("Audio status:", status)
        onPlaybackStateChanged: console.log("Audio playbackState:", playbackState)
    }

    Connections {
        target: radioXCore
        onPlaybackUrlChanged: {
            console.log("Setting audio source:", url)
            audioPlayer.source = url
            audioPlayer.play()
        }
        onPlaybackStopped: {
            audioPlayer.stop()
            audioPlayer.source = ""
        }
    }

    initialPage: Component { MainPage {} }
    cover: Component { CoverPage {} }

    Component.onCompleted: {
        radioXCore.refresh()
    }
}