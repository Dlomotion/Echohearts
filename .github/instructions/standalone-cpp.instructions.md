---
applyTo: "cpp/**/*.h,cpp/**/*.hpp,cpp/**/*.c,cpp/**/*.cc,cpp/**/*.cpp,cpp/**/CMakeLists.txt,.github/workflows/**/*.yml,.github/workflows/**/*.yaml"
---

# Standalone C++ verification instructions

This repository may harden engine-independent C++ domain logic, but it is not the UE5.8 runtime authority.

- Use modern, portable C++ with RAII, const-correctness, explicit ownership, bounds checks, deterministic initialization, and tests.
- CMake/MSVC/Clang/G++ success here proves only the standalone code tested here.
- Do not add Unreal reflection/gameplay code here as a second runtime track; migrate UE implementation to `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-`.
- Keep public gameplay concepts aligned with Vibrance, Density, Harmony, Purity and the canonical contracts repository.
- Diagnose exact tool output; never assume exit code 2 has one universal meaning.

## Toolchain resolution and routing (2026-10-06 intake)

- Executable Echohearts compiler resolution belongs only in `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-/BuildScripts/EchoheartsCompiler.py`; do not create a competing driver here. Route implementation/tests to BUILD and canonical documentation to `Dlomotion/Echohearts-Rebearth`.
- Explicit compiler targets must resolve via PATH or be regular executable files; reject blank, missing, directory, and unsupported targets. Never use `shutil.which(x) or x` as proof a tool exists, and never classify unknown executables as GNU.
- Use argv-based subprocess calls; filename blacklists (e.g. 'hack'/'override') are not security controls.
- Do not add an Unreal Blueprint string library or the supplied glyph/array example (`char PrimitiveByteFootprint = CharacterTokenString;` does not compile; glyph-to-integer conversion is unrelated). Do not invent tracking IDs 0x2C/0x2D.
- `STATIC CHECK PASSED` requires actual retained evidence; standalone C++ success does not verify UE5.8 runtime.
