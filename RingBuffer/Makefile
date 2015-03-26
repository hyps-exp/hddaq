CXX      = g++
CXXFLAGS = -O2 -Wall

INCLUDES = -I. -I../kol -I../EventData
LIBS     = -L../kol/lib -lkol \
           -L../EventData -lEventData \
           -lpthread

FLAGS    = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src

BIN_TGT =
BIN_OBJ =
LIB_TGT = libRingBuffer.a
LIB_OBJ = RingBuffer.o

###Stopping make delete intermediate files
.SECONDARY:

all: $(LIB_DIR)/$(LIB_TGT) \
     $(addprefix $(BIN_DIR)/, $(BIN_TGT))

$(LIB_DIR)/$(LIB_TGT): $(addprefix $(SRC_DIR)/, $(LIB_OBJ))
	@echo Creating $@ ...
	@mkdir -p $(LIB_DIR)
	@ar cr $@ $^
	@ranlib $@ 

$(BIN_DIR)/%: $(SRC_DIR)/%.o
	@echo Linking $@ ...
	@mkdir -p $(BIN_DIR)
	@$(CXX) -o $@ $^ $(LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(LIB_DIR)/*
	@rm -f $(BIN_DIR)/*
	@rm -f $(SRC_DIR)/*.o

