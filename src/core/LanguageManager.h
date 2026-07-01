#pragma once

#include <QObject>
#include <QTranslator>
#include <QGuiApplication>
#include <QQmlEngine>

class LanguageManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLocale READ currentLocale NOTIFY localeChanged)
public:
    explicit LanguageManager(QQmlEngine *engine, QObject *parent = nullptr);
    QString currentLocale() const { return m_currentLocale; }

    Q_INVOKABLE void setLanguage(const QString &locale); // Например "ru" или "en"

signals:
    void localeChanged();

 private:
    QQmlEngine *m_engine;
    QTranslator *m_translator;
    QString m_currentLocale = "en";
 };
