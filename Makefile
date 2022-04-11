
CFLAGS = -Wall
OBJS = ush.o argparse.o builtin.o

ush: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ush ${OBJS} ${LIBS}

clean: 
	rm ush ${OBJS}
