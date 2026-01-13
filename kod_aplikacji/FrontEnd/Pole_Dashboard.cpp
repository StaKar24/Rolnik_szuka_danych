#include "Pole_Dashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialog>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QDebug> 
#include <QSqlError>
#include "../Klasy_logiczne/Session.h"

PoleDashboardWindow::PoleDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Pól");
    resize(700, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    table = new QTableView(this);
    mainLayout->addWidget(table);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Dodaj pole", this);
    editBtn = new QPushButton("Edytuj pole", this);
    deleteBtn = new QPushButton("Usuń pole", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, this, &PoleDashboardWindow::dodajPoleDialog);
    connect(editBtn, &QPushButton::clicked, this, &PoleDashboardWindow::edytujPoleDialog);
    connect(deleteBtn, &QPushButton::clicked, this, &PoleDashboardWindow::usunPoleDialog);

    loadData();
}

void PoleDashboardWindow::loadData() {
    int rolnikId = Session::getInstance().getUserId();
    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString query = QString("SELECT id_pola, nazwa, powierzchnia, liczba_aktywnych_upraw "
        "FROM view_pole_dashboard WHERE id_rolnika = %1").arg(rolnikId);
    model->setQuery(query, db);
    table->setModel(model);
}

int PoleDashboardWindow::getPoleIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_pola FROM pole WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) return q.value(0).toInt();
    return -1;
}

void PoleDashboardWindow::dodajPoleDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Dodaj pole");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit, powEdit;
    form.addRow("Nazwa:", &nazwaEdit);
    form.addRow("Powierzchnia:", &powEdit);

    QPushButton okBtn("Dodaj");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT rolnik_dodaj_pole(:rolnikId, :nazwa, :pow)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":nazwa", nazwaEdit.text());
        q.bindValue(":pow", powEdit.text().toDouble());

        if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", q.value(0).toString()); loadData(); }

        dlg.accept();
        });

    dlg.exec();
}

void PoleDashboardWindow::edytujPoleDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Edytuj pole");
    QFormLayout form(&dlg);

    QLineEdit idEdit, nazwaEdit, powEdit;
    form.addRow("ID pola:", &idEdit);
    form.addRow("Nowa nazwa:", &nazwaEdit);
    form.addRow("Nowa powierzchnia:", &powEdit);

    QPushButton okBtn("Zapisz");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        QSqlQuery q(db);
        q.prepare("SELECT rolnik_edytuj_pole(:rolnikId, :id, :nazwa, :pow)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idEdit.text().toInt());
        q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
        q.bindValue(":pow", powEdit.text().isEmpty() ? QVariant() : powEdit.text().toDouble());

        if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", q.value(0).toString()); loadData(); }

        dlg.accept();
        });

    dlg.exec();
}

void PoleDashboardWindow::usunPoleDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Usuń pole");
    QFormLayout form(&dlg);

    QLineEdit nazwaEdit;
    form.addRow("Nazwa pola:", &nazwaEdit);

    QPushButton okBtn("Usuń");
    form.addRow(&okBtn);

    connect(&okBtn, &QPushButton::clicked, [&]() {
        int rolnikId = Session::getInstance().getUserId();
        int idPola = getPoleIdByName(nazwaEdit.text(), rolnikId);
        if (idPola == -1) { QMessageBox::warning(this, "Błąd", "Nie znaleziono pola o podanej nazwie."); return; }

        auto reply = QMessageBox::warning(this, "Uwaga",
            "Usunięcie pola może spowodować utratę powiązanych danych i informacji w innych tabelach. Czy na pewno chcesz kontynuować?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;

        QSqlQuery q(db);
        q.prepare("SELECT rolnik_usun_pole(:rolnikId, :id)");
        q.bindValue(":rolnikId", rolnikId);
        q.bindValue(":id", idPola);

        if (!q.exec()) { QMessageBox::critical(this, "Błąd SQL", q.lastError().text()); return; }
        if (q.next()) { QMessageBox::information(this, "Info", q.value(0).toString()); loadData(); }

        dlg.accept();
        });

    dlg.exec();
}
