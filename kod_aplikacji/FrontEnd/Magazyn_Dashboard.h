#pragma once

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QtSql/QSqlDatabase>

class MagazynyDashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit MagazynyDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajMagazynDialog();
    void edytujMagazynDialog();
    void usunMagazynDialog();

private:
    QTableView* table;
    QLineEdit* filterMagazynEdit;
    QCheckBox* onlyPlonyCheck;
    QCheckBox* onlySurowceCheck;
    QCheckBox* orderByIloscCheck;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;
};
