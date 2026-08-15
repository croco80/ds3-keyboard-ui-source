# Asset provenance

This project deliberately separates original material, permissively reused
mod assets, open-source runtime components, and game-derived containers.

## Original work in this repository

- The XInput proxy source
- The optional Seamless-mode launcher source
- Build and verification scripts
- All four sets of 17 keyboard/mouse prompt designs
- The Python renderer geometry for those designs
- Preview and QA sheets produced from those designs

The native code is MIT licensed. The original keycap artwork and renderer are
offered under CC0 1.0.

The designs use the Georgia typeface through the copy installed with Windows.
No font software is bundled.

## Menu archive basis

The distributed `01_common.tpf.dcx` uses the technical archive from
"Keyboard Icons in Menus (via Mod Engine or UXM)" 1.01 by Ytterbium3835:

https://www.nexusmods.com/darksouls3/mods/823

Every keyboard/mouse prompt target mapped in `TEXTURE-MAP.txt` is replaced by
our original artwork. The upstream author is credited. This binary container
is not committed to this source repository.

## 16:10 UI files

The 42 optional `.gfx` files are copied unchanged from "16x10 UI Fixes for
Dark Souls III (Steam Deck compatible)" 1.0 by sukerokushin:

https://www.nexusmods.com/darksouls3/mods/1832

The Nexus page currently permits asset use and modifications; attribution is
retained. The files remain separate, optional, and disabled by default. They
are not committed to this source repository or represented as our work.

## Runtime files

The distributed `modengine2.dll` and `lua.dll` are official, unmodified Mod
Engine 2 v2.1.0 files under the upstream MIT license. They are not built from
our source or committed here. The exact upstream source is:

https://github.com/soulsmods/ModEngine2/tree/release-2.1.0

## Not included

- `DarkSoulsIII.exe` or any other game executable
- Any packed original game installation archive
- Seamless Co-op binaries or assets
- Steam DLLs, App ID overrides, DRM bypasses, or license bypasses

