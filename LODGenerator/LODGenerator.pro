# This specifies the exe name
TARGET=LODGenerator
# location of .o files
OBJECTS_DIR=obj
# core Qt Libs to use add more here if needed
QT+=gui opengl core

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for hte setMacVisual whis is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
        cache()
        DEFINES +=QT5BUILD
}
# where to put moc auto generated files
MOC_DIR=moc
# Auto include all .cpp files in the project src directory (can specifiy individually if required)
SOURCES+=$$PWD/src/*.cpp
# same for the .h files
HEADERS+=$$PWD/include/*.h
# include Form ui files
FORMS += $$PWD/ui/*.ui
# and add the include dir into the search path for Qt and make
INCLUDEPATH+=./include
# where our exe is going to live (root of project)
DESTDIR=./
# add the other files
OTHER_FILES+=   shaders/*.glsl \
                models/*.obj \
                textures/*.tga \
                ui/* \
                moc/*
# were are going to default to a console
CONFIG += console
# on a mac we don't create a .app bundle file ( for ease of multiplatform use)
CONFIG-=app_bundle

#moves ui generated headers to correct folder
UI_DIR=$$PWD/ui

# note each command you add needs a ; as it will be run as a single line
# first check if we are shadow building or not easiest way is to check out against current

!equals(PWD, $${OUT_PWD}){
        !win32 {
            copydata.commands = echo "creating destination dirs" ;
            # now make a dir
            copydata.commands += mkdir -p $$OUT_PWD/shaders ;
            copydata.commands += mkdir -p $$OUT_PWD/models ;
            copydata.commands += mkdir -p $$OUT_PWD/textures ;
            copydata.commands += echo "copying files" ;
            # then copy the files
            copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/ ;
            copydata.commands += $(COPY_DIR) $$PWD/models/* $$OUT_PWD/models/ ;
            copydata.commands += $(COPY_DIR) $$PWD/textures/* $$OUT_PWD/textures/ ;
            # now make sure the first target is built before copy
            first.depends = $(first) copydata
            export(first.depends)
            export(copydata.commands)
            # now add it as an extra target
            QMAKE_EXTRA_TARGETS += first copydata
        }
        else {
            QMAKE_POST_LINK += echo "creating windows destination dirs" &
            # now make a dir
            QMAKE_POST_LINK += mkdir "\"$$OUT_PWD/shaders\"" &
            QMAKE_POST_LINK += mkdir "\"$$OUT_PWD/models\"" &
            QMAKE_POST_LINK += mkdir "\"$$OUT_PWD/textures\"" &
            QMAKE_POST_LINK += echo "copying files" &
            # then copy the files
            QMAKE_POST_LINK += $(COPY_DIR) "\"$$PWD/shaders\"" "\"$$OUT_PWD/shaders\"" &
            QMAKE_POST_LINK += $(COPY_DIR) "\"$$PWD/models\"" "\"$$OUT_PWD/models\"" &
            QMAKE_POST_LINK += $(COPY_DIR) "\"$$PWD/textures\"" "\"$$OUT_PWD/textures\""
            export(QMAKE_POST_LINK)
        }
}
# use this to suppress some warning from boost
unix*:QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
# basic compiler flags (not all appropriate for all platforms)
!win32:QMAKE_CXXFLAGS+= -msse -msse2 -msse3
macx:QMAKE_CXXFLAGS+= -arch x86_64
macx:INCLUDEPATH+=/usr/local/include/
linux-g++:QMAKE_CXXFLAGS +=  -march=native
linux-g++-64:QMAKE_CXXFLAGS +=  -march=native
# define the _DEBUG flag for the graphics lib
DEFINES +=NGL_DEBUG

unix:LIBS += -L/usr/local/lib
# add the ngl lib
unix:LIBS +=  -L/$(HOME)/NGL/lib -l NGL

# now if we are under unix and not on a Mac (i.e. linux)
linux-*{
                linux-*:QMAKE_CXXFLAGS +=  -march=native
                DEFINES += LINUX
}
DEPENDPATH+=include
# if we are on a mac define DARWIN
macx:DEFINES += DARWIN
# this is where to look for includes
!win32:INCLUDEPATH += $$(HOME)/NGL/include/

win32: {
        PRE_TARGETDEPS+=$$(NGLDIR)/lib/NGL.lib
        INCLUDEPATH+=-I $$(BOOST)/include/boost-1_61
        INCLUDEPATH+=$$(NGLDIR)/include/
        DEFINES+=GL42
        DEFINES+=WIN32
        DEFINES+=_WIN32
        DEFINES+=_USE_MATH_DEFINES
        LIBS+=-lopengl32
        LIBS+=-L$$(NGLDIR)/lib -lNGL
        LIBS+=-L$$(BOOST)/lib/x64 -llibboost_thread-vc140-mt-1_61
        DEFINES+=NO_DLL
}



