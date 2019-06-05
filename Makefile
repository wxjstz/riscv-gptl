ifndef CROSS_COMPILE
$(error missing option `CROSS_COMPILE`)
endif

ifndef PLATFORM
$(error missing option `PLATFORM`)
endif

ifeq ($(shell [ ! -d platform/$(PLATFORM) ] && echo y),y)
$(error path platform/$(PLATFORM) not exist)
endif

ifndef CONFIG_OPENSBI_ADDR
$(error missing option `CONFIG_OPENSBI_ADDR`)
endif

ifndef CONFIG_KERNEL_ADDR
$(error missing option `CONFIG_KERNEL_ADDR`)
endif

ifndef CONFIG_FDT_ADDR
$(error missing option `CONFIG_FDT_ADDR`)
endif

ifndef CONFIG_RESERVED_FOR_FDT
$(error missing option `CONFIG_RESERVED_FOR_FDT`)
endif

CONFIG_FW_START ?= 0x80000000
CONFIG_HEAP_SIZE ?= 65536
CONFIG_STACK_SIZE ?= 65536

CC := $(CROSS_COMPILE)gcc -Wno-builtin-declaration-mismatch -I includes -I platform/includes -I platform/$(PLATFORM)/includes

CC += $(if $(CONFIG_RISCV_ARCH), -march=$(CONFIG_RISCV_ARCH))
CC += $(if $(CONFIG_RISCV_ABI), -mabi=$(CONFIG_RISCV_ABI))
CC += $(if $(CONFIG_RISCV_CODEMODEL), -mcmodel=$(CONFIG_RISCV_CODEMODEL))

CC += -DCONFIG_OPENSBI_ADDR=$(CONFIG_OPENSBI_ADDR)
CC += -DCONFIG_KERNEL_ADDR=$(CONFIG_KERNEL_ADDR)
CC += -DCONFIG_FDT_ADDR=$(CONFIG_FDT_ADDR)
CC += -DCONFIG_RESERVED_FOR_FDT=$(CONFIG_RESERVED_FOR_FDT)

ifeq ($(DEBUG),y)
CC += -DDEBUG
endif

DIRECTORYS :=
DIRECTORYS += src
DIRECTORYS += platform/$(PLATFORM)

SOURCES :=
SOURCES += $(foreach d,$(DIRECTORYS),$(wildcard $(d)/*.c))
SOURCES += $(foreach d,$(DIRECTORYS),$(wildcard $(d)/*.S))

OBJECTS := $(addprefix build/,$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SOURCES))))
DEPENDS := $(patsubst %.o,%.d,$(OBJECTS))

generate_dependencies = \
		mkdir -p `dirname $(1)`;\
		echo DEP $(2);\
		printf "%s/" `dirname $(1)` > $(1);\
		$(CC) -MM $(2) >> $(1);\
		echo '\t'@echo CC $(2) >> $(1);\
		echo '\t'@$$\(CC\) -c $(2) -o $(1:%.d=%.o) >> $(1)

all: build/program.elf

info:
	@echo CROSS_COMPILE=$(CROSS_COMPILE)
	@echo PLATFORM=$(PLATFORM)
	@echo CONFIG_RISCV_ARCH=$(CONFIG_RISCV_ARCH)
	@echo CONFIG_RISCV_ABI=$(CONFIG_RISCV_ABI)
	@echo CONFIG_RISCV_CODEMODEL=$(CONFIG_RISCV_CODEMODEL)
	@echo CONFIG_FW_START=$(CONFIG_FW_START)
	@echo CONFIG_HEAP_SIZE=$(CONFIG_HEAP_SIZE)
	@echo CONFIG_STACK_SIZE=$(CONFIG_STACK_SIZE)
	@echo CONFIG_OPENSBI_ADDR=$(CONFIG_OPENSBI_ADDR)
	@echo CONFIG_KERNEL_ADDR=$(CONFIG_KERNEL_ADDR)
	@echo CONFIG_FDT_ADDR=$(CONFIG_FDT_ADDR)
	@echo CONFIG_RESERVED_FOR_FDT=$(CONFIG_RESERVED_FOR_FDT)

clean:
	@rm -rf build/

format:
	@indent -nut -kr $(filter %.c,$(SOURCES))
	@echo format completed

build/program.elf: $(OBJECTS) build/program.ldS
	@echo LD program.elf
	@$(CC) -nostdlib -T build/program.ldS $(OBJECTS) -o build/program.elf

build/program.ldS: includes/program.ld.h
	@echo GEN $@
	@sed \
		-e "s/{{CONFIG_FW_START}}/$(CONFIG_FW_START)/"		\
		-e "s/{{CONFIG_HEAP_SIZE}}/$(CONFIG_HEAP_SIZE)/"	\
		-e "s/{{CONFIG_STACK_SIZE}}/$(CONFIG_STACK_SIZE)/"	\
		$^ > $@

ifeq ($(MAKECMDGOALS),)
-include $(DEPENDS)
endif

build/src/%.d: src/%.c
	@$(call generate_dependencies,$@,$^)

build/src/%.d: src/%.S
	@$(call generate_dependencies,$@,$^)

build/platform/common/%.d: platform/common/%.c
	@$(call generate_dependencies,$@,$^)

build/platform/common/%.d: platform/common/%.S
	@$(call generate_dependencies,$@,$^)

build/platform/$(PLATFORM)/%.d: platform/$(PLATFORM)/%.c
	@$(call generate_dependencies,$@,$^)

build/platform/$(PLATFORM)/%.d: platform/$(PLATFORM)/%.S
	@$(call generate_dependencies,$@,$^)

