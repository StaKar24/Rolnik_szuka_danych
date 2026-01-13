#include "Zadanie_Dashboard.h"
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
#include <QStackedWidget>
#include <QCalendarWidget>
#include <QListWidget>

#include "../Klasy_logiczne/AuthService.h"
#include "../Klasy_logiczne/Session.h"
#include <QTimer>




int ZadaniaDashboardWindow::getMagazynIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_magazynu FROM Magazyn WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}
int ZadaniaDashboardWindow::getSurowiecIdByName(const QString& nazwa, int magazynId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_surowca FROM Surowiec WHERE nazwa = :nazwa AND id_magazynu = :magazynId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":magazynId", magazynId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;

}
int ZadaniaDashboardWindow::getPoleIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_pola FROM Pole WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}
int ZadaniaDashboardWindow::getBudynekIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_budynku FROM Budynek WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}
int ZadaniaDashboardWindow::getMaszynaIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_maszyny FROM Maszyna WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}
int ZadaniaDashboardWindow::getPracownikIdByFullName(QSqlDatabase db, const QString& fullName, int rolnikId) {
    QStringList parts = fullName.split(" ", Qt::SkipEmptyParts);
    if (parts.size() < 2) return -1;

    QString imie = parts[0];
    QString nazwisko = parts[1];

    QSqlQuery q(db);
    q.prepare("SELECT id_pracownika FROM Pracownik "
        "WHERE imie = :imie AND nazwisko = :nazwisko AND id_rolnika = :rolnikId");
    q.bindValue(":imie", imie);
    q.bindValue(":nazwisko", nazwisko);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();
    return -1;
}
int ZadaniaDashboardWindow::getZwierzeIdBySpecies(QSqlDatabase db, const QString& text, int rolnikId) {
    QStringList parts = text.split(QChar(' '), Qt::SkipEmptyParts);

    if (parts.size() == 1 && parts[0].contains(':')) {
        parts = parts[0].split(':', Qt::SkipEmptyParts);
    }

    if (parts.size() < 2)
        return -1;

    QString gatunek = parts[0];
    QString rasa = parts[1];

    QSqlQuery q(db);
    q.prepare("SELECT id_zwierzecia FROM Zwierze "
        "WHERE gatunek = :gatunek AND rasa = :rasa AND id_rolnika = :rolnikId");
    q.bindValue(":gatunek", gatunek);
    q.bindValue(":rasa", rasa);
    q.bindValue(":rolnikId", rolnikId);

    if (q.exec() && q.next())
        return q.value(0).toInt();

    return -1;
}
//int ZadaniaDashboardWindow::getUprawaIdByName(QSqlDatabase db, const QString& nazwa, int poleId) {
//    QSqlQuery q(db);
//    q.prepare(
//        "SELECT id_uprawy FROM Uprawa "
//        "WHERE id_pola = :poleId "
//        "  AND LOWER(nazwa) = LOWER(:nazwa) "
//        "  AND (data_zbioru IS NULL OR data_zbioru > CURRENT_DATE)"
//    );
//    q.bindValue(":poleId", poleId);
//    q.bindValue(":nazwa", nazwa);
//
//    if (q.exec() && q.next())
//        return q.value(0).toInt();
//
//    return -1;
//}




