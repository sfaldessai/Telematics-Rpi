# Makefile

SRC_DIR=./src

OUT=telematic

gps_read.o:
	gcc -c -I$(SRC_DIR) $(SRC_DIR)/gps_read.c -o $(SRC_DIR)/gps_read.o

stm32_read.o:
	gcc -c -I$(SRC_DIR) $(SRC_DIR)/stm32_read.c -o $(SRC_DIR)/stm32_read.o

uart.o:
	gcc -c -I$(SRC_DIR) $(SRC_DIR)/uart.c -o $(SRC_DIR)/uart.o

cloud_write.o:
	gcc -c -I$(SRC_DIR) $(SRC_DIR)/cloud_write.c -o $(SRC_DIR)/cloud_write.o

main: gps_read.o stm32_read.o uart.o cloud_write.o
	gcc -I$(SRC_DIR) $(SRC_DIR)/*.o $(SRC_DIR)/main.c -o $(OUT) -lpthread

clean:
	rm $(SRC_DIR)/*.o $(OUT)