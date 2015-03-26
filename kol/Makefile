# Makefile for kol

CXX	  = g++
CXXFLAGS  = -O2 -Wall

INCLUDES  = -I.
LIBS	  =

FLAGS     = $(CXXFLAGS) $(INCLUDES)

LIB_DIR   = lib
TMP_DIR   = tmp
SRC_DIR   = src

LIB_TGT   = libkol.a
LIB_OBJ   = koltcp.o kolthread.o kolsocket.o koluri.o 

SOURCES   = $(notdir $(wildcard $(SRC_DIR)/*.cc))
DEPENDS   = $(addprefix $(TMP_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT)

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(TMP_DIR)/, $(LIB_OBJ))
	@echo Creating $@
	@mkdir -p $(LIB_DIR)
	@ar cr $@ $^
	@ranlib $@

$(TMP_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@mkdir -p $(TMP_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(LIB_DIR)/*
	@rm -f $(TMP_DIR)/*

-include $(DEPENDS)