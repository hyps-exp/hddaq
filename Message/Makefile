CXX	 = g++
CXXFLAGS = -O2 -Wall

INCLUDES = -I. -I../kol
LIBS	 = -L../kol/lib -L./lib -lMessage -lkol -lpthread

FLAGS    = $(CFLAGS) $(INCLUDES)

LIB_DIR = lib
BIN_DIR = bin

SRC_DIR = src

all: $(LIB_DIR)/libMessage.a $(BIN_DIR)/msgd $(BIN_DIR)/msgs $(BIN_DIR)/msgc $(BIN_DIR)/msgtest $(BIN_DIR)/msgdtest $(BIN_DIR)/cmsgd $(BIN_DIR)/msgd_dummy
 
$(LIB_DIR)/libMessage.a: $(addprefix $(SRC_DIR)/, GlobalMessageClient.o Message.o MessageClient.o MessageSocket.o)
	@echo Creating $@ ...
	@ar cr $@ $^
	@ranlib $@ 

$(BIN_DIR)/msgd: $(SRC_DIR)/msgd.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/msgs: $(SRC_DIR)/msgs.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/msgc: $(SRC_DIR)/msgc.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/msgtest: $(SRC_DIR)/msgtest.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/msgdtest: $(SRC_DIR)/msgdtest.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/cmsgd: $(SRC_DIR)/cmsgd.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(BIN_DIR)/msgd_dummy: $(SRC_DIR)/msgd_dummy.o
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f *.o
	@rm -f $(LIB_DIR)/*
	@rm -f $(BIN_DIR)/*
	@rm -f $(SRC_DIR)/*.o

