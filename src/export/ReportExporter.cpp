#include "ReportExporter.h"
#include "../core/DataPoint.h"
#include <QFile>       // Для работы с файлами
#include <QTextStream> // для записи текста в файл
#include <QDebug>      // Для вывода отладочных сообщений
#include <QDateTime>   // Для получения текущего времени
#include <QPdfWriter>// Для создания PDF файлов
#include <QPainter>    // Для рисования (нужен для qpdfwriter)
#include <QTextDocument>//Для HTML в PDF (рендеринг HTML)
#include <QAbstractItemModel> // Для работы с моделью данных из QML

//Конструктор класса ReportExporter
ReportExporter::ReportExporter(QObject *parent) : QObject(parent)
{

}
//Экспорт данных в CSV файл
bool ReportExporter::exportToCSV(const QList<DataPoint>& data, const QString& filePath)
{
    //Генерируем содержимое CSV
    QString content = generateCSVContent(data);
    //Сохраняем в файл
    return saveToFile(content, filePath);

}

//Экспорт данных в PDF файл
bool ReportExporter::exportToPDF(const QList<DataPoint>& data, const QString& filePath)
{
    //Генерируем HTML с таблицей данных
    QString html = generateHTMLContent(data);

    //Создаем PDF Write (управляет созданием PDF)
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4)); // формат A4
    writer.setResolution(96);                     // Разрешение 96 DPI

    //QTextDocument умеет рендерить HTML
    QTextDocument doc;
    doc.setHtml(html);

    doc.print(&writer); //сохраняем HTML как PDF

    return true;

}

  //Экспорт текущих данных из модели QML

void ReportExporter::exportCurrentData(QObject* dataModel, const QString& filePath, const QString& format)
{
    //Преобразуем QObject* в QAbstractItemModel*
    QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(dataModel);
    if (!model){
        // Если модель невалидна - сигнал об ошибке
        emit exportFinished(false, filePath, "Invalid data model");
        return;

    }
    // Собираем данные из модели в QList<DataPoint>
    QList<DataPoint> data;
    for (int i = 0; i < model->rowCount(); ++i){

        QModelIndex index = model->index(i, 0);

        //Читает данные по ролям (как определено в DataModel::roleNames)
      // Qt::UserRole + 1 = TimestampRole
        // Qt::UserRole + 2 = TypeRole
        // Qt::UserRole + 3 = ValueRole
        // Qt::UserRole + 4 = UnitRole

    QDateTime timestamp = QDateTime::fromString(
        model->data(index, Qt::UserRole + 1).toString(),
        "yyyy-MM-dd hh:mm:ss");
    QString type = model->data(index, Qt::UserRole + 2). toString();
    double value = model->data(index, Qt::UserRole + 3). toDouble();
    QString unit = model->data(index, Qt::UserRole + 4). toString();


    data.append(DataPoint(timestamp, type, value, unit));

}
 //Вызываем соответствующий метод экспорта
bool success = false;
QString lowerFormat = format.toLower();
if (lowerFormat == "csv"){
    success = exportToCSV(data, filePath);

} else if (lowerFormat == "pdf"){
    success = exportToPDF(data, filePath);
} else {
    //Неподдерживаемый формат
    emit exportFinished(false, filePath, "Unsupported format: " + format);
    return;
}

//Сигнал о завершении экспорта
emit exportFinished(success, filePath, success ? "" : "Export failed");
}
//Генерируем содержимое CSV файла из данных

QString ReportExporter::generateCSVContent(const QList<DataPoint>& data)
{
    QString content;
    QTextStream stream(&content);

    //Заголовок колонок
    stream << "Timestamp, Type, Value, Unit\n";

    //Данные
    for (const DataPoint& point : data){
        stream << point.timestamp(). toString("yyyy-MM-dd hh:mm:ss") << ","
               << point.type() << ","
               << point.value() << ","
               << point.unit() << "\n";
}

    return content;
}

//Генерирует HTML содержимое для PDF отчета

QString ReportExporter::generateHTMLContent(const QList<DataPoint>& data)
{
    QString html;
    QTextStream stream(&html);

    // Начало HTML документа
    stream << "<!DOCTYPE html>\n"
           << "<html>\n"
           << "<head>\n"
           << "<meta charset=\"UTF-8\">\n"
           << "<title>DataMonitor Pro Report</title>\n"
           << "<style>\n"
           << "body { font-family: Arial, sans-serif; margin: 40px; }\n"
           << "h1 { color: #4caf50; }\n"
           << "table { border-collapse: collapse; width: 100%; margin-top: 20px; }\n"
           << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n"
           << "th { background-color: #4caf50; color: white; }\n"
           << "tr:nth-child(even) { background-color: #f2f2f2; }\n"
           << ".footer { margin-top: 30px; font-size: 12px; color: #666; text-align: center; }\n"
           << "</style>\n"
           << "</head>\n"
           << "<body>\n";

//Заголовок и информация
    stream << "<h1>DataMonitor Pro Report</h1>\n";
    stream << "<p>Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "</p>\n";
    stream << "<p>Total records: " << data.size() << "</p>\n";

    //Таблица с данными
    stream << "<table>\n";
    stream << "<thead>\n";
    stream << "<tr><th>Timestamp</th><th>Type</th><th>Value</th><th>Unit</th></tr>\n";
    stream << "</thead>\n";
    stream << "<tbody>\n";


    for (const DataPoint& point : data){

        stream << "<tr>"
               << "<td>" << point.timestamp().toString("yyyy-MM-dd hh:mm:ss") << "</td>"
               << "<td>" << point.type() << "</td>"
               << "<td>" << point.value() << "</td>"
               << "<td>" << point.unit() << "</td>"
               << "</tr>\n";

    }

    stream << "</tbody>\n";
    stream << "</table>\n";

    //Подвал
    stream << "<div class=\"footer\">Generated by DataMonitor Pro</div>\n";
    stream << "</body>\n";
    stream << "</html>\n";

    return html;
}

//Сохраняем текстовое содержимое в файл

bool ReportExporter::saveToFile(const QString& content, const QString& filePath)
{

    QFile file(filePath);

    //Открываем файл на запись (текстовый режим)
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){

        qDebug() << "Failed to open file: " << filePath;
        return false;

    }

    //Записываем содержимое

    QTextStream stream(&file);
    stream << content;
    file.close(); //Автоматически при разрушении QFile, но лучше явно

    qDebug() << "Exported to:" << filePath;
    return true;

}

























