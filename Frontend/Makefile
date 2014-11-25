CXX	 = g++
CXXFLAGS = -O2 -Wall

INCLUDES = -I./core -I./user -I../kol -I../Message
LIBS	 = -L../Message/lib -lMessage \
	   -L../kol/lib -lkol -lpthread \
           -L/usr/lib/gef -lvme

FLAGS	 = $(CXXFLAGS) $(INCLUDES)

BIN_DIR  = bin

CORE_DIR = core
CORE_OBJ = frontend.o daqthread.o controlthread.o \
           watchdogthread.o dummydevice.o pollthread.o

USER_DIR = user/pro
USER_OBJ = $(patsubst %.cc,%.o,$(wildcard $(USER_DIR)/*.cc))


$(BIN_DIR)/frontend: $(addprefix $(CORE_DIR)/,$(CORE_OBJ)) \
                     $(USER_OBJ)
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(LIBS)

clean:
	@echo Cleaning up ...
	@rm -f $(BIN_DIR)/frontend
	@rm -f $(CORE_DIR)/*.o
	@rm -f $(USER_DIR)/*.o

$(CORE_DIR)/%.o: $(CORE_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@

$(USER_DIR)/%.o: $(USER_DIR)/%.cc
	@echo Compiling $< ...
	@$(CXX) $(FLAGS) -c $< -o $@
