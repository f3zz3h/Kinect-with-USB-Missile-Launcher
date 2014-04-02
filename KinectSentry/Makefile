OSTYPE := $(shell uname -s)

SRC_FILES = \
	main.cpp \
	SceneDrawer.cpp \
	kbhit.cpp \
	signal_catch.cpp

#INC_DIRS += ../../../../../Samples/KinectSentry

EXE_NAME = KinectSentry

DEFINES += USE_GLUT

ifeq ("$(OSTYPE)","Darwin")
        LDFLAGS += -framework OpenGL -framework GLUT
else
        USED_LIBS += glut
endif

USED_LIBS += OpenNI GLU GL 

INC_DIRS += ../../Include =/usr/include/ni ../../Source/XnCommon =/usr/include/ni
BIN_DIR = ../Bin

USED_LIBS += OpenNI XnVNite_1_5_2 GL glut

include ../Build/Common/CommonCppMakefile
#include ../../Common/CommonCppMakefile

###ONLY REQUIRED DURING FIRST RUN
#laptop:
#	DEFINES += FIRST-RUN