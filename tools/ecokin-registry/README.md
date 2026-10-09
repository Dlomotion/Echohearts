# Eco-Kin registry QA utilities

These read-only, standalone tools audit a deliberately small CSV interchange
contract. They are optional legacy-language utilities, not a Dex, roster,
gameplay implementation, or source of canon approval. They never assign IDs,
apply proposed stats, or write files.

## Shared CSV contract: `ecokin-registry-v1`

Use this exact header and order (a 14-column subset of the review intake):

```csv
Name,DexID,Status,Vibrance,Density,Harmony,Purity,CanonicalDisposition,SourceRow,ProposedVibrance,ProposedDensity,ProposedHarmony,ProposedPurity,ProposalGate
```

Fields are unquoted UTF-8 text with comma delimiters and one record per physical
line. Quoted CSV fields (including escaped quotes and quoted commas) are
intentionally unsupported and rejected with a non-zero exit. Names and statuses
must be non-empty. `DexID` is either empty for an intake/archive row or exactly
`DEX-001` through `DEX-125`; blank IDs remain blank. Original attributes are
unsigned integer values from 0 through 100, in the fixed public attribute order
shown in the header. Proposed attributes are either all empty or all four
unsigned integers in the same range. A differing proposal is reported as
unapplied; the original values are never changed.

`Status` is one of `LOCKED CANON`, `MERGE-DUPLICATE`, `PENDING REVIEW`,
`PERMANENT DEX`, `RETIRED`, `LEGACY-HISTORICAL`, or `RENAME REQUIRED`.
Statuses and `CanonicalDisposition` are review metadata, not approval signals.
`SourceRow` is retained as a source reference and is not interpreted as a
production identifier. Repeated names are compared case-insensitively. The
input is bounded to 1,000 data rows, 4,096 bytes per record, and 256 bytes per
field.

Both programs emit the same deterministic `key=value` report and return 1 for
CSV, schema, or identity errors, 0 for an accepted intake (warnings do not
change the source), and 2 for usage or file-open errors. `PROPOSED_UNAPPLIED`
counts rows where proposed values differ from the original values.

The expected reports in `tests/expected/` are the shared domain-contract
fixtures for the standalone C++ registry validator tracked separately in issue
#20. The current default-branch C++ code does not yet expose an Eco-Kin
registry-audit CLI, so direct C++ executable parity is not claimed here.

## Build and run

Test fixture expectations without either compiler:

```sh
python3 tools/ecokin-registry/tests/verify.py
```

With GnuCOBOL 3.1.2-compatible `cobc` and FreeBASIC 1.09-compatible `fbc`:

```sh
cobc -x -free -o /tmp/ecokin-registry-audit tools/ecokin-registry/cobol/ecokin-registry-audit.cbl
fbc tools/ecokin-registry/basic/ecokin-registry-stats.bas -x /tmp/ecokin-registry-stats
python3 tools/ecokin-registry/tests/verify.py \
  --program /tmp/ecokin-registry-audit \
  --program /tmp/ecokin-registry-stats
```

The fixture harness runs every provided program against every golden CSV and
checks its report and exit code. Compiler versions present on a machine are
reported by `cobc --version` and `fbc -version`; no compiler is bundled here.
