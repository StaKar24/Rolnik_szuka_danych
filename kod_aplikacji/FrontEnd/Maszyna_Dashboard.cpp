#include "Maszyna_Dashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QMessageBox>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlError>

#include "../Klasy_logiczne/Session.h"

MaszynyDashboardWindow::MaszynyDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Maszyn");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Szukaj po nazwie maszyny...");
    filterBtn = new QPushButton("Filtruj", this);

    filterLayout->addWidget(filterEdit);
    filterLayout->addWidget(filterBtn);
    mainLayout->addLayout(filterLayout);

    // --- Tabela ---
    table = new QTableView(this);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table);

    // --- Przyciski ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj maszynę", this);
    editBtn = new QPushButton("Edytuj maszynę", this);
    setDateBtn = new QPushButton("Ustaw datę przeglądu", this);
    deleteBtn = new QPushButton("Usuń maszynę", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(setDateBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &MaszynyDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &MaszynyDashboardWindow::dodajMaszyneDialog);
    connect(editBtn, &QPushButton::clicked, this, &MaszynyDashboardWindow::edytujMaszyneDialog);
    connect(setDateBtn, &QPushButton::clicked, this, &MaszynyDashboardWindow::ustawDatePrzegladuDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &MaszynyDashboardWindow::usunMaszyneDialog);

    loadData();
}

void MaszynyDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();

    QString query = QString(
        "SELECT id_maszyny, nazwa, typ, data_przegladu, nazwa_budynku, aktywne_zadania "
        "FROM view_maszyny_info WHERE id_rolnika = %1"
    ).arg(rolnikId);

    if (!filterEdit->text().isEmpty()) {
        query += QString(" AND nazwa ILIKE '%%1%'").arg(filterEdit->text());
    }

    auto* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int MaszynyDashboardWindow::getBudynekIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_budynku FROM Budynek WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();

    return -1;
}

int MaszynyDashboardWindow::getMaszynaIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_maszyny FROM Maszyna WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();

    return -1;
}

void MaszynyDashboardWindow::dodajMaszyneDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj maszynę");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, typEdit, dataEdit, budynekEdit;
    form.addRow("Nazwa:", &nazwaEdit);
    form.addRow("Typ:", &typEdit);
    form.addRow("Data przeglądu (YYYY-MM-DD):", &dataEdit);
    form.addRow("Nazwa budynku:", &budynekEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int budynekId = getBudynekIdByName(budynekEdit.text(), rolnikId);

        if (budynekId == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono budynku.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT dodaj_maszyne(:rolnikId, :nazwa, :typ, :data, :budynekId)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":nazwa", nazwaEdit.text());
        q.bindValue(":typ", typEdit.text());
        q.bindValue(":data", dataEdit.text().isEmpty() ? QVariant() : dataEdit.text());
        q.bindValue(":budynekId", budynekId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }

        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void MaszynyDashboardWindow::edytujMaszyneDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj maszynę");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, typEdit, budynekEdit;
    form.addRow("ID maszyny:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Nowy typ:", &typEdit);
    form.addRow("Nazwa budynku:", &budynekEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int budynekId = getBudynekIdByName(budynekEdit.text(), rolnikId);

        if (budynekId == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono budynku.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT edytuj_maszyne(:id, :rolnikId, :nazwa, :typ, :budynekId)");
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":typ", typEdit.text().isEmpty() ? QVariant() : typEdit.text());
        q.bindValue(":budynekId", budynekId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }

        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void MaszynyDashboardWindow::ustawDatePrzegladuDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Ustaw datę przeglądu");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, dataEdit;
    form.addRow("Nazwa maszyny:", &nazwaEdit);
    form.addRow("Data przeglądu (YYYY-MM-DD):", &dataEdit);

    QPushButton okBtn("Ustaw");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int maszynaId = getMaszynaIdByName(nazwaEdit.text(), rolnikId);

        if (maszynaId == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono maszyny.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT ustaw_date_przegladu(:id, :data)");
        q.bindValue(":id", maszynaId);
        q.bindValue(":data", dataEdit.text());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }

        loadData();
        dlg.accept();
        });

    dlg.exec();
}

void MaszynyDashboardWindow::usunMaszyneDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń maszynę");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit;
    form.addRow("Nazwa maszyny:", &nazwaEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int maszynaId = getMaszynaIdByName(nazwaEdit.text(), rolnikId);

        auto reply = QMessageBox::warning(
            this,
            "Uwaga",
            "Usunięcie maszyny spowoduje utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No)
            return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_maszyne(:id)");
        q.bindValue(":id", maszynaId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }

        loadData();
        dlg.accept();
        });

    dlg.exec();
}
