all: ipa/mount/pub_html
	${THEOS}/toolchain/linux/iphone/bin/clang -F${THEOS}/vendor/lib -target arm64-apple-ios14.0 -target arm64-apple-darwin -isysroot ${THEOS}/sdks/iPhoneOS14.5.sdk -lobjc -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore AppDelegate.m main.m SceneDelegate.m SPWaterProgressIndicatorView.m BatteryInfoViewController.m SettingsViewController.m common.m -o Battman
	${THEOS}/toolchain/linux/iphone/bin/ldid -S../Battman.entitlements Battman
	cp Battman ipa/Payload/Battman.app/
	cd ipa;rm -f Battman.ipa;zip -r Battman.ipa ./Payload;cp Battman.ipa mount/pub_html/battman/