#pragma once
#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQueryModel>

class PracownicyDashboard : public QWidget {
    Q_OBJECT

public:
    explicit PracownicyDashboard(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void openDetails();

private:
    QTableView* table;
    QLineEdit* nazwiskoFilter;
    QSpinBox* minWykonane;
    QSpinBox* minAktywne;
    QPushButton* filterBtn;
    QPushButton* detailsBtn;

    QSqlDatabase db;
};
