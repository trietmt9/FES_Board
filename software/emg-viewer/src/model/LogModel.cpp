#include "LogModel.h"

LogModel::LogModel(QObject *parent) : QAbstractListModel(parent) {}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_lines.size());
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_lines.size()) {
        return {};
    }
    const QString &line = m_lines.at(index.row());

    switch (role) {
    case TextRole:
    case Qt::DisplayRole:
        return line;
    case SeverityRole:
        return severityOf(line);
    default:
        return {};
    }
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    // "line", not "text": a required property named "text" in a Label delegate
    // would shadow Label.text and render nothing.
    return {
        {TextRole, "line"},
        {SeverityRole, "severity"},
    };
}

QString LogModel::severityOf(const QString &line)
{
    // Zephyr's log backend writes "<err>", "<wrn>", "<inf>", "<dbg>" after the
    // timestamp, with CONFIG_LOG_BACKEND_SHOW_LEVEL enabled.
    if (line.contains(QStringLiteral("<err>"))) {
        return QStringLiteral("err");
    }
    if (line.contains(QStringLiteral("<wrn>"))) {
        return QStringLiteral("wrn");
    }
    if (line.contains(QStringLiteral("<inf>"))) {
        return QStringLiteral("inf");
    }
    if (line.contains(QStringLiteral("<dbg>"))) {
        return QStringLiteral("dbg");
    }
    return {};
}

void LogModel::append(const QString &line)
{
    if (line.isEmpty()) {
        return;
    }

    if (m_lines.size() >= kMaxLines) {
        beginRemoveRows({}, 0, 0);
        m_lines.removeFirst();
        endRemoveRows();
    }

    const int row = static_cast<int>(m_lines.size());
    beginInsertRows({}, row, row);
    m_lines.append(line);
    endInsertRows();

    emit countChanged();
}

void LogModel::clear()
{
    if (m_lines.isEmpty()) {
        return;
    }
    beginResetModel();
    m_lines.clear();
    endResetModel();
    emit countChanged();
}
