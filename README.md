# MostWantedMultiplayerFix

By default, MW 2005 LAN Server always attempts to listen on the first network interface. This is not always desirable as sometimes you would want the server to be bound to a secondary interface like a VPN adapter. This repo provides a DLL and a launcher that will force the game to listen on all available interfaces. If you experience an inconsistent behaviour when attempting to play Need For Speed Most Wanted 2005 on LAN or a VPN service like Zero Tier, this could be the reason.

This fix may also work on other Need For Speed games as long as the process name is `speed.exe`.

## How To

- Download the DLL and EXE from the [Releases](https://github.com/rajiteh/MostWantedMultiplayerFix/releases/) tab.
- Alternatively, you can clone and build the project.
- Copy the files to your Most Wanted 2005 installation, where speed.exe exists.
- There are few ways to apply the fix.
  - Run `MostWantedMultiplayerFixLauncher.exe` to launch the game with fix injected.
  - If the game is already running, you can simply open the exe and it will attach itself to the game.
  - Use a DLL injector (like SpecialK Custom Plugin feature) to inject the `MostWantedMultiplayerFix.dll` to the game process.
 
## TODO

- [ ] Figure out a way to apply the fix without DLL injection (DLL proxying?)
 
 

