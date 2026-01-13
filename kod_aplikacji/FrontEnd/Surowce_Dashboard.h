#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QSqlDatabase>

class SurowceDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QTableView* table;
    QLineEdit* filterEdit;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;

public:
    explicit SurowceDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajSurowiecDialog();
    void edytujSurowiecDialog();
    void usunSurowiecDialog();
    int getMagazynIdByName(const QString& nazwa, int rolnikId);
};
