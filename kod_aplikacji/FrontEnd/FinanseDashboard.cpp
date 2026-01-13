#include "FinanseDashboard.h"
#include "TransakcjaDetailsWindow.h"
#include "../Klasy_logiczne/Session.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QSqlQueryModel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QLabel>
#include <QFormLayout>
#include <QDateTime>

#include <QVector>
#include <QVariant>


#include "../Qcustomplot/qcustomplot.h"


struct SurowiecKupno {
    QString nazwa;
    double ilosc;
    double cenaJednostkowa;
};

struct PlonSprzedaz {
    int idPlonu;
    double ilosc;
    double cenaJednostkowa;
    QString nazwaPlonu;
};


//Funkcje pomocnicze do konwertowania 
QVariant konwertujSurowceKupno(const QVector<SurowiecKupno>& lista) {
    QString arr = "{";

    for (int i = 0; i < lista.size(); ++i) {
        arr += QString("\"(%1,%2,%3)\"")
            .arg(lista[i].nazwa)
            .arg(lista[i].ilosc)
            .arg(lista[i].cenaJednostkowa);

        if (i < lista.size() - 1)
            arr += ",";
    }

    arr += "}";
    return arr;
}

QVariant konwertujPlonySprzedaz(const QVector<PlonSprzedaz>& lista) {
    QString arr = "{";

    for (int i = 0; i < lista.size(); ++i) {
        arr += QString("\"(%1,%2,%3)\"")
            .arg(lista[i].idPlonu)
            .arg(lista[i].ilosc)
            .arg(lista[i].cenaJednostkowa);

        if (i < lista.size() - 1)
            arr += ",";
    }

    arr += "}";
    return arr;
}






int getMagazynIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_magazynu FROM Magazyn "
        "WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}

int getSurowiecIdByName(QSqlDatabase db, const QString& nazwa, int magazynId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_surowca FROM Surowiec "
        "WHERE nazwa = :nazwa AND id_magazynu = :idMagazynu");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":idMagazynu", magazynId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}

int getPlonIdByName(QSqlDatabase db, const QString& nazwa, int magazynId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_plonu FROM Plon "
        "WHERE nazwa = :nazwa AND id_magazynu = :idMagazynu");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":idMagazynu", magazynId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}



// --- główne okno finansów ---
FinanseDashboard::FinanseDashboard(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Finanse");
    resize(1000, 700);

    QVBoxLayout* main = new QVBoxLayout(this);

    tabs = new QTabWidget(this);
    main->addWidget(tabs);

    tabTransakcje = new QWidget();
    tabKupno = new QWidget();
    tabSprzedaz = new QWidget();

    tabs->addTab(tabTransakcje, "Transakcje");
    tabs->addTab(tabKupno, "Kup surowce");
    tabs->addTab(tabSprzedaz, "Sprzedaj plony");
    tabs->addTab(new QWidget(), "Wykres");

    initTransakcjeTab();
    initKupnoTab();
    initSprzedazTab();

    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (tabs->tabText(index) == "Wykres" && !wykresInitialized) {
            QWidget* wykresTab = tabs->widget(index);
            initWykresTab(wykresTab);
            wykresInitialized = true;
        }
        });



}