ZadaniaDashboardWindow::ZadaniaDashboardWindow(QSqlDatabase database, QWidget* parent)
        : QWidget(parent), db(database)
    {
        setWindowTitle("Dashboard Zadań");
        resize(1000, 700);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // --- Switch widoków ---
        stackedViews = new QStackedWidget(this);

        // --- Widok tabeli ---
        QWidget* tablePage = new QWidget(this);
        QVBoxLayout* tableLayout = new QVBoxLayout(tablePage);

        QHBoxLayout* filterLayout = new QHBoxLayout();
        filterNameEdit = new QLineEdit(this);
        filterNameEdit->setPlaceholderText("Szukaj po nazwie zadania...");
        filterStatusCombo = new QComboBox(this);
        filterStatusCombo->addItems({ "", "planowane", "w_trakcie", "anulowane", "wykonane"});
        filterUprawyCheck = new QCheckBox("Tylko z uprawami", this);
        filterZwierzeCheck = new QCheckBox("Tylko ze zwierzętami", this);
        filterBtn = new QPushButton("Filtruj", this);

        filterLayout->addWidget(filterNameEdit);
        filterLayout->addWidget(filterStatusCombo);
        filterLayout->addWidget(filterUprawyCheck);
        filterLayout->addWidget(filterZwierzeCheck);
        filterLayout->addWidget(filterBtn);

        tableLayout->addLayout(filterLayout);

        tableView = new QTableView(this);
        tableView->setSortingEnabled(true);
        tableLayout->addWidget(tableView);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        addBtn = new QPushButton("Dodaj zadanie", this);
        editBtn = new QPushButton("Edytuj zadanie", this);
        deleteBtn = new QPushButton("Usuń zadanie", this);
        executeBtn = new QPushButton("Wykonaj zadanie", this);
        changeStatusBtn = new QPushButton("Zmień status", this);

       

        switchViewBtn = new QPushButton("Przełącz na kalendarz", this);

        btnLayout->addWidget(addBtn);
        btnLayout->addWidget(editBtn);
        btnLayout->addWidget(deleteBtn);
        btnLayout->addWidget(changeStatusBtn);
        btnLayout->addWidget(executeBtn);
        btnLayout->addWidget(switchViewBtn);

        tableLayout->addLayout(btnLayout);

        stackedViews->addWidget(tablePage);

        // --- Widok kalendarza ---
        QWidget* calendarPage = new QWidget(this);
        QVBoxLayout* calLayout = new QVBoxLayout(calendarPage);

        calendarView = new QCalendarWidget(this);
        calendarTasksList = new QListWidget(this);

        calLayout->addWidget(calendarView);
        calLayout->addWidget(new QLabel("Zadania w wybranym dniu:", this));
        calLayout->addWidget(calendarTasksList);

        QPushButton* switchBackBtn = new QPushButton("Przełącz na tabelę", this);
        calLayout->addWidget(switchBackBtn);

        stackedViews->addWidget(calendarPage);

        mainLayout->addWidget(stackedViews);

        connect(switchViewBtn, &QPushButton::clicked, [&]() {
            stackedViews->setCurrentIndex(1); // kalendarz
            loadDataCalendar();
            });
        connect(switchBackBtn, &QPushButton::clicked, [&]() {
            stackedViews->setCurrentIndex(0); // tabela
            loadDataTable();
            });

        connect(calendarView, &QCalendarWidget::selectionChanged,
            this, &ZadaniaDashboardWindow::loadDataCalendar);
        connect(filterBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::loadDataTable);
        connect(addBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::dodajZadanieDialog);
        connect(editBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::edytujZadanieDialog);
        connect(deleteBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::usunZadanieDialog);
        connect(changeStatusBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::zmienStatusZadaniaDialog);
        connect(executeBtn, &QPushButton::clicked, this, &ZadaniaDashboardWindow::wykonajZadanieDialog);

        
    
        
        // Może do zrobienia: podwójne kliknięcie w tableView -> edycja/wykonanie
        loadDataTable();
    }

    void ZadaniaDashboardWindow::loadDataTable() {
        int rolnikId = Session::getInstance().getUserId();

        QString query = QString(
            "SELECT id_zadania, zadanie_nazwa, opis, status, data_start, data_koniec, "
            "pole_nazwa, uprawa_nazwa, pracownik_imie, pracownik_nazwisko, "
            "maszyna_nazwa, budynek_nazwa, zwierze_gatunek "
            "FROM view_zadania_szczegoly "
            "WHERE id_rolnika = %1"
        ).arg(rolnikId);

        // --- Filtry ---
        if (!filterNameEdit->text().isEmpty()) {
            query += QString(" AND zadanie_nazwa ILIKE '%%1%'").arg(filterNameEdit->text());
        }
        if (!filterStatusCombo->currentText().isEmpty()) {
            query += QString(" AND status = '%1'").arg(filterStatusCombo->currentText());
        }
        if (filterUprawyCheck->isChecked()) {
            query += " AND id_uprawy IS NOT NULL";
        }
        if (filterZwierzeCheck->isChecked()) {
            query += " AND id_zwierzecia IS NOT NULL";
        }

        query += " ORDER BY data_start ASC";

        QSqlQueryModel* model = new QSqlQueryModel(this);
        model->setQuery(query, db);
        tableView->setModel(model);
    }


    void ZadaniaDashboardWindow::loadDataCalendar() {
        int rolnikId = Session::getInstance().getUserId();
        QDate selectedDate = calendarView->selectedDate();

        calendarTasksList->clear();

        QSqlQuery q(db);
        q.prepare(
            "SELECT nazwa, status, imie, nazwisko "
            "FROM view_pracownik_zadania "
            "WHERE data_start = :data "
            "AND id_zadania IN (SELECT id_zadania FROM Zadanie WHERE id_rolnika = :rolnikId)"
        );
        q.bindValue(":data", selectedDate.toString("yyyy-MM-dd"));
        q.bindValue(":rolnikId", rolnikId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }

        while (q.next()) {
            QString nazwa = q.value("nazwa").toString();
            QString status = q.value("status").toString();
            QString imie = q.value("imie").toString();
            QString nazwisko = q.value("nazwisko").toString();

            QString osoba;
            if (imie.isEmpty() && nazwisko.isEmpty()) {
                QSqlQuery qq(db);
                qq.prepare("SELECT login FROM Uzytkownik WHERE id_uzytkownika = :id");
                qq.bindValue(":id", rolnikId);
                if (qq.exec() && qq.next()) {
                    osoba = qq.value(0).toString();
                }
                else {
                    osoba = "błąd ładowania loginu rolnika";
                }
            }
            else {
                osoba = imie + " " + nazwisko;
            }

            QString itemText = QString("%1 — %2 — %3").arg(nazwa, status, osoba);
            calendarTasksList->addItem(itemText);
        }
    }

     
    void ZadaniaDashboardWindow::dodajZadanieDialog() {
        QDialog dlg(this);
        dlg.setWindowTitle("Dodaj zadanie");
        QFormLayout form(&dlg);

        QLineEdit nazwaEdit, opisEdit, pracownikEdit, uprawaEdit, poleEdit, budynekEdit, maszynaEdit, zwierzeEdit, dataStartEdit;

        form.addRow("Nazwa:", &nazwaEdit);
        form.addRow("Opis:", &opisEdit);
        form.addRow("Pracownik (Imię Nazwisko):", &pracownikEdit);
        form.addRow("Uprawa (ID):", &uprawaEdit);
        form.addRow("Pole (nazwa):", &poleEdit);
        form.addRow("Budynek (nazwa):", &budynekEdit);
        form.addRow("Maszyna (nazwa):", &maszynaEdit);
        form.addRow("Zwierzę (gatunek rasa):", &zwierzeEdit);
        form.addRow("Data startu (YYYY-MM-DD):", &dataStartEdit);

        QPushButton okBtn("Dodaj");
        form.addRow(&okBtn);

        connect(&okBtn, &QPushButton::clicked, [&]() {
            int rolnikId = Session::getInstance().getUserId();

            int poleId = poleEdit.text().isEmpty() ? -1 : getPoleIdByName(db, poleEdit.text(), rolnikId);
            int budynekId = budynekEdit.text().isEmpty() ? -1 : getBudynekIdByName(db, budynekEdit.text(), rolnikId);
            int maszynaId = maszynaEdit.text().isEmpty() ? -1 : getMaszynaIdByName(db, maszynaEdit.text(), rolnikId);
            int pracownikId = pracownikEdit.text().isEmpty() ? -1 : getPracownikIdByFullName(db, pracownikEdit.text(), rolnikId);
            int zwierzeId = zwierzeEdit.text().isEmpty() ? -1 : getZwierzeIdBySpecies(db, zwierzeEdit.text(), rolnikId);
            
            //id nie nazwa
            int uprawaId = uprawaEdit.text().isEmpty() ? -1 : uprawaEdit.text().toInt();

            QSqlQuery q(db);
            q.prepare("SELECT utworz_zadanie(:nazwa, :opis, :rolnikId, :pracownikId, :uprawaId, :maszynaId, :budynekId, :zwierzeId, :poleId, :dataStart)");
            q.bindValue(":nazwa", nazwaEdit.text());
            q.bindValue(":opis", opisEdit.text());
            q.bindValue(":rolnikId", rolnikId);
            q.bindValue(":pracownikId", pracownikId == -1 ? QVariant() : pracownikId);
            q.bindValue(":uprawaId", uprawaId == -1 ? QVariant() : uprawaId);
            q.bindValue(":maszynaId", maszynaId == -1 ? QVariant() : maszynaId);
            q.bindValue(":budynekId", budynekId == -1 ? QVariant() : budynekId);
            q.bindValue(":zwierzeId", zwierzeId == -1 ? QVariant() : zwierzeId);
            q.bindValue(":poleId", poleId == -1 ? QVariant() : poleId);
            q.bindValue(":dataStart", dataStartEdit.text().isEmpty() ? QVariant() : dataStartEdit.text());

            if (!q.exec()) {
                QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
                return;
            }


            QMessageBox::information(this, "Info", "Dodano zadanie");
            loadDataTable();
            loadDataCalendar();
            dlg.accept();
            });

        dlg.exec();
    }





    void ZadaniaDashboardWindow::edytujZadanieDialog() {
        QDialog dlg(this);
        dlg.setWindowTitle("Edytuj zadanie");
        QFormLayout form(&dlg);

        QLineEdit idEdit, nazwaEdit, opisEdit, pracownikEdit, uprawaEdit, poleEdit, budynekEdit, maszynaEdit, zwierzeEdit, dataStartEdit;

        form.addRow("ID zadania:", &idEdit);
        form.addRow("Nowa nazwa:", &nazwaEdit);
        form.addRow("Nowy opis:", &opisEdit);
        form.addRow("Pracownik (Imię Nazwisko):", &pracownikEdit);
        form.addRow("Uprawa (Id):", &uprawaEdit);
        form.addRow("Pole (nazwa):", &poleEdit);
        form.addRow("Budynek (nazwa):", &budynekEdit);
        form.addRow("Maszyna (nazwa):", &maszynaEdit);
        form.addRow("Zwierzę (gatunek rasa):", &zwierzeEdit);
        form.addRow("Nowa data startu:", &dataStartEdit);

        QPushButton okBtn("Zapisz");
        form.addRow(&okBtn);

        connect(&okBtn, &QPushButton::clicked, [&]() {
            int rolnikId = Session::getInstance().getUserId();

            int poleId = poleEdit.text().isEmpty() ? -1 : getPoleIdByName(db, poleEdit.text(), rolnikId);
            int budynekId = budynekEdit.text().isEmpty() ? -1 : getBudynekIdByName(db, budynekEdit.text(), rolnikId);
            int maszynaId = maszynaEdit.text().isEmpty() ? -1 : getMaszynaIdByName(db, maszynaEdit.text(), rolnikId);
            int pracownikId = pracownikEdit.text().isEmpty() ? -1 : getPracownikIdByFullName(db, pracownikEdit.text(), rolnikId);
            int zwierzeId = zwierzeEdit.text().isEmpty() ? -1 : getZwierzeIdBySpecies(db, zwierzeEdit.text(), rolnikId);
            int uprawaId = uprawaEdit.text().isEmpty() ? -1 : uprawaEdit.text().toInt();

            QSqlQuery q(db);
            q.prepare("SELECT edytuj_zadanie(:id, :rolnikId, :nazwa, :opis, :pracownikId, :uprawaId, :poleId, :budynekId, :maszynaId, :zwierzeId, :dataStart)");
            q.bindValue(":id", idEdit.text().toInt());
            q.bindValue(":rolnikId", rolnikId);
            q.bindValue(":nazwa", nazwaEdit.text().isEmpty() ? QVariant() : nazwaEdit.text());
            q.bindValue(":opis", opisEdit.text().isEmpty() ? QVariant() : opisEdit.text());
            q.bindValue(":pracownikId", pracownikId == -1 ? QVariant() : pracownikId);
            q.bindValue(":uprawaId", uprawaId == -1 ? QVariant() : uprawaId);
            q.bindValue(":poleId", poleId == -1 ? QVariant() : poleId);
            q.bindValue(":budynekId", budynekId == -1 ? QVariant() : budynekId);
            q.bindValue(":maszynaId", maszynaId == -1 ? QVariant() : maszynaId);
            q.bindValue(":zwierzeId", zwierzeId == -1 ? QVariant() : zwierzeId);
            q.bindValue(":dataStart", dataStartEdit.text().isEmpty() ? QVariant() : dataStartEdit.text());

            if (!q.exec()) {
                QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
                return;
            }

            QMessageBox::information(this, "Info", q.value(0).toString());
            loadDataTable();
            loadDataCalendar();
            dlg.accept();
            });

        dlg.exec();
    }


    
    void ZadaniaDashboardWindow::usunZadanieDialog() {
        QDialog dlg(this);
        dlg.setWindowTitle("Usuń zadanie");
        QFormLayout form(&dlg);

        QLineEdit idEdit;
        form.addRow("ID zadania:", &idEdit);

        QPushButton okBtn("Usuń");
        form.addRow(&okBtn);

        connect(&okBtn, &QPushButton::clicked, [&]() {
            int rolnikId = Session::getInstance().getUserId();
            int idZadania = idEdit.text().toInt();

            if (idZadania <= 0) {
                QMessageBox::warning(this, "Błąd", "Podaj poprawne ID zadania.");
                return;
            }

            auto reply = QMessageBox::warning(
                this, "Uwaga",
                "Usunięcie zadania spowoduje utratę powiązanych danych. Czy na pewno chcesz kontynuować?",
                QMessageBox::Yes | QMessageBox::No
            );
            if (reply == QMessageBox::No) return;

            QSqlQuery q(db);
            q.prepare("SELECT usun_zadanie(:id, :rolnikId)");
            q.bindValue(":id", idZadania);
            q.bindValue(":rolnikId", rolnikId);

            if (!q.exec()) {
                QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
                return;
            }
            if (q.next()) {
                QMessageBox::information(this, "Info", q.value(0).toString());
                loadDataTable();
                loadDataCalendar();
            }
            dlg.accept();
            });

        dlg.exec();
    }


    void ZadaniaDashboardWindow::zmienStatusZadaniaDialog() {
        QDialog dlg(this);
        dlg.setWindowTitle("Zmień status zadania");
        QFormLayout form(&dlg);

        QLineEdit idEdit;
        QComboBox statusCombo;
        statusCombo.addItems({ "planowane", "w_trakcie", "anulowane" });

        form.addRow("ID zadania:", &idEdit);
        form.addRow("Nowy status:", &statusCombo);

        QPushButton okBtn("Zapisz");
        form.addRow(&okBtn);

        connect(&okBtn, &QPushButton::clicked, [&]() {
            int idZadania = idEdit.text().toInt();
            if (idZadania <= 0) {
                QMessageBox::warning(this, "Błąd", "Podaj poprawne ID zadania.");
                return;
            }

            int uzytkownikId = Session::getInstance().getUserId(); // id_uzytkownika (rolnika)
            QSqlQuery q(db);
            q.prepare("SELECT zmien_status_zadania(:idZadania, :uzytkownikId, :status)");
            q.bindValue(":idZadania", idZadania);
            q.bindValue(":uzytkownikId", uzytkownikId);
            q.bindValue(":status", statusCombo.currentText());

            if (!q.exec()) {
                QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
                return;
            }
            if (q.next()) {
                QMessageBox::information(this, "Info", "zmieniono status zadania");
                loadDataTable();
                loadDataCalendar();
            }
            dlg.accept();
            });

        dlg.exec();
    }


    void ZadaniaDashboardWindow::wykonajZadanieDialog() {
        QDialog dlg(this);
        dlg.setWindowTitle("Wykonaj zadanie");
        QFormLayout form(&dlg);

        QLineEdit idZadaniaEdit, magazynEdit;
        QCheckBox surowceCheck("Zużył surowce");
        QCheckBox plonyCheck("Pozyskał plony");

        form.addRow("ID zadania:", &idZadaniaEdit);
        form.addRow("Nazwa magazynu:", &magazynEdit);
        form.addRow(&surowceCheck);
        form.addRow(&plonyCheck);

        QPushButton okBtn("Wykonaj");
        form.addRow(&okBtn);

        connect(&okBtn, &QPushButton::clicked, [&]() {
            int rolnikId = Session::getInstance().getUserId();
            int idZadania = idZadaniaEdit.text().toInt();
            QString magazynNazwa = magazynEdit.text();

            int idMagazynu = getMagazynIdByName(magazynNazwa, rolnikId);
            

            QVector<ZuzycieSurowca> surowce;
            QVector<PozyskanyPlon> plony;

            if (surowceCheck.isChecked()) {
                surowce = zbierzSurowce(idMagazynu);
            }
            
            if (plonyCheck.isChecked()) {
                plony = zbierzPlony();
            }

            // --- Wywołanie funkcji SQL ---
            QSqlQuery q(db);
            q.prepare("SELECT wykonaj_zadanie(:idZadania, :rolnikId, :idMagazynu, :surowce, :plony)");
            q.bindValue(":idZadania", idZadania);
            q.bindValue(":rolnikId", rolnikId);
            q.bindValue(":idMagazynu", idMagazynu);

            if (surowceCheck.isChecked()) q.bindValue(":surowce", konwertujSurowce(surowce));
            if (plonyCheck.isChecked()) q.bindValue(":plony", konwertujPlony(plony));

            if (!q.exec()) {
                QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
                return;
            }
            if (q.next()) {
                QMessageBox::information(this, "Info", "zadanie zostało wykonane");
                loadDataTable();
                loadDataCalendar();
            }
            dlg.accept();
            });

        dlg.exec();
    }

    QVector<ZuzycieSurowca> ZadaniaDashboardWindow::zbierzSurowce(int idMagazynu) {
        QVector<ZuzycieSurowca> lista;
        bool finished = false;

        while (!finished) {
            QDialog dlg(this);
            dlg.setWindowTitle("Zużyte surowce");
            QFormLayout form(&dlg);

            QLineEdit nazwaEdit, iloscEdit;
            form.addRow("Nazwa surowca:", &nazwaEdit);
            form.addRow("Ilość:", &iloscEdit);

            QPushButton addBtn("Dodaj kolejny");
            QPushButton endBtn("Zakończ");
            form.addRow(&addBtn);
            form.addRow(&endBtn);

            connect(&addBtn, &QPushButton::clicked, [&]() {
                int idSurowca = getSurowiecIdByName(nazwaEdit.text(), idMagazynu);
                if (idSurowca == -1) {
                    QMessageBox::warning(this, "Błąd", "Nie znaleziono surowca.");
                    return;
                }
                lista.push_back({ idSurowca, iloscEdit.text().toDouble() });
                dlg.accept();
                });

            connect(&endBtn, &QPushButton::clicked, [&]() {
                int idSurowca = getSurowiecIdByName(nazwaEdit.text(), idMagazynu);
                if (!nazwaEdit.text().isEmpty() && idSurowca != -1) {
                    lista.push_back({ idSurowca, iloscEdit.text().toDouble() });
                }
                finished = true;
                dlg.accept();
                });

            dlg.exec();
        }

        return lista;
    }
    QVector<PozyskanyPlon> ZadaniaDashboardWindow::zbierzPlony() {
        QVector<PozyskanyPlon> lista;
        bool finished = false;

        while (!finished) {
            QDialog dlg(this);
            dlg.setWindowTitle("Pozyskane plony");
            QFormLayout form(&dlg);

            QLineEdit nazwaEdit, iloscEdit;
            form.addRow("Nazwa plonu:", &nazwaEdit);
            form.addRow("Ilość:", &iloscEdit);

            QPushButton addBtn("Dodaj kolejny");
            QPushButton endBtn("Zakończ");
            form.addRow(&addBtn);
            form.addRow(&endBtn);

            connect(&addBtn, &QPushButton::clicked, [&]() {
                lista.push_back({ nazwaEdit.text(), iloscEdit.text().toDouble() });
                dlg.accept();
                });

            connect(&endBtn, &QPushButton::clicked, [&]() {
                if (!nazwaEdit.text().isEmpty()) {
                    lista.push_back({ nazwaEdit.text(), iloscEdit.text().toDouble() });
                }
                finished = true;
                dlg.accept();
                });

            dlg.exec();
        }

        return lista;
    }

  
    QVariant ZadaniaDashboardWindow::konwertujSurowce(const QVector<ZuzycieSurowca>& lista) {
        QString arr = "{";

        for (int i = 0; i < lista.size(); ++i) {
            // element composite: (idSurowca, ilosc)
            arr += QString("\"(%1,%2)\"")
                .arg(lista[i].idSurowca)
                .arg(lista[i].ilosc);

            if (i < lista.size() - 1)
                arr += ",";
        }

        arr += "}";

        return arr;
    }
    QVariant ZadaniaDashboardWindow::konwertujPlony(const QVector<PozyskanyPlon>& lista) {
        QString arr = "{";

        for (int i = 0; i < lista.size(); ++i) {
            // element composite: (nazwa, ilosc)
            arr += QString("\"(%1,%2)\"")
                .arg(lista[i].nazwa)
                .arg(lista[i].ilosc);

            if (i < lista.size() - 1)
                arr += ",";
        }

        arr += "}";

        return arr;
    }
