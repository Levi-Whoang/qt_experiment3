#include "loginview.h"
#include "idatabase.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("登录");

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->addStretch(2);

    // 标题
    QLabel *titleLabel = new QLabel("欢迎使用诊疗测试系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("loginTitleLabel");
    outerLayout->addWidget(titleLabel);

    QLabel *subTitleLabel = new QLabel("医院诊疗信息管理平台 (默认账号: admin / 123456)", this);
    subTitleLabel->setAlignment(Qt::AlignCenter);
    subTitleLabel->setObjectName("loginSubtitleLabel");
    outerLayout->addWidget(subTitleLabel);

    outerLayout->addSpacing(28);

    // 登录表单卡片
    QWidget *card = new QWidget(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(380);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 28, 32, 28);
    cardLayout->setSpacing(18);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setSpacing(14);

    m_usernameEdit = new QLineEdit(card);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setText("admin");
    m_usernameEdit->setMinimumHeight(36);

    m_passwordEdit = new QLineEdit(card);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(36);

    formLayout->addRow("用户名：", m_usernameEdit);
    formLayout->addRow("密码：", m_passwordEdit);
    cardLayout->addLayout(formLayout);

    // 按钮行：注册 | 登录（与报告图1一致）
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(16);

    m_registerBtn = new QPushButton("注册", card);
    m_registerBtn->setObjectName("secondaryButton");
    m_registerBtn->setMinimumHeight(38);
    m_registerBtn->setCursor(Qt::PointingHandCursor);

    m_loginBtn = new QPushButton("登录", card);
    m_loginBtn->setObjectName("primaryButton");
    m_loginBtn->setMinimumHeight(38);
    m_loginBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_registerBtn);
    btnLayout->addWidget(m_loginBtn);
    cardLayout->addLayout(btnLayout);

    outerLayout->addWidget(card, 0, Qt::AlignHCenter);
    outerLayout->addStretch(3);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginView::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginView::onRegisterClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginView::onLoginClicked);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, [this]() {
        m_passwordEdit->setFocus();
    });
}

void LoginView::resetInputs()
{
    m_passwordEdit->clear();
    m_usernameEdit->setFocus();
}

void LoginView::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入提示", "用户名和密码不能为空！");
        return;
    }

    QString status = IDatabase::getInstance().userLogin(username, password);
    if (status == "loginOK") {
        emit loginSuccess();
    } else if (status == "wrongPassword") {
        QMessageBox::warning(this, "登录失败", "密码错误，请重新输入！");
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    } else if (status == "wrongUsername") {
        QMessageBox::warning(this, "登录失败", "用户名不存在，请检查或先注册账号！");
        m_usernameEdit->setFocus();
    } else {
        QMessageBox::critical(this, "数据库错误",
                              "数据库连接失败，无法验证登录：\n" +
                                  IDatabase::getInstance().lastErrorMessage());
    }
}

void LoginView::onRegisterClicked()
{
    if (!IDatabase::getInstance().isDatabaseOpen()) {
        QMessageBox::critical(this, "数据库错误",
                              "数据库未连接，无法注册：\n" +
                                  IDatabase::getInstance().lastErrorMessage());
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("新用户注册");
    dlg.setMinimumWidth(340);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;

    QLineEdit *fullnameEdit = new QLineEdit(&dlg);
    fullnameEdit->setPlaceholderText("例如：张医生");
    QLineEdit *userEdit = new QLineEdit(&dlg);
    userEdit->setPlaceholderText("请输入登录用户名");
    QLineEdit *passEdit = new QLineEdit(&dlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("请输入密码");
    QLineEdit *confirmEdit = new QLineEdit(&dlg);
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setPlaceholderText("请再次输入密码");

    form->addRow("姓名：", fullnameEdit);
    form->addRow("用户名：", userEdit);
    form->addRow("密码：", passEdit);
    form->addRow("确认密码：", confirmEdit);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("注册");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString fullname = fullnameEdit->text().trimmed();
    QString username = userEdit->text().trimmed();
    QString password = passEdit->text();
    QString confirm = confirmEdit->text();

    if (fullname.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名、用户名和密码均不能为空！");
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致！");
        return;
    }

    QString errMsg;
    if (IDatabase::getInstance().userRegister(fullname, username, password, errMsg)) {
        QMessageBox::information(this, "注册成功", "账号注册成功，请使用新账号登录！");
        m_usernameEdit->setText(username);
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    } else {
        QMessageBox::warning(this, "注册失败", errMsg);
    }
}
