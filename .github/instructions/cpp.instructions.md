---
applyTo: "cpp/**/*.h,cpp/**/*.hpp,cpp/**/*.cpp,cpp/**/*.cc,cpp/**/*.cxx,**/CMakeLists.txt"
---
## 2026-10-20 live runtime ownership correction (takes precedence)

- The active executable Unreal Engine 5.8 C++ project, build/package tooling, tests, and retained runtime evidence now live in `IN2Deep-Studios/ECO-KIN-Game`.
- The former `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-` and `Dlomotion/ECHOHEARTS-REBEARTH-` addresses are unavailable. Do not route work to them, recreate them, or create another Unreal project/module.
- UE gameplay/create/fix requests belong in the existing `EchoheartsRebearth.uproject` and `EchoheartsRebearth` module in the live game repository. This repository retains its declared non-runtime role.
- Any later line in this file that calls ECO-KIN-Game legacy/prototype-only or names the former BUILD repository as active is superseded by this section.
- Repository/static checks never prove UHT/UBT, Editor, PIE, networking, save/cloud, cooking, package launch, performance, target hardware, or EPUB rendering.


# Legacy C++ support instructions

Use modern, portable C++ for standalone utilities in this repository. Prefer RAII, const-correctness, scoped enums, explicit ownership, bounds checking, and warnings enabled.

This repository is not the authoritative Unreal runtime. Do not pretend that a g++/clang++/MSVC build here proves UE5.8 compatibility. When code is selected for production porting, adapt it deliberately into the `Echohearts` runtime module in `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-` and verify through UBT/UHT.

Preserve Echohearts nomenclature and the four production attributes: Vibrance, Density, Harmony, Purity.
