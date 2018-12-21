CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAGS = -O2 -Wall -fPIC

SLIB ?= libtty.a
DLIB ?= libtty.so.0.1
TARGET ?= $(SLIB) $(DLIB)

SRC_DIRS ?= .

OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

INC_DIRS := include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

all: $(TARGET)

$(SLIB): $(OBJS)
	echo $(OBJS)
	$(AR) cvr $@ $^

$(DLIB): $(OBJS)
	$(CC) -shared $^ -o $@

# c source
%.o : %.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@


.PHONY: clean all

clean:
	$(RM) -r $(OBJS) $(SLIB) $(DLIB)

