ifneq ($(KERNELRELEASE),)
obj-m	:= eth_hook_3.o

else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -fr .tmp_versions *.o *.ko *.mod *.mod.c .*.cmd

endif
