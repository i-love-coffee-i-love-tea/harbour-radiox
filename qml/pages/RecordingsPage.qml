import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

Page {
    id: recordingsPage

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: radioXCore.recordingsModel

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: radioXCore.refresh()
            }
        }

        header: PageHeader {
            title: qsTr("Recordings")
        }

        delegate: RecordingDelegate {
            recordingId: model.recordingId
            showName: model.showName
            subtitle: model.subtitle
            dateStr: model.dayHeader
            time: model.time
            playbackUrl: model.playbackUrl
            dayHeader: model.dayHeader ? model.dayHeader : ""
        }

        VerticalScrollDecorator {}
    }
}