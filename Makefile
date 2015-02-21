LIB = -lnsl -lsocket
OBJ = headers.o parser.o 
CXXFLAGS = -O2 -g

shttpd: shttpd.c $(OBJ)
	gcc $(CXXFLAGS) -o shttpd shttpd.c $(OBJ) $(LIB)

headers.o: headers.h
parser.o: parser.h

.PHONY: clean

clean:
	$(RM) shttpd $(OBJ)
