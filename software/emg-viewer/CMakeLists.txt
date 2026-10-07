cmake_minimum_required(VERSION 3.21)
project(emg-viewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 SerialPort Network)
qt_standard_project_setup(REQUIRES 6.4)

qt_add_executable(emg-viewer
    src/main.cpp

    src/core/Biquad.cpp
    src/core/Crc16.cpp
    src/core/EmgFilter.cpp
    src/core/Fft.cpp
    src/core/FrameParser.cpp
    src/core/HeartRate.cpp
    src/core/Spectrum.cpp
    src/core/SpectrumEngine.cpp

    src/io/ISampleSource.h      # header-only QObject: listed so AUTOMOC sees it
    src/io/Recorder.cpp
    src/io/ReplaySource.cpp
    src/io/SerialSource.cpp
    src/io/TcpClientSource.cpp

    src/model/Acquisition.cpp
    src/model/DeviceManager.cpp
    src/model/LogModel.cpp
    src/model/SignalModel.cpp
    src/model/LinkStats.h       # header-only Q_GADGET: same reason

    src/ui/SpectrumItem.cpp
    src/ui/SpectrumTrace.cpp
    src/ui/TraceGeometry.cpp
    src/ui/WaveItem.cpp
    src/ui/WaveTrace.cpp
)

target_compile_definitions(emg-viewer PRIVATE EMG_VIEWER_VERSION="${PROJECT_VERSION}")

# qmltyperegistrar includes headers by basename, so each directory must be on the path.
target_include_directories(emg-viewer PRIVATE src src/core src/io src/model src/ui)

# Bundled fonts: Inter and the Phosphor icon font.
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

# Keep every .qml at the module root, beside qmldir. Left in a qml/ subdirectory,
# `singleton` is silently dropped, Theme's properties read undefined, and the layout
# engine spins at 100% CPU with no error. Do not remove.
foreach(qml_file IN LISTS EMG_QML_FILES)
    get_filename_component(qml_base "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_base}")
endforeach()
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(emg-viewer
    URI EmgViewer
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"   # pinned: Qt 6.4 defaults to "/", 6.5+ to "/qt/qml"
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
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::SerialPort Qt6::Network
)

install(TARGETS emg-viewer RUNTIME DESTINATION bin)
