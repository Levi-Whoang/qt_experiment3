#include "managementviews.h"
#include "idatabase.h"
#include "sexdelegate.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVBoxLayout>

// ==================== DepartmentView ====================

DepartmentView::DepartmentView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("科室管理");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("请输入科室编号或名称...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(34);

    QPushButton *btnSearch = new QPushButton("查找", this);
    btnSearch->setObjectName("primaryButton");
    btnSearch->setMinimumSize(82, 34);

    QPushButton *btnAdd = new QPushButton("添加", this);
    btnAdd->setObjectName("successButton");
    btnAdd->setMinimumSize(82, 34);

    QPushButton *btnDelete = new QPushButton("删除", this);
    btnDelete->setObjectName("dangerButton");
    btnDelete->setMinimumSize(82, 34);

    QPushButton *btnEdit = new QPushButton("修改", this);
    btnEdit->setObjectName("secondaryButton");
    btnEdit->setMinimumSize(82, 34);

    toolLayout->addWidget(m_searchEdit, 1);
    toolLayout->addWidget(btnSearch);
    toolLayout->addWidget(btnAdd);
    toolLayout->addWidget(btnDelete);
    toolLayout->addWidget(btnEdit);
    mainLayout->addLayout(toolLayout);

    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    IDatabase &db = IDatabase::getInstance();
    if (db.initDepartmentModel()) {
        m_tableView->setModel(db.departmentTabModel);
        m_tableView->setSelectionModel(db.theDepartmentSelection);
    }

    mainLayout->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("tableStatusLabel");
    mainLayout->addWidget(m_statusLabel);

    connect(btnSearch, &QPushButton::clicked, this, &DepartmentView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &DepartmentView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &t) {
        if (t.isEmpty()) {
            IDatabase::getInstance().searchDepartment("");
            updateStatusLabel();
        }
    });
    connect(btnAdd, &QPushButton::clicked, this, &DepartmentView::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &DepartmentView::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &DepartmentView::onDeleteClicked);

    updateStatusLabel();
}

void DepartmentView::refreshView()
{
    IDatabase::getInstance().searchDepartment(m_searchEdit->text());
    updateStatusLabel();
}

void DepartmentView::updateStatusLabel()
{
    IDatabase &db = IDatabase::getInstance();
    int count = db.departmentTabModel ? db.departmentTabModel->rowCount() : 0;
    m_statusLabel->setText(count > 0 ? QString("共 %1 个科室。").arg(count)
                                     : "暂无符合条件的科室记录。");
}

void DepartmentView::onSearchClicked()
{
    IDatabase::getInstance().searchDepartment(m_searchEdit->text());
    updateStatusLabel();
}

void DepartmentView::onAddClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("新增科室");
    dlg.setMinimumWidth(320);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;
    QLineEdit *idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("例如：D005");
    QLineEdit *nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("例如：眼科");

    form->addRow("科室编号：", idEdit);
    form->addRow("科室名称：", nameEdit);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("保存");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString id = idEdit->text().trimmed();
    QString name = nameEdit->text().trimmed();
    if (id.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "提示", "科室编号和名称均不能为空！");
        return;
    }

    QString err;
    if (IDatabase::getInstance().addDepartment(id, name, err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "新增科室成功！");
    } else {
        QMessageBox::warning(this, "新增失败", err);
    }
}

void DepartmentView::onEditClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.theDepartmentSelection || !db.theDepartmentSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要修改的科室！");
        return;
    }

    int row = db.theDepartmentSelection->currentIndex().row();
    QSqlRecord rec = db.departmentTabModel->record(row);
    QString oldId = rec.value("ID").toString();

    QDialog dlg(this);
    dlg.setWindowTitle("修改科室");
    dlg.setMinimumWidth(320);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;
    QLineEdit *idEdit = new QLineEdit(oldId, &dlg);
    QLineEdit *nameEdit = new QLineEdit(rec.value("NAME").toString(), &dlg);

    form->addRow("科室编号：", idEdit);
    form->addRow("科室名称：", nameEdit);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("保存");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString id = idEdit->text().trimmed();
    QString name = nameEdit->text().trimmed();
    if (id.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "提示", "科室编号和名称均不能为空！");
        return;
    }

    QString err;
    if (db.updateDepartment(oldId, id, name, err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "修改科室成功！");
    } else {
        QMessageBox::warning(this, "修改失败", err);
    }
}

