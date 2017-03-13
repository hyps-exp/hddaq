# Makefile for Frontend/skeleton_node

TARGET	= skeleton_frontend

CORE_DIR	= ../core
SRC_DIR		= src
BIN_DIR		= bin
BLD_DIR 	= build

CXX		= g++
CXXFLAGS	= -O2 -Wall

INCLUDES	= -I. -I$(CORE_DIR) -I../../kol -I../../Message
LIBS		= -L../../Message/lib -lMessage \
			-L../../kol/lib -lkol \
			-lpthread

FLAGS		= $(CXXFLAGS) $(INCLUDES)

# If you want frontend to restart at the run end (stop command),
# enable the flag below.
#FLAGS    += -DEND_AT_STOP

CORE_SRC	= $(notdir $(wildcard $(CORE_DIR)/*.cc))
CORE_OBJ	= $(CORE_SRC:.cc=.o)
USER_SRC	= $(notdir $(wildcard $(SRC_DIR)/*.cc))
USER_OBJ	= $(USER_SRC:.cc=.o)

SOURCES		= $(CORE_SRC) $(USER_SRC)
DEPENDS		= $(addprefix $(BLD_DIR)/, $(SOURCES:.cc=.d))

###Stopping make delete intermediate files
.SECONDARY:

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(addprefix $(BLD_DIR)/,$(CORE_OBJ)) \
                      $(addprefix $(BLD_DIR)/,$(USER_OBJ))
	@echo Linking $@ ...
	@mkdir -p $(BIN_DIR)
	@$(CXX) -o $@ $^ $(LIBS)

$(BLD_DIR)/%.o: $(CORE_DIR)/%.cc
	@echo Compiling $< ...
	@mkdir -p $(BLD_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

$(BLD_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@mkdir -p $(BLD_DIR)
	@$(CXX) $(FLAGS) -MMD -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(BIN_DIR)/$(TARGET)
	@rm -f $(BLD_DIR)/*
	@rm -f $(SRC_DIR)/*~ *~

-include $(DEPENDS)
