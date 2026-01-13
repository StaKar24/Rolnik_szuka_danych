#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QSqlDatabase>
#include <QPushButton>

class FarmerDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QLabel* userLabel;
    QLabel* dateTimeLabel;
    QGridLayout* grid;
    QSqlDatabase db;

public:
    explicit FarmerDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void updateDateTime();
};
