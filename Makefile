OBJDIR = lib
BUILDDIR = bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC=$(call rwildcard,./,*.c)
OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(CPPSRC))

CFLAGS = -O3 -Wall -I./include -fno-objc-arc $(USER_CFLAGS)

CC = clang
CXX = clang

build: $(OBJS)
	@echo LINKING $(BUILDDIR)/iasm
	@$(CXX) $(OBJS) -o $(BUILDDIR)/iasm $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	@echo "CC $^ -> $@"
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(CFLAGS) -O3

delete_tmp:
	@echo "Deleting tmp directories"
	@rm -rf $(OBJDIR)
	@rm -rf $(BUILDDIR)

setup:
	@echo "Creating tmp directories"
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BUILDDIR)

clean: delete_tmp setup

run: build
	@$(BUILDDIR)/iasm

.PNONY: clean run
