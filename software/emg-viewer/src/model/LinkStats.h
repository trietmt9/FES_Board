#pragma once

// Link health, exposed to QML as a value type.
//
// Given the DRDY/START problems already met on this hardware, this panel is the
// first place to look when a capture goes wrong: a measured rate below nominal
// means the firmware is not keeping up with DRDY, CRC errors mean the wire is
// marginal, and seq gaps mean loss upstream of the host.

#include <QString>
#include <QtQml/qqmlregistration.h>

#include <cstdint>

struct LinkStats {
    Q_GADGET
    QML_VALUE_TYPE(linkStats)

    Q_PROPERTY(double bytesPerSecond MEMBER bytesPerSecond CONSTANT)
    Q_PROPERTY(double framesPerSecond MEMBER framesPerSecond CONSTANT)
    Q_PROPERTY(double measuredSps MEMBER measuredSps CONSTANT)
    Q_PROPERTY(int nominalSps MEMBER nominalSps CONSTANT)
    Q_PROPERTY(qulonglong dataFrames MEMBER dataFrames CONSTANT)
    Q_PROPERTY(qulonglong infoFrames MEMBER infoFrames CONSTANT)
    Q_PROPERTY(qulonglong crcErrors MEMBER crcErrors CONSTANT)
    Q_PROPERTY(qulonglong resyncs MEMBER resyncs CONSTANT)
    Q_PROPERTY(qulonglong droppedFrames MEMBER droppedFrames CONSTANT)
    Q_PROPERTY(qulonglong overflows MEMBER overflows CONSTANT)
    Q_PROPERTY(qulonglong totalSamples MEMBER totalSamples CONSTANT)
    Q_PROPERTY(qulonglong lostSamples MEMBER lostSamples CONSTANT)
    Q_PROPERTY(double lostPercent READ lostPercent CONSTANT)
    Q_PROPERTY(bool healthy READ healthy CONSTANT)

public:
    double bytesPerSecond = 0.0;
    double framesPerSecond = 0.0;
    double measuredSps = 0.0;
    int nominalSps = 0;

    qulonglong dataFrames = 0;
    qulonglong infoFrames = 0;
    qulonglong crcErrors = 0;
    qulonglong resyncs = 0;
    qulonglong droppedFrames = 0;
    qulonglong overflows = 0;  // firmware-side ring overflow flag seen
    qulonglong totalSamples = 0;

    // Conversions the firmware never delivered, inferred from the device's own
    // t_ms (ISampleSource::SampleLossTracker). Distinct from droppedFrames,
    // which is loss on the wire: this is loss upstream of the wire, in the
    // DRDY/SPI path. It is the number to look at when a trace "looks noisy" but
    // the link is clean - lost conversions do not leave a gap, they compress
    // time, which reads as jitter.
    qulonglong lostSamples = 0;

    double lostPercent() const
    {
        const qulonglong produced = totalSamples + lostSamples;
        return produced > 0 ? 100.0 * double(lostSamples) / double(produced) : 0.0;
    }

    // "Nothing has gone wrong yet" - errors are cumulative, so any non-zero
    // count is worth surfacing even if the link has since recovered.
    //
    // Sample loss is held to a tolerance rather than zero: t_ms has 1 ms
    // resolution and the STM32 and ADS1298 run off different oscillators, so a
    // long clean session still accumulates a few counts of quantisation and
    // ppm drift. 0.1 % is far below anything visible in a waveform and far
    // above that floor.
    bool healthy() const
    {
        return crcErrors == 0 && droppedFrames == 0 && overflows == 0 &&
               lostPercent() < 0.1;
    }
};
