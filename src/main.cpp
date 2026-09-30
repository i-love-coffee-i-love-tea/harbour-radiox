#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QTranslator>
#include <QLocale>
#include "radioxcore.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    QScopedPointer<QTranslator> translator(new QTranslator);
    if (translator->load(QLocale(), "harbour-radiox", "-", SailfishApp::pathTo("translations"))) {
        app->installTranslator(translator.data());
    }

    QScopedPointer<QQuickView> view(SailfishApp::createView());

    RadioXCore core;
    view->rootContext()->setContextProperty("radioXCore", &core);

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();
    return app->exec();
}