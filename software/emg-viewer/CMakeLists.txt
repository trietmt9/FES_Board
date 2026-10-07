cmake_minimum_required(VERSION 3.21)

project(emg-viewer
    VERSION 0.1.0
    DESCRIPTION "Real-time EMG acquisition and visualisation for the FES_Board"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 SerialPort)
qt_standard_project_setup(REQUIRES 6.4)

# ---------------------------------------------------------------------------
# emg_core - the acquisition engine: protocol decode, lock-free buffering, and
# the sources that fill it.
#
# Kept free of QML and QtQuick so it can be tested headlessly, with no event
# loop beyond a plain QCoreApplication and no display. That is what lets the
# replay throughput test run in CI.
# ---------------------------------------------------------------------------
add_library(emg_core STATIC
    src/core/Biquad.cpp
    src/core/Crc16.cpp
    src/core/EmgFilter.cpp
    src/core/Fft.cpp
    src/core/HeartRate.cpp
    src/core/FrameParser.cpp
    src/core/Spectrum.cpp
    src/core/SpectrumEngine.cpp
    src/io/ISampleSource.h   # header-only QObject; listed so AUTOMOC sees it
    src/io/SerialSource.cpp
    src/io/ReplaySource.cpp
    src/io/Recorder.cpp
)
target_include_directories(emg_core PUBLIC src)
target_link_libraries(emg_core PUBLIC Qt6::Core Qt6::SerialPort)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
# The model layer: acquisition, view state and metrics, device management. Listed
# once so the application and the headless model tests compile exactly the same
# sources.
set(EMG_MODEL_SOURCES
    src/model/Acquisition.cpp
    src/model/DeviceManager.cpp
    src/model/LogModel.cpp
    src/model/SignalModel.cpp
    # A header-only Q_GADGET: AUTOMOC only generates its meta-object for targets
    # that list it, and a test target that omits it fails to link.
    src/model/LinkStats.h
)

qt_add_executable(emg-viewer
    src/main.cpp
    ${EMG_MODEL_SOURCES}
    src/ui/SpectrumItem.cpp
    src/ui/SpectrumTrace.cpp
    src/ui/TraceGeometry.cpp
    src/ui/WaveItem.cpp
    src/ui/WaveTrace.cpp
)

target_compile_definitions(emg-viewer PRIVATE
    EMG_VIEWER_VERSION="${PROJECT_VERSION}"
)

# qmltyperegistrar emits #include directives by basename, so every directory
# holding a QML-exposed header has to be on the include path.
target_include_directories(emg-viewer PRIVATE
    src
    src/core
    src/io
    src/model
    src/ui
)

# Bundled fonts: Inter 400/500, the tabular-figure derivative, and Phosphor icons.
# See tools/make_fonts.py and assets/fonts/ (OFL / MIT licences sit beside them).
qt_add_resources(emg-viewer "fonts"
    PREFIX "/fonts"
    BASE assets/fonts
    FILES
        assets/fonts/Inter-Regular.otf
        assets/fonts/Inter-Medium.otf
        assets/fonts/InterTabular-Regular.otf
        assets/fonts/InterTabular-Medium.otf
        assets/fonts/Phosphor.ttf
)

set(EMG_QML_FILES
    qml/Main.qml
    qml/Theme.qml
    qml/Icon.qml
    qml/Kicker.qml
    qml/FadedRule.qml
    qml/KbdChip.qml
    qml/Tag.qml
    qml/Shadow.qml
    qml/Glow.qml
    qml/AppButton.qml
    qml/AppTextField.qml
    qml/Segmented.qml
    qml/Header.qml
    qml/SignalRail.qml
    qml/WavePanel.qml
    qml/ChannelRow.qml
    qml/MetricsPanel.qml
    qml/WifiDialog.qml
    qml/Diagnostics.qml
)

# Alias every .qml to the module root rather than a qml/ subdirectory.
#
# This is not cosmetic. qmldir lives at the module root, so if the files sit in
# a subdirectory, each file's implicit "same directory" import resolves its
# siblings directly - without qmldir - and a `singleton` declaration is silently
# ignored. Theme then resolves as a plain type whose properties are all
# undefined, which in turn makes every `2 * Theme.s5` NaN and spins the layout
# engine at 100% CPU. Keeping the files beside qmldir avoids all of it.
foreach(qml_file IN LISTS EMG_QML_FILES)
    get_filename_component(qml_base "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_base}")
