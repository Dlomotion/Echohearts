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
