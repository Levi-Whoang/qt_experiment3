#ifndef MANAGEMENTVIEWS_H
#define MANAGEMENTVIEWS_H

#include <QWidget>
#include <QLineEdit>
#include <QTableView>
#include <QLabel>

// 科室管理视图（选做）
class DepartmentView : public QWidget
{
    Q_OBJECT
public:
    explicit DepartmentView(QWidget *parent = nullptr);
    void refreshView();

private slots:
    void onSearchClicked();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

private:
    void updateStatusLabel();
    QLineEdit *m_searchEdit;
    QTableView *m_tableView;
    QLabel *m_statusLabel;
};

// 医生管理视图（选做）
class DoctorView : public QWidget
{
    Q_OBJECT
public:
    explicit DoctorView(QWidget *parent = nullptr);
    void refreshView();

private slots:
    void onSearchClicked();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

private:
    void updateStatusLabel();
    QLineEdit *m_searchEdit;
    QTableView *m_tableView;
    QLabel *m_statusLabel;
};

// 操作记录视图（选做）
class HistoryView : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryView(QWidget *parent = nullptr);
    void refreshView();

private slots:
    void onSearchClicked();

private:
    void updateStatusLabel();
    QLineEdit *m_searchEdit;
    QTableView *m_tableView;
    QLabel *m_statusLabel;
};

#endif // MANAGEMENTVIEWS_H
