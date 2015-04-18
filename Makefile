LIB = -lnsl
OBJ = headers.o parser.o 
CFLAGS = -O2 -g

shttpd: shttpd.c $(OBJ)
	gcc $(CFLAGS) -o shttpd shttpd.c $(OBJ) $(LIB)

headers.o: headers.h
parser.o: parser.h

.PHONY: clean

clean:
	$(RM) shttpd $(OBJ)
