#pragma once

#include <QWidget>
#include <QtSql/QSqlDatabase>

// forward declarations (przyspieszają kompilację)
class QTableView;
class QLineEdit;
class QComboBox;
class QPushButton;

class AdminDashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit AdminDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajRolnika();
    void dodajPracownika();
    void zmienDaneLogowania();
    void usunUzytkownika();
    void dezaktywujKonto();
    void aktywujKonto();

private:
    int getUserIdByLogin(const QString& login);

    QTableView* table;
    QLineEdit* filterEdit;
    QComboBox* roleFilter;
    QPushButton* filterBtn;

    QLineEdit* loginEdit;
    QLineEdit* passEdit;
    QLineEdit* imieEdit;
    QLineEdit* nazwiskoEdit;
    QLineEdit* rolnikLoginEdit;

    QPushButton* addFarmerBtn;
    QPushButton* addWorkerBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* deactivateBtn;
    QPushButton* activateBtn;

    QSqlDatabase db;
};
