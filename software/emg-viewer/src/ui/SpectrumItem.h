#pragma once

// The static layer of a channel row's frequency-domain plot - spec section 6.2.
//
// Painted, and only when something changes: the grid, the Hz labels, the EEG band
// columns with their Greek labels, the peak's "10.2 Hz" chip, and "device offline".
// The CURVE, its area fill, glow, the bars alternative and the peak dot are
// SpectrumTrace's, drawn by the GPU: stroking them with QPainter cost ~10 ms per row
// per update and held the frequency view at a third of a core.
//
// Layout of the plot area (8 px above, 18 px below for the Hz labels) is shared with
// SpectrumTrace through SpectrumLayout.h, so the curve cannot float off its grid.

#include <QColor>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

class SignalModel;

class SpectrumItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SignalModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int row MEMBER m_row NOTIFY styleChanged)
    /// Which layer this instance paints: the grid beneath the curve, or the peak's
    /// chip above it. Two instances are stacked around SpectrumTrace so the curve
    /// runs over the grid but under nothing - a label the curve cuts through is
    /// unreadable.
    Q_PROPERTY(Part part MEMBER m_part NOTIFY styleChanged)
    Q_PROPERTY(QString fontFamily MEMBER m_fontFamily NOTIFY styleChanged)
    /// Tabular figures for the axis and peak labels, so they do not shimmer.
    Q_PROPERTY(QString numberFamily MEMBER m_numberFamily NOTIFY styleChanged)

    Q_PROPERTY(QColor gridMinor MEMBER m_gridMinor NOTIFY styleChanged)
    Q_PROPERTY(QColor gridMajor MEMBER m_gridMajor NOTIFY styleChanged)
    Q_PROPERTY(QColor axisText MEMBER m_axisText NOTIFY styleChanged)
    Q_PROPERTY(QColor accent MEMBER m_accent NOTIFY styleChanged)
    Q_PROPERTY(QColor peakText MEMBER m_peakText NOTIFY styleChanged)
    Q_PROPERTY(QColor peakChip MEMBER m_peakChip NOTIFY styleChanged)
    Q_PROPERTY(QColor bandText MEMBER m_bandText NOTIFY styleChanged)
    Q_PROPERTY(QColor emptyText MEMBER m_emptyText NOTIFY styleChanged)

public:
    enum Part { Grid, Chip };
    Q_ENUM(Part)

    explicit SpectrumItem(QQuickItem *parent = nullptr);

    SignalModel *model() const { return m_model; }
    void setModel(SignalModel *m);

    void paint(QPainter *painter) override;

signals:
    void modelChanged();
    void styleChanged();

private slots:
    void onFrame();

private:
    SignalModel *m_model = nullptr;
    int m_row = 0;
    Part m_part = Grid;
    QString m_fontFamily = QStringLiteral("Inter");
    QString m_numberFamily = QStringLiteral("Inter Tabular");

    QColor m_gridMinor{0x29, 0x2B, 0x31};
    QColor m_gridMajor{0x3F, 0x42, 0x4D};
    QColor m_axisText{0x75, 0x79, 0x8C};
    QColor m_accent{0x91, 0x84, 0xD9};
    QColor m_peakText{0xD2, 0xCE, 0xFD};
    QColor m_peakChip{0x29, 0x2B, 0x31};
    QColor m_bandText{0x93, 0x97, 0xAB};
    QColor m_emptyText{0x93, 0x97, 0xAB};

    int m_idleFrames = 0;
};
