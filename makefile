# Makefile

SRC_DIR = ./src

CLIENT_CONTROLLER_DIR = $(SRC_DIR)/client_controller
CLOUD_SERVER_DIR = $(SRC_DIR)/cloud_server
GPS_MODULE_DIR = $(SRC_DIR)/gps_module
SERIAL_INTERFACE_DIR = $(SRC_DIR)/serial_interface
LOGGER_MODULE_DIR = $(SRC_DIR)/slog


OUT=telematic

slog.o:
	gcc -c -I$(LOGGER_MODULE_DIR) $(LOGGER_MODULE_DIR)/slog.c -o $(SRC_DIR)/slog.o -lpthread

gps_module.o:
	gcc -c -I$(GPS_MODULE_DIR) $(GPS_MODULE_DIR)/gps_read.c -o $(SRC_DIR)/gps_module.o -lpthread

serial_interface.o:
	gcc -c -I$(SERIAL_INTERFACE_DIR) $(SERIAL_INTERFACE_DIR)/serial_config.c -o $(SRC_DIR)/serial_interface.o -lpthread

client_controller.o:
	gcc -c -I$(CLIENT_CONTROLLER_DIR) $(CLIENT_CONTROLLER_DIR)/client_controller.c -o $(SRC_DIR)/client_controller.o -lpthread

cloud_write.o:
	gcc -c -I$(CLOUD_SERVER_DIR) $(CLOUD_SERVER_DIR)/cloud_write.c -o $(SRC_DIR)/cloud_write.o -lpthread

main: slog.o serial_interface.o gps_module.o client_controller.o cloud_write.o
	gcc -I$(SRC_DIR) $(SRC_DIR)/*.o $(SRC_DIR)/main.c -o $(OUT) -lpthread

clean:
	rm $(SRC_DIR)/*.o $(OUT)