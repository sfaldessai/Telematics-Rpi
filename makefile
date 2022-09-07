# Makefile

SRC_DIR = ./src

CLIENT_CONTROLLER_DIR = $(SRC_DIR)/client_controller
CLOUD_SERVER_DIR = $(SRC_DIR)/cloud_server
GPS_MODULE_DIR = $(SRC_DIR)/gps_module
SERIAL_INTERFACE_DIR = $(SRC_DIR)/serial_interface
CAN_BUS = $(SRC_DIR)/can_bus
LOGGER_MODULE_DIR = $(SRC_DIR)/logger
GLOABL_DIR = $(SRC_DIR)/global
UTILS_DIR = $(SRC_DIR)/utils

CFLAGS = -g -O2 -Wall -I
LIBS = -lpthread


OUT=telematic

logger.o:
	gcc -c $(CFLAGS) $(LOGGER_MODULE_DIR) $(LOGGER_MODULE_DIR)/logger.c -o $(SRC_DIR)/logger.o $(LIBS)
	
global.o:
	gcc -c $(CFLAGS) $(GLOABL_DIR) $(GLOABL_DIR)/global.c -o $(SRC_DIR)/global.o $(LIBS)

gps_module.o:
	gcc -c $(CFLAGS) $(GPS_MODULE_DIR) $(GPS_MODULE_DIR)/gps_read.c -o $(SRC_DIR)/gps_module.o $(LIBS)

serial_interface.o:
	gcc -c $(CFLAGS) $(SERIAL_INTERFACE_DIR) $(SERIAL_INTERFACE_DIR)/serial_config.c -o $(SRC_DIR)/serial_interface.o $(LIBS)

client_controller.o:
	gcc -c $(CFLAGS) $(CLIENT_CONTROLLER_DIR) $(CLIENT_CONTROLLER_DIR)/client_controller.c -o $(SRC_DIR)/client_controller.o $(LIBS)

can_interface.o:
	gcc -c $(CFLAGS) $(CAN_BUS) $(CAN_BUS)/can_interface.c -o $(SRC_DIR)/can_interface.o

can_bus.o:
	gcc -c $(CFLAGS) $(CAN_BUS) $(CAN_BUS)/can_bus.c -o $(SRC_DIR)/can_bus.o $(LIBS)
	
cloud_write.o:
	gcc -c $(CFLAGS) $(CLOUD_SERVER_DIR) $(CLOUD_SERVER_DIR)/cloud_write.c -o $(SRC_DIR)/cloud_write.o $(LIBS)

common_utils.o:
	gcc -c $(CFLAGS) $(UTILS_DIR) $(UTILS_DIR)/common_utils.c -o $(SRC_DIR)/common_utils.o

main: serial_interface.o gps_module.o client_controller.o cloud_write.o global.o logger.o can_interface.o can_bus.o
	gcc $(CFLAGS) $(SRC_DIR) $(SRC_DIR)/*.o $(SRC_DIR)/main.c -o $(OUT) $(LIBS)

clean:
	rm $(SRC_DIR)/*.o $(OUT)