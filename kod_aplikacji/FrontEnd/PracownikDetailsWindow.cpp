#include "PracownikDetailsWindow.h"
#include <QSqlQueryModel>

PracownikDetailsWindow::PracownikDetailsWindow(QSqlDatabase database, int id, QWidget* parent)
    : QWidget(parent), db(database), pracownikId(id)
{
    setWindowTitle("Szczegóły pracownika");
    resize(600, 400);

    QVBoxLayout* main = new QVBoxLayout(this);

    aktywne = new QTableView();
    wykonane = new QTableView();

    main->addWidget(new QLabel("Zadania aktywne:"));
    main->addWidget(aktywne);

    main->addWidget(new QLabel("Zadania wykonane:"));
    main->addWidget(wykonane);

    QPushButton* closeBtn = new QPushButton("Zamknij"); 
    main->addWidget(closeBtn); 
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    loadData();
}

void PracownikDetailsWindow::loadData() {
    QSqlQueryModel* m1 = new QSqlQueryModel(this);
    m1->setQuery(
        QString("SELECT nazwa, opis, data_start, data_koniec "
            "FROM Zadanie WHERE id_pracownika = %1 "
            "AND status IN ('planowane','w_trakcie')")
        .arg(pracownikId),
        db);
    aktywne->setModel(m1);

    QSqlQueryModel* m2 = new QSqlQueryModel(this);
    m2->setQuery(
        QString("SELECT nazwa, opis, data_start, data_koniec "
            "FROM Zadanie WHERE id_pracownika = %1 "
            "AND status = 'wykonane'")
        .arg(pracownikId),
        db);
    wykonane->setModel(m2);
}
