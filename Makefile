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
Q                  = @

.phony: packages clean

packages: $(SHASUM)
	$(Q) echo "Done."

$(SHASUM): $(SPR_LIBRARY) $(SPR_SDK) $(SPR_TOOLS)
	$(Q) -sha256sum $^ > $@

$(SPR_LIBRARY): $(OUT)
	$(Q) echo "Creating spresense.tar.gz ..."
	$(Q) tar -C Arduino15/packages/SPRESENSE/hardware/spresense/ -czf $@ $(INSTALLED_VERSION)

$(SPR_SDK): $(OUT)
	$(Q) echo "Creating spresense-sdk.tar.gz ..."
	$(Q) tar -C Arduino15/packages/SPRESENSE/tools/spresense-sdk/ -czf $@ $(INSTALLED_VERSION)

$(SPR_TOOLS): $(OUT)
	$(Q) echo "Creating spresense-tools.tar.gz ..."
	$(Q) tar -C Arduino15/packages/SPRESENSE/tools/spresense-tools/ -czf $@ $(INSTALLED_VERSION)

$(OUT):
	$(Q) mkdir -p $@

clean:
	$(Q) -rm -rf $(OUT)

