CFLAGS=-Wall -Wextra -pedantic -std=c99 -g

json: json.c tokenize.o util.o
	$(CC) $(CFLAGS) -o $@.out $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f json *.o