void DepartmentView::onDeleteClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.theDepartmentSelection || !db.theDepartmentSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的科室！");
        return;
    }

    int row = db.theDepartmentSelection->currentIndex().row();
    QSqlRecord rec = db.departmentTabModel->record(row);
    QString id = rec.value("ID").toString();
    QString name = rec.value("NAME").toString();

    if (QMessageBox::question(this, "确认删除",
                              QString("确定删除科室【%1】(%2) 吗？").arg(name, id),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QString err;
    if (db.deleteDepartment(id, err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "科室已删除！");
    } else {
        QMessageBox::warning(this, "删除失败", err);
    }
}

// ==================== DoctorView ====================

DoctorView::DoctorView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("医生管理");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("请输入医生编号、工号或姓名...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(34);

    QPushButton *btnSearch = new QPushButton("查找", this);
    btnSearch->setObjectName("primaryButton");
    btnSearch->setMinimumSize(82, 34);

    QPushButton *btnAdd = new QPushButton("添加", this);
    btnAdd->setObjectName("successButton");
    btnAdd->setMinimumSize(82, 34);

    QPushButton *btnDelete = new QPushButton("删除", this);
    btnDelete->setObjectName("dangerButton");
    btnDelete->setMinimumSize(82, 34);

    QPushButton *btnEdit = new QPushButton("修改", this);
    btnEdit->setObjectName("secondaryButton");
    btnEdit->setMinimumSize(82, 34);

    toolLayout->addWidget(m_searchEdit, 1);
    toolLayout->addWidget(btnSearch);
    toolLayout->addWidget(btnAdd);
    toolLayout->addWidget(btnDelete);
    toolLayout->addWidget(btnEdit);
    mainLayout->addLayout(toolLayout);

    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    IDatabase &db = IDatabase::getInstance();
    if (db.initDoctorModel()) {
        m_tableView->setModel(db.doctorTabModel);
        m_tableView->setSelectionModel(db.theDoctorSelection);
        int deptCol = db.doctorTabModel->fieldIndex("DEPARTMENT_ID");
        if (deptCol >= 0) {
            m_tableView->setItemDelegateForColumn(deptCol, new DepartmentDelegate(this));
        }
    }

    mainLayout->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("tableStatusLabel");
    mainLayout->addWidget(m_statusLabel);

    connect(btnSearch, &QPushButton::clicked, this, &DoctorView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &DoctorView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &t) {
        if (t.isEmpty()) {
            IDatabase::getInstance().searchDoctor("");
            updateStatusLabel();
        }
    });
    connect(btnAdd, &QPushButton::clicked, this, &DoctorView::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &DoctorView::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &DoctorView::onDeleteClicked);

    updateStatusLabel();
}

void DoctorView::refreshView()
{
    IDatabase::getInstance().searchDoctor(m_searchEdit->text());
    updateStatusLabel();
}

void DoctorView::updateStatusLabel()
{
    IDatabase &db = IDatabase::getInstance();
    int count = db.doctorTabModel ? db.doctorTabModel->rowCount() : 0;
    m_statusLabel->setText(count > 0 ? QString("共 %1 位医生。").arg(count)
                                     : "暂无符合条件的医生记录。");
}

void DoctorView::onSearchClicked()
{
    IDatabase::getInstance().searchDoctor(m_searchEdit->text());
    updateStatusLabel();
}

static void populateDepartmentCombo(QComboBox *combo, const QString &currentDeptId = QString())
{
    combo->addItem("未分配", "");
    QSqlQuery q("SELECT ID, NAME FROM Department ORDER BY ID");
    while (q.next()) {
        QString id = q.value(0).toString();
        QString name = q.value(1).toString();
        combo->addItem(QString("%1 (%2)").arg(name, id), id);
    }
    int idx = combo->findData(currentDeptId);
    if (idx >= 0) {
        combo->setCurrentIndex(idx);
    }
}

void DoctorView::onAddClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("新增医生");
    dlg.setMinimumWidth(340);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;

    QLineEdit *idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("例如：DOC004");
    QLineEdit *empNoEdit = new QLineEdit(&dlg);
    empNoEdit->setPlaceholderText("例如：E2026004");
    QLineEdit *nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("请输入医生姓名");
    QComboBox *deptCombo = new QComboBox(&dlg);
    populateDepartmentCombo(deptCombo);

    form->addRow("医生编号：", idEdit);
    form->addRow("工号：", empNoEdit);
    form->addRow("姓名：", nameEdit);
    form->addRow("所属科室：", deptCombo);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("保存");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString id = idEdit->text().trimmed();
    QString empNo = empNoEdit->text().trimmed();
    QString name = nameEdit->text().trimmed();
    if (id.isEmpty() || empNo.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "提示", "医生编号、工号和姓名均不能为空！");
        return;
    }

    QString err;
    if (IDatabase::getInstance().addDoctor(id, empNo, name, deptCombo->currentData().toString(), err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "新增医生成功！");
    } else {
        QMessageBox::warning(this, "新增失败", err);
    }
}

