CXX	 = g++
CXXFLAGS = -O2 -Wall

INCLUDES = -I. \
           -I../RingBuffer \
           -I../EventData \
           -I../ControlThread \
           -I../Message \
           -I../kol

LIBS	 = -L../RingBuffer/lib -lRingBuffer \
           -L../EventData/lib -lEventData \
           -L../ControlThread/lib -lControlThread \
           -L../Message/lib -lMessage \
           -L../kol/lib -lkol \
           -lpthread

FLAGS    = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src

BIN_TGT = EventBuilder
BIN_OBJ = EventBuilder.o builderThread.o readerThread.o syncReaderThread.o \
          slowReaderThread.o senderThread.o watchdog.o EbControl.o
LIB_TGT =
LIB_OBJ =

###Stopping make delete intermediate files
.SECONDARY:

all: $(BIN_DIR)/$(BIN_TGT) 

$(BIN_DIR)/$(BIN_TGT): $(addprefix $(SRC_DIR)/, $(BIN_OBJ))
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
