#pragma once

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QSqlQuery>

class PlonyDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QTableView* table;
    QLineEdit* filterEdit;
    QCheckBox* onlyRolneCheck;
    QCheckBox* onlyZwierzeCheck;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;

public:
    explicit PlonyDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    int getMagazynIdByName(const QString& nazwa, int rolnikId);
    void dodajPlonDialog();
    void edytujPlonDialog();
    void usunPlonDialog();
};
