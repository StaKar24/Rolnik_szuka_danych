#include "PracownicyDashboard.h"
#include "PracownikDetailsWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "../Klasy_logiczne/Session.h"
#include <iostream>

PracownicyDashboard::PracownicyDashboard(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Pracowników");
    resize(900, 600);

    QVBoxLayout* main = new QVBoxLayout(this);

    // --- FILTRY ---
    QHBoxLayout* filters = new QHBoxLayout();

    nazwiskoFilter = new QLineEdit();
    nazwiskoFilter->setPlaceholderText("Filtr nazwiska...");

    minWykonane = new QSpinBox();
    minWykonane->setRange(0, 999);
    minWykonane->setPrefix("Min wykonane: ");

    minAktywne = new QSpinBox();
    minAktywne->setRange(0, 999);
    minAktywne->setPrefix("Min aktywne: ");

    filterBtn = new QPushButton("Filtruj");

    filters->addWidget(nazwiskoFilter);
    filters->addWidget(minWykonane);
    filters->addWidget(minAktywne);
    filters->addWidget(filterBtn);

    main->addLayout(filters);

    // --- TABELA ---
    table = new QTableView();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(true);
    main->addWidget(table);

    // --- PRZYCISKI ---
    detailsBtn = new QPushButton("Pokaż szczegóły pracownika");
    main->addWidget(detailsBtn);

    connect(filterBtn, &QPushButton::clicked, this, &PracownicyDashboard::loadData);
    connect(detailsBtn, &QPushButton::clicked, this, &PracownicyDashboard::openDetails);

    loadData();
}

void PracownicyDashboard::loadData() {
    int rolnikId = Session::getInstance().getUserId();

    QString query = QString(
        "SELECT id_pracownika, imie, nazwisko, stanowisko, telefon, data_zatrudnienia, "
        "zadania_aktywne, zadania_wykonane "
        "FROM view_dashboard_pracownicy "
        "WHERE id_rolnika = %1").arg(rolnikId);

    if (!nazwiskoFilter->text().isEmpty()) {
        query += " AND nazwisko ILIKE '%" + nazwiskoFilter->text() + "%' ";
    }

    query += QString(" AND zadania_wykonane >= %1 AND zadania_aktywne >= %2")
        .arg(minWykonane->value())
        .arg(minAktywne->value());

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(query, db);
    table->setModel(model);
    // error handeling


}

void PracownicyDashboard::openDetails() {
    QModelIndex idx = table->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Błąd", "Wybierz pracownika.");
        return;
    }

    int idPracownika = table->model()->index(idx.row(), 0).data().toInt();

    auto* win = new PracownikDetailsWindow(db, idPracownika, this);
    win->show();
}
