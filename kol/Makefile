# Makefile for kol

include ../common.mk

INCLUDES  +=
LIBS	  +=

FLAGS     = $(CXXFLAGS) $(INCLUDES)

LIB_DIR   = lib
BLD_DIR   = build
SRC_DIR   = src

LIB_TGT   = libkol.a
LIB_OBJ   = koltcp.o kolthread.o kolsocket.o koluri.o

SOURCES   = $(notdir $(wildcard $(SRC_DIR)/*.cc))
DEPENDS   = $(addprefix $(BLD_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT)

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(BLD_DIR)/, $(LIB_OBJ))
	@echo Creating $@
	@mkdir -p $(LIB_DIR)
	@ar cr $@ $^
	@ranlib $@

$(BLD_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@mkdir -p $(BLD_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(LIB_DIR)/*
	@rm -f $(BLD_DIR)/*

-include $(DEPENDS)