endforeach()

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Headers carrying QML_ELEMENT / QML_SINGLETON must be listed so qmltyperegistrar
# sees them; the .cpp files alone are not enough.
qt_add_qml_module(emg-viewer
    URI EmgViewer
    VERSION 1.0
    # Pinned: Qt 6.4 defaults this to "/" while 6.5+ defaults to "/qt/qml", so
    # leaving it implicit would move the URL main.cpp loads.
    RESOURCE_PREFIX "/qt/qml"
    SOURCES
        src/model/Acquisition.h
        src/model/DeviceManager.h
        src/model/LogModel.h
        src/model/LinkStats.h
        src/model/SignalModel.h
        src/ui/SpectrumItem.h
        src/ui/SpectrumTrace.h
        src/ui/WaveItem.h
        src/ui/WaveTrace.h
    QML_FILES ${EMG_QML_FILES}
)

target_link_libraries(emg-viewer PRIVATE
    emg_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
)

install(TARGETS emg-viewer RUNTIME DESTINATION bin)

# ---------------------------------------------------------------------------
# Firmware protocol source, for cross-validation.
#
# firmware/ is a separate git repository nested in the same tree. It may not be
# checked out, so treat it as optional: without it the app still builds, we
# just lose the test that proves both halves of the protocol agree.
# ---------------------------------------------------------------------------
set(EMG_FIRMWARE_PROTO "${CMAKE_CURRENT_SOURCE_DIR}/../../firmware/proto"
    CACHE PATH "Path to firmware/proto, for protocol cross-validation tests")

if(EXISTS "${EMG_FIRMWARE_PROTO}/src/emg_frame.c")
    # Synthetic capture generator - lets the UI and replay path be developed and
    # regression-tested with no board attached.
    add_executable(emg_gen
        tools/emg_gen.c
        ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
    )
    target_include_directories(emg_gen PRIVATE ${EMG_FIRMWARE_PROTO}/inc)
    set_target_properties(emg_gen PROPERTIES C_STANDARD 99)
    target_link_libraries(emg_gen PRIVATE m)
endif()

include(CTest)

