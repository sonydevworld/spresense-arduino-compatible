# getspkinfo/Makefile

UNAME := $(shell uname -o 2>/dev/null)
GCC := gcc
SRC := src/getspkinfo.c

# Detect host platform
ifeq ($(UNAME),GNU/Linux)
  OUT := linux
else ifeq ($(UNAME),)
  OUT := macosx
else
  OUT := windows
endif

hide := @

# compile target source code
all: $(OUT)
	$(hide) $(GCC) $(SRC) -o $(OUT)/getspkinfo

# Create output directory
$(OUT):
	$(hide) mkdir -p $(OUT)
