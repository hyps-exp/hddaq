BIN_DIR = bin

TARGET  = example_frontend

all: $(addprefix $(BIN_DIR)/,$(TARGET))

$(BIN_DIR)/example_frontend:
	$(MAKE) -C example_node;

$(BIN_DIR)/vme01_frontend:
	$(MAKE) -C vme01_node;

$(BIN_DIR)/vme03_frontend:
	$(MAKE) -C vme03_node;

$(BIN_DIR)/vme04_frontend:
	$(MAKE) -C vme04_node;

$(BIN_DIR)/vme05_frontend:
	$(MAKE) -C vme05_node;

$(BIN_DIR)/vme06_frontend:
	$(MAKE) -C vme06_node;

clean:
	$(MAKE) -C example_node clean; 
	$(MAKE) -C vme01_node clean; 
	$(MAKE) -C vme03_node clean; 
	$(MAKE) -C vme04_node clean; 
	$(MAKE) -C vme05_node clean; 
	$(MAKE) -C vme06_node clean; 
