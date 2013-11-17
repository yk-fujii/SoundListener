TARGET = soundlistener
OBJS = main.o julius.o socket.o

CPPFLAGS= -g `libjulius-config --cflags` `libsent-config --cflags`
LDFLAGS=`libjulius-config --libs` `libsent-config --libs`
CC = gcc

all: $(OBJS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

%.o : %.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(EXTRA_LIBS)

clean:
	rm -f *.o soundlistener rec.raw
