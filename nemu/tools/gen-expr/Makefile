NAME = gen-expr
SRCS = gen-expr.c
include $(NEMU_HOME)/scripts/build.mk

$(APP): $(SRCS)
	gcc -O2 -Wall -Werror -o $@ $<