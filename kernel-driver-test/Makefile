
obj-m:=my-driver.o
mymodule-objs:=module
KDIR:=/lib/modules/`uname -r`/build
MAKE:=make
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean