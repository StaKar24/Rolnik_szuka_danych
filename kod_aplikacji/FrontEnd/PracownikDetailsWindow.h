#pragma once
#include <QWidget>
#include <QTableView>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class PracownikDetailsWindow : public QWidget {
    Q_OBJECT

public:
    explicit PracownikDetailsWindow(QSqlDatabase db, int id, QWidget* parent = nullptr);

private:
    void loadData();

    QTableView* aktywne;
    QTableView* wykonane;
    QPushButton* closeBtn;
    QSqlDatabase db;
    int pracownikId;
};
