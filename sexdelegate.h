#ifndef SEXDELEGATE_H
#define SEXDELEGATE_H

#include <QStyledItemDelegate>

// 自定义代理：将 Patient 表中的 SEX 整数 (1/0) 显示为 "男"/"女"，并支持下拉编辑
class SexDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SexDelegate(QObject *parent = nullptr);

    QString displayText(const QVariant &value, const QLocale &locale) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
};

// 自定义代理：将 Doctor 表中的 DEPARTMENT_ID 显示为对应的科室名称
class DepartmentDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DepartmentDelegate(QObject *parent = nullptr);

    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif // SEXDELEGATE_H
