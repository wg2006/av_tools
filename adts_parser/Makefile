#makefile:2014-8-10,gang@gmail.com

GCC ?= gcc
AR ?= ar
CFLAGS += -Wall -Wno-deprecated
TARGET = adts_parser_test

LDFLAGS += \

OBJS = \
	adts_parser.o \
	adts_parser_test.o

%.o : %.c
	$(GCC) $(CFLAGS) -c $< -o $@
        
all: $(OBJS) $(LIBS)
	$(GCC) -o $(TARGET)  $(OBJS) $(LDFLAGS)
        
clean:
	@rm -f $(OBJS) $(TARGET)
