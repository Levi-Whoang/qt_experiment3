#include "mainwindow.h"
#include "idatabase.h"
#include "loginview.h"
#include "welcomeview.h"
#include "patientview.h"
#include "patienteditview.h"
#include "managementviews.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QStyle>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("医院诊疗系统 - QtSQL (MySQL) & View-Model");
    resize(1020, 680);
    setMinimumSize(820, 560);

    initUI();
    applyStyleSheet();

    // 检查 MySQL 数据库连接状态
    IDatabase &db = IDatabase::getInstance();
    if (!db.isDatabaseOpen()) {
        QMessageBox::critical(this, "数据库连接失败",
                              "启动时连接 MySQL 数据库失败：\n" + db.lastErrorMessage() +
                                  "\n\n请检查 MySQL 服务是否启动，或点击左下角“数据库配置”修改连接参数。");
    }

    updateFooterInfo();
    goLoginView();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ==================== 顶部导航栏（与报告图1~图4一致） ====================
    QWidget *headerBar = new QWidget(central);
    headerBar->setObjectName("headerBar");
    headerBar->setFixedHeight(54);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    m_backBtn = new QPushButton("  返回", headerBar);
    m_backBtn->setObjectName("navButton");
    m_backBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    m_backBtn->setMinimumHeight(34);
    m_backBtn->setCursor(Qt::PointingHandCursor);

    m_titleLabel = new QLabel("登录", headerBar);
    m_titleLabel->setObjectName("headerTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_logoutBtn = new QPushButton("退出  ", headerBar);
    m_logoutBtn->setObjectName("navButton");
    m_logoutBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    m_logoutBtn->setLayoutDirection(Qt::RightToLeft);
    m_logoutBtn->setMinimumHeight(34);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(m_backBtn, 0, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();
    headerLayout->addWidget(m_logoutBtn, 0, Qt::AlignRight);

    rootLayout->addWidget(headerBar);

    // ==================== 中央页面栈 QStackedWidget ====================
    m_stackedWidget = new QStackedWidget(central);
    m_stackedWidget->setObjectName("pageStack");
    rootLayout->addWidget(m_stackedWidget, 1);

    // ==================== 底部状态栏（含数据库路径与开发者信息） ====================
    QWidget *footerBar = new QWidget(central);
    footerBar->setObjectName("footerBar");
    footerBar->setFixedHeight(36);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerBar);
    footerLayout->setContentsMargins(16, 0, 16, 0);
    footerLayout->setSpacing(12);

    m_dbStatusLabel = new QLabel(footerBar);
    m_dbStatusLabel->setObjectName("footerInfoLabel");

    m_dbSwitchBtn = new QPushButton("数据库配置", footerBar);
    m_dbSwitchBtn->setObjectName("footerLinkButton");
    m_dbSwitchBtn->setCursor(Qt::PointingHandCursor);

    // 按实验报告要求：在界面显眼位置展示开发者姓名和学号（请替换为自己的真实信息）
    m_developerLabel = new QLabel("开发者：李伟煌    学号：2024414290417", footerBar);
    m_developerLabel->setObjectName("developerLabel");

    footerLayout->addWidget(m_dbStatusLabel);
    footerLayout->addWidget(m_dbSwitchBtn);
    footerLayout->addStretch();
    footerLayout->addWidget(m_developerLabel);

    rootLayout->addWidget(footerBar);

    setCentralWidget(central);

    connect(m_backBtn, &QPushButton::clicked, this, &MainWindow::goPreviousView);
    connect(m_logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(m_dbSwitchBtn, &QPushButton::clicked, this, &MainWindow::onSelectDatabaseClicked);
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onStackChanged);
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QMainWindow, QWidget#pageStack {
            background-color: #f4f6f9;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 14px;
            color: #2c3e50;
        }
        QWidget#headerBar {
            background-color: #ffffff;
            border-bottom: 1px solid #e2e8f0;
        }
        QLabel#headerTitleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #1e293b;
        }
        QPushButton#navButton {
            background-color: transparent;
            color: #334155;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
        }
        QPushButton#navButton:hover:enabled {
            background-color: #eff6ff;
            border-color: #bfdbfe;
            color: #2563eb;
        }
        QPushButton#navButton:disabled {
            color: #cbd5e1;
        }
        QWidget#footerBar {
            background-color: #ffffff;
            border-top: 1px solid #e2e8f0;
        }
        QLabel#footerInfoLabel {
            color: #64748b;
            font-size: 12px;
        }
        QLabel#developerLabel {
            color: #475569;
            font-size: 12px;
            font-weight: 600;
        }
        QPushButton#footerLinkButton {
            background: transparent;
            border: none;
            color: #2563eb;
            font-size: 12px;
            text-decoration: underline;
        }
        QPushButton#footerLinkButton:hover {
            color: #1d4ed8;
        }
        QLabel#loginTitleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #1e293b;
        }
        QLabel#loginSubtitleLabel {
            font-size: 13px;
            color: #64748b;
            margin-top: 6px;
        }
        QLabel#welcomeGreetingLabel {
            font-size: 20px;
            font-weight: 600;
            color: #1e293b;
        }
        QWidget#loginCard, QWidget#editFormCard {
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
        }
        QLabel {
            color: #1e293b;
        }
        QDialog, QMessageBox {
            background-color: #ffffff;
            color: #1e293b;
        }
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 4px 10px;
            color: #1e293b;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #1e293b;
            selection-background-color: #dbeafe;
            selection-color: #1e3a8a;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #3b82f6;
        }
        QPushButton#primaryButton {
            background-color: #2563eb;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            font-weight: 600;
        }
        QPushButton#primaryButton:hover {
            background-color: #1d4ed8;
        }
        QPushButton#successButton {
            background-color: #10b981;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            font-weight: 600;
        }
        QPushButton#successButton:hover {
            background-color: #059669;
        }
        QPushButton#dangerButton {
            background-color: #ef4444;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            font-weight: 600;
        }
        QPushButton#dangerButton:hover {
            background-color: #dc2626;
        }
        QPushButton#secondaryButton {
            background-color: #ffffff;
            color: #334155;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            font-weight: 500;
        }
        QPushButton#secondaryButton:hover {
            background-color: #f1f5f9;
            border-color: #94a3b8;
        }
        QPushButton#moduleCardButton {
            background-color: #ffffff;
            color: #1e293b;
            border: 1px solid #cbd5e1;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 600;
        }
        QPushButton#moduleCardButton:hover {
            background-color: #eff6ff;
            border: 2px solid #2563eb;
            color: #1d4ed8;
        }
        QTableView {
            background-color: #ffffff;
            alternate-background-color: #f8fafc;
            color: #1e293b;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            gridline-color: #e2e8f0;
            selection-background-color: #dbeafe;
            selection-color: #1e3a8a;
        }
        QTableView::item {
            color: #1e293b;
            padding: 4px 8px;
        }
        QTableView::item:hover {
            background-color: #eff6ff;
            color: #1e293b;
        }
        QTableView::item:selected {
            background-color: #dbeafe;
            color: #1e3a8a;
        }
        QHeaderView::section {
            background-color: #f1f5f9;
            color: #334155;
            font-weight: 600;
            padding: 8px;
            border: none;
            border-right: 1px solid #e2e8f0;
            border-bottom: 1px solid #cbd5e1;
        }
        QLabel#tableStatusLabel {
            color: #64748b;
            font-size: 13px;
        }
    )");
}

