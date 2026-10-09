# Eco-Kin registry QA utilities

These read-only, standalone tools audit a deliberately small CSV interchange
contract. They are optional legacy-language utilities, not a Dex, roster,
gameplay implementation, or source of canon approval. They never assign IDs,
apply proposed stats, or write files.

## Shared CSV contract: `ecokin-registry-v1`

Use this exact header and order. It matches the 19 required columns in the
standalone C++ `EcoKinIntake::Parse` contract:

```csv
Name,DexID,Status,Vibrance,Density,Harmony,Purity,SourceRow,ReviewFlags,SuggestedMergeTarget,FormBaseCandidate,CanonicalDisposition,IDStatus,ProposedVibrance,ProposedDensity,ProposedHarmony,ProposedPurity,ProposalDelta,ProposalGate
```

Fields are unquoted UTF-8 text with comma delimiters and one record per physical
line. Quoted CSV fields (including escaped quotes and quoted commas) are
intentionally unsupported and rejected with a non-zero exit. Names and statuses
must be non-empty. `DexID` is either empty for an intake/archive row or exactly
`DEX-001` through `DEX-125`; blank IDs remain blank. `SourceRow` is a unique
positive integer provenance reference. Original and proposed attributes are
separate unsigned integer columns from 0 through 100, in the fixed public
attribute order shown in the header.

`Status` is one of `LOCKED CANON`, `MERGE-DUPLICATE`, `PENDING REVIEW`,
`PERMANENT DEX`, `RETIRED`, `LEGACY-HISTORICAL`, or `RENAME REQUIRED`.
`CanonicalDisposition`, `IDStatus`, and `ReviewFlags` are preserved as review
metadata, not approval signals. A no-change proposal must equal the original
attributes and use delta `0`; a draft proposal must have a non-zero delta equal
to proposed-total minus original-total. Drafts are always counted as
unapproved, never applied. `IDStatus` must distinguish an existing protected
slot from an unnumbered candidate, and merge/rename review states require a
non-empty `ReviewFlags` value. Repeated names are compared case-insensitively.
`IDStatus` is exactly `PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED`
for numbered rows or `NO PRODUCTION ID — DO NOT ASSIGN` for unnumbered rows.
Allowed dispositions are `INTAKE; CANON PROMOTION REQUIRED`, `CORE SLOT; PENDING
REVIEW`, `CORE SLOT; EXISTING PERMANENT LABEL`, `ARCHIVE; DO NOT SHIP`,
`INTAKE; RENAME REVIEW`, `CORE SLOT; HISTORICAL STATUS CONFLICT`, and `LOCKED
CORE IDENTITY; STAT TUNING PENDING`. `ProposalGate` is exactly `NO NUMERIC
CHANGE PROPOSED` or `DRAFT CAP SUGGESTION; NOT APPROVED`.
The intake's wider CSV may contain extra columns; export the 19 contract columns
in the exact order above before passing it to these fixed-schema tools. The
input is bounded to 1,000 data rows, 4,096 bytes per record, and 256 bytes per
field.

The projection can be made with Python's standard CSV module without editing
the source intake:

```sh
python3 - path/to/full-intake.csv /tmp/ecokin-registry-v1.csv <<'PY'
import csv
import sys

columns = (
    "Name", "DexID", "Status", "Vibrance", "Density", "Harmony", "Purity",
    "SourceRow", "ReviewFlags", "SuggestedMergeTarget", "FormBaseCandidate",
    "CanonicalDisposition", "IDStatus", "ProposedVibrance",
    "ProposedDensity", "ProposedHarmony", "ProposedPurity", "ProposalDelta",
    "ProposalGate",
)
with open(sys.argv[1], newline="", encoding="utf-8-sig") as source:
    rows = csv.DictReader(source)
    with open(sys.argv[2], "w", newline="", encoding="utf-8") as target:
        writer = csv.DictWriter(target, fieldnames=columns, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)
PY
```

Both programs emit the same deterministic `key=value` report and return 1 for
CSV, schema, or identity errors, 0 for an accepted intake (warnings do not
change the source), and 2 for usage or file-open errors. `PROPOSED_UNAPPLIED`
counts rows marked with the unapproved draft proposal gate.
Each accepted-shape input row also emits separate `ORIGINAL_STATS_n` and
`PROPOSED_STATS_n` lines in source order. The columns remain separate even
when proposed values differ from originals.

The golden inputs use the same required field contract and validation outcomes
as the standalone C++ registry validator tracked separately in issue #20 / PR
#24. The fixtures exercise protected IDs, review states, attribute bounds,
provenance, references, and unapplied proposals. The C++ CLI requires the full
554-row canonical intake, so small fixtures are compared against its domain
validator with canonical-count checks disabled. Quoted CSV is deliberately
rejected by these COBOL/FreeBASIC tools; the C++ parser supports valid quoted
fields.

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