// --- Zakładka: Transakcje ---
void FinanseDashboard::initTransakcjeTab() {
    auto* layout = new QVBoxLayout(tabTransakcje);

    // filtry
    QHBoxLayout* filters = new QHBoxLayout();
    QComboBox* typCombo = new QComboBox();
    typCombo->addItem("Wszystkie", "");
    typCombo->addItem("Kupno", "kupno");
    typCombo->addItem("Sprzedaż", "sprzedaz");

    QLineEdit* opisFilter = new QLineEdit();
    opisFilter->setPlaceholderText("Filtr po opisie...");

    QPushButton* filterBtn = new QPushButton("Filtruj");

    filters->addWidget(new QLabel("Typ:"));
    filters->addWidget(typCombo);
    filters->addWidget(opisFilter);
    filters->addWidget(filterBtn);

    layout->addLayout(filters);

    // tabela
    QTableView* table = new QTableView();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(true);
    layout->addWidget(table);

    // przyciski
    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* detailsBtn = new QPushButton("Szczegóły transakcji");
    btns->addWidget(detailsBtn);
    layout->addLayout(btns);

    // model
    auto loadData = [this, table, typCombo, opisFilter]() {
        int rolnikId = Session::getInstance().getUserId();

        QString query =
            "SELECT id_transakcji, typ, data_transakcji, opis, kwota "
            "FROM Transakcja "
            "WHERE id_rolnika = " + QString::number(rolnikId);

        QString typVal = typCombo->currentData().toString();
        if (!typVal.isEmpty()) {
            query += " AND typ = '" + typVal + "'";
        }

        if (!opisFilter->text().isEmpty()) {
            query += " AND opis ILIKE '%" + opisFilter->text() + "%'";
        }

        query += " ORDER BY data_transakcji DESC";

        QSqlQueryModel* model = new QSqlQueryModel(table);
        model->setQuery(query, db);

        if (model->lastError().isValid()) {
            QMessageBox::critical(table, "SQL Error", model->lastError().text(), QMessageBox::Ok);
        }

        table->setModel(model);
        table->resizeColumnsToContents();
        };

    QObject::connect(filterBtn, &QPushButton::clicked, loadData);
    loadData();

    QObject::connect(detailsBtn, &QPushButton::clicked, [this, table]() {
        QModelIndex idx = table->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::warning(this, "Błąd", "Wybierz transakcję.", QMessageBox::Ok);
            return;
        }

        int idTransakcji = table->model()->index(idx.row(), 0).data().toInt();
        QString typ = table->model()->index(idx.row(), 1).data().toString();

        auto* win = new TransakcjaDetailsWindow(db, idTransakcji, typ, this);
        win->show();
        });
}



