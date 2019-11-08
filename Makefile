#
# Makefile for creating Arduino boards manager package
#


ifdef RELEASE_NAME
R_NAME             = -$(RELEASE_NAME)
endif

INSTALLED_VERSION ?= 1.0.0
SHASUM             = shasum.txt
PWD                = $(shell pwd)

.phony: packages clean

packages: $(SHASUM)

$(SHASUM): spresense-tools$(R_NAME).tar.gz spresense-sdk$(R_NAME).tar.gz spresense$(R_NAME).tar.gz
	-sha256sum $^ > $@

spresense$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/hardware/spresense/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

spresense-sdk$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-sdk/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

spresense-tools$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-tools/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

clean:
	-rm spresense-v* spresense.zip spresense-sdk* spresense-tools*

