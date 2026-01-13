#include "StatystykiDashboard.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

StatystykiDashboard::StatystykiDashboard(QSqlDatabase db, int rolnikId, QWidget* parent)
    : QWidget(parent), db(db), rolnikId(rolnikId)
{
    auto* layout = new QVBoxLayout(this);

    // -------------------------
    // RAPORT GŁÓWNY
    // -------------------------
    raportGlowneLabel = new QLabel(this);
    raportGlowneLabel->setWordWrap(true);
    raportGlowneLabel->setStyleSheet("font-size: 14px; padding: 10px;");

    layout->addWidget(new QLabel("<h2>📊 Raport ogólny</h2>"));
    layout->addWidget(raportGlowneLabel);

    // -------------------------
    // RAPORT HAVING
    // -------------------------
    layout->addWidget(new QLabel("<h2>📈 Statystyki dodatkowe</h2>"));

    filtrTypCombo = new QComboBox(this);
    filtrTypCombo->addItem("Wszystkie", "");
    filtrTypCombo->addItem("Pracownicy powyżej średniej", "pracownik_powyzej_sredniej");
    filtrTypCombo->addItem("Rzadko używane maszyny", "rzadko_uzywana_maszyna");
    filtrTypCombo->addItem("Pola z dużą liczbą upraw", "pole_z_duza_liczba_upraw");
    filtrTypCombo->addItem("Popularne rośliny", "popularna_roslina");
    filtrTypCombo->addItem("Rekordowe dni", "rekordowy_dzien");

    layout->addWidget(filtrTypCombo);

    tabelaHaving = new QTableView(this);
    modelHaving = new QSqlQueryModel(this);
    tabelaHaving->setModel(modelHaving);

    layout->addWidget(tabelaHaving);

    // Ładowanie danych
    loadRaportGlowne();
    loadRaportHaving("");

    // Reakcja na zmianę filtra
    connect(filtrTypCombo, &QComboBox::currentTextChanged, [&](const QString&) {
        loadRaportHaving(filtrTypCombo->currentData().toString());
        });
}

void StatystykiDashboard::loadRaportGlowne() {
    QSqlQuery q(db);
    q.prepare("SELECT * FROM raport_statystyki_glowne WHERE id_rolnika = :id");
    q.bindValue(":id", rolnikId);

    if (!q.exec() || !q.next()) {
        raportGlowneLabel->setText("Brak danych.");
        return;
    }

    QString text;
    text += "• Najbardziej dochodowy miesiąc: <b>" + q.value("najbardziej_dochodowy_miesiac").toString() + "</b><br>";
    text += "• Największa transakcja: <b>" + q.value("najwieksza_transakcja").toString() + " zł</b><br>";
    text += "• Średnia wartość transakcji: <b>" + q.value("srednia_wartosc_transakcji").toString() + " zł</b><br>";
    text += "• Zysk netto w tym roku: <b>" + q.value("zysk_netto_rok").toString() + " zł</b><br>";
    text += "• Najczęściej używana maszyna: <b>" + q.value("najczesciej_uzywana_maszyna").toString() + "</b><br>";
    text += "• Najbardziej aktywny pracownik: <b>" + q.value("najbardziej_aktywny_pracownik").toString() + "</b><br>";
    text += "• Średnia liczba zadań na pracownika: <b>" + q.value("srednia_zadan_na_pracownika").toString() + "</b><br>";
    text += "• Łączna liczba zwierząt: <b>" + q.value("laczna_liczba_zwierzat").toString() + "</b><br>";
    text += "• Najczęściej uprawiana roślina: <b>" + q.value("najczesciej_uprawiana_roslina").toString() + "</b><br>";
    text += "• Pole z największą liczbą upraw: <b>" + q.value("pole_z_najwieksza_liczba_upraw").toString() + "</b><br>";
    
    raportGlowneLabel->setText(text);
}

void StatystykiDashboard::loadRaportHaving(const QString& typ) {
    QString query =
        "SELECT nazwa, wartosc "
        "FROM raport_statystyki_dod "
        "WHERE id_rolnika = :id";

    if (!typ.isEmpty()) {
        query += " AND typ = '" + typ + "'";
    }

    QString finalQuery = query;
    finalQuery.replace(":id", QString::number(rolnikId));

    modelHaving->setQuery(finalQuery, db);

}
