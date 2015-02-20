LIB = -lnsl
OBJ = parser.o
CXXFLAGS = -O2 -g

shttpd: shttpd.c $(OBJ)
	gcc $(CXXFLAGS) -o shttpd shttpd.c $(OBJ) $(LIB)

parser.o: parser.h

.PHONY: clean

clean:
	$(RM) shttpd $(OBJ)
