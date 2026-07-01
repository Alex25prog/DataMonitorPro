#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "controllers/MainController.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QQuickStyle>
#include "src/core/LanguageManager.h"
int main(int argc, char *argv[])
{

    qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_QUICK_CONTROLS_CONF", ":/qtquickcontrols2.conf");
    QQuickStyle::setStyle("Material");
    //qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");//Включить темную тему
    QApplication app(argc, argv);
    qDebug() << "Доступные SQL драйверы:" << QSqlDatabase::drivers();

    qDebug() << "1";

    QQmlApplicationEngine engine;
    qDebug() << "2";

    //Регистрируем GraphWidget для QML
    //qmlRegisterType<GraphWidget>("DataMonitorPro", 1, 0, "GraphWidget");
    
    // Создаем контроллер в HEAP
    MainController* controller = new MainController(&engine);
    qDebug() << "3";

    //регистрируем контроллер в QML под именем "controller"
    engine.rootContext()->setContextProperty("controller", controller);

    // Инициализация LanguageManager
    LanguageManager* languageManager = new LanguageManager(&engine, &engine);
    qDebug() << "4";

    // Регистрируем его в QML с именем languageManager
    engine.rootContext()->setContextProperty("languageManager", languageManager);
    qDebug() << "5";

    // Загружаем QML — путь должен совпадать с URI
    const QUrl url("qrc:/DataMonitorPro/qml/main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    qDebug() << "7";
    qDebug() << "Root objects:" << engine.rootObjects().size();

    return app.exec();

}