if(BUILD_TESTING)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)

    # Band-limited reconstruction. Header-only and firmware-independent, so this
    # one runs even without firmware/ checked out.
    qt_add_executable(tst_sampleloss tests/tst_sampleloss.cpp)
    target_include_directories(tst_sampleloss PRIVATE src)
    target_link_libraries(tst_sampleloss PRIVATE emg_core Qt6::Test)
    add_test(NAME sampleloss COMMAND tst_sampleloss)

    qt_add_executable(tst_resample tests/tst_resample.cpp)
    target_include_directories(tst_resample PRIVATE src)
    target_link_libraries(tst_resample PRIVATE emg_core Qt6::Test)
    add_test(NAME resample COMMAND tst_resample)

    # EMG conditioning chain: band-pass, mains notch, RMS envelope.
    qt_add_executable(tst_emgfilter tests/tst_emgfilter.cpp)
    target_include_directories(tst_emgfilter PRIVATE src)
    target_link_libraries(tst_emgfilter PRIVATE emg_core Qt6::Test)
    add_test(NAME emgfilter COMMAND tst_emgfilter)

    # QRS detection / heart rate, checked against synthetic ECG at known rates.
    qt_add_executable(tst_heartrate tests/tst_heartrate.cpp)
    target_include_directories(tst_heartrate PRIVATE src)
    target_link_libraries(tst_heartrate PRIVATE emg_core Qt6::Test)
    add_test(NAME heartrate COMMAND tst_heartrate)

    # FFT and Welch spectrum, checked against closed-form answers.
    qt_add_executable(tst_spectrum tests/tst_spectrum.cpp)
    target_include_directories(tst_spectrum PRIVATE src)
    target_link_libraries(tst_spectrum PRIVATE emg_core Qt6::Test)
    add_test(NAME spectrum COMMAND tst_spectrum)

    # Trace geometry: where a microvolt and a sample index land on the plot, and the
    # feathered stroke built around them. The amplitude / time-axis exactness the spec
    # asks for is a property of this code, checked here on numbers.
    qt_add_executable(tst_tracegeometry tests/tst_tracegeometry.cpp src/ui/TraceGeometry.cpp)
    target_include_directories(tst_tracegeometry PRIVATE src)
    target_link_libraries(tst_tracegeometry PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
    add_test(NAME tracegeometry COMMAND tst_tracegeometry)

    # The ring read the trace renderer depends on: a window that ends where it
    # was asked to, however far the producer has moved on.
    qt_add_executable(tst_samplering tests/tst_samplering.cpp)
    target_include_directories(tst_samplering PRIVATE src)
    target_link_libraries(tst_samplering PRIVATE emg_core Qt6::Test)
    add_test(NAME samplering COMMAND tst_samplering)

    # Live frequency-domain engine: 512-pt Hann, exponential averaging, axis
    # smoothing, peak / median / band power. Closed-form expectations only.
    qt_add_executable(tst_spectrumengine tests/tst_spectrumengine.cpp)
    target_include_directories(tst_spectrumengine PRIVATE src)
    target_link_libraries(tst_spectrumengine PRIVATE emg_core Qt6::Test)
    add_test(NAME spectrumengine COMMAND tst_spectrumengine)

    if(EXISTS "${EMG_FIRMWARE_PROTO}/src/emg_frame.c")
        find_package(Qt6 6.4 REQUIRED COMPONENTS Test)

        # Everything the window shows, with no window: presentation strings checked
        # against the spec's text, and measurements (BPM, alpha peak, EMG RMS)
        # checked against synthetic captures whose true answers are known. Replays
        # the real decode path; nothing is mocked.
        qt_add_executable(tst_signalmodel
            tests/tst_signalmodel.cpp
            ${EMG_MODEL_SOURCES}
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_signalmodel PRIVATE
            src src/core src/io src/model ${EMG_FIRMWARE_PROTO}/inc)
        target_link_libraries(tst_signalmodel PRIVATE emg_core Qt6::Test Qt6::Qml Qt6::SerialPort)
        set_source_files_properties(${EMG_FIRMWARE_PROTO}/src/emg_frame.c
            PROPERTIES C_STANDARD 99)
        add_test(NAME signalmodel COMMAND tst_signalmodel)

        # The Wi-Fi tab: address validation, what is remembered, and that Connect does
        # not claim a link that does not exist yet.
        qt_add_executable(tst_wifiaddress
            tests/tst_wifiaddress.cpp
            ${EMG_MODEL_SOURCES}
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_wifiaddress PRIVATE
            src src/core src/io src/model ${EMG_FIRMWARE_PROTO}/inc)
        target_link_libraries(tst_wifiaddress PRIVATE emg_core Qt6::Test Qt6::Qml Qt6::SerialPort)
        set_source_files_properties(${EMG_FIRMWARE_PROTO}/src/emg_frame.c
            PROPERTIES C_STANDARD 99)
        add_test(NAME wifiaddress COMMAND tst_wifiaddress)

        # The trace renderer, measured in pixels: amplitude and time axis exact at
        # every gain and window, markers on the right column. Added after a 1.8x
        # amplitude error and a stretched time axis both shipped looking correct.
        qt_add_executable(tst_waveitem
            tests/tst_waveitem.cpp
            ${EMG_MODEL_SOURCES}
            src/ui/WaveItem.cpp
            src/ui/WaveItem.h
            src/ui/WaveTrace.cpp
            src/ui/WaveTrace.h
            src/ui/TraceGeometry.cpp
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_waveitem PRIVATE
            src src/core src/io src/model src/ui ${EMG_FIRMWARE_PROTO}/inc)
        target_link_libraries(tst_waveitem PRIVATE emg_core Qt6::Test Qt6::Qml Qt6::Quick
            Qt6::SerialPort)
        add_test(NAME waveitem COMMAND tst_waveitem)

        qt_add_executable(tst_frameparser
            tests/tst_frameparser.cpp
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_frameparser PRIVATE
            src
            ${EMG_FIRMWARE_PROTO}/inc
        )
        target_link_libraries(tst_frameparser PRIVATE emg_core Qt6::Test)
        set_source_files_properties(${EMG_FIRMWARE_PROTO}/src/emg_frame.c
            PROPERTIES C_STANDARD 99)

        add_test(NAME frameparser COMMAND tst_frameparser)

        # End-to-end: capture file -> ReplaySource -> SampleRing, plus the
        # recorder round-trip. Shares the decode path with live acquisition.
        qt_add_executable(tst_replay
            tests/tst_replay.cpp
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_replay PRIVATE
            src
            ${EMG_FIRMWARE_PROTO}/inc
        )
        target_link_libraries(tst_replay PRIVATE emg_core Qt6::Test)

        add_test(NAME replay COMMAND tst_replay)

        # Plumbing: source -> filters -> envelope bank -> rings. The filter unit
        # tests do not cover the wiring, and that is where a regression hid.
        qt_add_executable(tst_pipeline
            tests/tst_pipeline.cpp
            ${EMG_FIRMWARE_PROTO}/src/emg_frame.c
        )
        target_include_directories(tst_pipeline PRIVATE src ${EMG_FIRMWARE_PROTO}/inc)
        target_link_libraries(tst_pipeline PRIVATE emg_core Qt6::Test)
        add_test(NAME pipeline COMMAND tst_pipeline)
    else()
        message(WARNING
            "firmware/proto not found at ${EMG_FIRMWARE_PROTO} - skipping the "
            "protocol cross-validation test. The app still builds.")
    endif()
endif()
