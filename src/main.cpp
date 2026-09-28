#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include "radioxcore.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    RadioXCore core;
    view->rootContext()->setContextProperty("radioXCore", &core);

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();
    return app->exec();
}