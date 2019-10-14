CC = clang # -fsanitize=address -g -fno-omit-frame-pointer# C compiler
CFLAGS = -fPIC -Wall -Werror -O3 # -D DEBUG # C flags
LDFLAGS = -shared  # linking flags
RM = rm -f  # rm command
TARGET_LIB = libmutator.so # target lib

SRCS = mutate.c random.c testcase.c util.c # source files
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c):%.c
	$(CC) $(CFLAGS) -MM $<

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

.PHONY: test
test: 
	$(CC) -I./ -L./ -W test/mutate_test.c -o test/mutate -lmutator -lpthread
	$(CC) -I./ -L./ -W test/mutate_multi_test.c -o test/mutate_multi -lmutator -lpthread 
	$(CC) -I./ -L./ -W test/mutate_stdin.c -o test/mutate_stdin -lmutator -lpthread 
	$(CC) -I./ -L./ -W test/mutate_file.c -o test/mutate_file -lmutator -lpthread 
	$(CC) -I./ -L./ -W test/mutate_dir.c -o test/mutate_dir -lmutator -lpthread

.PHONY: install
install:
	cp libmutator.so /usr/local/lib/
