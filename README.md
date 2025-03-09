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

### TODO
- [ ] Run as CLI
- [ ] Run as daemon
- [ ] Charge limit
- [ ] Wireless integration
- [ ] App rate limit
- [ ] Jetsam control
- [ ] Fan control

### License

MIT for now, may become non-free later.

## Disclaimer

DO NOT USE FOR PRODUCTION, NO WARRANTY GURARANTEED, USE AT YOUR OWN RISK.
