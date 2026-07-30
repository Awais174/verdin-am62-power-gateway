PROJECT_ROOT ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
CUBE_ROOT ?=
BUILD_DIR ?= $(PROJECT_ROOT)/build
CROSS_COMPILE ?= arm-none-eabi-

REFERENCE_DIR := $(CUBE_ROOT)/Projects/STM32MP157C-DK2/Applications/OpenAMP/OpenAMP_FreeRTOS_echo
HAL_DIR := $(CUBE_ROOT)/Drivers/STM32MP1xx_HAL_Driver
CMSIS_DIR := $(CUBE_ROOT)/Drivers/CMSIS
FREERTOS_DIR := $(CUBE_ROOT)/Middlewares/Third_Party/FreeRTOS/Source
OPENAMP_DIR := $(CUBE_ROOT)/Middlewares/Third_Party/OpenAMP

CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size

ELF := $(BUILD_DIR)/power-alarm-m4.elf
BIN := $(BUILD_DIR)/power-alarm-m4.bin
MAP := $(BUILD_DIR)/power-alarm-m4.map
OBJ_DIR := $(BUILD_DIR)/obj
LINKER_SCRIPT := $(REFERENCE_DIR)/STM32CubeIDE/CM4/stm32mp15xx_m4.ld

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFINES := \
	-DCORE_CM4 \
	-DMETAL_FREERTOS \
	-DMETAL_INTERNAL \
	-DMETAL_MAX_DEVICE_REGIONS=2 \
	-DVIRTIO_SLAVE_ONLY \
	-D__LOG_TRACE_IO_ \
	-DNO_ATOMIC_64_SUPPORT \
	-DUSE_HAL_DRIVER \
	-DSTM32MP157Cxx \
	'-D__weak=__attribute__((weak))' \
	'-D__packed=__attribute__((__packed__))'

INCLUDES := \
	-I$(PROJECT_ROOT)/include \
	-I$(REFERENCE_DIR)/Inc \
	-I$(HAL_DIR)/Inc \
	-I$(HAL_DIR)/Inc/Legacy \
	-I$(CMSIS_DIR)/Device/ST/STM32MP1xx/Include \
	-I$(CMSIS_DIR)/Include \
	-I$(FREERTOS_DIR)/include \
	-I$(FREERTOS_DIR)/portable/GCC/ARM_CM4F \
	-I$(OPENAMP_DIR)/open-amp/lib/include \
	-I$(OPENAMP_DIR)/libmetal/lib/include \
	-I$(OPENAMP_DIR)/virtual_driver

FW_CFLAGS := $(CPU_FLAGS) -std=gnu11 -Os -g3 -Wall \
	-ffunction-sections -fdata-sections -fstack-usage
FW_ASFLAGS := $(CPU_FLAGS) -g3
FW_LDFLAGS := $(CPU_FLAGS) -T$(LINKER_SCRIPT) \
	-specs=nano.specs -specs=nosys.specs \
	-Wl,--gc-sections -Wl,-Map=$(MAP) -Wl,--cref

APPLICATION_SOURCES := \
	$(PROJECT_ROOT)/src/main.c \
	$(PROJECT_ROOT)/src/board_stm32.c \
	$(PROJECT_ROOT)/src/ipc_openamp.c \
	$(REFERENCE_DIR)/Src/lock_resource.c \
	$(REFERENCE_DIR)/Src/mbox_ipcc.c \
	$(REFERENCE_DIR)/Src/openamp.c \
	$(REFERENCE_DIR)/Src/openamp_log.c \
	$(REFERENCE_DIR)/Src/rsc_table.c \
	$(REFERENCE_DIR)/Src/stm32mp1xx_hal_msp.c \
	$(REFERENCE_DIR)/Src/stm32mp1xx_hal_timebase_tim.c \
	$(REFERENCE_DIR)/Src/stm32mp1xx_it.c \
	$(REFERENCE_DIR)/Src/system_stm32mp1xx.c \
	$(REFERENCE_DIR)/STM32CubeIDE/syscalls.c

