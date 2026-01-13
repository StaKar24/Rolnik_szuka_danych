#pragma once

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QtSql/QSqlDatabase>

class PoleDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QTableView* table;
    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QSqlDatabase db;

public:
    explicit PoleDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    int getPoleIdByName(const QString& nazwa, int rolnikId);
    void dodajPoleDialog();
    void edytujPoleDialog();
    void usunPoleDialog();
};
