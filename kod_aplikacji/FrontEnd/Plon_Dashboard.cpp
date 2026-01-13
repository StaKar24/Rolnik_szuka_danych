#include "Plon_Dashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDialog>
#include <QSqlError>
#include "../Klasy_logiczne/Session.h"

PlonyDashboardWindow::PlonyDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Plonów");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* legenda = new QLabel(this);
    legenda->setText(
        "Legenda jednostek:\n"
        "- Objętość: litry (l)\n"
        "- Waga: kilogramy (kg)\n"
        "- Ilość: sztuki / liczba"
    );
    mainLayout->addWidget(legenda);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Szukaj po nazwie plonu...");
    onlyRolneCheck = new QCheckBox("Tylko roślinne", this);
    onlyZwierzeCheck = new QCheckBox("Tylko zwierzęce", this);
    filterBtn = new QPushButton("Filtruj", this);

    filterLayout->addWidget(filterEdit);
    filterLayout->addWidget(onlyRolneCheck);
    filterLayout->addWidget(onlyZwierzeCheck);
    filterLayout->addWidget(filterBtn);
    mainLayout->addLayout(filterLayout);

    // --- Tabela ---
    table = new QTableView(this);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table);

    // --- Przyciski ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj plon", this);
    editBtn = new QPushButton("Edytuj plon", this);
    deleteBtn = new QPushButton("Usuń plon", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &PlonyDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &PlonyDashboardWindow::dodajPlonDialog);
    connect(editBtn, &QPushButton::clicked, this, &PlonyDashboardWindow::edytujPlonDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &PlonyDashboardWindow::usunPlonDialog);

    loadData();
}

void PlonyDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();
    QString query = QString(
        "SELECT id_plonu, nazwa, magazyn, ilosc, gatunek_zwierzecia, "
        "rasa_zwierzecia, nazwa_uprawy, data_pozyskania "
        "FROM view_plony_info WHERE id_rolnika = %1").arg(rolnikId);

    if (!filterEdit->text().isEmpty()) {
        query += QString(" AND nazwa ILIKE '%%1%'").arg(filterEdit->text());
    }
    if (onlyRolneCheck->isChecked()) query += " AND nazwa_uprawy IS NOT NULL";
    if (onlyZwierzeCheck->isChecked()) query += " AND gatunek_zwierzecia IS NOT NULL";

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int PlonyDashboardWindow::getMagazynIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_magazynu FROM Magazyn WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) return q.value(0).toInt();
    return -1;
}

void PlonyDashboardWindow::dodajPlonDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj plon");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, iloscEdit, magazynEdit, uprawaIdEdit, zwierzeIdEdit, dataEdit;
    form.addRow("Nazwa plonu:", &nazwaEdit);
    form.addRow("Ilość (kg):", &iloscEdit);
    form.addRow("Magazyn:", &magazynEdit);
    form.addRow("ID uprawy (opcjonalnie):", &uprawaIdEdit);
    form.addRow("ID zwierzęcia (opcjonalnie):", &zwierzeIdEdit);
    form.addRow("Data pozyskania (YYYY-MM-DD):", &dataEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int magazynId = getMagazynIdByName(magazynEdit.text(), rolnikId);
        if (magazynId == -1) { QMessageBox::warning(this, "Błąd", "Nie znaleziono magazynu."); return; }

        QSqlQuery q(db);
        q.prepare("SELECT dodaj_plon(:rolnikId, :magazynId, :nazwa, :ilosc, :uprawaId, :zwierzeId, :data)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":magazynId", magazynId);
        q.bindValue(":nazwa", nazwaEdit.text());
        q.bindValue(":ilosc", iloscEdit.text().toDouble());
        q.bindValue(":uprawaId", uprawaIdEdit.text().isEmpty() ? QVariant() : uprawaIdEdit.text().toInt());
        q.bindValue(":zwierzeId", zwierzeIdEdit.text().isEmpty() ? QVariant() : zwierzeIdEdit.text().toInt());
        q.bindValue(":data", dataEdit.text().isEmpty() ? QVariant() : dataEdit.text());

        //if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", "dodano rekord plonu"); loadData(); }
        dlg.accept();
        });

    dlg.exec();
}

void PlonyDashboardWindow::edytujPlonDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj plon");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, iloscEdit, magazynEdit, dataEdit;
    form.addRow("ID plonu:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Nowa ilość (kg):", &iloscEdit);
    form.addRow("Nowy magazyn:", &magazynEdit);
    form.addRow("Nowa data pozyskania:", &dataEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int magazynId = magazynEdit.text().isEmpty() ? -1 : getMagazynIdByName(magazynEdit.text(), rolnikId);

        QSqlQuery q(db);
        q.prepare("SELECT edytuj_plon(:rolnikId, :id, :magazynId, :nazwa, :ilosc, :data)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":magazynId", magazynId == -1 ? QVariant() : magazynId);
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":ilosc", iloscEdit.text().isEmpty() ? QVariant() : iloscEdit.text().toDouble());
        q.bindValue(":data", dataEdit.text().isEmpty() ? QVariant() : dataEdit.text());

        if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", "zedytowano rekord plonu"); loadData(); }
        dlg.accept();
        });

    dlg.exec();
}

void PlonyDashboardWindow::usunPlonDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń plon");
    QFormLayout form(&dlg);

    QLineEdit idEdit;
    form.addRow("ID plonu:", &idEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idPlonu = idEdit.text().toInt();
        if (idPlonu <= 0) { QMessageBox::warning(this, "Błąd", "Podaj poprawne ID plonu."); return; }

        auto reply = QMessageBox::warning(
            this, "Uwaga",
            "Usunięcie plonu spowoduje utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::No) return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_plon(:rolnikId, :id)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idPlonu);

        if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", "usuniento rekord plonu"); loadData(); }
        dlg.accept();
        });

    dlg.exec();
}
