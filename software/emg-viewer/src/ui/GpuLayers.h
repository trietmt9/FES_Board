#pragma once

// Scene-graph scaffolding shared by the GPU-drawn plot items: a root node holding
// N back-to-front layers, each a triangle mesh drawn with QSGVertexColorMaterial.
//
// Anti-aliasing here is geometric (an opaque core with a per-vertex-alpha skirt, see
// TraceGeometry.h) because this Qt build has no ShaderTools, so no custom material
// and no MSAA-independent smoothing. Colours are premultiplied.

#include "TraceGeometry.h"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGNode>
#include <QSGVertexColorMaterial>

#include <cstring>

namespace trace {

static_assert(sizeof(Vertex) == sizeof(QSGGeometry::ColoredPoint2D),
              "trace::Vertex must match QSGGeometry::ColoredPoint2D byte for byte");

template <int N>
struct GpuLayers : QSGNode {
    QSGGeometryNode *layer[N] = {};

    GpuLayers()
    {
        for (int i = 0; i < N; ++i) {
            auto *n = new QSGGeometryNode;
            auto *g = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0,
                                      QSGGeometry::UnsignedIntType);
            g->setDrawingMode(QSGGeometry::DrawTriangles);
            n->setGeometry(g);
            n->setFlag(QSGNode::OwnsGeometry);
            n->setMaterial(new QSGVertexColorMaterial);     // blending is on by default
            n->setFlag(QSGNode::OwnsMaterial);
            appendChildNode(n);
            layer[i] = n;
        }
    }

    void upload(int i, const Mesh &m)
    {
        QSGGeometryNode *n = layer[i];
        QSGGeometry *g = n->geometry();
        g->allocate(m.vertices.size(), m.indices.size());
        if (!m.vertices.isEmpty()) {
            std::memcpy(g->vertexData(), m.vertices.constData(),
                        std::size_t(m.vertices.size()) * sizeof(Vertex));
            std::memcpy(g->indexData(), m.indices.constData(),
                        std::size_t(m.indices.size()) * sizeof(std::uint32_t));
        }
        n->markDirty(QSGNode::DirtyGeometry);
    }
};

} // namespace trace
