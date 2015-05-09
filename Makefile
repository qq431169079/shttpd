LIB = -lnsl
OBJ = o
CFLAGS = -g -O2 -Wall -msse -mmmx
OBJS = shttpd.$(OBJ) headers.$(OBJ) parser.$(OBJ) info.$(OBJ) slog.$(OBJ)

shttpd: $(OBJS)
	$(CC) $(CFLAGS) -o shttpd $(OBJS) $(LIB)

shttpd.$(OBJ): headers.h parser.h info.h slog.h stdinc.h
headers.$(OBJ): headers.h stdinc.h
parser.$(OBJ): parser.h stdinc.h
info.$(OBJ): info.h stdinc.h
slog.$(OBJ): slog.h stdinc.h


.PHONY: clean

clean:
	$(RM) shttpd $(OBJS)
