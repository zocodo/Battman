ifneq ($(shell which gsed),)
SED = gsed
else
SED = sed
endif

LOCAL_IP := $(shell /sbin/ifconfig | $(SED) -n "s/.*inet \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\) .*/\1/p" | tail -1)

ifeq ($(shell test -e ${THEOS}/toolchain/linux/iphone/bin/clang && echo exists), exists)
CC = ${THEOS}/toolchain/linux/iphone/bin/clang
else ifeq ($(shell which xcrun),)
CC = cc
else ifneq ($(shell xcrun -sdk iphoneos cc --version > /dev/null 2>&1; echo $$?), 0)
CC = cc
else
CC = $(shell xcrun -sdk iphoneos cc)
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

SDKROOT := ${THEOS}/sdks/iPhoneOS14.5.sdk
LIBS := -lobjc
FRAMEWORKS := -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore -framework IOKit

SOURCES := 	main.m \
		AppDelegate.m \
		SceneDelegate.m \
		BatteryInfoViewController.m BatteryDetailsViewController.m \
		SettingsViewController.m \
		common.m \
		BatteryCellView/BatteryCellView.m \
		BatteryCellView/SPWaterProgressIndicatorView.m \
		BatteryCellView/BatteryInfoTableViewCell.m \
		battery_utils/battery_info.c battery_utils/libsmc.c

# TODO: Target splitting
# TODO: gettext generation
all:
	rm -rf build/
	mkdir -p build/Payload/Battman.app/
	# TODO: Compile sources as *.o first
	$(CC) -F${THEOS}/vendor/lib -target arm64-apple-ios14.0 -target arm64-apple-darwin \
		-isysroot ${THEOS}/sdks/iPhoneOS14.5.sdk $(LIBS) $(FRAMEWORKS) \
		-fobjc-arc \
		$(CFLAGS) $(LDFLAGS) \
		$(SOURCES) -o build/Payload/Battman.app/Battman
	$(CODESIGN)../Battman.entitlements build/Payload/Battman.app/Battman
	$(SED) 's/\$$(DEVELOPMENT_LANGUAGE)/en/;s/\$$(EXECUTABLE_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_IDENTIFIER)/ct.battman/;s/\$$(PRODUCT_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_PACKAGE_TYPE)/APPL/' Info.plist > build/Payload/Battman.app/Info.plist
	cd build; zip -r Battman.ipa ./Payload; rm -rf Payload
ifneq (,$(wildcard ./mount/pub_html))
	@echo Publishing to public website
	cp build/Battman.ipa mount/pub_html/battman/
	$(SED) 's/LATEST_UPDATE/$(shell date)/' mount/pub_html/battman/install_template.html>mount/pub_html/battman/index.html
	@echo Install at your site.
else ifneq (,$(wildcard /var/www/html/files))
	@echo Publishing to local apache server
	cp build/Battman.ipa /var/www/html/files/battman/
	$(SED) 's/LOCAL_IP/${LOCAL_IP}/;s/LATEST_UPDATE/$(shell date)/' /var/www/html/files/battman/install_template.html>/var/www/html/files/battman/index.html
	@echo Install at: ${LOCAL_IP}
else
	@echo Not publishing.
endif
