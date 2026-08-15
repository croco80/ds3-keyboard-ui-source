# Architecture

## Normal launch

1. Windows loads `xinput1_3.dll` beside `DarkSoulsIII.exe`.
2. The proxy loads the real Windows `%SystemRoot%\System32\xinput1_4.dll`.
3. XInput exports are forwarded to the real library.
4. Only when the process filename is `DarkSoulsIII.exe`, the proxy reads
   `DS3 Keyboard Icons\settings.ini`.
5. If exactly one resolution toggle is enabled, the proxy searches the loaded
   game's readable PE sections for the corresponding original width/height
   pair and replaces matching pairs in memory. It then updates
   `GraphicsConfig.xml`.
6. The proxy loads the selected private Mod Engine 2 runtime:
   normal or 16:10 UI mode, with or without Seamless Co-op chainloading.

## Optional Seamless launch

1. `DS3 Seamless + Keyboard UI.exe` checks for `DarkSoulsIII.exe`,
   `xinput1_3.dll`, and the user's `SeamlessCoop\ds3sc.dll`.
2. It sets `DS3_KEYBOARD_UI_SEAMLESS=1` only in its child-process environment.
3. It starts the adjacent `DarkSoulsIII.exe` with that folder as the working
   directory.
4. The proxy observes the flag and chooses a config whose `external_dlls`
   contains `SeamlessCoop\ds3sc.dll`.

Launching the game normally does not set the flag and therefore does not
chainload Seamless Co-op through this mod.

## Failure behavior

- Outside `DarkSoulsIII.exe`, the DLL acts only as an XInput forwarder.
- If the real system XInput library cannot load, DLL initialization fails.
- If settings are missing or invalid, optional features remain disabled.
- If the expected resolution pair is not found, no process values are changed
  and `GraphicsConfig.xml` is not rewritten.
- If a memory write fails, already changed pairs are restored.
- If the selected Mod Engine 2 file is absent, the game continues without this
  mod's assets.

