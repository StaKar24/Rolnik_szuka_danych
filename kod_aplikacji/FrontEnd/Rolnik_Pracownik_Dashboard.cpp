#include <QWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <libpq-fe.h>

#include <QCoreApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QFormLayout>
#include <QComboBox>
#include <QVariant>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlTableModel>
#include <QTableView>
#include <QCheckBox>
#include <QSpinBox>

#include "../Klasy_logiczne/AuthService.h"
#include "../Klasy_logiczne/Session.h"
#include <QTimer>




class PracownikDetailsWindow : public QWidget {
    Q_OBJECT

    QTableView* aktywne;
    QTableView* wykonane;
    QSqlDatabase db;
    int pracownikId;

public:
    PracownikDetailsWindow(QSqlDatabase database, int id, QWidget* parent = nullptr)
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

        loadData();
    }

private:
    void loadData() {
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
};






class PracownicyDashboard : public QWidget {
    Q_OBJECT

        QTableView* table;
    QLineEdit* nazwiskoFilter;
    QSpinBox* minWykonane;
    QSpinBox* minAktywne;
    QPushButton* filterBtn;
    QPushButton* detailsBtn;

    QSqlDatabase db;

public:
    PracownicyDashboard(QSqlDatabase database, QWidget* parent = nullptr)
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

private slots:
    void loadData() {
        int rolnikId = Session::getInstance().getUserId();

        QString query =
            "SELECT id_pracownika, imie, nazwisko, stanowisko, telefon, data_zatrudnienia, "
            "zadania_aktywne, zadania_wykonane "
            "FROM view_dashboard_pracownicy "
            "WHERE id_rolnika = :rolnikId ";

        if (!nazwiskoFilter->text().isEmpty()) {
            query += " AND nazwisko ILIKE '%" + nazwiskoFilter->text() + "%' ";
        }

        query += QString(" HAVING zadania_wykonane >= %1 AND zadania_aktywne >= %2")
            .arg(minWykonane->value())
            .arg(minAktywne->value());

        QSqlQueryModel* model = new QSqlQueryModel(this);
       

        model->setQuery(query, db);
        table->setModel(model);
    }

    void openDetails() {
        QModelIndex idx = table->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::warning(this, "Błąd", "Wybierz pracownika.");
            return;
        }

        int idPracownika = table->model()->index(idx.row(), 0).data().toInt();

        PracownikDetailsWindow* win = new PracownikDetailsWindow(db, idPracownika, this);
        win->show();
    }
};


#include "moc_PracownicyDashboard.cpp"
#include "moc_PracownikDetailsWindow.cpp"
