#include "Zwierze_Dashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug> 
#include <QSqlError>
#include "../Klasy_logiczne/Session.h"

ZwierzetaDashboardWindow::ZwierzetaDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Zwierząt");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Filtry ---
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Szukaj po gatunku lub rasie...");
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
    addBtn = new QPushButton("Dodaj zwierzę", this);
    editBtn = new QPushButton("Edytuj zwierzę", this);
    deleteBtn = new QPushButton("Usuń zwierzę", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(filterBtn, &QPushButton::clicked, this, &ZwierzetaDashboardWindow::loadData);
    connect(addBtn, &QPushButton::clicked, this, &ZwierzetaDashboardWindow::dodajZwierzeDialog);
    connect(editBtn, &QPushButton::clicked, this, &ZwierzetaDashboardWindow::edytujZwierzeDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &ZwierzetaDashboardWindow::usunZwierzeDialog);

    loadData();
}

void ZwierzetaDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();
    QString query = QString("SELECT id_zwierzecia, gatunek, rasa, ilosc, budynek "
        "FROM view_zwierzeta_info WHERE id_rolnika = %1").arg(rolnikId);

    if (!filterEdit->text().isEmpty()) {
        query += QString(" AND (gatunek ILIKE '%%1%' OR rasa ILIKE '%%1%')").arg(filterEdit->text());
    }

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
}

int ZwierzetaDashboardWindow::getBudynekIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_budynku FROM Budynek WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

int ZwierzetaDashboardWindow::getZwierzeIdByGatunekRasa(const QString& gatunek, const QString& rasa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_zwierzecia FROM Zwierze WHERE gatunek = :gatunek AND rasa = :rasa AND id_rolnika = :rolnikId");
    q.bindValue(":gatunek", gatunek);
    q.bindValue(":rasa", rasa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

void ZwierzetaDashboardWindow::dodajZwierzeDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj zwierzę");
    QFormLayout form(&dlg);

    QLineEdit gatunekEdit, rasaEdit, iloscEdit, budynekEdit;
    form.addRow("Gatunek:", &gatunekEdit);
    form.addRow("Rasa:", &rasaEdit);
    form.addRow("Ilość:", &iloscEdit);
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
        q.prepare("SELECT dodaj_zwierze(:rolnikId, :budynekId, :gatunek, :rasa, :ilosc)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":budynekId", budynekId);
        q.bindValue(":gatunek", gatunekEdit.text());
        q.bindValue(":rasa", rasaEdit.text());
        q.bindValue(":ilosc", iloscEdit.text().toInt());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "dodano rekord zwierząt");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void ZwierzetaDashboardWindow::edytujZwierzeDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj zwierzę");
    QFormLayout form(&dlg);

    QLineEdit idEdit, budynekEdit, iloscEdit;
    form.addRow("ID zwierzęcia:", &idEdit);
    form.addRow("Nowy budynek:", &budynekEdit);
    form.addRow("Nowa ilość:", &iloscEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int budynekId = budynekEdit.text().isEmpty() ? -1 : getBudynekIdByName(budynekEdit.text(), rolnikId);

        QSqlQuery q(db);
        q.prepare("SELECT edytuj_zwierze(:id, :budynekId, :ilosc)");
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":budynekId", budynekId == -1 ? QVariant() : budynekId);
        q.bindValue(":ilosc", iloscEdit.text().isEmpty() ? QVariant() : iloscEdit.text().toInt());

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "poprawna edycja rekordu");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}

void ZwierzetaDashboardWindow::usunZwierzeDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń zwierzę");
    QFormLayout form(&dlg);

    QLineEdit gatunekEdit, rasaEdit;
    form.addRow("Gatunek:", &gatunekEdit);
    form.addRow("Rasa:", &rasaEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idZwierzecia = getZwierzeIdByGatunekRasa(gatunekEdit.text(), rasaEdit.text(), rolnikId);

        auto reply = QMessageBox::warning(this, "Uwaga",
            "Usunięcie kategorii zwierząt może spowodować utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;

        QSqlQuery q(db);
        q.prepare("SELECT usun_zwierze(:id)");
        q.bindValue(":id", idZwierzecia);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "Usuniento zwierzę");
            loadData();
        }
        dlg.accept();
        });

    dlg.exec();
}
