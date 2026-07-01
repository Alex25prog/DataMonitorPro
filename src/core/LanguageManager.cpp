#include "LanguageManager.h"

LanguageManager::LanguageManager(QQmlEngine *engine, QObject *parent)
    :QObject(parent), m_engine(engine) {
    m_translator = new QTranslator(this);
}

void LanguageManager::setLanguage(const QString &locale) {

    // Выгружаем новый файл .qm из ресурсов
    qApp->removeTranslator(m_translator);

    QString file = QString(":/translations/app_%1.qm").arg(locale);

    // Загружаем новый файл .qm из ресурсов
    // Путь ":/translations/app_" + locale + ".qm" соответствует тому, что мы указали в Cmake
    if (m_translator->load(file))
    {
        qApp->installTranslator(m_translator);
        qDebug() << "Translator loaded:" << file;
    }
    else
    {
        qDebug() << "Cannot load" << file;
    }

    m_currentLocale = locale; // Сохраняем состояние
    emit localeChanged(); // Уведомление QML
    // Этот метод принудительно заставяет QML перечислить все qsTr()
    m_engine->retranslate();
}