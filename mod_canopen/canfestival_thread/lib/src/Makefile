EXTRA_CFLAGS := -D DEBUG -DDEBUG_WAR_CONSOLE_ON -DDEBUG_ERR_CONSOLE_ON

ifeq ($(TIMERS_DRIVER), timers_kernel)
USE_KERNEL_TIMER = true
endif

ifeq ($(USE_KERNEL_TIMER), true)
obj-y += symbols.o
endif

obj-y += dcf.o emcy.o lifegrd.o lss.o nmtMaster.o nmtSlave.o objacces.o pdo.o sdo.o states.o sync.o timer.o