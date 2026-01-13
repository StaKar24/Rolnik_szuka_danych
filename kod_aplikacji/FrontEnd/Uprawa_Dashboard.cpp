#include "Uprawa_Dashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug> 
#include <QSqlError>
#include "../Klasy_logiczne/Session.h"

UprawyDashboardWindow::UprawyDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Upraw");
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Szukaj po nazwie uprawy...");
    onlyActiveCheck = new QCheckBox("Tylko niezebrane", this);
    filterBtn = new QPushButton("Filtruj", this);

    filterLayout->addWidget(filterEdit);
    filterLayout->addWidget(onlyActiveCheck);
    filterLayout->addWidget(filterBtn);
    mainLayout->addLayout(filterLayout);

    // --- Tabela ---
    table = new QTableView(this);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table);

    // --- Przyciski ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj uprawę", this);
    editBtn = new QPushButton("Edytuj uprawę", this);
    deleteBtn = new QPushButton("Usuń uprawę", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &UprawyDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &UprawyDashboardWindow::dodajUpraweDialog);
    connect(editBtn, &QPushButton::clicked, this, &UprawyDashboardWindow::edytujUpraweDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &UprawyDashboardWindow::usunUpraweDialog);

    loadData();
}

void UprawyDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();
    QString query = QString("SELECT id_uprawy, nazwa_pola, nazwa_uprawy, data_siewu, data_zbioru "
        "FROM view_uprawy_info WHERE id_rolnika = %1").arg(rolnikId);

    if (!filterEdit->text().isEmpty()) {
        query += QString(" AND nazwa_uprawy ILIKE '%%1%'").arg(filterEdit->text());
    }
    if (onlyActiveCheck->isChecked()) {
        query += " AND data_zbioru IS NULL";
    }

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int UprawyDashboardWindow::getPoleIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_pola FROM pole WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

void UprawyDashboardWindow::dodajUpraweDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj uprawę");
    QFormLayout form(&dlg);

    QLineEdit poleNazwaEdit, nazwaEdit;
    form.addRow("Nazwa pola:", &poleNazwaEdit);
    form.addRow("Nazwa uprawy:", &nazwaEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int poleId = getPoleIdByName(poleNazwaEdit.text(), rolnikId);

        if (poleId == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono pola o podanej nazwie.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT dodaj_uprawe(:rolnikId, :poleId, :nazwa)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":poleId", poleId);
        q.bindValue(":nazwa", nazwaEdit.text());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "Dodano Uprawe");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void UprawyDashboardWindow::edytujUpraweDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj uprawę");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, siewEdit, zbiorEdit;
    form.addRow("ID uprawy:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Data siewu (YYYY-MM-DD):", &siewEdit);
    form.addRow("Data zbioru (YYYY-MM-DD):", &zbiorEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT edytuj_uprawe(:rolnikId, :id, :nazwa, :siew, :zbior)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":siew", siewEdit.text().isEmpty() ? QVariant() : siewEdit.text());
        q.bindValue(":zbior", zbiorEdit.text().isEmpty() ? QVariant() : zbiorEdit.text());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "Zedytowano Uprawe");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void UprawyDashboardWindow::usunUpraweDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń uprawę");
    QFormLayout form(&dlg);

    QLineEdit idEdit;
    form.addRow("ID uprawy:", &idEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idUprawy = idEdit.text().toInt();

        auto reply = QMessageBox::warning(this, "Uwaga",
            "Usunięcie uprawy spowoduje utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_uprawe(:rolnikId, :id)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idUprawy);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "Usuniento Uprawe");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}