void FinanseDashboard::initSprzedazTab() {
    auto* layout = new QVBoxLayout(tabSprzedaz);

    QFormLayout* form = new QFormLayout();

    QLineEdit* magazynEdit = new QLineEdit();
    QLineEdit* opisEdit = new QLineEdit();

    form->addRow("Nazwa magazynu:", magazynEdit);
    form->addRow("Opis transakcji:", opisEdit);

    layout->addLayout(form);

    QPushButton* dodajPozycjeBtn = new QPushButton("Dodaj plony do sprzedaży");
    QPushButton* wykonajBtn = new QPushButton("Zapisz transakcję sprzedaży");
    layout->addWidget(dodajPozycjeBtn);
    layout->addWidget(wykonajBtn);

    // lista pozycji plonów
    auto* plonyLista = new QVector<PlonSprzedaz>();

    // --- Dodawanie pozycji ---
    QObject::connect(dodajPozycjeBtn, &QPushButton::clicked, [this, plonyLista]() {
        bool finished = false;

        while (!finished) {
            QDialog dlg(this);
            dlg.setWindowTitle("Dodaj plon do sprzedaży");
            QFormLayout form(&dlg);

            QLineEdit nazwaEdit, iloscEdit, cenaEdit;
            form.addRow("Nazwa plonu:", &nazwaEdit);
            form.addRow("Ilość:", &iloscEdit);
            form.addRow("Cena jednostkowa:", &cenaEdit);

            QPushButton addBtn("Dodaj kolejny");
            QPushButton endBtn("Zakończ");
            form.addRow(&addBtn);
            form.addRow(&endBtn);

            QObject::connect(&addBtn, &QPushButton::clicked, [&]() {
                if (nazwaEdit.text().isEmpty()) {
                    QMessageBox::warning(&dlg, "Błąd", "Nazwa plonu nie może być pusta.", QMessageBox::Ok);
                    return;
                }

                // UWAGA: plony mają ID, więc musimy znaleźć id_plonu
                int rolnikId = Session::getInstance().getUserId();
                int idMagazynu = -1; // nie znamy tutaj magazynu, ID pobierzemy później
                // plon musi być znaleziony po nazwie i magazynie — ID magazynu pobierzemy dopiero przy zapisie

                // tymczasowo zapisujemy nazwę, ID znajdziemy później
                PlonSprzedaz p;
                p.idPlonu = -1; // placeholder
                p.ilosc = iloscEdit.text().toDouble();
                p.cenaJednostkowa = cenaEdit.text().toDouble();

                // nazwa plonu przechowana tymczasowo w polu idPlonu jako -1
                // właściwe ID znajdziemy później
                p.idPlonu = -999; // znacznik do późniejszego uzupełnienia

                // zapisujemy nazwę w polu cenaJednostkowa? Nie — zrobimy osobną strukturę
                // więc dodajemy pomocniczą strukturę:
                p.nazwaPlonu = nazwaEdit.text();

                plonyLista->push_back(p);
                dlg.accept();
                });

            QObject::connect(&endBtn, &QPushButton::clicked, [&]() {
                if (!nazwaEdit.text().isEmpty()) {
                    PlonSprzedaz p;
                    p.idPlonu = -999;
                    p.ilosc = iloscEdit.text().toDouble();
                    p.cenaJednostkowa = cenaEdit.text().toDouble();
                    p.nazwaPlonu = nazwaEdit.text();
                    plonyLista->push_back(p);
                }
                finished = true;
                dlg.accept();
                });

            dlg.exec();
        }
        });

    // --- Zapis transakcji ---
    QObject::connect(wykonajBtn, &QPushButton::clicked, [this, magazynEdit, opisEdit, plonyLista]() {
        int rolnikId = Session::getInstance().getUserId();
        QString magazynNazwa = magazynEdit->text();

        int idMagazynu = getMagazynIdByName(db, magazynNazwa, rolnikId);
        if (idMagazynu == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono magazynu.", QMessageBox::Ok);
            return;
        }

        if (plonyLista->isEmpty()) {
            QMessageBox::warning(this, "Błąd", "Brak pozycji plonów do sprzedaży.", QMessageBox::Ok);
            return;
        }

        // --- Uzupełniamy ID plonów po nazwie ---
        for (auto& p : *plonyLista) {
            int idPlonu = getPlonIdByName(db, p.nazwaPlonu, idMagazynu);
            if (idPlonu == -1) {
                QMessageBox::warning(this, "Błąd", "Nie znaleziono plonu: " + p.nazwaPlonu, QMessageBox::Ok);
                return;
            }
            p.idPlonu = idPlonu;
        }

        // --- Wywołanie SQL ---
        QSqlQuery q(db);
        q.prepare("SELECT sprzedaj_plony(:idRolnika, :idMagazynu, :opis, :plony)");
        q.bindValue(":idRolnika", rolnikId);
        q.bindValue(":idMagazynu", idMagazynu);
        q.bindValue(":opis", opisEdit->text());
        q.bindValue(":plony", konwertujPlonySprzedaz(*plonyLista));

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text(), QMessageBox::Ok);
            return;
        }

        QMessageBox::information(this, "Info", "Zapisano transakcję sprzedaży.", QMessageBox::Ok);
        plonyLista->clear();
        });
}


