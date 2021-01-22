ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE =
endif
ifeq ($(AS),)
AS	= $(CROSS_COMPILE)as
endif
ifeq ($(LD),)
LD	= $(CROSS_COMPILE)ld
endif
ifeq ($(CC),)
CC	= $(CROSS_COMPILE)gcc
endif
ifeq ($(CPP),)
CPP	= $(CC) -E
endif
ifeq ($(AR),)
AR	= $(CROSS_COMPILE)ar
endif
ifeq ($(NM),)
NM	= $(CROSS_COMPILE)nm
endif
ifeq ($(STRIP),)
STRIP	= $(CROSS_COMPILE)strip
endif
ifeq ($(OBJCOPY),)
OBJCOPY	= $(CROSS_COMPILE)objcopy
endif
ifeq ($(OBJDUMP),)
OBJDUMP	= $(CROSS_COMPILE)objdump
endif

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

TOPDIR := $(shell pwd)
export TOPDIR

obj-y += cobra_sys/
obj-y += mod_canopen/

CFLAGS += -Wall -O2 -fPIC -g -rdynamic

SUBDIR := $(shell find $(patsubst %/,%,$(filter %/, $(obj-y))) -maxdepth 5 -type d)
CFLAGS += $(foreach f, $(filter %/include, $(SUBDIR)), -I $(TOPDIR)/$(f))

ifeq ($(LDFLAGS),)
LDFLAGS := -lpthread -lrt
endif
export CFLAGS LDFLAGS

TARGET := cobra_gauguin

all : build $(TARGET)
	@echo $(TARGET) has been built!

build:
	make -C ./ -f $(TOPDIR)/Makefile.build

$(TARGET) : built-in.o
	$(CC) $(CFLAGS) -o $(TARGET) built-in.o $(LDFLAGS)

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
