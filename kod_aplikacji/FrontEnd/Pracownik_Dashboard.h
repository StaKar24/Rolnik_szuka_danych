#pragma once
#include <QWidget>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QVector>

struct ZuzycieSurowca { int idSurowca; double ilosc; };
struct PozyskanyPlon { QString nazwa; double ilosc; };

class PracownikDashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit PracownikDashboardWindow(QSqlDatabase db, int userId, QWidget* parent = nullptr);

private:
    QSqlDatabase db;
    int userId;          // id_uzytkownika
    int pracownikId;     // id_pracownika

    QLabel* infoLabel;
    QTableView* tabelaZadan;
    QSqlQueryModel* modelZadan;

    QPushButton* btnOdswiez;
    QPushButton* btnZmienStatus;
    QPushButton* btnWykonaj;

    void loadPracownikInfo();
    void loadZadania();
    void zmienStatusZadania();
    void wykonajZadanie();


    int getMagazynIdByName(const QString& nazwa, int rolnikId);
    int getSurowiecIdByName(const QString& nazwa, int magazynId);

    QVector<ZuzycieSurowca> zbierzSurowce(int idMagazynu);
    QVector<PozyskanyPlon> zbierzPlony();
    QVariant konwertujSurowce(const QVector<ZuzycieSurowca>& lista);
    QVariant konwertujPlony(const QVector<PozyskanyPlon>& lista);




};
