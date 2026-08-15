# Security notes

This document describes behavior that may be relevant to antivirus or manual
file review.

## Why an antivirus heuristic may flag the proxy

The unsigned `xinput1_3.dll` is intentionally placed beside a game executable,
loads another DLL, and optionally calls `VirtualProtect` to change resolution
values inside the current game process. Those patterns overlap with generic
malware heuristics even though their purpose here is local game modding.

The relevant implementation is fully visible in
`src/xinput-proxy/XInput-Proxy.cpp`.

## Boundaries

The proxy:

- runs only inside the process that loaded it;
- activates mod behavior only when the process basename is
  `DarkSoulsIII.exe`;
- loads the system XInput library from the Windows system directory;
- loads Mod Engine 2 only from one of four fixed local subdirectories;
- patches only matching 32-bit resolution width/height pairs in readable
  sections of the currently loaded game image;
- writes only the Dark Souls III `GraphicsConfig.xml` file when a runtime
  resolution patch succeeded;
- has no socket, HTTP, registry, service, scheduled-task, driver, credential,
  process-injection, or remote-thread code.

The launcher:

- starts only the adjacent `DarkSoulsIII.exe`;
- sets one temporary child-process environment variable;
- does not download, update, inject into an unrelated process, or modify a
  game executable;
- requires the user's separately installed Seamless Co-op DLL.

## Online use

This is a game mod loader. Modified processes may be incompatible with normal
anti-cheat or matchmaking. The release documentation recommends offline mode
or Seamless Co-op's isolated matchmaking. It does not disable or bypass any
anti-cheat, DRM, Steam App ID, or licensing check.

## Reporting

Security reports should identify the exact source path, release filename, and
SHA-256 involved. Release hashes are listed in `RELEASE-MANIFEST.md`.

