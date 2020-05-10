TARGET = Jamtaba2
TEMPLATE = app

!novideo {
    DEFINES += USE_VIDEO_GRABBER
} else {
    message(DISABLE VIDEO)
}

include(../Jamtaba-common.pri)

linux {
message(INFO: for build on Ubuntu try)
message($ sudo apt-get install -y libavformat-dev libswscale-dev libvorbis-dev libminiupnpc-dev portaudio19-dev libz-dev libx264-dev)
message(* for disable VST plugin add CONFIG+=novst)
message(* for disable VIDEO add CONFIG+=novideo)
message(* for use mp3lame add CONFIG+=mp3lame and on Ubuntu try)
message("  $ sudo apt-get install -y libmp3lame-dev")
} else {
    INCLUDEPATH += $$ROOT_PATH/libs/includes/portaudio
}

VPATH += $$SOURCE_PATH
VPATH += $$SOURCE_PATH/Standalone

INCLUDEPATH += $$SOURCE_PATH/Libs
INCLUDEPATH += $$SOURCE_PATH/Libs/RtMidi

!novst {
    DEFINES += USE_VST_PLUGIN
} else {
    message(DISABLE VST plugin)
}
INCLUDEPATH += $$ROOT_PATH/libs/includes/rtmidi

INCLUDEPATH += $$SOURCE_PATH/Standalone

INCLUDEPATH += $$SOURCE_PATH/Standalone/gui


HEADERS += MainControllerStandalone.h \
    ../../src/Common/audio/core/LinuxAlsaAudioDriver.h
HEADERS += gui/MainWindowStandalone.h
HEADERS += gui/PreferencesDialogStandalone.h
HEADERS += gui/LocalTrackViewStandalone.h
HEADERS += gui/LocalTrackGroupViewStandalone.h
HEADERS += gui/ScanFolderPanel.h
HEADERS += gui/FxPanel.h
HEADERS += gui/FxPanelItem.h
HEADERS += gui/MidiToolsDialog.h
HEADERS += gui/CrashReportDialog.h
HEADERS += audio/PortAudioDriver.h
HEADERS += audio/Host.h
HEADERS += midi/RtMidiDriver.h
HEADERS += PluginFinder.h
HEADERS += Libs/SingleApplication/singleapplication.h
HEADERS += Libs/RtMidi/RtMidi.h

mac:HEADERS += AU/AudioUnitHost.h
mac:HEADERS += AU/AudioUnitPlugin.h

SOURCES += main.cpp \
    ../../src/Common/audio/core/LinuxAlsaAudioDriver.cpp
SOURCES += MainControllerStandalone.cpp
SOURCES += ConfiguratorStandalone.cpp
SOURCES += gui/MainWindowStandalone.cpp
SOURCES += gui/PreferencesDialogStandalone.cpp
SOURCES += gui/LocalTrackViewStandalone.cpp
SOURCES += gui/LocalTrackGroupViewStandalone.cpp
SOURCES += gui/ScanFolderPanel.cpp
SOURCES += gui/FxPanel.cpp
SOURCES += gui/FxPanelItem.cpp
SOURCES += gui/MidiToolsDialog.cpp
SOURCES += midi/RtMidiDriver.cpp
SOURCES += PluginFinder.cpp
SOURCES += Libs/SingleApplication/singleapplication.cpp
SOURCES += Libs/RtMidi/RtMidi.cpp
SOURCES += audio/PortAudioDriver.cpp
SOURCES += gui/CrashReportDialog.cpp

mac:SOURCES += AU/AudioUnitHost.cpp
mac:SOURCES += AU/AudioUnitPluginFinder.cpp

!novst {
    INCLUDEPATH += $$VST_SDK_PATH/VST2_SDK/pluginterfaces/vst2.x

    DEFINES += USE_VST_PLUGIN

    HEADERS += vst/VstPlugin.h
    HEADERS += vst/VstHost.h
    HEADERS += vst/VstLoader.h
    HEADERS += vst/VstPluginFinder.h
    HEADERS += vst/Utils.h

    SOURCES += vst/VstPlugin.cpp
    SOURCES += vst/VstHost.cpp
    SOURCES += vst/VstPluginFinder.cpp
    SOURCES += vst/Utils.cpp
    SOURCES += vst/VstLoader.cpp

    win32{
        SOURCES += vst/WindowsVstPluginChecker.cpp
    }
    macx{
        OBJECTIVE_SOURCES += vst/MacVstPluginChecker.mm
    }
    linux{
        SOURCES += vst/LinuxVstPluginChecker.cpp
    }
}

