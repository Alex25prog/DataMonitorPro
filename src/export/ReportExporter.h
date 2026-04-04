#ifndef REPORTEXPORTER_H
#define REPORTEXPORTER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../core/DataPoint.h"

class ReportExporter : public QObject
{
    Q_OBJECT

public:
    explicit ReportExporter(QObject *parent = nullptr);

    Q_INVOKABLE bool exportToCSV(const QList<DataPoint>& data, const QString& filePath);
    Q_INVOKABLE bool exportToPDF(const QList<DataPoint>& data, const QString& filePath);
    Q_INVOKABLE void exportCurrentData(QObject* dataModel, const QString& filePath, const QString& format);

signals:
    void exportFinished(bool success, const QString& filePath, const QString& error);
    void exportProgress(int percent);


private:
    QString generateCSVContent(const QList<DataPoint>& data);
    QString generateHTMLContent(const QList<DataPoint>& data);
    bool saveToFile(const QString& content, const QString& filePath);
};

#endif
