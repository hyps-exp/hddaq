CXX	  = g++
CXXFLAGS  = -Wall

INCLUDES  = -I.
LIBS	  =

FLAGS     = $(CXXFLAGS) $(INCLUDES)

LIB_DIR = lib
SRC_DIR = src
OBJ = koltcp.o kolthread.o kolsocket.o koluri.o 

$(LIB_DIR)/libkol.a: $(addprefix $(SRC_DIR)/, $(OBJ))
	@echo Creating $@
	@ar cr $@ $^
	@ranlib $@ 

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@

clean:
	@echo Cleaning up ...
	@rm -f $(SRC_DIR)/*.o
	@rm -f $(LIB_DIR)/libkol.a