void FinanseDashboard::initKupnoTab() {
    auto* layout = new QVBoxLayout(tabKupno);

    QFormLayout* form = new QFormLayout();

    QLineEdit* magazynEdit = new QLineEdit();
    QLineEdit* opisEdit = new QLineEdit();

    form->addRow("Nazwa magazynu:", magazynEdit);
    form->addRow("Opis transakcji:", opisEdit);

    layout->addLayout(form);

    QPushButton* dodajPozycjeBtn = new QPushButton("Dodaj surowce do zakupu");
    QPushButton* wykonajBtn = new QPushButton("Zapisz transakcję kupna");
    layout->addWidget(dodajPozycjeBtn);
    layout->addWidget(wykonajBtn);

    // lista pozycji surowców
    auto* surowceLista = new QVector<SurowiecKupno>();

    // --- Dodawanie pozycji ---
    QObject::connect(dodajPozycjeBtn, &QPushButton::clicked, [this, surowceLista]() {
        bool finished = false;

        while (!finished) {
            QDialog dlg(this);
            dlg.setWindowTitle("Dodaj surowiec do zakupu");
            QFormLayout form(&dlg);

            QLineEdit nazwaEdit, iloscEdit, cenaEdit;
            form.addRow("Nazwa surowca:", &nazwaEdit);
            form.addRow("Ilość:", &iloscEdit);
            form.addRow("Cena jednostkowa:", &cenaEdit);

            QPushButton addBtn("Dodaj kolejny");
            QPushButton endBtn("Zakończ");
            form.addRow(&addBtn);
            form.addRow(&endBtn);

            QObject::connect(&addBtn, &QPushButton::clicked, [&]() {
                if (nazwaEdit.text().isEmpty()) {
                    QMessageBox::warning(&dlg, "Błąd", "Nazwa surowca nie może być pusta.", QMessageBox::Ok);
                    return;
                }

                SurowiecKupno s{
                    nazwaEdit.text(),
                    iloscEdit.text().toDouble(),
                    cenaEdit.text().toDouble()
                };

                surowceLista->push_back(s);
                dlg.accept();
                });

            QObject::connect(&endBtn, &QPushButton::clicked, [&]() {
                if (!nazwaEdit.text().isEmpty()) {
                    SurowiecKupno s{
                        nazwaEdit.text(),
                        iloscEdit.text().toDouble(),
                        cenaEdit.text().toDouble()
                    };
                    surowceLista->push_back(s);
                }
                finished = true;
                dlg.accept();
                });

            dlg.exec();
        }
        });

    // --- Zapis transakcji ---
    QObject::connect(wykonajBtn, &QPushButton::clicked, [this, magazynEdit, opisEdit, surowceLista]() {
        int rolnikId = Session::getInstance().getUserId();
        QString magazynNazwa = magazynEdit->text();

        int idMagazynu = getMagazynIdByName(db, magazynNazwa, rolnikId);
        if (idMagazynu == -1) {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono magazynu.", QMessageBox::Ok);
            return;
        }

        if (surowceLista->isEmpty()) {
            QMessageBox::warning(this, "Błąd", "Brak pozycji surowców do zakupu.", QMessageBox::Ok);
            return;
        }

        // --- Wywołanie SQL ---
        QSqlQuery q(db);
        q.prepare("SELECT kup_surowce(:idRolnika, :idMagazynu, :opis, :surowce)");
        q.bindValue(":idRolnika", rolnikId);
        q.bindValue(":idMagazynu", idMagazynu);
        q.bindValue(":opis", opisEdit->text());
        q.bindValue(":surowce", konwertujSurowceKupno(*surowceLista));

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text(), QMessageBox::Ok);
            return;
        }

        QMessageBox::information(this, "Info", "Zapisano transakcję kupna.", QMessageBox::Ok);
        surowceLista->clear();
        });
}




void FinanseDashboard::initWykresTab(QWidget* tab) {
    if (!tab) return;
    auto* layout = new QVBoxLayout(tab);

    // --- WYKRES (QCustomPlot) ---
    QCustomPlot* plot = new QCustomPlot(tab);
    layout->addWidget(plot);

    // Oś czasu
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeFormat("yyyy-MM");
    plot->xAxis->setTicker(dateTimeTicker);
    plot->xAxis->setLabel("Miesiąc");
    plot->yAxis->setLabel("Zysk netto (PLN)");

    // --- ŁADOWANIE DANYCH ALL TIME ---
    plot->clearGraphs();
    QCPGraph* graph = plot->addGraph();

    graph->setPen(QPen(Qt::blue));
    graph->setBrush(QBrush(QColor(0, 0, 255, 20)));

    QVector<double> ticks, values;

    QSqlQuery q(db);
    q.prepare(
        "SELECT DATE_TRUNC('month', data_transakcji), "
        "SUM(CASE WHEN typ='sprzedaz' THEN kwota ELSE 0 END) - "
        "SUM(CASE WHEN typ='kupno' THEN kwota ELSE 0 END) "
        "FROM Transakcja "
        "WHERE id_rolnika = :r "
        "GROUP BY 1 ORDER BY 1"
    );
    q.bindValue(":r", Session::getInstance().getUserId());
    q.exec();

    while (q.next()) {
        ticks.append(q.value(0).toDateTime().toSecsSinceEpoch());
        values.append(q.value(1).toDouble());
    }

    graph->setData(ticks, values);

    if (!ticks.isEmpty()) {
        plot->rescaleAxes();
        plot->replot();
    }
}
