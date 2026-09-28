import QtQuick 2.6
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import "pages"
import "cover"

ApplicationWindow {
    id: app
    _defaultPageOrientations: Orientation.All

    property string currentPlaybackUrl: ""

    Audio {
        id: audioPlayer
        onError: {
            console.warn("Audio error:", errorString)
        }
    }

    Connections {
        target: radioXCore
        onPlaybackUrlChanged: {
            currentPlaybackUrl = url
            audioPlayer.source = url
            audioPlayer.play()
        }
        onPlaybackStopped: {
            audioPlayer.stop()
        }
    }

    initialPage: Component { MainPage {} }
    cover: Component { CoverPage {} }

    Component.onCompleted: radioXCore.refresh()
}