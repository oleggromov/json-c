CC=clang
CFLAGS=-pedantic -Wall -Wextra -std=c99 -g
# Why murmur3_32.o is not in OBJS?
OBJS=args.o json.o hashmap.o dynarray.o tokenize.o parse.o serialize.o util.o murmur3_32.o read_file.o

json: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o json.out

dynarray_test: dynarray_test.c dynarray.o util.o
	$(CC) $(CFLAGS) -o $@.out $^

dynarray2_test: dynarray2_test.c dynarray2.o util.o
	$(CC) $(CFLAGS) -o $@.out $^

hashmap_test: hashmap_test.c hashmap.o murmur3_32.o util.o
	$(CC) $(CFLAGS) -o $@.out $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.out *.o *.dSYM
