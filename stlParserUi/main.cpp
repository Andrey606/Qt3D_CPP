#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <stlparser.h>
#include <widget.h>
#include <lines.h>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
/*
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);*/

    stlParser parser;
    parser.uploadFile();
    parser.overhangsThatShouldHaveSupports();

    Widget w(700, 700);
    w._header = parser._header;
    w._overhangsTriangles= parser._overhangsTriangles;
    w._triangles = parser._triangles;
    w.show();


    return app.exec();
}
