#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "controllers/MainController.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QQuickStyle>
#include "src/core/LanguageManager.h"
#include "src/models/CandleData.h"
#include <QSslSocket>


// Функция для регистрация типа
static void registerTypes() {
    qRegisterMetaType<CandleData>("CandleData");
    qRegisterMetaType<QList<CandleData>>("QList<CandleData>");
    qRegisterMetaType<QVariantList>("QVariantList");
}
int main(int argc, char *argv[])
{
    // Регистрируем типы до создания QApplication
    registerTypes();

    //qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_QUICK_CONTROLS_CONF", ":/qtquickcontrols2.conf");
    QQuickStyle::setStyle("Material");

    QApplication app(argc, argv);

    qDebug() << "DataMonitorPro Startup";
    qDebug() << "SSL supported:" << QSslSocket::supportsSsl();
    qDebug() << "SSL version:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "Доступные SQL драйверы:" << QSqlDatabase::drivers();


    QQmlApplicationEngine engine;

    // Создаем контроллер в HEAP
    MainController* controller = new MainController(&engine);

    //регистрируем контроллер в QML под именем "controller"
    engine.rootContext()->setContextProperty("controller", controller);

    // Инициализация LanguageManager
    LanguageManager* languageManager = new LanguageManager(&engine, &engine);

    // Регистрируем его в QML с именем languageManager
    engine.rootContext()->setContextProperty("languageManager", languageManager);

    // Загружаем QML — путь должен совпадать с URI
    const QUrl url("qrc:/DataMonitorPro/qml/main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    qDebug() << "Root objects:" << engine.rootObjects().size();

    return app.exec();

}