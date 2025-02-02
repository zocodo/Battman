LOCAL_IP:=$(shell /sbin/ifconfig |sed -n "s/.*inet \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\) .*/\1/p"|tail -1)

all:
	rm -rf build/
	mkdir -p build/Payload/Battman.app/
	${THEOS}/toolchain/linux/iphone/bin/clang -F${THEOS}/vendor/lib -target arm64-apple-ios14.0 -target arm64-apple-darwin -isysroot ${THEOS}/sdks/iPhoneOS14.5.sdk -lobjc -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore \
		AppDelegate.m main.m SceneDelegate.m \
		BatteryCellView/BatteryCellView.m BatteryCellView/SPWaterProgressIndicatorView.m \
		BatteryCellView/BatteryInfoTableViewCell.m \
		BatteryInfoViewController.m SettingsViewController.m common.m \
		-o build/Payload/Battman.app/Battman
	${THEOS}/toolchain/linux/iphone/bin/ldid -S../Battman.entitlements build/Payload/Battman.app/Battman
	sed 's/\$$(DEVELOPMENT_LANGUAGE)/en/;s/\$$(EXECUTABLE_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_IDENTIFIER)/ct.battman/;s/\$$(PRODUCT_NAME)/Battman/;s/\$$(PRODUCT_BUNDLE_PACKAGE_TYPE)/APPL/' Info.plist > build/Payload/Battman.app/Info.plist
	cd build;zip -r Battman.ipa ./Payload;rm -rf Payload
ifneq (,$(wildcard ./mount/pub_html))
	@echo Publishing to public website
	cp build/Battman.ipa mount/pub_html/battman/
	sed 's/LATEST_UPDATE/$(shell date)/' mount/pub_html/battman/install_template.html>mount/pub_html/battman/index.html
	@echo Install at your site.
else ifneq (,$(wildcard /var/www/html/files))
	@echo Publishing to local apache server
	cp build/Battman.ipa /var/www/html/files/battman/
	sed 's/LOCAL_IP/${LOCAL_IP}/;s/LATEST_UPDATE/$(shell date)/' /var/www/html/files/battman/install_template.html>/var/www/html/files/battman/index.html
	@echo Install at: ${LOCAL_IP}
else
	@echo Not publishing.
endif
