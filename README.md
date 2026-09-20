# Echohearts

Independent C++ source and verification repository for **Echohearts: Rebearth / Echohearts: Resonance Arena / Eco-Kin**.

The canonical game-design and Unreal-development repository remains `Dlomotion/Echohearts-Rebearth`. This repository is a second source-code/verification track used to duplicate tested domain logic, harden C++ quality checks, and verify development databases without overwriting the main design repository.

## Current verified source scope

The duplicated C++ domain core currently covers:

- marketplace listing validation and economy sink calculations
- explicit rejection of Eco-Kin, Eco-Kin eggs, and sentient partners from commerce
- player housing/furniture placement validation
- cross-region recurring event windows
- deterministic daily-mission selection with anti-friction rules

The source is under `cpp/` and is intentionally engine-independent until the actual UE5.8 project source is available for integration.

## Build and test

```bash
cmake -S cpp -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Verify a SQLite database

```bash
./build/echo_verify --database path/to/echohearts.db --level 5
```

Levels range from 1 to 10. Level 5 is the normal starting point; level 10 runs the deepest current read-only verification. See `docs/VERIFICATION.md` for the exact checks and the distinction between Echohearts verification, Git `fsck`, and IBM ELM `repotools -verify`.

## Clone

SSH:

```bash
git clone git@github.com:Dlomotion/Echohearts.git
```

HTTPS:

```bash
git clone https://github.com/Dlomotion/Echohearts.git
```

`git@github.com:Dlomotion/Echohearts.git` is a repository URL, not an SSH key. Never commit a private key.

## Naming standard

- `EcoKin` in code identifiers.
- `Eco-Kin` in prose.
- `Eco-Kin's` only for singular possessive grammar.

## Verification status

This repository can verify the standalone C++ source and SQLite test database through GitHub Actions. It does **not** yet prove UE5.8 compilation, networking, save migration, replication, gameplay performance, or production database correctness because those production artifacts are not present here yet.

© 2026 Into Deep Studios and Donta L. Owens. All rights reserved.
