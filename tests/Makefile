
objects = serialize.o
objects+= radio_handler.o
objects+= ../pic18_time.o

CFLAGS+="-I.."
CFLAGS+="-I../canlib/"

VPATH+=..

all: serialize_test radio_handler_test
	./serialize_test
	./radio_handler_test

serialize_test: $(objects) serialize_test.o
	gcc -o $@ $^ $(CFLAGS)

radio_handler_test: $(objects) radio_handler_test.o
	gcc -o $@ $^ $(CFLAGS)

%.o: %.c
	gcc -c -o $@ $< $(CFLAGS)

$(out): $(objects)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	-@rm $(objects) $(out)
