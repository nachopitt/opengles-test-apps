TARGETS := moving-triangle-x11
BIN_DIR := bin

RM := rm -rf

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

$(TARGETS): %: $(BIN_DIR)/%

$(BIN_DIR)/%-x11: %-x11.cpp | $(BIN_DIR)
	$(CXX) $^ -o $@ -DUSE_X11 $(shell pkg-config egl x11 --cflags --libs)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	$(RM) $(BIN_DIR)
