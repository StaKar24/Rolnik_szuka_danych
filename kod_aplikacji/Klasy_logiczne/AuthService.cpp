#include "AuthService.h"
#include "Session.h"
#include <iostream>

bool AuthService::loginUser(const std::string& login, const std::string& password) {
    const char* paramValues[1] = { login.c_str() };
    PGresult* res = PQexecParams(conn,
        "SELECT id_uzytkownika, haslo, rola FROM gospodarka_testy.uzytkownik WHERE login = $1",
        1, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Error -> zapytanie: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) == 0) {
        std::cout << "Urzytkownik nie istnieje." << std::endl;
        PQclear(res);
        return false;
    }

    int userId = std::stoi(PQgetvalue(res, 0, 0));
    std::string dbPass = PQgetvalue(res, 0, 1);
    std::string role = PQgetvalue(res, 0, 2);

    PQclear(res);

    //bez hashowania 
    if (password == dbPass) {
        Session::getInstance().login(userId, role); //dane userid i role tymane w session
        std::cout << "Udane logowanie: UserId " << userId << " Rola: " << role << std::endl;
        return true;
    }
    else {
        std::cout << "Nie poprawne hasło." << std::endl;
        return false;
    }
}


