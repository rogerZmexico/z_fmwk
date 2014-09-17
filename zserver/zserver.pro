TEMPLATE	= app
TARGET		= zserver

#~ CONFIG		+= qt warn_on release
CONFIG		+= qt warn_on debug

DEPENDPATH	= ../../../include
INCLUDEPATH += ../hdr

# FOR lizard-ARM CROSS-BUILDING ONLY::START (COMMENT THIS TO BUILD FOR HOST)
#~ INCLUDEPATH += /opt/lizard/rfs/usr/include/asm/arch
#~ INCLUDEPATH += /opt/lizard/rfs/usr/include
#~ LIBS		+=  -L/opt/lizard/rfs/lib
# FOR lizard-ARM CROSS-BUILDING ONLY::END

UI_DIR  = ../ui

#~ quilib section:-----------------------------------------
INCLUDEPATH += ../../../tools/designer
LIBS += -lqui
#~ ----------------------------------------------------------------

#~ polymer section:-----------------------------------------
#~ LIBS		+=  -L/opt/lizard/rfs/opt/lib
INCLUDEPATH += /opt/polymer-0.3.1/style
LIBS += -lpolymer
#~ ----------------------------------------------------------------

#~ ----------------------------------------------------------------
#~ INCLUDEPATH += /opt/QtCurve-KDE3-1.7.0/style
#~ INCLUDEPATH += /opt/QtCurve-KDE3-1.7.0/common
#~ INCLUDEPATH += /opt/QtCurve-KDE3-1.7.0/config
#~ INCLUDEPATH += /opt/QtCurve-KDE3-1.7.0/build

#~ LIBS		+=  -L/opt/QtCurve-KDE3-1.7.0/build/style
#~ LIBS += -lqtcurve
#~ ----------------------------------------------------------------

HEADERS = \
    ../hdr/g_label.h \
    ../hdr/rotor.h \
    ../hdr/plot.h \
    ../hdr/mywidgetfactory.h \
    ../hdr/mysockets.h \
    ../hdr/globals.h \
    ../hdr/zserver.h

SOURCES	= \
    ../src/g_label.cpp \
    ../src/rotor.cpp \
    ../src/plot.cpp \
    ../src/mywidgetfactory.cpp \
    ../src/mysockets.cpp \
    ../src/zserver.cpp \
    ../src/main.cpp

FORMS   = \
    ../ui/splash.ui

# TO EMBED PYTHON ON HOST_____________________START
# XXX Top of the build tree and source tree
#~ INCLUDEPATH += /opt/Python-2.4.4-host/Include
#~ INCLUDEPATH += /opt/Python-2.4.4-host
#~ #LIBS +=  /opt/Python-2.4.4-host/libpython2.4.a
#~ LIBS +=  /usr/lib/libpython2.4.so
#~ LIBS +=  -lpthread -ldl  -lutil -lm
# TO EMBED PYTHON ON HOST______________________END

# TO EMBED PYTHON ON ARM _____________________START
# XXX Top of the build tree and source tree
#~ INCLUDEPATH += /opt/Python-2.4.4/Include
#~ INCLUDEPATH += /opt/Python-2.4.4
#~ LIBS +=  -lpthread -ldl  -lpthread -lutil
#~ LIBS +=  /opt/eldk/eldk-41-arm/rootfs-big/lib/libm.so.6
#~ LIBS +=  /opt/eldk/eldk-41-arm/rootfs-big/lib/libc.so.6
#~ #LIBS +=  /opt/Python-2.4.4/libpython2.4.a
#~ LIBS +=  /opt/eldk/arm/opt/lib/libpython2.4.so
# TO EMBED PYTHON ON ARM ______________________END
