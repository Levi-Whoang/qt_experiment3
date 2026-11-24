#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>

class LoginView;
class WelcomeView;
class PatientView;
class PatientEditView;
class DepartmentView;
class DoctorView;
class HistoryView;

// 主控视图（MasterView）：负责顶部导航栏、QStackedWidget 页面栈切换与底部状态栏
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void goLoginView();
    void goWelcomeView();
    void goPatientView();
    void goPatientEditView(int rowIndex, bool isAddMode);
    void goDepartmentView();
    void goDoctorView();
    void goHistoryView();
    void goPreviousView();

private slots:
    void onLogoutClicked();
    void onSelectDatabaseClicked();
    void onStackChanged(int index);

private:
    void initUI();
    void applyStyleSheet();
    void pushWidgetToStack(QWidget *widget);
    void updateFooterInfo();

    QPushButton *m_backBtn;
    QLabel *m_titleLabel;
    QPushButton *m_logoutBtn;

    QStackedWidget *m_stackedWidget;

    QLabel *m_dbStatusLabel;
    QPushButton *m_dbSwitchBtn;
    QLabel *m_developerLabel;

    LoginView *m_loginView = nullptr;
    WelcomeView *m_welcomeView = nullptr;
    PatientView *m_patientView = nullptr;
    PatientEditView *m_patientEditView = nullptr;
    DepartmentView *m_departmentView = nullptr;
    DoctorView *m_doctorView = nullptr;
    HistoryView *m_historyView = nullptr;
};

#endif // MAINWINDOW_H
