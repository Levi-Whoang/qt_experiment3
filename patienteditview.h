#ifndef PATIENTEDITVIEW_H
#define PATIENTEDITVIEW_H

#include <QWidget>
#include <QComboBox>
#include <QDataWidgetMapper>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>

class PatientEditView : public QWidget
{
    Q_OBJECT
public:
    explicit PatientEditView(int rowIndex, bool isAddMode, QWidget *parent = nullptr);

signals:
    void goPreviousView();

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    int m_rowIndex;
    bool m_isAddMode;
    QString m_originalId;

    QDataWidgetMapper *m_mapper;
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_idCardEdit;
    QComboBox *m_sexCombo;
    QDateEdit *m_dobEdit;
    QDoubleSpinBox *m_heightSpin;
    QDoubleSpinBox *m_weightSpin;
    QLineEdit *m_phoneEdit;
};

#endif // PATIENTEDITVIEW_H
