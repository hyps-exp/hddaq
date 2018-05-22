# Makefile for EventBuilder

include ../common.mk

INCLUDES += -I../RingBuffer \
            -I../EventData \
            -I../ControlThread \
            -I../Message \
            -I../kol
LIBS     += -L../RingBuffer/lib -lRingBuffer \
            -L../EventData/lib -lEventData \
            -L../ControlThread/lib -lControlThread \
            -L../Message/lib -lMessage \
            -L../kol/lib -lkol

FLAGS    = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin
BLD_DIR = build
SRC_DIR = src

BIN_TGT = EventBuilder
BIN_OBJ = EventBuilder.o builderThread.o readerThread.o syncReaderThread.o \
          slowReaderThread.o senderThread.o watchdog.o EbControl.o
LIB_TGT =
LIB_OBJ =

SOURCES   = $(notdir $(wildcard $(SRC_DIR)/*.cc))
DEPENDS   = $(addprefix $(BLD_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(BIN_DIR)/$(BIN_TGT)

$(BIN_DIR)/$(BIN_TGT): $(addprefix $(BLD_DIR)/, $(BIN_OBJ))
	@echo Linking $@ ...
	@mkdir -p $(BIN_DIR)
	@$(CXX) -o $@ $^ $(LIBS)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@mkdir -p $(BLD_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(LIB_DIR)/*
	@rm -f $(BIN_DIR)/*
	@rm -f $(BLD_DIR)/*

-include $(DEPENDS)
