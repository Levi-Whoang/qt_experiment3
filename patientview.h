#ifndef PATIENTVIEW_H
#define PATIENTVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QTableView>
#include <QLabel>

class PatientView : public QWidget
{
    Q_OBJECT
public:
    explicit PatientView(QWidget *parent = nullptr);
    void refreshView();

signals:
    void goPatientEditView(int rowIndex, bool isAddMode);

private slots:
    void onSearchClicked();
    void onAddClicked();
    void onDeleteClicked();
    void onEditClicked();

private:
    void updateStatusLabel(const QString &keyword = QString());

    QLineEdit *m_searchEdit;
    QTableView *m_tableView;
    QLabel *m_statusLabel;
};

#endif // PATIENTVIEW_H
