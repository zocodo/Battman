# Makefile for Linux

all:
	${THEOS}/toolchain/linux/iphone/bin/clang -target arm64-apple-ios14.5 -isysroot ${THEOS}/sdks/iPhoneOS14.5.sdk thermhid_nomacsdk.c -framework IOKit -framework CoreFoundation -o thermhid
	${THEOS}/toolchain/linux/iphone/bin/ldid -Sios.entitlements thermhid