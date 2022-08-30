# Makefile

SRC_DIR = ./src

CLIENT_CONTROLLER_DIR = $(SRC_DIR)/client_controller
CLOUD_SERVER_DIR = $(SRC_DIR)/cloud_server
GPS_MODULE_DIR = $(SRC_DIR)/gps_module
SERIAL_INTERFACE_DIR = $(SRC_DIR)/serial_interface
LOGGER_MODULE_DIR = $(SRC_DIR)/logger
DB_MODULE_DIR = $(SRC_DIR)/database

CFLAGS = -g -O2 -Wall -I
LIBS = -lpthread -lsqlite3


OUT=telematic

logger.o:
	gcc -c $(CFLAGS) $(LOGGER_MODULE_DIR) $(LOGGER_MODULE_DIR)/logger.c -o $(SRC_DIR)/logger.o $(LIBS)

gps_module.o:
	gcc -c $(CFLAGS) $(GPS_MODULE_DIR) $(GPS_MODULE_DIR)/gps_read.c -o $(SRC_DIR)/gps_module.o $(LIBS)

serial_interface.o:
	gcc -c $(CFLAGS) $(SERIAL_INTERFACE_DIR) $(SERIAL_INTERFACE_DIR)/serial_config.c -o $(SRC_DIR)/serial_interface.o $(LIBS)

client_controller.o:
	gcc -c $(CFLAGS) $(CLIENT_CONTROLLER_DIR) $(CLIENT_CONTROLLER_DIR)/client_controller.c -o $(SRC_DIR)/client_controller.o $(LIBS)

cloud_write.o:
	gcc -c $(CFLAGS) $(CLOUD_SERVER_DIR) $(CLOUD_SERVER_DIR)/cloud_write.c -o $(SRC_DIR)/cloud_write.o $(LIBS)

db_handler.o:
	gcc -c $(CFLAGS) $(DB_MODULE_DIR) $(DB_MODULE_DIR)/db_handler.c -o $(SRC_DIR)/db_handler.o $(LIBS)

main: serial_interface.o gps_module.o client_controller.o cloud_write.o logger.o db_handler.o
	gcc $(CFLAGS) $(SRC_DIR) $(SRC_DIR)/*.o $(SRC_DIR)/main.c -o $(OUT) $(LIBS)

clean:
	rm $(SRC_DIR)/*.o $(OUT)