#include "idatabase.h"

#include <QDate>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

IDatabase::IDatabase(QObject *parent)
    : QObject(parent),
      m_host("localhost"),
      m_port(3306),
      m_dbName("hospital"),
      m_dbUser("root"),
      m_dbPassword("027410")
{
    initDatabase();
}

bool IDatabase::initDatabase()
{
    m_lastError.clear();

    if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        m_lastError = "未加载 QMYSQL 驱动，请确认程序目录下存在 sqldrivers/qsqlmysql.dll 及 libmysql.dll。";
        qWarning() << m_lastError;
        return false;
    }

    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_database = QSqlDatabase::database("qt_sql_default_connection");
        if (m_database.isOpen()) {
            m_database.close();
        }
    } else {
        m_database = QSqlDatabase::addDatabase("QMYSQL");
    }

    m_database.setHostName(m_host);
    m_database.setPort(m_port);
    m_database.setUserName(m_dbUser);
    m_database.setPassword(m_dbPassword);
    m_database.setDatabaseName(m_dbName);

    if (!m_database.open()) {
        QString firstErr = m_database.lastError().text();
        m_database.setDatabaseName("");
        if (m_database.open()) {
            QSqlQuery initDbQuery(m_database);
            QString safeDb = m_dbName;
            safeDb.replace("`", "");
            if (initDbQuery.exec(QString("CREATE DATABASE IF NOT EXISTS `%1` "
                                         "DEFAULT CHARACTER SET utf8mb4 "
                                         "DEFAULT COLLATE utf8mb4_unicode_ci")
                                     .arg(safeDb))) {
                m_database.close();
                m_database.setDatabaseName(m_dbName);
                if (!m_database.open()) {
                    m_lastError = QString("连接 MySQL 失败 (%1:%2/%3)：%4")
                                      .arg(m_host)
                                      .arg(m_port)
                                      .arg(m_dbName, m_database.lastError().text());
                    return false;
                }
            } else {
                m_lastError = QString("自动创建数据库 `%1` 失败：%2")
                                  .arg(safeDb, initDbQuery.lastError().text());
                m_database.close();
                return false;
            }
        } else {
            m_lastError = QString("连接 MySQL 失败 (%1:%2/%3)：%4")
                              .arg(m_host)
                              .arg(m_port)
                              .arg(m_dbName, firstErr);
            qWarning() << m_lastError;
            return false;
        }
    }

    if (!createTablesIfNeeded()) {
        return false;
    }

    initPatientModel();
    initDepartmentModel();
    initDoctorModel();
    initHistoryModel();

    return true;
}

bool IDatabase::reconnectDatabase(const QString &host, int port, const QString &dbName,
                                  const QString &user, const QString &password)
{
    m_host = host.trimmed();
    m_port = port;
    m_dbName = dbName.trimmed();
    m_dbUser = user.trimmed();
    m_dbPassword = password;
    return initDatabase();
}

bool IDatabase::isDatabaseOpen() const
{
    return m_database.isValid() && m_database.isOpen();
}

QString IDatabase::connectionSummary() const
{
    return QString("%1@%2:%3/%4").arg(m_dbUser, m_host).arg(m_port).arg(m_dbName);
}

QString IDatabase::lastErrorMessage() const
{
    return m_lastError;
}

