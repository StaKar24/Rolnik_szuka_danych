#pragma once
#include <QWidget>
#include <QLabel>
#include <QTableView>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQueryModel>

class StatystykiDashboard : public QWidget {
    Q_OBJECT

public:
    explicit StatystykiDashboard(QSqlDatabase db, int rolnikId, QWidget* parent = nullptr);

private:
    QSqlDatabase db;
    int rolnikId;

    QLabel* raportGlowneLabel;
    QComboBox* filtrTypCombo;
    QTableView* tabelaHaving;
    QSqlQueryModel* modelHaving;

    void loadRaportGlowne();
    void loadRaportHaving(const QString& typ);
};