HAL_SOURCES := \
	$(HAL_DIR)/Src/stm32mp1xx_hal.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_cortex.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_dma.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_dma_ex.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_exti.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_gpio.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_hsem.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_ipcc.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_mdma.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_pwr.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_pwr_ex.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_rcc.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_rcc_ex.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_tim.c \
	$(HAL_DIR)/Src/stm32mp1xx_hal_tim_ex.c

FREERTOS_SOURCES := \
	$(FREERTOS_DIR)/croutine.c \
	$(FREERTOS_DIR)/event_groups.c \
	$(FREERTOS_DIR)/list.c \
	$(FREERTOS_DIR)/queue.c \
	$(FREERTOS_DIR)/stream_buffer.c \
	$(FREERTOS_DIR)/tasks.c \
	$(FREERTOS_DIR)/timers.c \
	$(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.c \
	$(FREERTOS_DIR)/portable/MemMang/heap_4.c

LIBMETAL_SOURCES := \
	$(OPENAMP_DIR)/libmetal/lib/device.c \
	$(OPENAMP_DIR)/libmetal/lib/init.c \
	$(OPENAMP_DIR)/libmetal/lib/io.c \
	$(OPENAMP_DIR)/libmetal/lib/log.c \
	$(OPENAMP_DIR)/libmetal/lib/shmem.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/condition.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/freertos_device.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/freertos_init.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/freertos_io.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/freertos_shmem.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/time.c \
	$(OPENAMP_DIR)/libmetal/lib/system/freertos/template/sys.c

OPENAMP_SOURCES := \
	$(OPENAMP_DIR)/open-amp/lib/remoteproc/remoteproc_virtio.c \
	$(OPENAMP_DIR)/open-amp/lib/rpmsg/rpmsg.c \
	$(OPENAMP_DIR)/open-amp/lib/rpmsg/rpmsg_virtio.c \
	$(OPENAMP_DIR)/open-amp/lib/virtio/virtio.c \
	$(OPENAMP_DIR)/open-amp/lib/virtio/virtqueue.c

C_SOURCES := $(APPLICATION_SOURCES) $(HAL_SOURCES) $(FREERTOS_SOURCES) \
	$(LIBMETAL_SOURCES) $(OPENAMP_SOURCES)
ASM_SOURCES := $(REFERENCE_DIR)/STM32CubeIDE/startup_stm32mp15xx.s
C_OBJECTS := $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(notdir $(basename $(C_SOURCES)))))
ASM_OBJECTS := $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(notdir $(basename $(ASM_SOURCES)))))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean check size

all: check $(ELF) $(BIN)

check:
	@test -n "$(CUBE_ROOT)" || { echo "CUBE_ROOT is required" >&2; exit 1; }
	@test -f "$(REFERENCE_DIR)/Inc/FreeRTOSConfig.h" || { \
		echo "STM32CubeMP1 v1.6.0 OpenAMP_FreeRTOS_echo not found" >&2; exit 1; }
	@test -f "$(LINKER_SCRIPT)" || { echo "Linker script not found" >&2; exit 1; }

define compile_c
$(OBJ_DIR)/$(notdir $(basename $(1))).o: $(1)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(DEFINES) $(INCLUDES) $(FW_CFLAGS) -MMD -MP -c $$< -o $$@
endef

define compile_asm
$(OBJ_DIR)/$(notdir $(basename $(1))).o: $(1)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(DEFINES) $(INCLUDES) $(FW_ASFLAGS) -c $$< -o $$@
endef

$(foreach source,$(C_SOURCES),$(eval $(call compile_c,$(source))))
$(foreach source,$(ASM_SOURCES),$(eval $(call compile_asm,$(source))))

$(ELF): $(OBJECTS) $(LINKER_SCRIPT)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FW_LDFLAGS) $(OBJECTS) -Wl,--start-group -lc -lm -Wl,--end-group -o $@
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

size: $(ELF)
	$(SIZE) $(ELF)

clean:
	rm -rf $(BUILD_DIR)

-include $(C_OBJECTS:.o=.d)