void DoctorView::onEditClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.theDoctorSelection || !db.theDoctorSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要修改的医生！");
        return;
    }

    int row = db.theDoctorSelection->currentIndex().row();
    QSqlRecord rec = db.doctorTabModel->record(row);
    QString oldId = rec.value("ID").toString();

    QDialog dlg(this);
    dlg.setWindowTitle("修改医生信息");
    dlg.setMinimumWidth(340);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout;

    QLineEdit *idEdit = new QLineEdit(oldId, &dlg);
    QLineEdit *empNoEdit = new QLineEdit(rec.value("EMPLOYEENO").toString(), &dlg);
    QLineEdit *nameEdit = new QLineEdit(rec.value("NAME").toString(), &dlg);
    QComboBox *deptCombo = new QComboBox(&dlg);
    populateDepartmentCombo(deptCombo, rec.value("DEPARTMENT_ID").toString());

    form->addRow("医生编号：", idEdit);
    form->addRow("工号：", empNoEdit);
    form->addRow("姓名：", nameEdit);
    form->addRow("所属科室：", deptCombo);
    layout->addLayout(form);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Ok)->setText("保存");
    btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString id = idEdit->text().trimmed();
    QString empNo = empNoEdit->text().trimmed();
    QString name = nameEdit->text().trimmed();
    if (id.isEmpty() || empNo.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "提示", "医生编号、工号和姓名均不能为空！");
        return;
    }

    QString err;
    if (db.updateDoctor(oldId, id, empNo, name, deptCombo->currentData().toString(), err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "修改医生成功！");
    } else {
        QMessageBox::warning(this, "修改失败", err);
    }
}

void DoctorView::onDeleteClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.theDoctorSelection || !db.theDoctorSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的医生！");
        return;
    }

    int row = db.theDoctorSelection->currentIndex().row();
    QSqlRecord rec = db.doctorTabModel->record(row);
    QString id = rec.value("ID").toString();
    QString name = rec.value("NAME").toString();

    if (QMessageBox::question(this, "确认删除",
                              QString("确定删除医生【%1】(%2) 吗？").arg(name, id),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QString err;
    if (db.deleteDoctor(id, err)) {
        updateStatusLabel();
        QMessageBox::information(this, "成功", "医生已删除！");
    } else {
        QMessageBox::warning(this, "删除失败", err);
    }
}

// ==================== HistoryView ====================

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("操作记录");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按用户ID、操作内容或时间搜索...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(34);

    QPushButton *btnSearch = new QPushButton("查找", this);
    btnSearch->setObjectName("primaryButton");
    btnSearch->setMinimumSize(82, 34);

    QPushButton *btnRefresh = new QPushButton("刷新", this);
    btnRefresh->setObjectName("secondaryButton");
    btnRefresh->setMinimumSize(82, 34);

    toolLayout->addWidget(m_searchEdit, 1);
    toolLayout->addWidget(btnSearch);
    toolLayout->addWidget(btnRefresh);
    mainLayout->addLayout(toolLayout);

    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    IDatabase &db = IDatabase::getInstance();
    if (db.initHistoryModel()) {
        m_tableView->setModel(db.historyTabModel);
    }

    mainLayout->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("tableStatusLabel");
    mainLayout->addWidget(m_statusLabel);

    connect(btnSearch, &QPushButton::clicked, this, &HistoryView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &HistoryView::onSearchClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &HistoryView::refreshView);

    updateStatusLabel();
}

void HistoryView::refreshView()
{
    IDatabase::getInstance().searchHistory(m_searchEdit->text());
    updateStatusLabel();
}

void HistoryView::onSearchClicked()
{
    IDatabase::getInstance().searchHistory(m_searchEdit->text());
    updateStatusLabel();
}

void HistoryView::updateStatusLabel()
{
    IDatabase &db = IDatabase::getInstance();
    int count = db.historyTabModel ? db.historyTabModel->rowCount() : 0;
    m_statusLabel->setText(QString("共记录 %1 条用户操作日志。").arg(count));
}
