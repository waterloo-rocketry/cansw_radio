
out=unit_tests

objects = serialize_test.o
objects+= serialize.o

CFLAGS+="-I.."

VPATH+=..

all: $(out)
	./$(out)

%.o: %.c
	gcc -c -o $@ $< $(CFLAGS)

$(out): $(objects)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	-@rm $(objects) $(out)
