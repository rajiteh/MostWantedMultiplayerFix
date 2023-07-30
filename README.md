# MostWantedMultiplayerFix

By default, MW 2005 LAN Server always attempts to listen on the first network interface. This is not always desirable as sometimes you would want the server to be bound to a secondary interface like a VPN adapter. This repo provides a DLL and a launcher that will force the game to listen on all available interfaces. If you experience an inconsistent behaviour when attempting to play Need For Speed Most Wanted 2005 on LAN or a VPN service like Zero Tier, this could be the reason.

This fix may also work on other Need For Speed games as long as the process name is `speed.exe`.

## How To

- Download the files from the [Releases](https://github.com/rajiteh/MostWantedMultiplayerFix/releases/) tab.
- Alternatively, you can clone and build the project (Note: The `.asi` file is simply the `.dll` renamed, there is no other change).

### ASI Loader

Most NFSMW Repacks are pre-packaged with an ASI Loader/Plugin Injector. You can check by looking at the presence of a `scripts/` folder with a bunch of  `.asi` files in them. Download the `MostWantedMultiplayerFix.asi` to the `scripts/` folder in your game directory and the plugin will be loaded automatically.

### DLL Injector

If your game already uses a DLL injector like Special K Custom Plugin or you have your own preferred mechanism to do so, you may simply download the `MostWantedMultiplayerFix.dll` file and ensure that it gets injected to the process.

### Standalone Launcher

This method requires no additional plugin frameworks, simply copy the files `MostWantedMultiplayerFix.exe` and `MostWantedMultiplayerFix.dll` to the game directory where `speed.exe` is. Run the executable and it will launch the game, or if the game is already running it will attempt to inject itself to the running process.

NOTE: Antivirus software may flag the binary as malicious as it is performing a DLL injection.  

 
 

