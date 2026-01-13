#include "Admin_Dashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlError>

#include "../Klasy_logiczne/Session.h"

AdminDashboardWindow::AdminDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Admin Dashboard");
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- filtry ---
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Filtruj po loginie...");
    mainLayout->addWidget(filterEdit);

    roleFilter = new QComboBox(this);
    roleFilter->addItems({ "Wszystkie", "admin", "rolnik", "pracownik" });
    mainLayout->addWidget(roleFilter);

    filterBtn = new QPushButton("Filtruj", this);
    mainLayout->addWidget(filterBtn);

    table = new QTableView(this);
    mainLayout->addWidget(table);

    connect(filterBtn, &QPushButton::clicked, this, &AdminDashboardWindow::loadData);

    // --- panel akcji ---
    QFormLayout* formLayout = new QFormLayout();

    loginEdit = new QLineEdit(this);
    passEdit = new QLineEdit(this);
    passEdit->setEchoMode(QLineEdit::Password);
    imieEdit = new QLineEdit(this);
    nazwiskoEdit = new QLineEdit(this);
    rolnikLoginEdit = new QLineEdit(this);

    formLayout->addRow("Login:", loginEdit);
    formLayout->addRow("Haslo:", passEdit);
    formLayout->addRow("Imie:", imieEdit);
    formLayout->addRow("Nazwisko:", nazwiskoEdit);
    formLayout->addRow("Login rolnika (dla pracownika):", rolnikLoginEdit);

    mainLayout->addLayout(formLayout);

    // przyciski
    QHBoxLayout* btnLayout = new QHBoxLayout();

    addFarmerBtn = new QPushButton("Dodaj rolnika", this);
    addWorkerBtn = new QPushButton("Dodaj pracownika", this);
    editBtn = new QPushButton("Edytuj dane logowania", this);
    deleteBtn = new QPushButton("Usun uzytkownika", this);
    deactivateBtn = new QPushButton("Dezaktywuj konto", this);
    activateBtn = new QPushButton("Aktywuj konto", this);

    btnLayout->addWidget(addFarmerBtn);
    btnLayout->addWidget(addWorkerBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(deactivateBtn);
    btnLayout->addWidget(activateBtn);

    mainLayout->addLayout(btnLayout);

    connect(addFarmerBtn, &QPushButton::clicked, this, &AdminDashboardWindow::dodajRolnika);
    connect(addWorkerBtn, &QPushButton::clicked, this, &AdminDashboardWindow::dodajPracownika);
    connect(editBtn, &QPushButton::clicked, this, &AdminDashboardWindow::zmienDaneLogowania);
    connect(deleteBtn, &QPushButton::clicked, this, &AdminDashboardWindow::usunUzytkownika);
    connect(deactivateBtn, &QPushButton::clicked, this, &AdminDashboardWindow::dezaktywujKonto);
    connect(activateBtn, &QPushButton::clicked, this, &AdminDashboardWindow::aktywujKonto);

    loadData();
}

void AdminDashboardWindow::loadData() {
    QString filtr = filterEdit->text();
    QString filtrRola = roleFilter->currentText();

    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString query = "SELECT * FROM lista_uzytkownikow WHERE 1=1";

    if (!filtr.isEmpty())
        query += " AND login ILIKE '%" + filtr + "%'";

    if (filtrRola != "Wszystkie")
        query += " AND rola = '" + filtrRola + "'";

    model->setQuery(query, db);
    table->setModel(model);
}

int AdminDashboardWindow::getUserIdByLogin(const QString& login) {
    QSqlQuery q(db);
    q.prepare("SELECT id_uzytkownika FROM Uzytkownik WHERE login = :login");
    q.bindValue(":login", login);

    if (!q.exec() || !q.next())
        return -1;

    return q.value(0).toInt();
}

void AdminDashboardWindow::dodajRolnika() {
    QSqlQuery q(db);
    q.prepare("SELECT admin_dodaj_rolnika(:login, :haslo)");
    q.bindValue(":login", loginEdit->text());
    q.bindValue(":haslo", passEdit->text());

    if (!q.exec()) {
        QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}

void AdminDashboardWindow::dodajPracownika() {
    int rolnikId = getUserIdByLogin(rolnikLoginEdit->text());
    if (rolnikId == -1) {
        QMessageBox::warning(this, "Blad", "Nie znaleziono rolnika.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT admin_dodaj_pracownika(:login, :haslo, :imie, :nazwisko, :rolnikId)");
    q.bindValue(":login", loginEdit->text());
    q.bindValue(":haslo", passEdit->text());
    q.bindValue(":imie", imieEdit->text());
    q.bindValue(":nazwisko", nazwiskoEdit->text());
    q.bindValue(":rolnikId", rolnikId);

    if (!q.exec()) {
        QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}

void AdminDashboardWindow::zmienDaneLogowania() {
    int id = getUserIdByLogin(loginEdit->text());
    if (id == -1) {
        QMessageBox::warning(this, "Blad", "Nie znaleziono uzytkownika.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT admin_zmien_dane_logowania(:id, :login, :haslo)");
    q.bindValue(":id", id);
    q.bindValue(":login", loginEdit->text());
    q.bindValue(":haslo", passEdit->text());

    if (!q.exec()) {
        QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}

void AdminDashboardWindow::usunUzytkownika() {
    int id = getUserIdByLogin(loginEdit->text());
    if (id == -1) {
        QMessageBox::warning(this, "Blad", "Nie znaleziono uzytkownika.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT admin_usun_uzytkownika(:id)");
    q.bindValue(":id", id);

    if (q.exec() && q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}

void AdminDashboardWindow::dezaktywujKonto() {
    int id = getUserIdByLogin(loginEdit->text());
    if (id == -1) {
        QMessageBox::warning(this, "Blad", "Nie znaleziono uzytkownika.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT admin_dezaktywuj_konto(:id)");
    q.bindValue(":id", id);

    if (!q.exec()) {
        QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}

void AdminDashboardWindow::aktywujKonto() {
    int id = getUserIdByLogin(loginEdit->text());
    if (id == -1) {
        QMessageBox::warning(this, "Blad", "Nie znaleziono uzytkownika.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT admin_aktywuj_konto(:id)");
    q.bindValue(":id", id);

    if (!q.exec()) {
        QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        QMessageBox::information(this, "Info", q.value(0).toString());
        loadData();
    }
}
