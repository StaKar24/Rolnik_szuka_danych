
#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QStackedWidget>
#include <QTableView>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVector>

struct ZuzycieSurowca { int idSurowca; double ilosc; };
struct PozyskanyPlon { QString nazwa; double ilosc; };

class ZadaniaDashboardWindow : public QWidget {
    Q_OBJECT
private:
    QStackedWidget* stackedViews;
    QTableView* tableView;
    QCalendarWidget* calendarView;
    QListWidget* calendarTasksList;

    QLineEdit* filterNameEdit;
    QComboBox* filterStatusCombo;
    QCheckBox* filterUprawyCheck;
    QCheckBox* filterZwierzeCheck;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* executeBtn;
    QPushButton* changeStatusBtn;
    QPushButton* switchViewBtn;

    QSqlDatabase db;

public:
    explicit ZadaniaDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadDataTable();
    void loadDataCalendar();

    void dodajZadanieDialog();
    void edytujZadanieDialog();
    void usunZadanieDialog();
    void zmienStatusZadaniaDialog();
    void wykonajZadanieDialog();


    int getMagazynIdByName(const QString& nazwa, int rolnikId);
    int getSurowiecIdByName(const QString& nazwa, int magazynId);
    int getPoleIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId);
    int getBudynekIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId);
    int getMaszynaIdByName(QSqlDatabase db, const QString& nazwa, int rolnikId);
    int getPracownikIdByFullName(QSqlDatabase db, const QString& fullName, int rolnikId);
    int getZwierzeIdBySpecies(QSqlDatabase db, const QString& text, int rolnikId);
    //int getUprawaIdByName(QSqlDatabase db, const QString& nazwa, int poleId);




    QVector<ZuzycieSurowca> zbierzSurowce(int idMagazynu);
    QVector<PozyskanyPlon> zbierzPlony();
    QVariant konwertujSurowce(const QVector<ZuzycieSurowca>& lista);
    QVariant konwertujPlony(const QVector<PozyskanyPlon>& lista);
};





