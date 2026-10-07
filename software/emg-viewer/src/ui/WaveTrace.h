#pragma once

// The moving part of a channel row: the trace, its glow, the head dot, and in
// Sweep mode the fading erase band - drawn by the GPU.
//
// This replaced a QPainter version that was correct and unusably slow: stroking one
// row with its glow took 77 ms on the development machine (475 ms at 5000 vertices)
// against a 16 ms frame budget, and the application sat at ~100 % of a core. A
// trace is a few thousand vertices; handed to the scene graph as triangles it costs
// the CPU a fraction of a millisecond.
//
// Anti-aliasing without shaders (this Qt build ships no ShaderTools, so no custom
// materials): the stroke is a triangle mesh with an opaque core and a one-pixel
// skirt whose per-vertex alpha fades to zero, drawn with QSGVertexColorMaterial.
//
// The static layer - grid, marker lines, "device offline" - is WaveItem's, and is
// repainted only when something about it changes.

#include "TraceGeometry.h"

#include <QColor>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>

#include <vector>

class SignalModel;

class WaveTrace : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SignalModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int row MEMBER m_row NOTIFY styleChanged)

    Q_PROPERTY(QColor trace MEMBER m_trace NOTIFY styleChanged)
    Q_PROPERTY(QColor glow MEMBER m_glow NOTIFY styleChanged)
    Q_PROPERTY(QColor head MEMBER m_head NOTIFY styleChanged)

public:
    explicit WaveTrace(QQuickItem *parent = nullptr);

    SignalModel *model() const { return m_model; }
    void setModel(SignalModel *m);

    /// The polylines the next frame will stroke, in plot pixels. This is the exact
    /// function updatePaintNode() uses, exposed so a test can check that the model's
    /// gain, window and head reach the geometry - end to end, on numbers.
    trace::Built buildCurrent();

signals:
    void modelChanged();
    void styleChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

private slots:
    void onFrame();

private:
    SignalModel *m_model = nullptr;
    int m_row = 0;

    QColor m_trace{0xB5, 0xAB, 0xFC};
    QColor m_glow{0x91, 0x84, 0xD9};
    QColor m_head{0xE7, 0xE5, 0xFE};

    std::vector<float> m_buf;
    int m_idleFrames = 0;
};