bool IDatabase::createTablesIfNeeded()
{
    QSqlQuery query(m_database);

    // 与现有 hospital 库中的 user / patient 表结构保持完全兼容（INT AUTO_INCREMENT）
    const QStringList statements = {
        R"(CREATE TABLE IF NOT EXISTS `user` (
            ID INT PRIMARY KEY AUTO_INCREMENT,
            FULLNAME VARCHAR(50) NOT NULL,
            USERNAME VARCHAR(50) NOT NULL UNIQUE,
            PASSWORD VARCHAR(100) NOT NULL
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)",
        R"(CREATE TABLE IF NOT EXISTS `patient` (
            ID INT PRIMARY KEY AUTO_INCREMENT,
            ID_CARD VARCHAR(20),
            NAME VARCHAR(50) NOT NULL,
            SEX INT DEFAULT 1,
            DOB DATE,
            HEIGHT DOUBLE DEFAULT 0,
            WEIGHT DOUBLE DEFAULT 0,
            MOBILPHONE VARCHAR(20),
            AGE INT DEFAULT 0,
            CREATEDTIMESTAMP DATETIME DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)",
        R"(CREATE TABLE IF NOT EXISTS `department` (
            ID VARCHAR(32) PRIMARY KEY,
            NAME VARCHAR(50) NOT NULL UNIQUE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)",
        R"(CREATE TABLE IF NOT EXISTS `doctor` (
            ID VARCHAR(32) PRIMARY KEY,
            EMPLOYEENO VARCHAR(32) NOT NULL UNIQUE,
            NAME VARCHAR(50) NOT NULL,
            DEPARTMENT_ID VARCHAR(32),
            CONSTRAINT fk_doctor_dept
                FOREIGN KEY (DEPARTMENT_ID) REFERENCES `department`(ID)
                ON DELETE SET NULL ON UPDATE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)",
        R"(CREATE TABLE IF NOT EXISTS `history` (
            ID INT PRIMARY KEY AUTO_INCREMENT,
            USER_ID INT,
            EVENT VARCHAR(200) NOT NULL,
            TIMESTAMP DATETIME DEFAULT CURRENT_TIMESTAMP,
            CONSTRAINT fk_history_user
                FOREIGN KEY (USER_ID) REFERENCES `user`(ID)
                ON DELETE SET NULL ON UPDATE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)",
        R"(INSERT IGNORE INTO `user` (FULLNAME, USERNAME, PASSWORD)
           VALUES ('管理员', 'admin', '123456'))",
        R"(INSERT IGNORE INTO `department` (ID, NAME) VALUES ('D001', '内科'))",
        R"(INSERT IGNORE INTO `department` (ID, NAME) VALUES ('D002', '外科'))",
        R"(INSERT IGNORE INTO `department` (ID, NAME) VALUES ('D003', '儿科'))",
        R"(INSERT IGNORE INTO `department` (ID, NAME) VALUES ('D004', '骨科'))",
        R"(INSERT IGNORE INTO `doctor` (ID, EMPLOYEENO, NAME, DEPARTMENT_ID)
           VALUES ('DOC001', 'E2026001', '张医生', 'D001'))",
        R"(INSERT IGNORE INTO `doctor` (ID, EMPLOYEENO, NAME, DEPARTMENT_ID)
           VALUES ('DOC002', 'E2026002', '李医生', 'D002'))",
        R"(INSERT IGNORE INTO `doctor` (ID, EMPLOYEENO, NAME, DEPARTMENT_ID)
           VALUES ('DOC003', 'E2026003', '王医生', 'D003'))"
    };

    for (const QString &sql : statements) {
        if (!query.exec(sql)) {
            m_lastError = QString("初始化 MySQL 数据表失败：%1").arg(query.lastError().text());
            qWarning() << m_lastError;
            return false;
        }
    }
    return true;
}

// ==================== 用户登录与注册 ====================

QString IDatabase::userLogin(const QString &username, const QString &password)
{
    if (!isDatabaseOpen()) {
        return "dbError";
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT ID, FULLNAME, USERNAME, PASSWORD FROM `user` WHERE USERNAME = :user");
    query.bindValue(":user", username);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return "dbError";
    }

    if (query.first()) {
        QString storedPass = query.value("PASSWORD").toString();
        if (storedPass == password) {
            m_currentUserId = query.value("ID").toString();
            m_currentUserFullName = query.value("FULLNAME").toString();
            m_currentUsername = query.value("USERNAME").toString();
            logEvent(QString("用户登录：%1 (%2)").arg(m_currentUserFullName, m_currentUsername));
            return "loginOK";
        }
        return "wrongPassword";
    }
    return "wrongUsername";
}

bool IDatabase::userRegister(const QString &fullname, const QString &username,
                             const QString &password, QString &errorMsg)
{
    if (!isDatabaseOpen()) {
        errorMsg = "数据库未连接！";
        return false;
    }

    QSqlQuery checkQuery(m_database);
    checkQuery.prepare("SELECT COUNT(*) FROM `user` WHERE USERNAME = :user");
    checkQuery.bindValue(":user", username);
    if (!checkQuery.exec() || !checkQuery.next()) {
        errorMsg = checkQuery.lastError().text();
        return false;
    }
    if (checkQuery.value(0).toInt() > 0) {
        errorMsg = "该用户名已存在，请更换其他用户名。";
        return false;
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare("INSERT INTO `user` (FULLNAME, USERNAME, PASSWORD) "
                        "VALUES (:fullname, :username, :password)");
    insertQuery.bindValue(":fullname", fullname);
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);

    if (!insertQuery.exec()) {
        errorMsg = insertQuery.lastError().text();
        return false;
    }

    return true;
}

