#pragma once

// Bounded list of console lines for the debug pane.
//
// These are the unframed ASCII bytes the parser passes through - the banner,
// "Chip ID = 0x92", OVERFLOW, and the 2 s watchdog warning - which reach us for
// free because the frame magic is non-ASCII (ARCHITECTURE.md 4.2).

#include <QAbstractListModel>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

class LogModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        SeverityRole, // "err" | "wrn" | "inf" | "dbg" | ""
    };

    explicit LogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void append(const QString &line);
    void clear();

signals:
    void countChanged();

private:
    static QString severityOf(const QString &line);

    // Enough to cover a long bring-up session; old lines fall off the front so
    // an overnight soak cannot exhaust memory.
    static constexpr int kMaxLines = 5000;

    QStringList m_lines;
};
