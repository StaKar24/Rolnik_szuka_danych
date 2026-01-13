#include "Rolnik_Dashboard.h"

#include "Uprawa_Dashboard.h"
#include "Maszyna_Dashboard.h"
#include "Budynek_Dashboard.h"
#include "Zwierze_Dashboard.h"
#include "Magazyn_Dashboard.h"
#include "Surowce_Dashboard.h"
#include "Plon_Dashboard.h"
#include "Zadanie_Dashboard.h"
#include "Pole_Dashboard.h"
#include "PracownicyDashboard.h"
#include "FinanseDashboard.h"
#include "StatystykiDashboard.h"
#include "../Klasy_logiczne/Session.h"
#include "../Klasy_logiczne/AuthService.h"

#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QSqlError>
#include <iostream>


void checkPrzegladyAlert(QSqlDatabase db, int rolnikId, QWidget* parent) {
    QSqlQuery q(db);
    q.prepare(
        "SELECT nazwa, typ, data_przegladu, dni_do_przegladu "
        "FROM view_przeglady_alert "
        "WHERE id_rolnika = :id"
    );
    q.bindValue(":id", rolnikId);

    if (!q.exec()) {
        QMessageBox::critical(parent, "Błąd SQL", q.lastError().text());
        return;
    }

    if (!q.next()) {
        // brak alertów — nic nie pokazujemy
        return;
    }

    
    QString msg = "<h3>🚨 Zbliżające się przeglądy maszyn</h3>";
    msg += "<p>Poniższe maszyny wymagają przeglądu w ciągu 3 dni:</p><ul>";

    do {
        QString nazwa = q.value("nazwa").toString();
        QString typ = q.value("typ").toString();
        QString data = q.value("data_przegladu").toDate().toString("yyyy-MM-dd");
        int dni = q.value("dni_do_przegladu").toInt();

        msg += "<li><b>" + nazwa + "</b> (" + typ + ") — przegląd: <b>" + data +
            "</b> (za " + QString::number(dni) + " dni)</li>";
    } while (q.next());

    msg += "</ul>";

    QMessageBox::warning(parent, "Przeglądy maszyn", msg);
}









FarmerDashboardWindow::FarmerDashboardWindow(QSqlDatabase database, QWidget* parent)
    : QWidget(parent), db(database)
{
    setWindowTitle("Dashboard Rolnika");
    resize(900, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Pasek górny ---
    QHBoxLayout* topLayout = new QHBoxLayout();
    userLabel = new QLabel(this);
    dateTimeLabel = new QLabel(this);

    int uid = Session::getInstance().getUserId();
    QString login;
    QSqlQuery q(db);
    q.prepare("SELECT login FROM Uzytkownik WHERE id_uzytkownika = :id");
    q.bindValue(":id", uid);
    if (q.exec() && q.next()) {
        login = q.value(0).toString();
    }

    userLabel->setText("Zalogowany: " + login);
    topLayout->addWidget(userLabel);
    topLayout->addStretch();
    topLayout->addWidget(dateTimeLabel);
    mainLayout->addLayout(topLayout);

    // --- Timer ---
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FarmerDashboardWindow::updateDateTime);
    timer->start(1000);
    
    checkPrzegladyAlert(db, uid, this);
    updateDateTime();

    // --- Grid z modułami ---
    grid = new QGridLayout();
    QStringList modules = {
        "Plony", "Uprawy", "Pola", "Magazyn", "Surowce",
        "Finanse", "Zadania", "Zwierzęta", "Budynki", "Pracownicy", "Maszyny", "Statystyki"
    };

    int row = 0, col = 0;
    for (const QString& mod : modules) {
        QPushButton* btn = new QPushButton(mod, this);
        btn->setFixedSize(150, 150);
        btn->setStyleSheet("font-size: 16px; font-weight: bold;");
        grid->addWidget(btn, row, col);

        connect(btn, &QPushButton::clicked, this, [this, mod]() {
            if (mod == "Pola") {
                PoleDashboardWindow* poleWin = new PoleDashboardWindow(db);
                poleWin->show();
            }
            if (mod == "Uprawy") {
                UprawyDashboardWindow* uprWin = new UprawyDashboardWindow(db);
                uprWin->show();
            }
            if (mod == "Maszyny") {
                MaszynyDashboardWindow* maszWin = new MaszynyDashboardWindow(db);
                maszWin->show();
            }
            if (mod == "Budynki") {
                BudynkiDashboardWindow* budWin = new BudynkiDashboardWindow(db);
                budWin->show();
            }
            if (mod == "Zwierzęta") {
                ZwierzetaDashboardWindow* zweWin = new ZwierzetaDashboardWindow(db);
                zweWin->show();
            }
            if (mod == "Magazyn") {
                MagazynyDashboardWindow* magWin = new MagazynyDashboardWindow(db);
                magWin->show();
            }
            if (mod == "Surowce") {
                SurowceDashboardWindow* surWin = new SurowceDashboardWindow(db);
                surWin->show();
            }
            if (mod == "Plony") {
                PlonyDashboardWindow* plWin = new PlonyDashboardWindow(db);
                plWin->show();
            }
            if (mod == "Zadania") {
                ZadaniaDashboardWindow* zadWin = new ZadaniaDashboardWindow(db);
                zadWin->show();
            }
            if (mod == "Pracownicy") {
                PracownicyDashboard* praWin = new PracownicyDashboard(db);
                praWin->show();
            }
            if (mod == "Finanse") {
                FinanseDashboard* fiWin = new FinanseDashboard(db);
                fiWin->show();
            }
            if (mod == "Statystyki") {
                StatystykiDashboard* staWin = new StatystykiDashboard(db, Session::getInstance().getUserId());
                staWin->show();
            }
            });

        col++;
        if (col == 4) { col = 0; row++; }
    }

    mainLayout->addLayout(grid);
}

void FarmerDashboardWindow::updateDateTime() {
    dateTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}