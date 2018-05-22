# Makefile for CotrolThread

include ../common.mk

INCLUDES += -I../Message \
            -I../kol
LIBS	 += -L./lib -lMessage \
            -L../kol/lib -lkol

FLAGS    = $(CXXFLAGS) $(INCLUDES)

LIB_DIR  = lib
BIN_DIR  = bin
BLD_DIR  = build
SRC_DIR  = src

BIN_TGT  =
BIN_OBJ  =
LIB_TGT  = libControlThread.a
LIB_OBJ  = consoleThread.o controlThread.o GlobalInfo.o NodeId.o statableThread.o

SOURCES   = $(notdir $(wildcard $(SRC_DIR)/*.cc))
DEPENDS   = $(addprefix $(BLD_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT) \
     $(addprefix $(BIN_DIR)/, $(BIN_TGT))

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(BLD_DIR)/, $(LIB_OBJ))
	@echo Creating $@ ...
	@mkdir -p $(LIB_DIR)
	@ar cr $@ $^
	@ranlib $@

$(BIN_DIR)/%: $(BLD_DIR)/%.o
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
