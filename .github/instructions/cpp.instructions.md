---
applyTo: "cpp/**/*.h,cpp/**/*.hpp,cpp/**/*.cpp,cpp/**/*.cc,cpp/**/*.cxx,**/CMakeLists.txt"
---

# Legacy C++ support instructions

Use modern, portable C++ for standalone utilities in this repository. Prefer RAII, const-correctness, scoped enums, explicit ownership, bounds checking, and warnings enabled.

This repository is not the authoritative Unreal runtime. Do not pretend that a g++/clang++/MSVC build here proves UE5.8 compatibility. When code is selected for production porting, adapt it deliberately into the `Echohearts` runtime module in `Dlomotion/ECHOHEARTS-REBEARTH-BUILD-` and verify through UBT/UHT.

Preserve Echohearts nomenclature and the four production attributes: Vibrance, Density, Harmony, Purity.
