---
applyTo: "cpp/**/*.h,cpp/**/*.hpp,cpp/**/*.c,cpp/**/*.cc,cpp/**/*.cpp,cpp/**/CMakeLists.txt,.github/workflows/**/*.yml,.github/workflows/**/*.yaml"
---
## 2026-10-20 live runtime ownership correction (takes precedence)

- The active executable Unreal Engine 5.8 C++ project, build/package tooling, tests, and retained runtime evidence now live in `IN2Deep-Studios/ECO-KIN-Game`.
- The former `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-` and `Dlomotion/ECHOHEARTS-REBEARTH-` addresses are unavailable. Do not route work to them, recreate them, or create another Unreal project/module.
- UE gameplay/create/fix requests belong in the existing `EchoheartsRebearth.uproject` and `EchoheartsRebearth` module in the live game repository. This repository retains its declared non-runtime role.
- Any later line in this file that calls ECO-KIN-Game legacy/prototype-only or names the former BUILD repository as active is superseded by this section.
- Repository/static checks never prove UHT/UBT, Editor, PIE, networking, save/cloud, cooking, package launch, performance, target hardware, or EPUB rendering.


# Standalone C++ verification instructions

This repository may harden engine-independent C++ domain logic, but it is not the UE5.8 runtime authority.

- Use modern, portable C++ with RAII, const-correctness, explicit ownership, bounds checks, deterministic initialization, and tests.
- CMake/MSVC/Clang/G++ success here proves only the standalone code tested here.
- Do not add Unreal reflection/gameplay code here as a second runtime track; migrate UE implementation to `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-`.
- Keep public gameplay concepts aligned with Vibrance, Density, Harmony, Purity and the canonical contracts repository.
- Diagnose exact tool output; never assume exit code 2 has one universal meaning.
