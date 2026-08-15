# Nexus release manifest

These hashes identify the four original packages submitted to Nexus Mods.

## ZIP archives

| Variant | SHA-256 |
| --- | --- |
| Antique Brass | `DE83F0F4F3B7AF53CB8005B97E8636B4A7CE59A85409C25FD37C759DC8DF3435` |
| Ashen Iron | `2ABAFCBE16E616C3E3359C82C59BE46D1D64029D6104695EB45F82361FC3FD6F` |
| Rusted Ember | `85BF5180E13FCCA2BC655D0B06C8CE45D9F391A9463887BB663BBDDF0B14FA70` |
| Thin Frame | `974A883D83A696F3B45B62589C5DF9E1C4AFAF212E2C92BB3EE8EE4A5FCCE3C9` |

## Common custom binaries

Every variant contains these identical binaries:

| File | Size | SHA-256 |
| --- | ---: | --- |
| `xinput1_3.dll` | 17,920 | `0FAB51021393699D573DB40DAEF2DDCBC92CBAA546497B7820593245C0B3E8E1` |
| `DS3 Seamless + Keyboard UI.exe` | 6,144 | `DD85D43C9ED4EA927AD62A64C43396854D9787135F0F400809764C19C2D10695` |

## Corresponding source embedded in every ZIP

| File | SHA-256 |
| --- | --- |
| `XInput-Proxy-SOURCE.cpp` | `EEF3F75603BE9C5380B694BCC650053A4DC7B04DA9EFBB20E90B05E49D0F6E4A` |
| `Seamless-Mode-Launcher-SOURCE.cpp` | `DAF5C0B1810D20FC7FEFC6D6BA235DFD7C28FC3531BC2F02F51165A8A2CB35FD` |
| `Win32-XInput-MINIMAL.h` | `85DB9116630DD119D7B59EA04F0984F63E018A1E87EED323B7FB8B3749F9A657` |
| `XInput-Proxy-EXPORTS.def` | `F471937C8D114478F021D4B2BC36374696349170244A5F25AD11CEA8AC965333` |

The repository renames the two `.cpp` files for readability without modifying
their bytes.

## Common third-party runtime files

| File | SHA-256 |
| --- | --- |
| Mod Engine 2 `modengine2.dll` | `0E6105840422A7828FAF137EFB1C5823C8C8E6CFB044B12A360307BDA745175E` |
| Mod Engine 2 `lua.dll` | `F1559F6DB79887717E7568257F7A65AD86BC5ECFEF948CD05B5EE77DE41E14D5` |

Those are credited, unmodified Mod Engine 2 v2.1.0 binaries and are not stored
in this source repository.

