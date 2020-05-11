# flash_writer/Makefile

UNAME := $(shell uname -o 2>/dev/null)
PYINSTALLER := pyinstaller
SRC := scripts/flash_writer.py

# Detect host platform
ifeq ($(UNAME),GNU/Linux)
  OUT := linux
else ifeq ($(UNAME),)
  OUT := macosx
else
  OUT := windows
endif

hide := @

OPT := --onefile --noconsole
TEXT := --add-data=text/eula_description.txt:text
IMAGE := --add-data=image/SPRESENSE.png:image

# compile target source code
all: $(OUT)
	$(hide) $(PYINSTALLER) $(OPT) $(TEXT) $(IMAGE) $(SRC)
	$(hide) mv dist/* $(OUT)/
	$(hide) rm -rf build dist *.spec

# Create output directory
$(OUT):
	$(hide) mkdir -p $(OUT)
