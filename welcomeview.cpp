#include "welcomeview.h"
#include "idatabase.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

WelcomeView::WelcomeView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("欢迎");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch(2);

    m_greetingLabel = new QLabel("欢迎使用医院诊疗系统，请选择功能模块", this);
    m_greetingLabel->setAlignment(Qt::AlignCenter);
    m_greetingLabel->setObjectName("welcomeGreetingLabel");
    mainLayout->addWidget(m_greetingLabel);

    mainLayout->addSpacing(36);

    QHBoxLayout *gridLayout = new QHBoxLayout;
    gridLayout->setSpacing(28);
    gridLayout->addStretch();

    auto createModuleButton = [this](const QString &title, const QString &subtitle) -> QPushButton * {
        QPushButton *btn = new QPushButton(title + "\n" + subtitle, this);
        btn->setObjectName("moduleCardButton");
        btn->setFixedSize(165, 145);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    QPushButton *btnDepartment = createModuleButton("科室管理", "Department");
    QPushButton *btnDoctor = createModuleButton("医生管理", "Doctor");
    QPushButton *btnPatient = createModuleButton("患者管理", "Patient");
    QPushButton *btnHistory = createModuleButton("操作记录", "History");

    gridLayout->addWidget(btnDepartment);
    gridLayout->addWidget(btnDoctor);
    gridLayout->addWidget(btnPatient);
    gridLayout->addWidget(btnHistory);
    gridLayout->addStretch();

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch(3);

    connect(btnDepartment, &QPushButton::clicked, this, &WelcomeView::goDepartmentView);
    connect(btnDoctor, &QPushButton::clicked, this, &WelcomeView::goDoctorView);
    connect(btnPatient, &QPushButton::clicked, this, &WelcomeView::goPatientView);
    connect(btnHistory, &QPushButton::clicked, this, &WelcomeView::goHistoryView);

    refreshGreeting();
}

void WelcomeView::refreshGreeting()
{
    QString name = IDatabase::getInstance().currentUserFullName();
    if (name.isEmpty()) {
        m_greetingLabel->setText("欢迎使用医院诊疗系统，请选择管理模块");
    } else {
        m_greetingLabel->setText(QString("欢迎回来，%1！请选择要进入的管理模块").arg(name));
    }
}
