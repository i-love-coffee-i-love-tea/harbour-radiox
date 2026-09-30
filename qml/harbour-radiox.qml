import QtQuick 2.6
import QtMultimedia 5.0
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: app
    _defaultPageOrientations: Orientation.All

    Audio {
        id: audioPlayer
        autoLoad: true
    }

    Connections {
        target: radioXCore
        onPlaybackUrlChanged: {
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