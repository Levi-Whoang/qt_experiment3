#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QItemSelectionModel>
#include <QString>

class IDatabase : public QObject
{
    Q_OBJECT
public:
    static IDatabase &getInstance()
    {
        static IDatabase instance;
        return instance;
    }

    bool isDatabaseOpen() const;
    QString connectionSummary() const;
    QString lastErrorMessage() const;

    QString host() const { return m_host; }
    int port() const { return m_port; }
    QString dbName() const { return m_dbName; }
    QString dbUser() const { return m_dbUser; }
    QString dbPassword() const { return m_dbPassword; }

    bool reconnectDatabase(const QString &host, int port, const QString &dbName,
                           const QString &user, const QString &password);

    // 用户认证与注册
    QString userLogin(const QString &username, const QString &password);
    bool userRegister(const QString &fullname, const QString &username,
                      const QString &password, QString &errorMsg);
    QString currentUserId() const { return m_currentUserId; }
    QString currentUserFullName() const { return m_currentUserFullName; }
    QString currentUsername() const { return m_currentUsername; }
    void logout();

    QString phoneColumnName() const { return m_phoneColumn; }

    // 患者表模型与操作
    bool initPatientModel();
    bool searchPatient(const QString &keyword);
    int addNewPatient();
    bool submitPatientEdit();
    void revertPatientEdit();
    bool deleteCurrentPatient(QString &errorMsg);

    // 科室表模型与操作
    bool initDepartmentModel();
    bool searchDepartment(const QString &keyword);
    bool addDepartment(const QString &id, const QString &name, QString &errorMsg);
    bool updateDepartment(const QString &oldId, const QString &id, const QString &name, QString &errorMsg);
    bool deleteDepartment(const QString &id, QString &errorMsg);
    QString departmentNameById(const QString &deptId) const;

    // 医生表模型与操作
    bool initDoctorModel();
    bool searchDoctor(const QString &keyword);
    bool addDoctor(const QString &id, const QString &empNo, const QString &name,
                   const QString &deptId, QString &errorMsg);
    bool updateDoctor(const QString &oldId, const QString &id, const QString &empNo,
                      const QString &name, const QString &deptId, QString &errorMsg);
    bool deleteDoctor(const QString &id, QString &errorMsg);

    // 操作记录表模型与操作
    bool initHistoryModel();
    bool searchHistory(const QString &keyword);
    void logEvent(const QString &event);

    QSqlTableModel *patientTabModel = nullptr;
    QItemSelectionModel *thePatientSelection = nullptr;

    QSqlTableModel *departmentTabModel = nullptr;
    QItemSelectionModel *theDepartmentSelection = nullptr;

    QSqlTableModel *doctorTabModel = nullptr;
    QItemSelectionModel *theDoctorSelection = nullptr;

    QSqlTableModel *historyTabModel = nullptr;

private:
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(const IDatabase &) = delete;
    IDatabase &operator=(const IDatabase &) = delete;

    bool initDatabase();
    bool createTablesIfNeeded();

    QSqlDatabase m_database;
    QString m_host = "localhost";
    int m_port = 3306;
    QString m_dbName = "hospital";
    QString m_dbUser = "root";
    QString m_dbPassword = "";
    QString m_lastError;

    QString m_currentUserId;
    QString m_currentUserFullName;
    QString m_currentUsername;
    QString m_phoneColumn = "MOBILPHONE";
};

#endif // IDATABASE_H
