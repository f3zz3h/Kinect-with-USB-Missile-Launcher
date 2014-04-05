OSTYPE := $(shell uname -s)

LIBS_ROOT=/home/luke/FinalYearProject/openni
OPENNI_ROOT=$(LIBS_ROOT)/OpenNI
NITE_ROOT=$(LIBS_ROOT)/NITE-Bin-Dev-Linux-x64-v1.5.2.23

SRC_FILES = \
	main.cpp \
	SceneDrawer.cpp \
	kbhit.cpp \
	signal_catch.cpp \
	launcher.cpp \
	tracking.cpp


EXE_NAME = KinectSentry

DEFINES += USE_GLUT

ifeq ("$(OSTYPE)","Darwin")
        LDFLAGS += -framework OpenGL -framework GLUT
else
        USED_LIBS += glut
endif

USED_LIBS += OpenNI GLU GL 

INC_DIRS += $(NITE_ROOT)/Include =/usr/include/ni $(NITE_ROOT)/Source/XnCommon =/usr/include/ni
BIN_DIR = ./Output
#LDFLAGS=-lusb-1.0

USED_LIBS += OpenNI XnVNite_1_5_2 GL glut

include $(NITE_ROOT)/Samples/Build/Common/CommonCppMakefile
#include ../../Common/CommonCppMakefile

###ONLY REQUIRED DURING FIRST RUN
#laptop:
#	DEFINES += FIRST_RUN