void IDatabase::logout()
{
    if (!m_currentUserId.isEmpty()) {
        logEvent(QString("用户退出登录：%1 (%2)").arg(m_currentUserFullName, m_currentUsername));
    }
    m_currentUserId.clear();
    m_currentUserFullName.clear();
    m_currentUsername.clear();
}

// ==================== 患者模型 ====================

bool IDatabase::initPatientModel()
{
    if (!isDatabaseOpen()) {
        return false;
    }

    if (!patientTabModel) {
        patientTabModel = new QSqlTableModel(this, m_database);
    }
    patientTabModel->setTable("patient");
    patientTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    patientTabModel->setSort(patientTabModel->fieldIndex("ID"), Qt::AscendingOrder);

    if (!patientTabModel->select()) {
        m_lastError = patientTabModel->lastError().text();
        return false;
    }

    // 自动兼容 MOBILPHONE 或 MOBILEPHONE 字段名
    if (patientTabModel->fieldIndex("MOBILPHONE") >= 0) {
        m_phoneColumn = "MOBILPHONE";
    } else if (patientTabModel->fieldIndex("MOBILEPHONE") >= 0) {
        m_phoneColumn = "MOBILEPHONE";
    }

    patientTabModel->setHeaderData(patientTabModel->fieldIndex("ID"), Qt::Horizontal, "ID");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("ID_CARD"), Qt::Horizontal, "身份证号");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("NAME"), Qt::Horizontal, "姓名");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("SEX"), Qt::Horizontal, "性别");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("DOB"), Qt::Horizontal, "出生日期");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("HEIGHT"), Qt::Horizontal, "身高(cm)");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("WEIGHT"), Qt::Horizontal, "体重(kg)");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex(m_phoneColumn), Qt::Horizontal, "手机号");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("AGE"), Qt::Horizontal, "年龄");
    patientTabModel->setHeaderData(patientTabModel->fieldIndex("CREATEDTIMESTAMP"), Qt::Horizontal, "创建时间");

    if (!thePatientSelection) {
        thePatientSelection = new QItemSelectionModel(patientTabModel, this);
    }
    return true;
}

bool IDatabase::searchPatient(const QString &keyword)
{
    if (!patientTabModel) {
        return false;
    }
    QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        patientTabModel->setFilter("");
    } else {
        QString safe = trimmed;
        safe.replace("'", "''");
        QString filter = QString("NAME LIKE '%%1%' OR CAST(ID AS CHAR) LIKE '%%1%' "
                                 "OR ID_CARD LIKE '%%1%' OR %2 LIKE '%%1%'")
                             .arg(safe, m_phoneColumn);
        patientTabModel->setFilter(filter);
    }
    return patientTabModel->select();
}

