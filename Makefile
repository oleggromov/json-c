CFLAGS=-Wall -Wextra -pedantic -std=c99 -g

json: json.c dynarray.o hashmap.o tokenize.o parse.o util.o murmur3_32.o
	$(CC) $(CFLAGS) -o $@.out $^

hashmap: hashmap.c murmur3_32.o
	$(CC) $(CFLAGS) -o $@.out $^

dynarray_test: dynarray_test.c dynarray.o util.o
	$(CC) $(CFLAGS) -o $@.out $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f json *.o
