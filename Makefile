.phony: packages windows linux macosx base extract clean_base clean

BUILDCHAIN_PKGNAME_PREFIX=gcc-arm-none-eabi-5.4.1-
BUILDCHAIN_VERSION=5.4.1
BUILDCHAIN_PATH=packages/SPRESENSE/tools/gcc-arm-none-eabi/

ARCHIVE_PREFIX=manual-install-spresense-arduino-
PREBUILT_PGK=spresense-prebuilt-sdk-SPRESENSE_ADN_1.0.0_SDK_1.0.001.zip
INSTALLED_VERSION?=1.0.0
SHASUM=shasum.txt
ZIP_SETTINGS=-q9r
TMPDIR=/tmp/
PWD=$(shell pwd)
OS=windows linux macosx

ifdef RELEASE_NAME
R_NAME=-$(RELEASE_NAME)
endif

packages: $(SHASUM) $(OS) clean_base

$(SHASUM): spresense-tools$(R_NAME).tar.gz spresense-sdk$(R_NAME).tar.gz spresense$(R_NAME).tar.gz $(addsuffix  .tar.gz, $(addprefix $(BUILDCHAIN_PKGNAME_PREFIX), $(OS)))
	-sha256sum $^ > $@

spresense$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/hardware/spresense/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

spresense-sdk$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-sdk/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

spresense-tools$(R_NAME).tar.gz: $(BUILDCHAIN_PATH)
	tar -C Arduino15/packages/SPRESENSE/tools/spresense-tools/ -czf $(PWD)/$@ $(INSTALLED_VERSION)

$(BUILDCHAIN_PKGNAME_PREFIX)%.tar.gz:
	tar -C Arduino15/$(BUILDCHAIN_PATH)  -czf $(PWD)/$@ $(BUILDCHAIN_VERSION)/$(subst .tar.gz, ,$*)

$(OS): base
	cp $(TMPDIR)$(ARCHIVE_PREFIX).zip $(ARCHIVE_PREFIX)$@$(R_NAME).zip
	(cd Arduino15/; zip $(ZIP_SETTINGS) ../$(ARCHIVE_PREFIX)$@$(R_NAME).zip  $(BUILDCHAIN_PATH)$(BUILDCHAIN_VERSION)/$@/*)

base: $(BUILDCHAIN_PATH)
	(cd Arduino15/; zip $(ZIP_SETTINGS) $(TMPDIR)$(ARCHIVE_PREFIX) . -x$(BUILDCHAIN_PATH)$(BUILDCHAIN_VERSION)/*)

$(BUILDCHAIN_PATH):
	./tools/prepare_arduino.sh -H Windows -s $(PREBUILT_PGK) -g $(BUILDCHAIN_PKGNAME_PREFIX)windows.zip
	./tools/prepare_arduino.sh -H Linux64 -s $(PREBUILT_PGK) -g $(BUILDCHAIN_PKGNAME_PREFIX)linux.zip
	./tools/prepare_arduino.sh -H Mac -s $(PREBUILT_PGK) -g $(BUILDCHAIN_PKGNAME_PREFIX)macosx.zip

extract: $(addsuffix _extract, $(OS))
	@mkdir -p extract/packages/tools extract/packages/sdk extract/packages/spresense
	-tar -xf spresense-tools*.tar.gz -C extract/packages/tools
	-tar -xf spresense-sdk*.tar.gz -C extract/packages/sdk
	-tar -xf spresense-v*.tar.gz -C extract/packages/spresense
	@-tar -xf spresense.tar.gz -C extract/packages/spresense

%_extract:
	@mkdir -p extract/$(ARCHIVE_PREFIX)/$* extract/gcc/
	unzip -q $(ARCHIVE_PREFIX)$**.zip -d extract/$(ARCHIVE_PREFIX)/$*
	tar -xf $(BUILDCHAIN_PKGNAME_PREFIX)$*.tar.gz -C extract/gcc/

clean_base:
	-rm -v $(TMPDIR)$(ARCHIVE_PREFIX).zip

clean:
	-rm -rf extract
	-rm  $(TMPDIR)$(ARCHIVE_PREFIX).zip
	-rm  $(ARCHIVE_PREFIX)*.zip
	-rm -r $(BUILDCHAIN_PATH)
	-rm spresense-v* spresense.zip spresense-sdk* spresense-tools* $(ARCHIVE_PREFIX)*
	-rm  $(BUILDCHAIN_PKGNAME_PREFIX)*.tar.gz