void MainWindow::pushWidgetToStack(QWidget *widget)
{
    m_stackedWidget->addWidget(widget);
    m_stackedWidget->setCurrentIndex(m_stackedWidget->count() - 1);
    m_titleLabel->setText(widget->windowTitle());
}

void MainWindow::updateFooterInfo()
{
    IDatabase &db = IDatabase::getInstance();
    QString userStr = db.currentUserFullName().isEmpty()
                          ? "未登录"
                          : QString("当前用户：%1").arg(db.currentUserFullName());
    QString dbStr = db.isDatabaseOpen()
                        ? QString("MySQL已连接: %1").arg(db.connectionSummary())
                        : QString("MySQL未连接 (%1)").arg(db.connectionSummary());
    m_dbStatusLabel->setText(QString("%1  |  %2").arg(userStr, dbStr));
}

void MainWindow::onStackChanged(int index)
{
    if (index < 0) {
        return;
    }
    QWidget *cur = m_stackedWidget->widget(index);
    if (cur) {
        m_titleLabel->setText(cur->windowTitle());
    }

    bool isLoginPage = (cur == m_loginView);
    // 在登录页禁用返回与退出；在欢迎页仅允许退出不允许返回登录页
    m_logoutBtn->setEnabled(!isLoginPage);
    m_backBtn->setEnabled(index > 1);
}

void MainWindow::goLoginView()
{
    // 清空已有页面栈
    while (m_stackedWidget->count() > 0) {
        QWidget *w = m_stackedWidget->widget(0);
        m_stackedWidget->removeWidget(w);
        w->deleteLater();
    }
    m_welcomeView = nullptr;
    m_patientView = nullptr;
    m_patientEditView = nullptr;
    m_departmentView = nullptr;
    m_doctorView = nullptr;
    m_historyView = nullptr;

    m_loginView = new LoginView(this);
    connect(m_loginView, &LoginView::loginSuccess, this, &MainWindow::goWelcomeView);
    pushWidgetToStack(m_loginView);
    updateFooterInfo();
}

