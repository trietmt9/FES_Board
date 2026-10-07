#pragma once

// The moving part of a spectrum row: the curve (or bars), its area fill and glow,
// and the peak dot - drawn by the GPU, like WaveTrace and for the same reason.
// QPainter spent ~10 ms per row per update on the glow alone; the frequency view sat
// at a third of a core.
//
// The static layer - grid, Hz labels, EEG band columns, the peak's label chip - is
// SpectrumItem's. Both read the layout from SpectrumLayout.h so the curve cannot
// drift off its own axis.

#include <QColor>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>

class SignalModel;

class SpectrumTrace : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SignalModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int row MEMBER m_row NOTIFY styleChanged)

    Q_PROPERTY(QColor accent MEMBER m_accent NOTIFY styleChanged)
    Q_PROPERTY(QColor line MEMBER m_line NOTIFY styleChanged)
    Q_PROPERTY(QColor peakDot MEMBER m_peakDot NOTIFY styleChanged)

public:
    explicit SpectrumTrace(QQuickItem *parent = nullptr);

    SignalModel *model() const { return m_model; }
    void setModel(SignalModel *m);

signals:
    void modelChanged();
    void styleChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

private:
    SignalModel *m_model = nullptr;
    int m_row = 0;

    QColor m_accent{0x91, 0x84, 0xD9};
    QColor m_line{0xB5, 0xAB, 0xFC};
    QColor m_peakDot{0xE7, 0xE5, 0xFE};
};
