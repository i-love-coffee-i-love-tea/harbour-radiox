import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

Page {
    id: sendetippsPage

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: radioXCore.sendetippsModel

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: radioXCore.refresh()
            }
        }

        header: PageHeader {
            title: qsTr("Sendetipps")
        }

        delegate: SendetippDelegate {
            title: model.title
            showName: model.showName
            showSlug: model.showSlug
            dateTime: model.dateTime
            description: model.description
            imageUrl: model.imageUrl
        }

        VerticalScrollDecorator {}
    }
}