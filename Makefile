TARGET_EXEC := commands 

BUILD_DIR := ./build
SRC_DIRS := ./src
INC_DIR := ./include

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
INCLDS := $(shell find $(INC_DIR) -name '*.hpp' -or -name '*.h')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := $(shell find $(INC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: % $(INCLDS)
	mkdir -p $(dir $@)
	$(CXX) $(INC_FLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: clean_all
clean_all:
	rm -rf $(BUILD_DIR)
	rm -f ./warhammer
