#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QWidget>
#include <QLabel>

class WelcomeView : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomeView(QWidget *parent = nullptr);
    void refreshGreeting();

signals:
    void goDepartmentView();
    void goDoctorView();
    void goPatientView();
    void goHistoryView();

private:
    QLabel *m_greetingLabel;
};

#endif // WELCOMEVIEW_H
