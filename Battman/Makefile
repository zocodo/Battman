.PHONY: all publish_mounted_site publish_local_site publish_none clean

SED = sed

IPHONEOS_DEPLOYMENT_TARGET := 13.0

LOCAL_IP := $(shell /sbin/ifconfig | $(SED) -n "s/.*inet \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\) .*/\1/p" | tail -1)

SDKROOT := ${THEOS}/sdks/iPhoneOS14.5.sdk
LIBS := -lobjc
FRAMEWORKS := -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore -framework IOKit -framework WebKit

CFLAGS = -target arm64-apple-ios${IPHONEOS_DEPLOYMENT_TARGET} -target arm64-apple-darwin -fobjc-arc -fconstant-cfstrings
LDFLAGS = -target arm64-apple-ios$(IPHONEOS_DEPLOYMENT_TARGET) -target arm64-apple-darwin $(LIBS) $(FRAMEWORKS) -Wl,-dead_strip_dylibs
THEOS := ~/theos

ifeq ($(shell test -e ../.git && echo exists), exists)
COMMIT_HASH := $(shell git rev-parse --short HEAD)
else
COMMIT_HASH :=
endif

ifeq ($(shell test -e ${THEOS}/toolchain/linux/iphone/bin/clang && echo exists), exists)
CC = ${THEOS}/toolchain/linux/iphone/bin/clang
CFLAGS += -F${THEOS}/vendor/lib -isysroot $(SDKROOT)
LDFLAGS += -isysroot $(SDKROOT)
else ifneq ($(shell which xcrun),)
CC = xcrun -sdk iphoneos cc
else
CC = cc
endif

ifeq ($(shell test -e ${THEOS}/toolchain/linux/iphone/bin/ldid && echo exists), exists)
CODESIGN = ${THEOS}/toolchain/linux/iphone/bin/ldid -S
else ifneq ($(shell which ldid),)
CODESIGN = ldid -S
else ifneq ($(shell which codesign),)
# nessessary space
CODESIGN = codesign -f -s --entitlements 
else
CODESIGN = echo "UNSIGNED: "
endif

ifneq (,$(wildcard ./mount/pub_html))
	PUBLISH_TARGET := publish_mounted_site
else ifneq (,$(wildcard /var/www/html/files))
	PUBLISH_TARGET := publish_local_site
else
	PUBLISH_TARGET := publish_none
endif

SOURCES := $(wildcard *.m *.c */*.m */*.c */*.S)
PRESRC := $(addprefix build/preprocessed/,$(SOURCES))
#OBJECTS:=$(addprefix build/objects/,$(addsuffix .o,$(SOURCES)))
OBJECTS := $(subst build/preprocessed,build/objects,$(addsuffix .o,$(PRESRC)))
OBJECTS += build/objects/license/constants.c.o

all: $(PUBLISH_TARGET)

build/preprocessed/license/constants.c: license/license.html
	echo "const unsigned char lh_arr[]={">$@
	openssl sha256 -c license/license.html|$(SED) "s/^.*= /0x/g;s/:/,0x/g" >>$@
	echo "};">>$@
$(PRESRC): build/preprocessed/%: % Localizations/*.po Localizations/*.sh
	@mkdir -p $(@D)
	./Localizations/preprocess.sh $< > $@

#$(OBJECTS): build/objects/%.o: %
$(OBJECTS): build/objects/%.o: build/preprocessed/%
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I.$(subst build/preprocessed,,$(dir $<)) -c $< -o $@

build/Payload/Battman.app/Battman: $(OBJECTS) ../Battman.entitlements
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(filter %.o,$^) -o $@
	$(CODESIGN)../Battman.entitlements $@
build/Payload/Battman.app/Info.plist: Info.plist
	$(SED) 's/\$$(DEVELOPMENT_LANGUAGE)/en/;s/\$$(EXECUTABLE_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_IDENTIFIER)/com.torrekie.Battman/;s/\$$(PRODUCT_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_PACKAGE_TYPE)/APPL/;s/\$$(COMMIT_HASH)/$(COMMIT_HASH)/' Info.plist > build/Payload/Battman.app/Info.plist
build/Payload/Battman.app/1024.png: ../Battman.svg
	./generate_icons.sh ../Battman.svg build/Payload/Battman.app/
build/Payload/Battman.app/license.html: license/license.html
	cp $^ $@
build/Battman.ipa: build/Payload/Battman.app/Battman build/Payload/Battman.app/Info.plist build/Payload/Battman.app/1024.png build/Payload/Battman.app/license.html
	cd build/ && zip -r Battman.ipa ./Payload
#ifeq ($(shell test $(IPHONEOS_DEPLOYMENT_TARGET) -lt 13.0; echo $$?),0)
#	# TODO: .deb process for iOS 12
#	@echo "###### WARNING ######"
#	@echo "This Makefile is generating an .ipa which not designed for your target."
#	@echo "Please repack it to .deb manually in order to work on your OS!"
#endif
# FIXME: test: Illegal number: 13.0

%/Battman.ipa: build/Battman.ipa
	cp $< $@
%/index.html: %/install_template.html build/Battman.ipa
	$(SED) 's/LOCAL_IP/${LOCAL_IP}/;s/LATEST_UPDATE/$(shell date)/' /var/www/html/files/battman/install_template.html>/var/www/html/files/battman/index.html

publish_mounted_site: mount/pub_html/battman/ \
			mount/pub_html/battman/Battman.ipa \
			mount/pub_html/battman/index.html
	@echo Install on your site.
publish_local_site: /var/www/html/files/battman \
			/var/www/html/files/battman/Battman.ipa \
			/var/www/html/files/battman/index.html
	@echo Install at: ${LOCAL_IP}
publish_none: build/Battman.ipa
	@echo Not publishing.
clean:
	rm -rf build/
