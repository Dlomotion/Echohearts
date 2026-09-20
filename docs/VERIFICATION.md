# Echohearts Verification Guide

Status: VERIFIED TOOLING DESIGN / TARGET GAME DATABASE INTEGRATION NOT YET VERIFIED

This repository contains an independent C++ verification source for Echohearts. It mirrors the already-tested meta-system domain core and adds a repository/database verifier for SQLite-backed development data.

## Important correction about IBM repotools

IBM Engineering Lifecycle Management `repotools-*.sh/.bat -verify` is an IBM-specific administration command for IBM Jazz/ELM repositories. It is not a GitHub repository verifier and must not be run against Echohearts source code or an arbitrary SQLite game database.

The useful idea retained here is the graduated 1-10 verification model. `echo_verify` is an Echohearts-owned implementation with different checks appropriate to SQLite and C++ development.

## Build

```bash
cmake -S cpp -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Database verify command

```bash
./build/echo_verify --database path/to/echohearts.db --level 5
```

Level 5 is the normal starting point. Use level 10 for the deepest current offline verification.

### Echohearts verification levels

1. Open database read-only and fetch SQLite version.
2. Read and enumerate `sqlite_schema`.
3. Run `PRAGMA quick_check`.
4. Run full `PRAGMA integrity_check`.
5. Run `PRAGMA foreign_key_check`.
6. Read every user table with `COUNT(*)`.
7. Validate core metadata pragmas.
8. Enable `cell_size_check` and rerun integrity verification.
9. Prepare a read query against every user table.
10. Repeat full integrity, foreign-key, and table-read verification as a final deep pass.

A failure returns a non-zero process exit code so CI can block a merge.

## SQLite integrity facts

`PRAGMA integrity_check` checks low-level formatting and consistency, including malformed records, missing pages, index consistency, and several constraint classes. It does not check foreign-key violations, so `PRAGMA foreign_key_check` is a separate required check in this verifier.

`PRAGMA quick_check` is intentionally faster and less exhaustive than `integrity_check`.

The verifier opens databases read-only. For a stable authoritative verification of a live game database, stop writers or verify a transactionally consistent backup/snapshot. Do not copy a database file while another process is mutating it and then call the copied bytes authoritative.

## Git repository integrity

Git object/database integrity is a separate concern from the gameplay database. For a local clone, use Git's native object verification:

```bash
git fsck --full
```

That checks Git object connectivity/integrity. It does not validate game data, C++ behavior, Unreal assets, save compatibility, economy rules, or database foreign keys.

## C++ quality gates

The CI pipeline currently enforces:

- C++17 build through CMake
- `-Wall -Wextra -Wpedantic -Werror` on GCC/Clang
- CTest execution
- cppcheck warning/performance/portability analysis
- SQLite valid-database verification at levels 1, 5, and 10
- a negative test proving a broken foreign key is rejected

Future gates should be added only when the project has the required source and dependencies, including sanitizers, clang-tidy profiles, Unreal Engine 5.8 Automation tests, persistence/recovery tests, replication tests, fuzzing of untrusted inputs, and migration compatibility tests.

## SSH clone correction

`git@github.com:Dlomotion/Echohearts.git` is an SSH repository URL, not an SSH key.

Valid clone commands are:

```bash
git clone git@github.com:Dlomotion/Echohearts.git
```

or:

```bash
git clone https://github.com/Dlomotion/Echohearts.git
```

SSH private keys must remain private and should never be committed to this repository. Add the public key to the GitHub account through GitHub's supported authentication settings, then test locally with `ssh -T git@github.com`.

## Naming lock

Use:

- `EcoKin` for code identifiers where hyphens are invalid or undesirable.
- `Eco-Kin` for the franchise creature term in prose.
- `Eco-Kin's` only as the singular possessive form when grammatically correct.

Do not use `Eco_kin`, `Ech-kin`, `Echo-Kin` as the current creature brand, or inconsistent variants in new source.

© 2026 Into Deep Studios and Donta L. Owens. All rights reserved.
