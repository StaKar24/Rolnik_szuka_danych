#include "../Klasy_logiczne/Session.h"
#include "../Klasy_logiczne/AuthService.h"
#include <iostream>

int main() {
    PGconn* conn = PQconnectdb("host=localhost port=5432 dbname=postgres user=postgres password=postgres options='-c search_path=gospodarka_testy'");
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Blad polaczenia: " << PQerrorMessage(conn) << std::endl;
        return 1;
    }

    AuthService auth(conn);

    std::string login, password;
    std::cout << "Login: ";
    std::cin >> login;
    std::cout << "Haslo: ";
    std::cin >> password;

    if (auth.loginUser(login, password)) {

        int uid = Session::getInstance().getUserId();
        std::string role = Session::getInstance().getRole();

        std::cout << "Welcome, user " << uid << " with role " << role << std::endl;

        if (role == "admin") {
            std::cout << "Admin dashboard..." << std::endl;
        }
        else if (role == "rolnik") {
            std::cout << "Rolnik dashboard..." << std::endl;
        }
        else if (role == "pracownik") {
            std::cout << "Pracownik dashboard..." << std::endl;
        }
    }

    PQfinish(conn);
    return 0;
}
