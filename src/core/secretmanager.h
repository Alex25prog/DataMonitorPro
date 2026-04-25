#ifndef SECRETMANAGER_H
#define SECRETMANAGER_H

#include <QString>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>


class SecretManager
{
public:
    //Получить API ключ OpenWeatherMap
    static QString getWeatherApiKey() {
        return readSecret("weather_api.key");
    }

    //Получить пароль от БД
    static QString getDbPassword() {
        return readSecret("db_password.key");
    }

    //Получить любой секрет от имени файла
    static QString readSecret(const QString& filename) {
        //Приоритет 1: .secret в текущей директории
        QString path = QCoreApplication::applicationDirPath() + "/.secrets/" + filename;
        if (QFile::exists(path)) {
            return readFile(path);
        }

        //Приоритет 2: .secrets в домашней директории пользователя
        path = QCoreApplication::applicationDirPath() + "/../../.secrets" + filename;
        if (QFile::exists(path)) {
            return readFile(path);
        }

        //Приоритет 3: .secrets в корне проекта (для разработки)
        path = QDir::homePath() + "/.config/DataMonitorPro/secrets" + filename; //Относительно build директории
        if (QFile::exists(path)) {
            return readFile(path);
        }

        //Приоритет 4: Абсолютный путь для WSL/Windows
        path = "/mnt/d/Projects/DataMonitorPro/.secrets/" + filename;
        if (QFile::exists(path)) {
            return readFile(path);
        }

        path = "D:/Projects/DataMonitorPro/.secrets/" + filename;
        if (QFile::exists(path)) {
            return readFile(path);
        }

        qDebug() << "Secret not found:" << filename;
        return QString();
    }

private:
    static QString readFile(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Filed to open secret file:" << path;
            return QString();
        }

        QString content = QString::fromUtf8(file.readAll()).trimmed();
        file.close();

        qDebug() << "loaded secret from:" << path;
        return content;

    }
};


#endif





