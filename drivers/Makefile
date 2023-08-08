PWD ?= $(shell pwd)
BUILD_DIR ?= $(shell pwd)

ARCH ?= arm64
CROSS_COMPILE ?= aarch64-buildroot-linux-gnu-

obj-m := gmsl2_sensors.o

KERNEL_SRC_DIR ?= /work/nvidia/source/Linux_for_Tegra/sources/kernel/kernel-5.10
KERNEL_OUT_DIR ?= /work/nvidia/source/Linux_for_Tegra/sources/kernel_out

TARGET_SENSING_SENSORS_DTS := tegra234-p3737-camera-sensing-overlay

.PHONY: all
all: clean defconfig modules dtbo

defconfig:
	make -C "${KERNEL_SRC_DIR}" ARCH="${ARCH}" O="${KERNEL_OUT_DIR}" \
		LOCALVERSION="-tegra" \
		CROSS_COMPILE="${CROSS_COMPILE}" \
		tegra_defconfig

modules:
	make -C "${KERNEL_SRC_DIR}" ARCH="${ARCH}" O="${KERNEL_OUT_DIR}" src="${PWD}" M="${BUILD_DIR}" \
		LOCALVERSION="-tegra" \
		CROSS_COMPILE="${CROSS_COMPILE}" \
		-j4 \
		--output-sync=target modules

dtbo:
	dtc -O dtb -o "${TARGET_SENSING_SENSORS_DTS}".dtbo -@ "${TARGET_SENSING_SENSORS_DTS}".dts

clean:
	make -C "${KERNEL_SRC_DIR}" ARCH="${ARCH}" O="${KERNEL_OUT_DIR}" src="${PWD}" M="${BUILD_DIR}" \
		LOCALVERSION="-tegra" \
		CROSS_COMPILE="${CROSS_COMPILE}" \
		clean
	rm -rf "${TARGET_SENSING_SENSORS_DTS}".dtbo
