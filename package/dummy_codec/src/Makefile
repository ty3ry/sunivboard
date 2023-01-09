obj-y += dummy_codec.o
#ccflags-y := -DDEBUG -g -std=gnu99 -Wno-error

.PHONY: all clean

all:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) clean
