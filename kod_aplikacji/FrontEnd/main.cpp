#include <QApplication>
#include "LogIn_Page.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <libpq-fe.h>


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "main_connection");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("postgres");
    db.setConnectOptions("options='-c search_path=gospodarka'");

    if (!db.open()) {
        QMessageBox::critical(nullptr, "Error",
            QString("QtSql connection failed:\n%1").arg(db.lastError().text()));
        return 1;
    }

    //qDebug() << "Available SQL drivers after path update:" << QSqlDatabase::drivers();


    PGconn* conn; conn = PQconnectdb("host=localhost port=5432 dbname=postgres user=postgres password=postgres options='-c search_path=gospodarka'");
    if (PQstatus(conn) != CONNECTION_OK) {
        QMessageBox::critical(nullptr, "Error",
            QString("Database connection failed:\n%1")
            .arg(PQerrorMessage(conn)));
        return 1;
    }

    LoginWindow window(conn, db);
    window.show();

    int result = app.exec();

    PQfinish(conn);
    return result;
}
