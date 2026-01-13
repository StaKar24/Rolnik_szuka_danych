#pragma once

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QtSql/QSqlDatabase>

class MaszynyDashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit MaszynyDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();

    void dodajMaszyneDialog();
    void edytujMaszyneDialog();
    void ustawDatePrzegladuDialog();
    void usunMaszyneDialog();

private:
    int getBudynekIdByName(const QString& nazwa, int rolnikId);
    int getMaszynaIdByName(const QString& nazwa, int rolnikId);

    QTableView* table;
    QLineEdit* filterEdit;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* setDateBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;
};