int IDatabase::addNewPatient()
{
    if (!patientTabModel) {
        return -1;
    }
    int row = patientTabModel->rowCount();
    patientTabModel->insertRow(row);

    QSqlRecord curRec = patientTabModel->record(row);
    // ID 由 MySQL AUTO_INCREMENT 自动生成，不手动赋值
    curRec.setGenerated("ID", false);
    curRec.setValue("SEX", 1);
    curRec.setValue("DOB", QDate(2000, 1, 1));
    curRec.setValue("HEIGHT", 170.0);
    curRec.setValue("WEIGHT", 60.0);
    curRec.setValue("AGE", QDate::currentDate().year() - 2000);
    curRec.setValue("CREATEDTIMESTAMP", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    patientTabModel->setRecord(row, curRec);
    return row;
}

bool IDatabase::submitPatientEdit()
{
    if (!patientTabModel) {
        return false;
    }
    bool ok = patientTabModel->submitAll();
    if (!ok) {
        m_lastError = patientTabModel->lastError().text();
    } else {
        patientTabModel->select();
    }
    return ok;
}

void IDatabase::revertPatientEdit()
{
    if (patientTabModel) {
        patientTabModel->revertAll();
        patientTabModel->select();
    }
}

bool IDatabase::deleteCurrentPatient(QString &errorMsg)
{
    if (!patientTabModel || !thePatientSelection) {
        errorMsg = "数据模型未初始化。";
        return false;
    }

    QModelIndex curIndex = thePatientSelection->currentIndex();
    if (!curIndex.isValid()) {
        errorMsg = "请先选中一条患者记录。";
        return false;
    }

    QSqlRecord rec = patientTabModel->record(curIndex.row());
    QString pid = rec.value("ID").toString();
    QString pname = rec.value("NAME").toString();

    patientTabModel->removeRow(curIndex.row());
    if (!patientTabModel->submitAll()) {
        errorMsg = patientTabModel->lastError().text();
        patientTabModel->revertAll();
        return false;
    }

    patientTabModel->select();
    logEvent(QString("删除患者：%1 (ID:%2)").arg(pname, pid));
    return true;
}

// ==================== 科室模型 ====================

bool IDatabase::initDepartmentModel()
{
    if (!isDatabaseOpen()) {
        return false;
    }
    if (!departmentTabModel) {
        departmentTabModel = new QSqlTableModel(this, m_database);
    }
    departmentTabModel->setTable("department");
    departmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    departmentTabModel->setSort(departmentTabModel->fieldIndex("ID"), Qt::AscendingOrder);

    if (!departmentTabModel->select()) {
        m_lastError = departmentTabModel->lastError().text();
        return false;
    }
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("ID"), Qt::Horizontal, "科室编号");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("NAME"), Qt::Horizontal, "科室名称");

    if (!theDepartmentSelection) {
        theDepartmentSelection = new QItemSelectionModel(departmentTabModel, this);
    }
    return true;
}

bool IDatabase::searchDepartment(const QString &keyword)
{
    if (!departmentTabModel) {
        return false;
    }
    QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        departmentTabModel->setFilter("");
    } else {
        QString safe = trimmed;
        safe.replace("'", "''");
        departmentTabModel->setFilter(QString("ID LIKE '%%1%' OR NAME LIKE '%%1%'").arg(safe));
    }
    return departmentTabModel->select();
}

bool IDatabase::addDepartment(const QString &id, const QString &name, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO `department` (ID, NAME) VALUES (:id, :name)");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (departmentTabModel) {
        departmentTabModel->select();
    }
    logEvent(QString("新增科室：%1 (%2)").arg(name, id));
    return true;
}

bool IDatabase::updateDepartment(const QString &oldId, const QString &id, const QString &name, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE `department` SET ID = :id, NAME = :name WHERE ID = :oldId");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":oldId", oldId);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (departmentTabModel) {
        departmentTabModel->select();
    }
    logEvent(QString("修改科室：%1 (%2)").arg(name, id));
    return true;
}

bool IDatabase::deleteDepartment(const QString &id, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM `department` WHERE ID = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (departmentTabModel) {
        departmentTabModel->select();
    }
    if (doctorTabModel) {
        doctorTabModel->select();
    }
    logEvent(QString("删除科室：%1").arg(id));
    return true;
}

QString IDatabase::departmentNameById(const QString &deptId) const
{
    if (!isDatabaseOpen() || deptId.isEmpty()) {
        return "";
    }
    QSqlQuery query(m_database);
    query.prepare("SELECT NAME FROM `department` WHERE ID = :id");
    query.bindValue(":id", deptId);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return deptId;
}

// ==================== 医生模型 ====================

bool IDatabase::initDoctorModel()
{
    if (!isDatabaseOpen()) {
        return false;
    }
    if (!doctorTabModel) {
        doctorTabModel = new QSqlTableModel(this, m_database);
    }
    doctorTabModel->setTable("doctor");
    doctorTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    doctorTabModel->setSort(doctorTabModel->fieldIndex("ID"), Qt::AscendingOrder);

    if (!doctorTabModel->select()) {
        m_lastError = doctorTabModel->lastError().text();
        return false;
    }
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("ID"), Qt::Horizontal, "医生编号");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("EMPLOYEENO"), Qt::Horizontal, "工号");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("NAME"), Qt::Horizontal, "姓名");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("DEPARTMENT_ID"), Qt::Horizontal, "所属科室");

    if (!theDoctorSelection) {
        theDoctorSelection = new QItemSelectionModel(doctorTabModel, this);
    }
    return true;
}

