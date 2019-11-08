#
# Makefile for creating Arduino boards manager package
#


RELEASE_NAME      ?= v1.0.0
VERSION_PATTERN    = ^v[0-9]{1}.[0-9]{1}.[0-9]{1}$$

OUT               ?= out
INSTALLED_VERSION ?= 1.0.0
SHASUM             = $(OUT)/shasum.txt
SPR_LIBRARY        = $(OUT)/spresense-$(RELEASE_NAME).tar.gz
SPR_SDK            = $(OUT)/spresense-sdk-$(RELEASE_NAME).tar.gz
SPR_TOOLS          = $(OUT)/spresense-tools-$(RELEASE_NAME).tar.gz
PWD                = $(shell pwd)
Q                  = @

.phony: check packages clean

packages: check $(SHASUM)
	$(Q) echo "Done."

check:
	$(Q) echo $(RELEASE_NAME) | grep -E $(VERSION_PATTERN) > /dev/null \
		|| (echo "ERROR: Invalid version name $(RELEASE_NAME)." &&  exit 1)

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

