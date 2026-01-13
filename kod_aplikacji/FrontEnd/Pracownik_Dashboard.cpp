#include "Pracownik_Dashboard.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

int getRolnikIdByWorkerUserId(QSqlDatabase db, int userId) {
    QSqlQuery q(db);
    q.prepare(
        "SELECT id_rolnika "
        "FROM Pracownik "
        "WHERE id_uzytkownika = :id"
    );
    q.bindValue(":id", userId);

    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }

    return -1; // brak powiązanego rolnika
}



PracownikDashboardWindow::PracownikDashboardWindow(QSqlDatabase db, int userId, QWidget* parent)
    : QWidget(parent), db(db), userId(userId)
{
    auto* layout = new QVBoxLayout(this);
    QSqlQuery q(db);
    q.prepare("SELECT id_pracownika FROM Pracownik WHERE id_uzytkownika = :id"); 
    q.bindValue(":id", userId); 
    if (q.exec() && q.next()) {
         pracownikId = q.value(0).toInt();
    }



    infoLabel = new QLabel(this);
    infoLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 5px;");
    layout->addWidget(infoLabel);

    loadPracownikInfo();



    modelZadan = new QSqlQueryModel(this);
    tabelaZadan = new QTableView(this);
    tabelaZadan->setModel(modelZadan);
    tabelaZadan->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabelaZadan->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(new QLabel("<h3>Twoje zadania</h3>"));
    layout->addWidget(tabelaZadan);

    loadZadania();


    auto* btnLayout = new QHBoxLayout();

    btnOdswiez = new QPushButton("Odśwież");
    btnZmienStatus = new QPushButton("Zmień status");
    btnWykonaj = new QPushButton("Oznacz jako wykonane");

    btnLayout->addWidget(btnOdswiez);
    btnLayout->addWidget(btnZmienStatus);
    btnLayout->addWidget(btnWykonaj);

    layout->addLayout(btnLayout);

    connect(btnOdswiez, &QPushButton::clicked, this, &PracownikDashboardWindow::loadZadania);
    connect(btnZmienStatus, &QPushButton::clicked, this, &PracownikDashboardWindow::zmienStatusZadania);
    connect(btnWykonaj, &QPushButton::clicked, this, &PracownikDashboardWindow::wykonajZadanie);
}

void PracownikDashboardWindow::loadPracownikInfo() {
    QSqlQuery q(db);
    q.prepare("SELECT id_pracownika, imie, nazwisko, stanowisko "
        "FROM Pracownik WHERE id_uzytkownika = :id");
    q.bindValue(":id", userId);

    if (!q.exec() || !q.next()) {
        infoLabel->setText("Błąd: nie znaleziono danych pracownika.");
        return;
    }

    
    QString imie = q.value("imie").toString();
    QString nazwisko = q.value("nazwisko").toString();
    QString stanowisko = q.value("stanowisko").toString();

    infoLabel->setText("👤 " + imie + " " + nazwisko + " — " + stanowisko);
}

void PracownikDashboardWindow::loadZadania() {
    QString query =
        "SELECT id_zadania, nazwa, status, data_start, data_koniec "
        "FROM view_pracownik_zadania "
        "WHERE id_pracownika = " + QString::number(pracownikId) + " "
        "AND status IN ('planowane', 'w_trakcie')";

    modelZadan->setQuery(query, db);
}

void PracownikDashboardWindow::zmienStatusZadania() {
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

        int uzytkownikId = userId; // id_uzytkownika 
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
            loadZadania();
        }
        dlg.accept();
        });

    dlg.exec();
}

void PracownikDashboardWindow::wykonajZadanie() {
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
        int rolnikId = getRolnikIdByWorkerUserId(db, userId);
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
        q.prepare("SELECT wykonaj_zadanie(:idZadania, :pracownikId, :idMagazynu, :surowce, :plony)");
        q.bindValue(":idZadania", idZadania);
        q.bindValue(":pracownikId", pracownikId); //!!!!!!!!!!!!!!
        q.bindValue(":idMagazynu", idMagazynu);

        if (surowceCheck.isChecked()) q.bindValue(":surowce", konwertujSurowce(surowce));
        if (plonyCheck.isChecked()) q.bindValue(":plony", konwertujPlony(plony));

        if (!q.exec()) {
            QMessageBox::critical(this, "Błąd SQL", q.lastError().text());
            return;
        }
        if (q.next()) {
            QMessageBox::information(this, "Info", "zadanie zostało wykonane");
            loadZadania();
        }
        dlg.accept();
        });

    dlg.exec();
}





QVector<ZuzycieSurowca> PracownikDashboardWindow::zbierzSurowce(int idMagazynu) {
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
QVector<PozyskanyPlon> PracownikDashboardWindow::zbierzPlony() {
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
QVariant PracownikDashboardWindow::konwertujSurowce(const QVector<ZuzycieSurowca>& lista) {
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
QVariant PracownikDashboardWindow::konwertujPlony(const QVector<PozyskanyPlon>& lista) {
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

int PracownikDashboardWindow::getMagazynIdByName(const QString& nazwa, int rolnikId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_magazynu FROM Magazyn WHERE nazwa = :nazwa AND id_rolnika = :rolnikId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":rolnikId", rolnikId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}
int PracownikDashboardWindow::getSurowiecIdByName(const QString& nazwa, int magazynId) {
    QSqlQuery q(db);
    q.prepare("SELECT id_surowca FROM Surowiec WHERE nazwa = :nazwa AND id_magazynu = :magazynId");
    q.bindValue(":nazwa", nazwa);
    q.bindValue(":magazynId", magazynId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;

}