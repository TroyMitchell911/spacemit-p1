obj-m+=spacemit-pmic.o
KERNEL_DIR=/home/troy/k1/kernel

all:
	make -C $(KERNEL_DIR) M=$(PWD) CROSS_COMPILE=riscv64-linux-gnu- ARCH=riscv modules
clean:
	make -C ${KERNEL_DIR} M=$(PWD) CROSS_COMPILE=riscv64-linux-gnu- ARCH=riscv clean
