#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QTableView>
#include <QSqlDatabase>

class UprawyDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QTableView* table;
    QLineEdit* filterEdit;
    QCheckBox* onlyActiveCheck;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;

public:
    explicit UprawyDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajUpraweDialog();
    void edytujUpraweDialog();
    void usunUpraweDialog();
    int getPoleIdByName(const QString& nazwa, int rolnikId);
};
