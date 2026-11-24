#include "sexdelegate.h"
#include "idatabase.h"

#include <QComboBox>

SexDelegate::SexDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QString SexDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale);
    if (value.isNull()) {
        return "";
    }
    return value.toInt() == 1 ? "男" : "女";
}

QWidget *SexDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox *combo = new QComboBox(parent);
    combo->addItem("男", 1);
    combo->addItem("女", 0);
    return combo;
}

void SexDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    int sexVal = index.model()->data(index, Qt::EditRole).toInt();
    int idx = combo->findData(sexVal);
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
}

void SexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    model->setData(index, combo->currentData(), Qt::EditRole);
}

// ==================== DepartmentDelegate ====================

DepartmentDelegate::DepartmentDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QString DepartmentDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale);
    QString deptId = value.toString();
    if (deptId.isEmpty()) {
        return "未分配";
    }
    QString name = IDatabase::getInstance().departmentNameById(deptId);
    return name.isEmpty() ? deptId : QString("%1 (%2)").arg(name, deptId);
}
