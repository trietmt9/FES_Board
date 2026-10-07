#pragma once

// What each signal type IS, as the spec defines it - one table, in one place.
//
// Biosignal Monitor - Qt Spec.md scatters these across sections 4, 5.2, 5.3,
// 6.1 and 6.2: row names, filter corners, vertical range, nominal sampling rate,
// spectrum display range. They are gathered here so a change is one edit and so
// a test can hold the whole table against the spec's text.
//
// Plain data on purpose: no Qt, no strings the UI would have to translate. The
// model turns it into what QML binds to.

#include <array>
#include <cstddef>

namespace emg {

enum class SignalType { Ecg = 0, Eeg = 1, Emg = 2 };
inline constexpr int kSignalTypeCount = 3;

struct ChannelRowInfo {
    const char *name;       ///< "I", "Fp1", "Biceps"
    const char *location;   ///< "Limb lead", "Frontal", "Right arm"
};

struct SignalProfile {
    SignalType type;
    const char *id;           ///< "ecg" - also the settings key
    const char *name;         ///< "ECG" - short, for the rail and the kicker
    const char *title;        ///< "Electrocardiogram" - the panel title
    const char *channelNoun;  ///< "leads" / "channels" - rail meta line

    int rowCount;
    std::array<ChannelRowInfo, 4> rows;

    // Conditioning band, section 5.2 filter tags.
    double highpassHz;
    double lowpassHz;

    /// Vertical half-height at gain x1, microvolts (section 6.1). The plot shows
    /// +/- this at gain 1, +/- this/gain otherwise.
    double rangeUv;

    /// Rate the spec labels the signal with (section 4), and the rate the
    /// spectrum is decimated towards so its resolution matches the footer.
    double nominalRateHz;

    // Frequency-domain view, section 6.2.
    double displayMaxHz;      ///< right edge of the spectrum
    double peakMinHz;         ///< peak search ignores everything below this
    double gridStepHz;        ///< vertical grid spacing; every 2nd line is major
    bool showBands;           ///< EEG: shaded delta/theta/alpha/beta/gamma columns
};

inline const SignalProfile &signalProfile(SignalType t)
{
    static const std::array<SignalProfile, kSignalTypeCount> kProfiles = {{
        {SignalType::Ecg, "ecg", "ECG", "Electrocardiogram", "leads",
         3,
         {{{"I", "Limb lead"}, {"II", "Limb lead"}, {"V1", "Precordial"}, {"", ""}}},
         0.5, 40.0, 1600.0, 250.0,
         40.0, 0.8, 5.0, false},

        {SignalType::Eeg, "eeg", "EEG", "Electroencephalogram", "channels",
         4,
         {{{"Fp1", "Frontal"}, {"C3", "Central"}, {"O1", "Occipital"}, {"O2", "Occipital"}}},
         0.5, 45.0, 70.0, 256.0,
         45.0, 0.8, 5.0, true},

        {SignalType::Emg, "emg", "EMG", "Electromyogram", "channels",
         2,
         {{{"Biceps", "Right arm"}, {"Triceps", "Right arm"}, {"", ""}, {"", ""}}},
         20.0, 450.0, 1600.0, 1000.0,
         450.0, 20.0, 50.0, false},
    }};
    return kProfiles[static_cast<std::size_t>(t)];
}

/// EEG bands, section 6.2 / 7: delta 0.5-4, theta 4-8, alpha 8-13, beta 13-30,
/// gamma 30-45 Hz. Half-open [lo, hi) so adjacent bands tile with no overlap.
struct EegBand {
    const char *symbol;   ///< Greek letter
    const char *name;     ///< "Delta"
    double loHz;
    double hiHz;
};

inline constexpr std::array<EegBand, 5> kEegBands = {{
    {"\xCE\xB4", "Delta", 0.5, 4.0},     // delta
    {"\xCE\xB8", "Theta", 4.0, 8.0},     // theta
    {"\xCE\xB1", "Alpha", 8.0, 13.0},    // alpha
    {"\xCE\xB2", "Beta", 13.0, 30.0},    // beta
    {"\xCE\xB3", "Gamma", 30.0, 45.0},   // gamma
}};

} // namespace emg
