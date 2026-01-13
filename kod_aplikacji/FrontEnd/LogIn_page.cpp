
#include "LogIn_Page.h"

LoginWindow::LoginWindow(PGconn* conn, QSqlDatabase database, QWidget* parent)
    : QWidget(parent), mainDb(database)
{
    auth = new AuthService(conn);

    setWindowTitle("Login");
    resize(300, 200);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Login
    layout->addWidget(new QLabel("Login:"));
    loginEdit = new QLineEdit(this);
    layout->addWidget(loginEdit);

    // Password
    layout->addWidget(new QLabel("Password:"));
    passEdit = new QLineEdit(this);
    passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passEdit);

    // Login button
    loginBtn = new QPushButton("Log In", this);
    layout->addWidget(loginBtn);

    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLogin);
}

void LoginWindow::onLogin() {
    std::string login = loginEdit->text().toUtf8().constData();
    std::string pass = passEdit->text().toUtf8().constData();
    qDebug() << "Login entered:" << loginEdit->text();

    if (auth->loginUser(login, pass)) {
        int uid = Session::getInstance().getUserId();
        std::string role = Session::getInstance().getRole();
        qDebug() << "User role:" << QString::fromStdString(role);

        if (role == "admin") {
            AdminDashboardWindow* adminWin = new AdminDashboardWindow(mainDb);
            adminWin->show();
            this->close();
        }
        else if (role == "rolnik") {
            FarmerDashboardWindow* rolWin = new FarmerDashboardWindow(QSqlDatabase::database("main_connection"));
            rolWin->show();
            this->close();
        }
        else if (role == "pracownik") {
            PracownikDashboardWindow* praWin = new PracownikDashboardWindow(QSqlDatabase::database("main_connection"), uid);
            praWin->show();
            this->close();
        }
    }
    else {
        QMessageBox::critical(this, "Error",
            "Login failed. Check credentials or account status.");
    }
}


