NAME = gen-expr
SRCS = gen-expr.c
include $(NEMU_HOME)/scripts/build.mk

$(NAME): $(SRCS)
	gcc -O2 -Wall -Werror -o $@ $<