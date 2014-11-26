CXX	 = g++
CXXFLAGS = -O2 -Wall

INCLUDES = -I. -I../kol
LIBS	 = -L./lib -lMessage \
           -L../kol/lib -lkol \
           -lpthread

FLAGS    = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src

BIN_TGT = msgd msgs msgc msgtest msgdtest cmsgd msgd_dummy
BIN_OBJ =
LIB_TGT = libMessage.a
LIB_OBJ = GlobalMessageClient.o Message.o MessageClient.o MessageSocket.o

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT) \
     $(addprefix $(BIN_DIR)/, $(BIN_TGT))

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(SRC_DIR)/, $(LIB_OBJ))
	@echo Creating $@ ...
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
