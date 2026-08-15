# Dark Souls III Keyboard UI + Optional 16:10 Support

This repository is the complete review source for four Dark Souls III UI mod
packages published on Nexus Mods:

- Antique Brass
- Ashen Iron
- Rusted Ember
- Thin Frame

All four packages use identical native code. They differ only in their
original keyboard and mouse prompt artwork.

## What the code does

`xinput1_3.dll` is an x64 XInput proxy loaded by Dark Souls III. It forwards
XInput calls to the Windows system `xinput1_4.dll`, selects one of four local
Mod Engine 2 configurations, loads the keyboard UI assets, and optionally
applies the user-selected 16:10 resolution in the running game process.

`DS3 Seamless + Keyboard UI.exe` is a small optional launcher. It verifies
that the game, proxy, and the user's separately installed Seamless Co-op DLL
exist, sets a temporary child-process environment variable, and starts
`DarkSoulsIII.exe`. It does not contain or redistribute Seamless Co-op.

The default configuration enables only the keyboard/mouse prompts. Both the
16:10 UI placement assets and runtime resolution adjustment are disabled by
default.

## Security-relevant behavior

- No network access, telemetry, updater, service, driver, or persistence.
- No game executable is included or modified on disk.
- The optional resolution feature uses `VirtualProtect` only on matching
  resolution values in the running Dark Souls III process.
- The optional resolution feature updates the user's existing
  `%APPDATA%\DarkSoulsIII\GraphicsConfig.xml`.
- Mod Engine 2 is loaded from a fixed private subdirectory of the mod.
- The launcher targets `DarkSoulsIII.exe` beside itself and expects Steam to
  be running. There is no DRM or licensing bypass code.

See [SECURITY.md](docs/SECURITY.md) for the detailed behavior and threat model.

## Build the native components

Requirements:

- Windows 10 or 11 x64
- Visual Studio Build Tools with the Desktop development with C++ workload
- PowerShell 5.1 or later

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-native.ps1
```

Outputs are written to `build\native`:

- `xinput1_3.dll`
- `DS3 Seamless + Keyboard UI.exe`

The release binaries were built with Microsoft C/C++ 19.50.35727 and Linker
14.50. A later compatible compiler may produce different hashes. See
[BUILDING.md](docs/BUILDING.md) for every compiler/linker flag and verification
step.

## Render the original artwork

Create a Python environment and install the pinned dependency:

```powershell
py -3 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements-artwork.txt
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\render-artwork.ps1 `
  -Python .\.venv\Scripts\python.exe
```

The renderers create the 512 px masters, exact 32 px prompt PNGs, and preview
images from code. The packaged artwork source and output PNGs are also retained
under `artwork\` for direct review.

## Repository map

- `src\xinput-proxy` - complete source and exports for `xinput1_3.dll`
- `src\seamless-launcher` - complete optional launcher source
- `src\win32-imports` - minimal Win32 import definitions used by the build
- `artwork` - renderer source and output PNGs for all four styles
- `config` - all four Mod Engine 2 configurations and default settings
- `scripts` - native build, artwork render, and release verification scripts
- `docs` - build details, architecture, security notes, release hashes, and
  asset provenance
- `third-party` - included third-party license texts; no third-party binary is
  stored in this source repository

## Release correspondence

The exact uploaded ZIP and binary SHA-256 values are recorded in
[RELEASE-MANIFEST.md](docs/RELEASE-MANIFEST.md). The source files in this
repository match the source embedded in each uploaded package.

## Licensing and third-party material

Our native code and repository scripts are MIT licensed. The newly drawn
keyboard/mouse artwork and renderer source are dedicated under CC0 1.0.
Those licenses do not apply to third-party components or game-derived files.

The distributed packages use credited assets from two Nexus Mods projects and
unmodified Mod Engine 2 v2.1.0 binaries. See
[ASSET-PROVENANCE.md](docs/ASSET-PROVENANCE.md) and
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

Dark Souls, Dark Souls III, and related assets are property of their respective
owners. This is an unofficial fan project and is not affiliated with or
endorsed by FromSoftware or Bandai Namco.
