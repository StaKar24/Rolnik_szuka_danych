#pragma once

#include <QWidget>
#include <QTableView>
#include <QCheckBox>
#include <QPushButton>
#include <QtSql/QSqlDatabase>

class BudynkiDashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit BudynkiDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajBudynekDialog();
    void edytujBudynekDialog();
    void usunBudynekDialog();

private:
    int getBudynekIdByName(const QString& nazwa, int rolnikId);

    QTableView* table;
    QCheckBox* filterMaszyny;
    QCheckBox* filterZwierzęta;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;
};
