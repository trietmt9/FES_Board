#pragma once

// The static layer of a channel row's time-domain plot - the spec's WaveItem
// (Biosignal Monitor - Qt Spec.md, sections 5.3 and 6.1) minus its moving parts.
//
// What it draws, with QPainter, only when something changes:
//   grid       25 vertical divisions (every 5th major) x 8 horizontal
//   markers    dashed verticals, with an "M1" chip on the top row only
//   offline    "Device offline" / "No signal on this channel"
//
// The TRACE is WaveTrace's. It was drawn here at first and was correct and
// unusably slow: stroking one row with its glow cost 77 ms on the development
// machine (475 ms at 5000 vertices) against a 16 ms frame budget, with the
// application pinned at ~100 % of a core. WaveTrace hands the same geometry to the
// GPU. See TraceGeometry.h.
//
// Marker x positions come from trace::sampleX(), the function that places the
// samples themselves, so a marker cannot drift from the sample it annotates.

#include <QColor>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

class SignalModel;

class WaveItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SignalModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int row MEMBER m_row NOTIFY styleChanged)
    /// Marker chips ("M1") are drawn on the top row only; the dashed lines are
    /// drawn on every row so they read as one line down the whole panel.
    Q_PROPERTY(bool markerLabels MEMBER m_markerLabels NOTIFY styleChanged)
    Q_PROPERTY(QString fontFamily MEMBER m_fontFamily NOTIFY styleChanged)

    // Colours come from Theme.qml; nothing in here hard-codes a token.
    Q_PROPERTY(QColor gridMinor MEMBER m_gridMinor NOTIFY styleChanged)
    Q_PROPERTY(QColor gridMajor MEMBER m_gridMajor NOTIFY styleChanged)
    Q_PROPERTY(QColor gridCenter MEMBER m_gridCenter NOTIFY styleChanged)
    Q_PROPERTY(QColor markerLine MEMBER m_markerLine NOTIFY styleChanged)
    Q_PROPERTY(QColor markerFill MEMBER m_markerFill NOTIFY styleChanged)
    Q_PROPERTY(QColor markerText MEMBER m_markerText NOTIFY styleChanged)
    Q_PROPERTY(QColor emptyText MEMBER m_emptyText NOTIFY styleChanged)

public:
    explicit WaveItem(QQuickItem *parent = nullptr);

    SignalModel *model() const { return m_model; }
    void setModel(SignalModel *m);

    void paint(QPainter *painter) override;

signals:
    void modelChanged();
    void styleChanged();

private slots:
    void onFrame();

private:
    void drawGrid(QPainter *p, qreal w, qreal h) const;
    void drawEmpty(QPainter *p, qreal w, qreal h) const;

    SignalModel *m_model = nullptr;
    int m_row = 0;
    bool m_markerLabels = false;
    QString m_fontFamily = QStringLiteral("Inter");

    QColor m_gridMinor{0x29, 0x2B, 0x31};
    QColor m_gridMajor{0x3F, 0x42, 0x4D};
    QColor m_gridCenter{0x59, 0x5D, 0x6C};
    QColor m_markerLine{0xD2, 0xCE, 0xFD};
    QColor m_markerFill{0x42, 0x3A, 0x6A};
    QColor m_markerText{0xE7, 0xE5, 0xFE};
    QColor m_emptyText{0x93, 0x97, 0xAB};

    int m_idleFrames = 0;
};
