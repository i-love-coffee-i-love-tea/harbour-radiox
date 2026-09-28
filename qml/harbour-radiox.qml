import QtQuick 2.6
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: app
    _defaultPageOrientations: Orientation.All

    Connections {
        target: radioXCore
        onPlaybackUrlChanged: {
            console.log("Playback URL:", url)
        }
    }

    initialPage: Component { MainPage {} }
    cover: Component { CoverPage {} }

    Component.onCompleted: {
        console.log("App starting, calling refresh...")
        radioXCore.refresh()
    }
}