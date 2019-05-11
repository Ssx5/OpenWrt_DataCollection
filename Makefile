# build executable on typing make
TARGET_LDFLAGS:= -lpthread -lmosquitto -lmodbus -lm -luci
LDFLAGS = -lpthread -lmosquitto -lmodbus -lm -luci

remoteIO: main.o rio_config.o rio_modbus.o rio_mqtt.o rio_thread.o
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -I. -Iinclude -o $@ $^ -std=gnu99 -D DEBUG

clean:
	rm -f *.o remateIO
