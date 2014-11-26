CXX	  = g++
CXXFLAGS  = -Wall

INCLUDES  = -I.
LIBS	  =

FLAGS     = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src

BIN_TGT =
BIN_OBJ =
LIB_TGT = libkol.a
LIB_OBJ = koltcp.o kolthread.o kolsocket.o koluri.o 

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT)

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(SRC_DIR)/, $(LIB_OBJ))
	@echo Creating $@
	@ar cr $@ $^
	@ranlib $@ 

$(BIN_DIR)/%: $(SRC_DIR)/%.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(LIB_DIR)/*
	@rm -f $(BIN_DIR)/*
	@rm -f $(SRC_DIR)/*.o

