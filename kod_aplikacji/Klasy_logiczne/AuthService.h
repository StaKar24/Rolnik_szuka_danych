#pragma once
#include <string>
#include <libpq-fe.h>

class AuthService {
    PGconn* conn;
public:
    AuthService(PGconn* c) : conn(c) {}

    bool loginUser(const std::string& login, const std::string& password);
};
