#pragma once
#include <string>

class Session {
private:
    int userId;
    std::string role;

    Session() : userId(-1) {} //Singleton

public:
    static Session& getInstance() {
        static Session instance;
        return instance;
    }

    void login(int id, const std::string& role_) {
        userId = id;
        role = role_;
    }

    void logout() {
        userId = -1;  //-1 dla wylogowania 
        role.clear();
    }

    int getUserId() const { return userId; }
    std::string getRole() const { return role; }

    Session(const Session&) = delete; //Singleton (usuwoanie kopoiowanie)
    void operator=(const Session&) = delete;
};
