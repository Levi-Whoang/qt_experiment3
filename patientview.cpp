#include "patientview.h"
#include "idatabase.h"
#include "sexdelegate.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlRecord>
#include <QVBoxLayout>

PatientView::PatientView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("患者管理");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    // 顶部工具栏：搜索框 + 查找 + 添加 + 删除 + 修改（与报告图3一致）
    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("请输入患者姓名、ID、身份证号或手机号...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(34);

    QPushButton *btnSearch = new QPushButton("查找", this);
    btnSearch->setObjectName("primaryButton");
    btnSearch->setMinimumSize(82, 34);
    btnSearch->setCursor(Qt::PointingHandCursor);

    QPushButton *btnAdd = new QPushButton("添加", this);
    btnAdd->setObjectName("successButton");
    btnAdd->setMinimumSize(82, 34);
    btnAdd->setCursor(Qt::PointingHandCursor);

    QPushButton *btnDelete = new QPushButton("删除", this);
    btnDelete->setObjectName("dangerButton");
    btnDelete->setMinimumSize(82, 34);
    btnDelete->setCursor(Qt::PointingHandCursor);

    QPushButton *btnEdit = new QPushButton("修改", this);
    btnEdit->setObjectName("secondaryButton");
    btnEdit->setMinimumSize(82, 34);
    btnEdit->setCursor(Qt::PointingHandCursor);

    toolLayout->addWidget(m_searchEdit, 1);
    toolLayout->addWidget(btnSearch);
    toolLayout->addWidget(btnAdd);
    toolLayout->addWidget(btnDelete);
    toolLayout->addWidget(btnEdit);

    mainLayout->addLayout(toolLayout);

    // 患者列表 TableView
    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSortingEnabled(true);

    IDatabase &db = IDatabase::getInstance();
    if (db.initPatientModel()) {
        m_tableView->setModel(db.patientTabModel);
        m_tableView->setSelectionModel(db.thePatientSelection);

        int sexCol = db.patientTabModel->fieldIndex("SEX");
        if (sexCol >= 0) {
            m_tableView->setItemDelegateForColumn(sexCol, new SexDelegate(this));
        }

        m_tableView->setColumnWidth(0, 90);
        m_tableView->setColumnWidth(1, 165);
        m_tableView->setColumnWidth(2, 85);
        m_tableView->setColumnWidth(3, 60);
        m_tableView->setColumnWidth(4, 100);
        m_tableView->setColumnWidth(5, 80);
        m_tableView->setColumnWidth(6, 80);
        m_tableView->setColumnWidth(7, 120);
        m_tableView->setColumnWidth(8, 60);
    }

    mainLayout->addWidget(m_tableView, 1);

    // 底部记录统计/查询状态提示
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("tableStatusLabel");
    mainLayout->addWidget(m_statusLabel);

    connect(btnSearch, &QPushButton::clicked, this, &PatientView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PatientView::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.isEmpty()) {
            IDatabase::getInstance().searchPatient("");
            updateStatusLabel("");
        }
    });
    connect(btnAdd, &QPushButton::clicked, this, &PatientView::onAddClicked);
    connect(btnDelete, &QPushButton::clicked, this, &PatientView::onDeleteClicked);
    connect(btnEdit, &QPushButton::clicked, this, &PatientView::onEditClicked);
    connect(m_tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &idx) {
        if (idx.isValid()) {
            emit goPatientEditView(idx.row(), false);
        }
    });

    updateStatusLabel();
}

void PatientView::refreshView()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.isDatabaseOpen()) {
        m_statusLabel->setText("数据库未连接，无法加载患者数据。");
        return;
    }
    db.searchPatient(m_searchEdit->text());
    updateStatusLabel(m_searchEdit->text());
}

void PatientView::updateStatusLabel(const QString &keyword)
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.patientTabModel) {
        m_statusLabel->setText("数据模型未就绪");
        return;
    }
    int count = db.patientTabModel->rowCount();
    if (count == 0 && !keyword.trimmed().isEmpty()) {
        m_statusLabel->setText(QString("未找到与 \"%1\" 匹配的患者记录（清空搜索框可恢复全部列表）。")
                                   .arg(keyword.trimmed()));
    } else if (count == 0) {
        m_statusLabel->setText("当前暂无患者记录，点击“添加”新增患者。");
    } else {
        m_statusLabel->setText(QString("共显示 %1 条患者记录（双击某行或点击“修改”可编辑详情）。").arg(count));
    }
}

void PatientView::onSearchClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.isDatabaseOpen()) {
        QMessageBox::critical(this, "数据库错误", "数据库未连接，无法查询！");
        return;
    }
    QString keyword = m_searchEdit->text().trimmed();
    db.searchPatient(keyword);
    updateStatusLabel(keyword);

    if (db.patientTabModel && db.patientTabModel->rowCount() == 0 && !keyword.isEmpty()) {
        QMessageBox::information(this, "查询结果",
                                 QString("未找到包含 \"%1\" 的患者信息！").arg(keyword));
    }
}

void PatientView::onAddClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.isDatabaseOpen()) {
        QMessageBox::critical(this, "数据库错误", "数据库未连接，无法新增！");
        return;
    }
    // 清除过滤以便在完整模型中新增
    if (!m_searchEdit->text().isEmpty()) {
        m_searchEdit->clear();
        db.searchPatient("");
    }
    int newRow = db.addNewPatient();
    if (newRow >= 0) {
        emit goPatientEditView(newRow, true);
    }
}

void PatientView::onDeleteClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.thePatientSelection || !db.thePatientSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先在表格中选择要删除的患者！");
        return;
    }

    int row = db.thePatientSelection->currentIndex().row();
    QSqlRecord rec = db.patientTabModel->record(row);
    QString name = rec.value("NAME").toString();
    QString id = rec.value("ID").toString();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除患者【%1】(ID: %2) 吗？此操作不可撤销。").arg(name, id),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QString errMsg;
    if (db.deleteCurrentPatient(errMsg)) {
        updateStatusLabel(m_searchEdit->text());
        QMessageBox::information(this, "删除成功", "患者记录已成功删除！");
    } else {
        QMessageBox::critical(this, "删除失败", errMsg);
    }
}

void PatientView::onEditClicked()
{
    IDatabase &db = IDatabase::getInstance();
    if (!db.thePatientSelection || !db.thePatientSelection->currentIndex().isValid()) {
        QMessageBox::warning(this, "提示", "请先在表格中选择要修改的患者！");
        return;
    }
    int row = db.thePatientSelection->currentIndex().row();
    emit goPatientEditView(row, false);
}
