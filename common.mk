# common.mk

CXX		:= g++
CXXFLAGS	:= -O2 -Wall
# CXXFLAGS	+= -mtune=native -march=native -mfpmath=both

INCLUDES 	:= -I.
LIBS		:= -lpthread -lrt
