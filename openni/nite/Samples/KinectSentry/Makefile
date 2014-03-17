OSTYPE := $(shell uname -s)

SRC_FILES = \
	main.cpp \
	SceneDrawer.cpp \
	kbhit.cpp \
	signal_catch.cpp

INC_DIRS += ../../../../../Samples/Players

EXE_NAME = KinectSentry

DEFINES += USE_GLUT
USED_LIBS += glut

INC_DIRS += ../../Include =/usr/include/ni ../../Source/XnCommon =/usr/include/ni
BIN_DIR = ../Bin

USED_LIBS += OpenNI XnVNite_1_5_2

include ../Build/Common/CommonCppMakefile

