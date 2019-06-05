
TARGET_DIR := ../../build/util/gptl-tools

SOURCES := $(wildcard *.c)

OBJECTS := $(addprefix $(TARGET_DIR)/,$(SOURCES:%.c=%.o))
DEPENDS := $(OBJECTS:%.o=%.d)

generate_dependencies = \
		mkdir -p `dirname $(1)`;\
		echo DEP $(2);\
		printf "%s/" `dirname $(1)` > $(1);\
		$(CC) -MM $(2) >> $(1);\
		echo '\t'@echo CC $(2) >> $(1);\
		echo '\t'@$$\(CC\) -g -c $(2) -o $(1:%.d=%.o) >> $(1)

all: $(TARGET_DIR)/gptl-tools

clean:
	@rm -rf $(TARGET_DIR)

format:
	@indent -linux $(filter %.c,$(SOURCES))
	@echo format completed

$(TARGET_DIR)/gptl-tools: $(OBJECTS)
	@echo GEN $@
	@$(CC) -o $@ $^

ifeq ($(MAKECMDGOALS),)
-include $(DEPENDS)
endif

$(TARGET_DIR)/%.d: %.c
	@$(call generate_dependencies,$@,$^)

