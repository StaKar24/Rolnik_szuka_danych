#pragma once
#include <QWidget>
#include <QTableView>
#include <QSqlDatabase>

class TransakcjaDetailsWindow : public QWidget {
    Q_OBJECT
public:
    explicit TransakcjaDetailsWindow(QSqlDatabase db, int idTransakcji, const QString& typ, QWidget* parent = nullptr);

private:
    void loadData();

    QSqlDatabase db;
    int idTransakcji;
    QString typ;
    QTableView* tabela;
};
