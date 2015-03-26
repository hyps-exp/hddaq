# Makefile for Massage/test

CXX	  = g++
CXXFLAGS  = -O2 -Wall

INCLUDES  = -I../ -I../../kol
LIBS	  = -L../lib -lMessage \
            -L../../kol/lib -lkol \
            -lpthread

FLAGS     = $(CXXFLAGS) $(INCLUDES)

BIN_DIR   = bin
TMP_DIR   = tmp

BIN_TGT   = msgs msgc msgtest msgdtest msgd_dummy

SOURCES   = $(wildcard *.cc)
DEPENDS   = $(addprefix $(TMP_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(addprefix $(BIN_DIR)/, $(BIN_TGT))

$(BIN_DIR)/%: $(TMP_DIR)/%.o
	@echo Linking $@ ...
	@mkdir -p $(BIN_DIR)
	@$(CXX) -o $@ $^ $(LIBS)

$(TMP_DIR)/%.o: %.cc
	@echo Compiling $< ...
	@mkdir -p $(TMP_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(BIN_DIR)/*
	@rm -f $(TMP_DIR)/*

-include $(DEPENDS)