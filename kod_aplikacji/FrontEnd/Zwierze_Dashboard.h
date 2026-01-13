#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QCheckBox>
#include <QSqlDatabase>

class ZwierzetaDashboardWindow : public QWidget {
    Q_OBJECT

private:
    QTableView* table;
    QLineEdit* filterEdit;
    QPushButton* filterBtn;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;

    QSqlDatabase db;

public:
    explicit ZwierzetaDashboardWindow(QSqlDatabase database, QWidget* parent = nullptr);

private slots:
    void loadData();
    void dodajZwierzeDialog();
    void edytujZwierzeDialog();
    void usunZwierzeDialog();
    int getBudynekIdByName(const QString& nazwa, int rolnikId);
    int getZwierzeIdByGatunekRasa(const QString& gatunek, const QString& rasa, int rolnikId);
};
