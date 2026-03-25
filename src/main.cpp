#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "controllers/MainController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    
    // Создаем контроллер
    MainController controller(&engine);
    

    //регистрируем контроллер в QML под именем "controller"
    engine.rootContext()->setContextProperty("controller", &controller);

    // Загружаем QML — путь должен совпадать с URI
    const QUrl url("qrc:/DataMonitorPro/qml/main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}