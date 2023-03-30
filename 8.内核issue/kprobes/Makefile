# Makefile for kprobe_demo

DIR := /usr/src/kernels/3.10.0-1160.88.1.el7.x86_64

# The target module
obj-m := kprobes.o

# Path to the kernel source
ifndef DIR
KDIR := /usr/src/kernels/$(shell uname -r)/
else 
KDIR := $(DIR)
endif

# Current directory
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
