#include "Surowce_Dashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug> 
#include <QSqlError>
#include "../Klasy_logiczne/Session.h"

SurowceDashboardWindow::SurowceDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Surowców");
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
    filterEdit->setPlaceholderText("Szukaj po nazwie lub kategorii...");
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
    addBtn = new QPushButton("Dodaj surowiec", this);
    editBtn = new QPushButton("Edytuj surowiec", this);
    deleteBtn = new QPushButton("Usuń surowiec", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &SurowceDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &SurowceDashboardWindow::dodajSurowiecDialog);
    connect(editBtn, &QPushButton::clicked, this, &SurowceDashboardWindow::edytujSurowiecDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &SurowceDashboardWindow::usunSurowiecDialog);

    loadData();
}

void SurowceDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();
    QString query = QString("SELECT id_surowca, nazwa, kategoria, ilosc, magazyn "
        "FROM view_surowce_info WHERE id_rolnika = %1").arg(rolnikId);

    if (!filterEdit->text().isEmpty()) {
        query += QString(" AND (nazwa ILIKE '%%1%' OR kategoria ILIKE '%%1%')").arg(filterEdit->text());
    }

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int SurowceDashboardWindow::getMagazynIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_magazynu FROM Magazyn WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

void SurowceDashboardWindow::dodajSurowiecDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj surowiec");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, kategoriaEdit, iloscEdit, magazynEdit;
    form.addRow("Nazwa:", &nazwaEdit);
    form.addRow("Kategoria:", &kategoriaEdit);
    form.addRow("Ilość:", &iloscEdit);
    form.addRow("Magazyn:", &magazynEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int magazynId = getMagazynIdByName(magazynEdit.text(), rolnikId);
        if (magazynId == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono magazynu.");
            return;
        }

        QSqlQuery q(db);
        q.prepare("SELECT dodaj_surowiec(:rolnikId, :magazynId, :nazwa, :kategoria, :ilosc)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":magazynId", magazynId);
        q.bindValue(":nazwa", nazwaEdit.text());
        q.bindValue(":kategoria", kategoriaEdit.text());
        q.bindValue(":ilosc", iloscEdit.text().isEmpty() ? QVariant() : iloscEdit.text().toDouble());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "dodano rekord surowca");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void SurowceDashboardWindow::edytujSurowiecDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj surowiec");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, kategoriaEdit, iloscEdit, magazynEdit;
    form.addRow("ID surowca:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Nowa kategoria:", &kategoriaEdit);
    form.addRow("Nowa ilość:", &iloscEdit);
    form.addRow("Nowy magazyn:", &magazynEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int magazynId = magazynEdit.text().isEmpty() ? -1 : getMagazynIdByName(magazynEdit.text(), rolnikId);

        QSqlQuery q(db);
        q.prepare("SELECT edytuj_surowiec(:rolnikId, :id, :nazwa, :kategoria, :ilosc, :magazynId)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":kategoria", kategoriaEdit.text().isEmpty() ? QVariant() : kategoriaEdit.text());
        q.bindValue(":ilosc", iloscEdit.text().isEmpty() ? QVariant() : iloscEdit.text().toDouble());
        q.bindValue(":magazynId", magazynId == -1 ? QVariant() : magazynId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "zedytowano rekord surowca");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void SurowceDashboardWindow::usunSurowiecDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń surowiec");
    QFormLayout form(&dlg);

    QLineEdit idEdit;
    form.addRow("ID surowca:", &idEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idSurowca = idEdit.text().toInt();

        if (idSurowca <= 0) {
            QMessageBox::warning(this, "Błąd", "Podaj poprawne ID surowca.");
            return;
        }

        auto reply = QMessageBox::warning(
            this, "Uwaga",
            "Usunięcie surowca spowoduje utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::No) return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_surowiec(:rolnikId, :id)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idSurowca);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "usuniento rekord surowca");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}
