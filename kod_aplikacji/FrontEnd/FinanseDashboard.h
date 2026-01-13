#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QSqlDatabase>
#include <QChartView> 

class FinanseDashboard : public QWidget {
    Q_OBJECT

public:
    explicit FinanseDashboard(QSqlDatabase db, QWidget* parent = nullptr);

private:
    QSqlDatabase db;
    QTabWidget* tabs;

    QWidget* tabTransakcje;
    QWidget* tabKupno;
    QWidget* tabSprzedaz;
    QWidget* tabWykres;

    QChartView* chartView = nullptr; 
    bool wykresInitialized = false;

    void initTransakcjeTab();
    void initKupnoTab();
    void initSprzedazTab();
    void initWykresTab(QWidget* tab);
};
