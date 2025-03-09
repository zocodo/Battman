# Battman
Strayers' modern battery manager for their good old iOS devices.

### Advantages
- [x] Purely constructed with Objective-C & C
- [x] UI purely written with brilliant Objective-C codes
- [x] NO StoryBoards, NO additional binaries
- [x] NO nasty Swift and SwiftUI involved
- [x] NEITHER Swift NOR SwiftUI involved, they are disgusting
- [x] NO CocoaPods, NO Swift Packages, NO external code requirements
- [x] Compile WITH/WITHOUT Xcode
- [x] Obtain & Operate directly from/with your hardware with the deepest raw data
- [x] Supporting iPhone & iPad & iPod & Xcode Simulator & Apple Silicon Macs
- [x] Highly integrated with your battery Gas Gauge IC that manufactured by Texas Instruments
- [x] Show as much as power informations than IOPS & PowerManagement provided
- [x] Identifying your power adapters, wireless chargers, or even your MagSafe accessories

### Only Battman Can Do

What other battery utils made for iOS hasn’t done
(As of 9th Sun Mar 2025 UTC+0)
- [x] Complete NotChargingReason decoding (see [not_charging_reason.h](Battman/battery_utils/not_charging_reason.h))
- [x] Texas Intruments Impedance Track™ information retrieving
- [x] Real-time charging current/voltage reading
- [x] Running perfectly when in Xcode Simulator

### Requirements

- Jailbroken or install with TrollStore
- iOS 13+ / macOS 11+ (backports welcomed)
- arm64 (A7+ / M1+)
- Gettext libintl (Optional, for localizations)
- GTK+ 3 (Optional, for running under GTK+ based WM)

### Download

I don't provide prebuilts at current stage, Battman is still not as completed as how I thought. If you really desired to use Battman, build by yourself or contact me for a prebuilt package.

```bash
# On macOS, install Xcode and directly build in it
# On Linux or BSD, make sure a LLVM cross toolchain and iPhoneOS.sdk is prepared, modify Battman/Makefile if needed
# On iOS, when you using Torrekie/Comdartiwerk as bootstrap
apt install git odcctools bash clang make sed grep ld64 ldid libintl-dev iphoneos.sdk
git clone https://https://github.com/Torrekie/Battman
cd Battman
make -C Battman all
# Produced Battman.ipa will under $(CWD)/Battman/build/Battman.ipa
```

### TODO
- [ ] Run as CLI
- [ ] Run as daemon
- [ ] Charge limit
- [ ] Wireless integration
- [ ] App rate limit
- [ ] Jetsam control
- [ ] Fan control

### License

MIT for now, may become non-free later, you won't blame me if I want to make living with this right?

## Disclaimer

DO NOT USE FOR PRODUCTION, NO WARRANTY GURARANTEED, USE AT YOUR OWN RISK.
