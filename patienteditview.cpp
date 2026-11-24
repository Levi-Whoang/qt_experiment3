#include "patienteditview.h"
#include "idatabase.h"

#include <QDate>
#include <QDateTime>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlRecord>
#include <QVBoxLayout>

PatientEditView::PatientEditView(int rowIndex, bool isAddMode, QWidget *parent)
    : QWidget(parent),
      m_rowIndex(rowIndex),
      m_isAddMode(isAddMode),
      m_mapper(new QDataWidgetMapper(this))
{
    setWindowTitle(isAddMode ? "新增患者信息" : "编辑患者信息");

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->addStretch(1);

    // 中间卡片（布局参考报告图4：左侧表单，右侧保存/取消按钮）
    QWidget *card = new QWidget(this);
    card->setObjectName("editFormCard");
    card->setFixedWidth(560);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(32, 28, 32, 28);
    cardLayout->setSpacing(24);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setSpacing(12);

    m_idEdit = new QLineEdit(card);
    m_idEdit->setReadOnly(true);
    m_idEdit->setPlaceholderText(isAddMode ? "保存时自动生成" : "");
    m_idEdit->setMinimumHeight(32);

    m_nameEdit = new QLineEdit(card);
    m_nameEdit->setPlaceholderText("必填，请输入患者姓名");
    m_nameEdit->setMinimumHeight(32);

    m_idCardEdit = new QLineEdit(card);
    m_idCardEdit->setPlaceholderText("请输入18位身份证号");
    m_idCardEdit->setMaxLength(20);
    m_idCardEdit->setMinimumHeight(32);

    m_sexCombo = new QComboBox(card);
    m_sexCombo->addItem("男", 1);
    m_sexCombo->addItem("女", 0);
    m_sexCombo->setMinimumHeight(32);

    m_dobEdit = new QDateEdit(card);
    m_dobEdit->setCalendarPopup(true);
    m_dobEdit->setDisplayFormat("yyyy/M/d");
    m_dobEdit->setDateRange(QDate(1900, 1, 1), QDate::currentDate());
    m_dobEdit->setMinimumHeight(32);

    m_heightSpin = new QDoubleSpinBox(card);
    m_heightSpin->setRange(0.0, 260.0);
    m_heightSpin->setDecimals(1);
    m_heightSpin->setSuffix(" cm");
    m_heightSpin->setMinimumHeight(32);

    m_weightSpin = new QDoubleSpinBox(card);
    m_weightSpin->setRange(0.0, 300.0);
    m_weightSpin->setDecimals(1);
    m_weightSpin->setSuffix(" kg");
    m_weightSpin->setMinimumHeight(32);

    m_phoneEdit = new QLineEdit(card);
    m_phoneEdit->setPlaceholderText("请输入11位手机号");
    m_phoneEdit->setMaxLength(20);
    m_phoneEdit->setMinimumHeight(32);

    formLayout->addRow("ID：", m_idEdit);
    formLayout->addRow("姓名：", m_nameEdit);
    formLayout->addRow("身份证：", m_idCardEdit);
    formLayout->addRow("性别：", m_sexCombo);
    formLayout->addRow("出生日期：", m_dobEdit);
    formLayout->addRow("身高：", m_heightSpin);
    formLayout->addRow("体重：", m_weightSpin);
    formLayout->addRow("手机号：", m_phoneEdit);

    cardLayout->addLayout(formLayout, 1);

    // 右侧底部对齐的保存 / 取消按钮（与图4一致）
    QVBoxLayout *btnLayout = new QVBoxLayout;
    btnLayout->addStretch();

    QPushButton *btnSave = new QPushButton("保存", card);
    btnSave->setObjectName("primaryButton");
    btnSave->setFixedSize(108, 36);
    btnSave->setCursor(Qt::PointingHandCursor);

    QPushButton *btnCancel = new QPushButton("取消", card);
    btnCancel->setObjectName("secondaryButton");
    btnCancel->setFixedSize(108, 36);
    btnCancel->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(btnSave);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(btnCancel);

    cardLayout->addLayout(btnLayout);

    outerLayout->addWidget(card, 0, Qt::AlignHCenter);
    outerLayout->addStretch(2);

    // 绑定 QDataWidgetMapper 到 QSqlTableModel
    IDatabase &db = IDatabase::getInstance();
    QSqlTableModel *tabModel = db.patientTabModel;
    if (tabModel) {
        m_mapper->setModel(tabModel);
        m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

        if (!m_isAddMode) {
            m_mapper->addMapping(m_idEdit, tabModel->fieldIndex("ID"));
        }
        m_mapper->addMapping(m_nameEdit, tabModel->fieldIndex("NAME"));
        m_mapper->addMapping(m_idCardEdit, tabModel->fieldIndex("ID_CARD"));
        m_mapper->addMapping(m_heightSpin, tabModel->fieldIndex("HEIGHT"));
        m_mapper->addMapping(m_weightSpin, tabModel->fieldIndex("WEIGHT"));
        m_mapper->addMapping(m_phoneEdit, tabModel->fieldIndex(db.phoneColumnName()));

        m_mapper->setCurrentIndex(m_rowIndex);

        QSqlRecord rec = tabModel->record(m_rowIndex);
        m_originalId = m_isAddMode ? QString() : rec.value("ID").toString();

        int sexVal = rec.value("SEX").isNull() ? 1 : rec.value("SEX").toInt();
        int sexIdx = m_sexCombo->findData(sexVal);
        m_sexCombo->setCurrentIndex(sexIdx >= 0 ? sexIdx : 0);

        QDate dob = rec.value("DOB").toDate();
        if (!dob.isValid()) {
            dob = QDate::fromString(rec.value("DOB").toString(), "yyyy-MM-dd");
        }
        if (!dob.isValid()) {
            dob = QDate(2000, 1, 1);
        }
        m_dobEdit->setDate(dob);
    }

    connect(btnSave, &QPushButton::clicked, this, &PatientEditView::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &PatientEditView::onCancelClicked);
}

