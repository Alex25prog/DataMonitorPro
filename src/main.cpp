#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "controllers/MainController.h"
#include <QSqlDatabase>
#include <QDebug>
#include "graphics/graphwidget.h"
#include <QQuickStyle>
int main(int argc, char *argv[])
{
    //QQuickStyle::setStyle("Material");
    //qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");//Включить темную тему
    QApplication app(argc, argv);
    qDebug() << "Доступные SQL драйверы:" << QSqlDatabase::drivers();

    QQmlApplicationEngine engine;

    //Регистрируем GraphWidget для QML
    qmlRegisterType<GraphWidget>("DataMonitorPro", 1, 0, "GraphWidget");
    
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