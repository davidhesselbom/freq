# -------------------------------------------------
# Project created by QtCreator 2009-11-06T11:26:14
# -------------------------------------------------


# features directory
qtfeatures = ../qtfeatures/
win32:qtfeatures = ..\\\\qtfeatures\\\\

####################
# Project settings

TARGET = sonicawe
!win32:customtarget {
    # Changing the target would also change the name of the genereated VC++ project file which would break the configuration in the .sln-file. Therefore, in windows, rename the generated executable afterwards instead.
    TARGET = $$CUSTOMTARGET
}

customtarget {
    DEFINES += TARGETNAME=$${TARGETNAME}
}

testlib {
    TEMPLATE = lib
    win32:TEMPLATE = vclib
    CONFIG += sharedlib
    DEFINES += SAWE_EXPORTDLL
} else {
    DEFINES += SAWE_NODLL
    TEMPLATE = app
    win32:TEMPLATE = vcapp
    win32:CONFIG -= embed_manifest_dll
    win32:CONFIG += embed_manifest_exe
}


CONFIG += $${qtfeatures}buildflags
#CONFIG += console # console output
# QMAKE_CXXFLAGS_DEBUG can't be changed in a .prf (feature) file
QMAKE_CXXFLAGS_DEBUG += -D_DEBUG

unix:QMAKE_CXXFLAGS_RELEASE += -fopenmp
unix:QMAKE_LFLAGS_RELEASE += -fopenmp
win32:QMAKE_CXXFLAGS_RELEASE += /openmp

####################
# Source code

RESOURCES += \
    ui/icon-resources.qrc \

SOURCES += \
    adapters/*.cpp \
    filters/*.cpp \
    heightmap/*.cpp \
    sawe/*.cpp \
    signal/*.cpp \
    tfr/fft4g.c \
    tfr/*.cpp \
    tools/*.cpp \
    tools/commands/*.cpp \
    tools/support/*.cpp \
    tools/selections/*.cpp \
    tools/selections/support/*.cpp \
    ui/*.cpp \

#Windows Icon
win32:SOURCES += sonicawe.rc \

HEADERS += \
    adapters/*.h \
    filters/*.h \
    heightmap/*.h \
    sawe/*.h \
    signal/*.h \
    tfr/*.h \
    tools/*.h \
    tools/commands/*.h \
    tools/support/*.h \
    tools/selections/*.h \
    tools/selections/support/*.h \
    ui/*.h \

PRECOMPILED_HEADER += sawe/project_header.h

# Qt Creator crashes every now and then in Windows if form filenames are expressed with wildcards
FORMS += \
    ui/mainwindow.ui \
    ui/propertiesselection.ui \
    ui/propertiesstroke.ui \
    tools/aboutdialog.ui \
    tools/commentview.ui \
    tools/selectionviewinfo.ui \
    tools/transforminfoform.ui \
    tools/exportaudiodialog.ui \
    tools/harmonicsinfoform.ui \
    tools/matlaboperationwidget.ui \
    tools/selections/rectangleform.ui \
    sawe/enterlicense.ui \
    tools/settingsdialog.ui \
    tools/dropnotifyform.ui \
    tools/sendfeedback.ui \
    tools/commands/commandhistory.ui \
    tools/splashscreen.ui \

CUDA_SOURCES += \
    filters/*.cu \
    heightmap/*.cu \
    tfr/*.cu \
    tools/support/*.cu \
    tools/selections/support/*.cu \

SHADER_SOURCES += \
    heightmap/heightmap.frag \
    heightmap/heightmap.vert \
    heightmap/heightmap_noshadow.vert \

CONFIGURATION_SOURCES = \
    sawe/configuration/configuration.cpp

FEATURE_SOURCES += \
    $${qtfeatures}*.prf

# "Other files" for Qt Creator
OTHER_FILES += \
    $$CUDA_SOURCES \
    $$SHADER_SOURCES \
    $$CONFIGURATION_SOURCES \
    $$FEATURE_SOURCES \
    sonicawe.rc \
    ../README \

# "Other files" for Visual Studio
OTHER_FILES_VS += \
    $$SHADER_SOURCES \
    $$FEATURE_SOURCES \
    *.pro \
    ../README \

CONFIG += $${qtfeatures}otherfilesvs


####################
# Build settings
CONFIG += $${qtfeatures}sawelibs
QT += opengl
QT += network
DEFINES += SAWE_NO_MUTEX
#DEFINES += CUDA_MEMCHECK_TEST


####################
# Temporary output

TMPDIR=
customtarget: TMPDIR=target/$${TARGETNAME}
testlib {
    TMPDIR = lib/$${TMPDIR}
}

CONFIG += $${qtfeatures}tmpdir


# #######################################################################
# OpenCL
# #######################################################################
useopencl {
    SOURCES += \
        tfr/clfft/*.cpp

    HEADERS += \
        tfr/clfft/*.h

    CONFIG += $${qtfeatures}opencl
}


# #######################################################################
# CUDA
# #######################################################################
usecuda: CONFIG += $${qtfeatures}cuda


# #######################################################################
# Deploy configuration
# #######################################################################
# This whole configuration construct is rather unstable as it doesn't
# necessarily use the same compiler flags as the rest of the project. A
# proposed solutions is to crate a new project with the file
# configuration.cpp only.
CONFIGURATION_DEFINES += SONICAWE_BRANCH="$$system(git rev-parse --abbrev-ref HEAD)"
CONFIGURATION_DEFINES += SONICAWE_REVISION="$$system(git rev-parse --short HEAD)"
usecuda: CONFIGURATION_DEFINES += USE_CUDA

configuration.name = configuration
configuration.input = CONFIGURATION_SOURCES
configuration.dependency_type = TYPE_C
configuration.variable_out = OBJECTS
configuration.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
CONFIGURATION_FLAGS = $$QMAKE_CXXFLAGS
macx:CONFIGURATION_DEFINES += MAC_OS_X_VERSION_MAX_ALLOWED=1070
macx:CONFIGURATION_FLAGS += $$QMAKE_CFLAGS_X86_64
CONFIG(debug, debug|release): CONFIGURATION_FLAGS += $$QMAKE_CXXFLAGS_DEBUG
else:CONFIGURATION_FLAGS += $$QMAKE_CXXFLAGS_RELEASE
win32:CONFIGURATION_FLAGS += /EHsc
win32:CXX_OUTPARAM = /Fo
else:CXX_OUTPARAM = "-o "
unix:testlib:CONFIGURATION_FLAGS += -fPIC
configuration.commands = $${QMAKE_CXX} \
    $${CONFIGURATION_FLAGS} \
    $$join(CONFIGURATION_DEFINES,'" -D"','-D"','"') \
    $$join(DEFINES,'" -D"','-D"','"') \
    $(INCPATH) \
    -c ${QMAKE_FILE_IN} \
    $${CXX_OUTPARAM}"${QMAKE_FILE_OUT}"
QMAKE_EXTRA_COMPILERS += configuration
