#include "Budynek_Dashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlError>

#include "../Klasy_logiczne/Session.h"

BudynkiDashboardWindow::BudynkiDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Budynków");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterMaszyny = new QCheckBox("Tylko z maszynami", this);
    filterZwierzęta = new QCheckBox("Tylko ze zwierzętami", this);
    filterBtn = new QPushButton("Filtruj", this);

    filterLayout->addWidget(filterMaszyny);
    filterLayout->addWidget(filterZwierzęta);
    filterLayout->addWidget(filterBtn);
    mainLayout->addLayout(filterLayout);

    // --- Tabela ---
    table = new QTableView(this);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table);

    // --- Przyciski ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj budynek", this);
    editBtn = new QPushButton("Edytuj budynek", this);
    deleteBtn = new QPushButton("Usuń budynek", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &BudynkiDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &BudynkiDashboardWindow::dodajBudynekDialog);
    connect(editBtn, &QPushButton::clicked, this, &BudynkiDashboardWindow::edytujBudynekDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &BudynkiDashboardWindow::usunBudynekDialog);

    loadData();
}

void BudynkiDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();

    QString query = QString(
        "SELECT id_budynku, nazwa, typ, pojemnosc || ' m2' AS pojemnosc, "
        "liczba_zwierzat, liczba_maszyn "
        "FROM view_budynki_info WHERE id_rolnika = %1")
        .arg(rolnikId);

    if (filterMaszyny->isChecked())
        query += " AND liczba_maszyn > 0";
    if (filterZwierzęta->isChecked())
        query += " AND liczba_zwierzat > 0";

    auto* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int BudynkiDashboardWindow::getBudynekIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_budynku FROM Budynek WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();

    return -1;
}

void BudynkiDashboardWindow::dodajBudynekDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj budynek");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, typEdit, pojEdit;
    form.addRow("Nazwa:", &nazwaEdit);
    form.addRow("Typ:", &typEdit);
    form.addRow("Pojemność (m2):", &pojEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT dodaj_budynek(:rolnikId, :nazwa, :typ, :poj)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":nazwa", nazwaEdit.text());
        q.bindValue(":typ", typEdit.text().isEmpty() ? QVariant() : typEdit.text());
        q.bindValue(":poj", pojEdit.text().isEmpty() ? QVariant() : pojEdit.text().toInt());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void BudynkiDashboardWindow::edytujBudynekDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj budynek");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, typEdit, pojEdit;
    form.addRow("ID budynku:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Nowy typ:", &typEdit);
    form.addRow("Nowa pojemność (m2):", &pojEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT edytuj_budynek(:rolnikId, :id, :nazwa, :typ, :poj)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":typ", typEdit.text().isEmpty() ? QVariant() : typEdit.text());
        q.bindValue(":poj", pojEdit.text().isEmpty() ? QVariant() : pojEdit.text().toInt());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void BudynkiDashboardWindow::usunBudynekDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń budynek");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit;
    form.addRow("Nazwa budynku:", &nazwaEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idBudynku = getBudynekIdByName(nazwaEdit.text(), rolnikId);

        if (idBudynku == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono budynku.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT usun_budynek(:rolnikId, :id)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idBudynku);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        loadData();
        dlg.accept();
        });

    dlg.exec();
}
