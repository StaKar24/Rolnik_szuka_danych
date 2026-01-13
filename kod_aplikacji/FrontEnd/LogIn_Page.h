#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <libpq-fe.h>

#include "../Klasy_logiczne/AuthService.h"
#include "../Klasy_logiczne/Session.h"
#include "Rolnik_Dashboard.h"
#include "Admin_Dashboard.h"
#include "Pracownik_Dashboard.h"

class LoginWindow : public QWidget {
    Q_OBJECT
private:
    QLineEdit* loginEdit;
    QLineEdit* passEdit;
    QPushButton* loginBtn;
    AuthService* auth;
    QSqlDatabase mainDb;

public:
    explicit LoginWindow(PGconn* conn, QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void onLogin();
};