void MainWindow::goWelcomeView()
{
    m_welcomeView = new WelcomeView(this);
    connect(m_welcomeView, &WelcomeView::goDepartmentView, this, &MainWindow::goDepartmentView);
    connect(m_welcomeView, &WelcomeView::goDoctorView, this, &MainWindow::goDoctorView);
    connect(m_welcomeView, &WelcomeView::goPatientView, this, &MainWindow::goPatientView);
    connect(m_welcomeView, &WelcomeView::goHistoryView, this, &MainWindow::goHistoryView);

    pushWidgetToStack(m_welcomeView);
    updateFooterInfo();
}

void MainWindow::goPatientView()
{
    m_patientView = new PatientView(this);
    connect(m_patientView, &PatientView::goPatientEditView, this, &MainWindow::goPatientEditView);
    pushWidgetToStack(m_patientView);
}

void MainWindow::goPatientEditView(int rowIndex, bool isAddMode)
{
    m_patientEditView = new PatientEditView(rowIndex, isAddMode, this);
    connect(m_patientEditView, &PatientEditView::goPreviousView, this, &MainWindow::goPreviousView);
    pushWidgetToStack(m_patientEditView);
}

void MainWindow::goDepartmentView()
{
    m_departmentView = new DepartmentView(this);
    pushWidgetToStack(m_departmentView);
}

void MainWindow::goDoctorView()
{
    m_doctorView = new DoctorView(this);
    pushWidgetToStack(m_doctorView);
}

void MainWindow::goHistoryView()
{
    m_historyView = new HistoryView(this);
    pushWidgetToStack(m_historyView);
}

void MainWindow::goPreviousView()
{
    int count = m_stackedWidget->count();
    if (count <= 2) {
        return;
    }

    QWidget *cur = m_stackedWidget->widget(count - 1);
    // 如果直接点击顶部返回键离开患者编辑页，回滚未提交的编辑
    if (cur == m_patientEditView) {
        IDatabase::getInstance().revertPatientEdit();
        m_patientEditView = nullptr;
    } else if (cur == m_patientView) {
        m_patientView = nullptr;
    } else if (cur == m_departmentView) {
        m_departmentView = nullptr;
    } else if (cur == m_doctorView) {
        m_doctorView = nullptr;
    } else if (cur == m_historyView) {
        m_historyView = nullptr;
    }

    m_stackedWidget->setCurrentIndex(count - 2);
    m_stackedWidget->removeWidget(cur);
    cur->deleteLater();

    // 返回到患者管理页时自动刷新表格数据
    QWidget *nowCur = m_stackedWidget->currentWidget();
    if (nowCur == m_patientView && m_patientView) {
        m_patientView->refreshView();
    } else if (nowCur == m_departmentView && m_departmentView) {
        m_departmentView->refreshView();
    } else if (nowCur == m_doctorView && m_doctorView) {
        m_doctorView->refreshView();
    } else if (nowCur == m_historyView && m_historyView) {
        m_historyView->refreshView();
    }
}

void MainWindow::onLogoutClicked()
{
    if (QMessageBox::question(this, "退出登录",
                              "确定要退出当前账号并返回登录界面吗？",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    IDatabase::getInstance().logout();
    goLoginView();
}

void MainWindow::onSelectDatabaseClicked()
{
    IDatabase &db = IDatabase::getInstance();

    QDialog dlg(this);
    dlg.setWindowTitle("MySQL 数据库连接配置");
    dlg.setMinimumWidth(360);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;

    QLineEdit *hostEdit = new QLineEdit(db.host(), &dlg);
    QSpinBox *portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1, 65535);
    portSpin->setValue(db.port());
    QLineEdit *dbNameEdit = new QLineEdit(db.dbName(), &dlg);
    QLineEdit *userEdit = new QLineEdit(db.dbUser(), &dlg);
    QLineEdit *passEdit = new QLineEdit(db.dbPassword(), &dlg);
    passEdit->setEchoMode(QLineEdit::Password);

    form->addRow("主机地址：", hostEdit);
    form->addRow("端口：", portSpin);
    form->addRow("数据库名：", dbNameEdit);
    form->addRow("用户名：", userEdit);
    form->addRow("密码：", passEdit);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("连接");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    if (db.reconnectDatabase(hostEdit->text(), portSpin->value(),
                             dbNameEdit->text(), userEdit->text(), passEdit->text())) {
        updateFooterInfo();
        QMessageBox::information(this, "连接成功",
                                 "已成功连接 MySQL：\n" + db.connectionSummary());
        goLoginView();
    } else {
        updateFooterInfo();
        QMessageBox::critical(this, "连接失败", db.lastErrorMessage());
    }
}