FORMS += gui/MidiToolsDialog.ui
FORMS += gui/CrashReportDialog.ui

#conditional sources to different platforms
win32{
    SOURCES += audio/WindowsPortAudioDriver.cpp
}
macx{

    AU_SDK_PATH = "$$PWD/../../AU_SDK"
    VPATH += $$AU_SDK_PATH

    SOURCES += audio/MacPortAudioDriver.cpp

    HEADERS += AU/AudioUnitPlugin.h
    OBJECTIVE_SOURCES += AU/AudioUnitPlugin.mm

    INCLUDEPATH += $$AU_SDK_PATH
}
linux{
    SOURCES += audio/LinuxPortAudioDriver.cpp
}


win32{

    #supressing warning about missing .pdb files
    QMAKE_LFLAGS += /ignore:4099

    !contains(QMAKE_TARGET.arch, x86_64) {
        #message("msvc x86 build") ## Windows x86 (32bit) specific build here
        LIBS_PATH = "static/win32-msvc"

        #after a lot or research Ezee found this userfull link explaining how compile to be compatible with Windows XP: http://www.tripleboot.org/?p=423
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    } else {
        #message("msvc x86_64 build") ## Windows x64 (64bit) specific build here
        LIBS_PATH = "static/win64-msvc"
    }

    CONFIG(release, debug|release): LIBS += -L$$PWD/../../libs/$$LIBS_PATH -lportaudio
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libs/$$LIBS_PATH/ -lportaudiod

    LIBS += -L$$PWD/../../libs/$$LIBS_PATH -lminimp3 -lvorbisfile -lvorbis -logg -lx264 -lavcodec -lavutil -lavformat -lswscale -lswresample -lstackwalker -lminiupnpc

    CONFIG(release, debug|release) {
        #ltcg - http://blogs.msdn.com/b/vcblog/archive/2009/02/24/quick-tips-on-using-whole-program-optimization.aspx
        QMAKE_CXXFLAGS_RELEASE +=  -GL -Gy -Gw
        QMAKE_LFLAGS_RELEASE += /LTCG
    }

    LIBS += -lwinmm -lole32 -lws2_32 -lAdvapi32 -lUser32 -lPsapi
    LIBS += -lIPHlpApi # used by miniupnp lib
    LIBS += -lSecur32   # used by libx264

    #performance monitor lib
    QMAKE_CXXFLAGS += -DPSAPI_VERSION=1

    RC_FILE = ../Jamtaba2.rc #windows icon

    QMAKE_LFLAGS += "/NODEFAULTLIB:libcmt"
}

macx{
    #message("Mac x86_64 build")
    LIBS_PATH = "static/mac64"

    LIBS += -L$$PWD/../../libs/$$LIBS_PATH -lportaudio -lminimp3 -lvorbisfile -lvorbisenc -lvorbis -logg -lx264 -lavcodec -lavutil -lavformat -lswscale -lswresample -liconv -lminiupnpc
    LIBS += -framework IOKit
    LIBS += -framework CoreAudio
    LIBS += -framework CoreMidi
    LIBS += -framework AudioToolbox
    LIBS += -framework AudioUnit
    LIBS += -framework CoreServices
    LIBS += -framework Carbon
    LIBS += -framework Cocoa

    #mac osx doc icon
    ICON = ../Jamtaba.icns
}

linux{
    contains(QMAKE_HOST.arch, x86_64) {
        LIBS_PATH = "static/linux64"
    } else {
        LIBS_PATH = "static/linux32"
    }
    message($$LIBS_PATH)

    mp3lame {
        DEFINES += __LINUX_LAME__
        LIBS += -lmp3lame
    }
    INCLUDEPATH += /usr/include/miniupnpc

    LIBS += -L$$PWD/../../libs/$$LIBS_PATH -lportaudio -lvorbisfile -lvorbisenc -lvorbis -logg -lavformat -lavcodec -lswscale -lavutil -lswresample -lminiupnpc -lx264
    LIBS += -lasound
    LIBS += -ldl
    LIBS += -lz
}
