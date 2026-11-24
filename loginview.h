#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginView : public QWidget
{
    Q_OBJECT
public:
    explicit LoginView(QWidget *parent = nullptr);
    void resetInputs();

signals:
    void loginSuccess();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_registerBtn;
    QPushButton *m_loginBtn;
};

#endif // LOGINVIEW_H
