#
# Makefile for creating Arduino boards manager package
#


ifdef RELEASE_NAME
R_NAME             = -$(RELEASE_NAME)
endif

OUT               ?= out
INSTALLED_VERSION ?= 1.0.0
SHASUM             = $(OUT)/shasum.txt
SPR_LIBRARY        = $(OUT)/spresense$(R_NAME).tar.gz
SPR_SDK            = $(OUT)/spresense-sdk$(R_NAME).tar.gz
SPR_TOOLS          = $(OUT)/spresense-tools$(R_NAME).tar.gz
PWD                = $(shell pwd)

.phony: packages clean

packages: $(SHASUM)

$(SHASUM): $(SPR_LIBRARY) $(SPR_SDK) $(SPR_TOOLS)
	-sha256sum $^ > $@

$(SPR_LIBRARY): $(OUT)
	tar -C Arduino15/packages/SPRESENSE/hardware/spresense/ -czf $@ $(INSTALLED_VERSION)

$(SPR_SDK): $(OUT)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-sdk/ -czf $@ $(INSTALLED_VERSION)

$(SPR_TOOLS): $(OUT)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-tools/ -czf $@ $(INSTALLED_VERSION)

$(OUT):
	mkdir -p $@

clean:
	-rm -rf $(OUT)

