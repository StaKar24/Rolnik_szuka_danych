#include "Magazyn_Dashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlError>

#include "../Klasy_logiczne/Session.h"

MagazynyDashboardWindow::MagazynyDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Magazynów");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterMagazynEdit = new QLineEdit(this);
    filterMagazynEdit->setPlaceholderText("Szukaj w magazynie...");
    onlyPlonyCheck = new QCheckBox("Tylko plony", this);
    onlySurowceCheck = new QCheckBox("Tylko surowce", this);
    orderByIloscCheck = new QCheckBox("Sortuj po ilości", this);
    filterBtn = new QPushButton("Filtruj", this);

    filterLayout->addWidget(filterMagazynEdit);
    filterLayout->addWidget(onlyPlonyCheck);
    filterLayout->addWidget(onlySurowceCheck);
    filterLayout->addWidget(orderByIloscCheck);
    filterLayout->addWidget(filterBtn);
    mainLayout->addLayout(filterLayout);

    // --- Tabela ---
    table = new QTableView(this);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table);

    // --- Przyciski ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj magazyn", this);
    editBtn = new QPushButton("Edytuj magazyn", this);
    deleteBtn = new QPushButton("Usuń magazyn", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &MagazynyDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &MagazynyDashboardWindow::dodajMagazynDialog);
    connect(editBtn, &QPushButton::clicked, this, &MagazynyDashboardWindow::edytujMagazynDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &MagazynyDashboardWindow::usunMagazynDialog);

    loadData();
}

void MagazynyDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();

    QString query = QString(
        "SELECT id_magazynu, nazwa_magazynu, typ, id_elementu, nazwa, ilosc "
        "FROM view_zawartosc_magazynow "
        "WHERE id_magazynu IN (SELECT id_magazynu FROM Magazyn WHERE id_rolnika = %1)")
        .arg(rolnikId);

    if (!filterMagazynEdit->text().isEmpty()) {
        query += QString(" AND nazwa ILIKE '%%1%'").arg(filterMagazynEdit->text());
    }
    if (onlyPlonyCheck->isChecked())
        query += " AND typ = 'plon'";
    if (onlySurowceCheck->isChecked())
        query += " AND typ = 'surowiec'";
    if (orderByIloscCheck->isChecked())
        query += " ORDER BY ilosc DESC";

    auto* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

void MagazynyDashboardWindow::dodajMagazynDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj magazyn");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit;
    form.addRow("Nazwa magazynu:", &nazwaEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT dodaj_magazyn(:rolnikId, :nazwa)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":nazwa", nazwaEdit.text());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void MagazynyDashboardWindow::edytujMagazynDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj magazyn");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit;
    form.addRow("ID magazynu:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        QSqlQuery q(db);
        q.prepare("SELECT edytuj_magazyn(:id, :nazwa)");
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":nazwa", nazwaEdit.text());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void MagazynyDashboardWindow::usunMagazynDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń magazyn");
    QFormLayout form(&dlg);

    QLineEdit idEdit;
    form.addRow("ID magazynu:", &idEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int idMagazynu = idEdit.text().toInt();

        auto reply = QMessageBox::warning(
            this,
            "Uwaga",
            "Usunięcie magazynu może spowodować utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::No)
            return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_magazyn(:id)");
        q.bindValue(":id", idMagazynu);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}