void PatientEditView::onSaveClicked()
{
    QString name = m_nameEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "字段校验", "患者姓名不能为空！");
        m_nameEdit->setFocus();
        return;
    }

    IDatabase &db = IDatabase::getInstance();

    // 同步 QDataWidgetMapper 控件数据到 Model
    m_mapper->submit();

    QSqlTableModel *tabModel = db.patientTabModel;
    if (tabModel) {
        QDate dob = m_dobEdit->date();
        int age = QDate::currentDate().year() - dob.year();
        if (QDate::currentDate().dayOfYear() < dob.dayOfYear()) {
            age--;
        }
        if (age < 0) {
            age = 0;
        }

        tabModel->setData(tabModel->index(m_rowIndex, tabModel->fieldIndex("NAME")), name);
        tabModel->setData(tabModel->index(m_rowIndex, tabModel->fieldIndex("SEX")), m_sexCombo->currentData());
        tabModel->setData(tabModel->index(m_rowIndex, tabModel->fieldIndex("DOB")), dob.toString("yyyy-MM-dd"));
        tabModel->setData(tabModel->index(m_rowIndex, tabModel->fieldIndex("AGE")), age);

        if (m_isAddMode) {
            tabModel->setData(tabModel->index(m_rowIndex, tabModel->fieldIndex("CREATEDTIMESTAMP")),
                              QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        }
    }

    if (!db.submitPatientEdit()) {
        QMessageBox::critical(this, "保存失败",
                              "写入数据库失败：\n" + db.lastErrorMessage());
        return;
    }

    db.logEvent(QString("%1患者：%2").arg(m_isAddMode ? "新增" : "修改", name));
    QMessageBox::information(this, "保存成功",
                             m_isAddMode ? "新增患者信息成功！" : "修改患者信息成功！");
    emit goPreviousView();
}

void PatientEditView::onCancelClicked()
{
    IDatabase::getInstance().revertPatientEdit();
    emit goPreviousView();
}
