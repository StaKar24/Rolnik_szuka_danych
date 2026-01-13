#include "TransakcjaDetailsWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlError>

TransakcjaDetailsWindow::TransakcjaDetailsWindow(QSqlDatabase database, int id, const QString& t, QWidget* parent)
    : QWidget(parent), db(database), idTransakcji(id), typ(t)
{
    setWindowTitle("Szczegóły transakcji");
    resize(600, 400);

    QVBoxLayout* main = new QVBoxLayout(this);

    tabela = new QTableView();
    main->addWidget(new QLabel("Pozycje transakcji:"));
    main->addWidget(tabela);

    QPushButton* closeBtn = new QPushButton("Zamknij");
    main->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    loadData();
}

void TransakcjaDetailsWindow::loadData() {
    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString query;

    if (typ == "kupno") {
        query =
            "SELECT s.nazwa, ts.ilosc, ts.cena_jednostkowa, "
            "(ts.ilosc * ts.cena_jednostkowa) AS wartosc "
            "FROM Transakcja_Surowiec ts "
            "JOIN Surowiec s ON s.id_surowca = ts.id_surowca "
            "WHERE ts.id_transakcji = " + QString::number(idTransakcji);
    }
    else {
        query =
            "SELECT p.nazwa, tp.ilosc, tp.cena_jednostkowa, "
            "(tp.ilosc * tp.cena_jednostkowa) AS wartosc "
            "FROM Transakcja_Plon tp "
            "JOIN Plon p ON p.id_plonu = tp.id_plonu "
            "WHERE tp.id_transakcji = " + QString::number(idTransakcji);
    }

    model->setQuery(query, db);

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "SQL Error", model->lastError().text(), QMessageBox::Ok);
    }

    tabela->setModel(model);
    tabela->resizeColumnsToContents();
}