bool IDatabase::searchDoctor(const QString &keyword)
{
    if (!doctorTabModel) {
        return false;
    }
    QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        doctorTabModel->setFilter("");
    } else {
        QString safe = trimmed;
        safe.replace("'", "''");
        doctorTabModel->setFilter(QString("ID LIKE '%%1%' OR EMPLOYEENO LIKE '%%1%' OR NAME LIKE '%%1%'").arg(safe));
    }
    return doctorTabModel->select();
}

bool IDatabase::addDoctor(const QString &id, const QString &empNo, const QString &name,
                          const QString &deptId, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO `doctor` (ID, EMPLOYEENO, NAME, DEPARTMENT_ID) "
                  "VALUES (:id, :empNo, :name, :deptId)");
    query.bindValue(":id", id);
    query.bindValue(":empNo", empNo);
    query.bindValue(":name", name);
    query.bindValue(":deptId", deptId.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : deptId);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (doctorTabModel) {
        doctorTabModel->select();
    }
    logEvent(QString("新增医生：%1 (%2)").arg(name, empNo));
    return true;
}

bool IDatabase::updateDoctor(const QString &oldId, const QString &id, const QString &empNo,
                             const QString &name, const QString &deptId, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE `doctor` SET ID = :id, EMPLOYEENO = :empNo, NAME = :name, DEPARTMENT_ID = :deptId "
                  "WHERE ID = :oldId");
    query.bindValue(":id", id);
    query.bindValue(":empNo", empNo);
    query.bindValue(":name", name);
    query.bindValue(":deptId", deptId.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : deptId);
    query.bindValue(":oldId", oldId);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (doctorTabModel) {
        doctorTabModel->select();
    }
    logEvent(QString("修改医生：%1 (%2)").arg(name, empNo));
    return true;
}

bool IDatabase::deleteDoctor(const QString &id, QString &errorMsg)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM `doctor` WHERE ID = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        errorMsg = query.lastError().text();
        return false;
    }
    if (doctorTabModel) {
        doctorTabModel->select();
    }
    logEvent(QString("删除医生：%1").arg(id));
    return true;
}

// ==================== 操作记录模型 ====================

bool IDatabase::initHistoryModel()
{
    if (!isDatabaseOpen()) {
        return false;
    }
    if (!historyTabModel) {
        historyTabModel = new QSqlTableModel(this, m_database);
    }
    historyTabModel->setTable("history");
    historyTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    historyTabModel->setSort(historyTabModel->fieldIndex("ID"), Qt::DescendingOrder);

    if (!historyTabModel->select()) {
        m_lastError = historyTabModel->lastError().text();
        return false;
    }
    historyTabModel->setHeaderData(historyTabModel->fieldIndex("ID"), Qt::Horizontal, "记录ID");
    historyTabModel->setHeaderData(historyTabModel->fieldIndex("USER_ID"), Qt::Horizontal, "用户ID");
    historyTabModel->setHeaderData(historyTabModel->fieldIndex("EVENT"), Qt::Horizontal, "操作内容");
    historyTabModel->setHeaderData(historyTabModel->fieldIndex("TIMESTAMP"), Qt::Horizontal, "操作时间");
    return true;
}

bool IDatabase::searchHistory(const QString &keyword)
{
    if (!historyTabModel) {
        return false;
    }
    QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        historyTabModel->setFilter("");
    } else {
        QString safe = trimmed;
        safe.replace("'", "''");
        historyTabModel->setFilter(QString("CAST(USER_ID AS CHAR) LIKE '%%1%' OR EVENT LIKE '%%1%' OR CAST(TIMESTAMP AS CHAR) LIKE '%%1%'").arg(safe));
    }
    return historyTabModel->select();
}

void IDatabase::logEvent(const QString &event)
{
    if (!isDatabaseOpen()) {
        return;
    }
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO `history` (USER_ID, EVENT, TIMESTAMP) "
                  "VALUES (:uid, :event, :ts)");
    if (m_currentUserId.isEmpty()) {
        query.bindValue(":uid", QVariant(QMetaType(QMetaType::Int)));
    } else {
        query.bindValue(":uid", m_currentUserId.toInt());
    }
    query.bindValue(":event", event);
    query.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (query.exec() && historyTabModel) {
        historyTabModel->select();
    }
}
