       >>SOURCE FORMAT FREE
identification division.
program-id. EcoKinRegistryAudit.

environment division.
input-output section.
file-control.
    select registry-file assign to dynamic input-path
        organization is line sequential
        file status is registry-file-status.

data division.
file section.
fd registry-file.
01 registry-record pic x(4096).

working-storage section.
78 max-rows value 1000.
01 input-path pic x(512) value spaces.
01 registry-file-status pic xx.
01 end-of-file pic 9 value 0.
01 header-seen pic 9 value 0.
01 row-count pic 9(8) value 0.
01 numbered-count pic 9(8) value 0.
01 unnumbered-count pic 9(8) value 0.
01 pending-count pic 9(8) value 0.
01 legacy-retired-count pic 9(8) value 0.
01 proposed-unapplied-count pic 9(8) value 0.
01 duplicate-ids pic 9(8) value 0.
01 invalid-ids pic 9(8) value 0.
01 duplicate-names pic 9(8) value 0.
01 invalid-names pic 9(8) value 0.
01 invalid-attributes pic 9(8) value 0.
01 invalid-states pic 9(8) value 0.
01 csv-errors pic 9(8) value 0.
01 schema-errors pic 9(8) value 0.
01 errors-total pic 9(8) value 0.
01 parsed-field-count pic 9(4) value 0.
01 field-table.
   05 field-value pic x(256) occurs 14 times.
01 stat-report-table.
   05 stat-report pic x(16) occurs 14 times.
01 seen-table.
   05 seen-entry occurs 1000 times.
      10 seen-name pic x(256).
      10 seen-id pic x(7).
01 seen-count pic 9(4) value 0.
01 row-index pic 9(4) value 0.
01 field-index pic 9(4) value 0.
01 char-index pic 9(4) value 0.
01 field-offset pic 9(4) value 0.
01 record-length pic 9(4) value 0.
01 quote-found pic 9 value 0.
01 malformed-row pic 9 value 0.
01 valid-id pic 9 value 0.
01 valid-stat pic 9 value 0.
01 stat-index pic 9(4) value 0.
01 stat-length pic 9(4) value 0.
01 proposal-count pic 9(4) value 0.
01 proposal-changed pic 9 value 0.
01 numeric-stat pic 9(3) value 0.
01 current-name pic x(256) value spaces.
01 current-id pic x(7) value spaces.
01 stat-text pic x(256) value spaces.
01 status-text pic x(256) value spaces.
01 disposition-text pic x(256) value spaces.
01 rows-text pic z(7)9.
01 numbered-text pic z(7)9.
01 unnumbered-text pic z(7)9.
01 pending-text pic z(7)9.
01 legacy-retired-text pic z(7)9.
01 proposed-unapplied-text pic z(7)9.
01 duplicate-ids-text pic z(7)9.
01 invalid-ids-text pic z(7)9.
01 duplicate-names-text pic z(7)9.
01 invalid-names-text pic z(7)9.
01 invalid-attributes-text pic z(7)9.
01 invalid-states-text pic z(7)9.
01 csv-errors-text pic z(7)9.
01 schema-errors-text pic z(7)9.
01 errors-text pic z(7)9.
01 row-number-text pic z(7)9.

procedure division.
main.
    accept input-path from command-line
    if function trim(input-path) = spaces
        display "Usage: ecokin-registry-audit INPUT.csv"
        move 2 to return-code
        goback
    end-if

    open input registry-file
    if registry-file-status not = "00"
        display "ERROR: cannot read registry CSV"
        move 2 to return-code
        goback
    end-if

    perform until end-of-file = 1
        read registry-file
            at end
                move 1 to end-of-file
            not at end
                if registry-file-status = "00"
                    perform process-record
                else
                    add 1 to schema-errors
                    move 1 to header-seen
                end-if
        end-read
    end-perform
    close registry-file

    if header-seen = 0
        add 1 to schema-errors
    end-if

    compute errors-total =
        duplicate-ids + invalid-ids + duplicate-names + invalid-names
        + invalid-attributes + invalid-states + csv-errors + schema-errors
    perform display-report

    if errors-total > 0
        move 1 to return-code
    else
        move 0 to return-code
    end-if
    goback.

process-record.
    initialize field-table
    initialize stat-report-table
    move 1 to field-index
    move 0 to field-offset quote-found malformed-row
    compute record-length =
        function length(function trim(registry-record trailing))
    perform varying char-index from 1 by 1
        until char-index > record-length
        evaluate registry-record(char-index:1)
            when '"'
                move 1 to quote-found
            when ","
                if field-index < 14
                    add 1 to field-index
                    move 0 to field-offset
                else
                    move 1 to malformed-row
                end-if
            when other
                if field-offset < 256
                    add 1 to field-offset
                    move registry-record(char-index:1)
                        to field-value(field-index)(field-offset:1)
                else
                    move 1 to malformed-row
                end-if
        end-evaluate
    end-perform
    move field-index to parsed-field-count

    if quote-found = 1
        add 1 to csv-errors
        if header-seen = 0
            move 1 to header-seen
        end-if
    else
        if malformed-row = 1 or parsed-field-count not = 14
            add 1 to schema-errors
            if header-seen = 0
                move 1 to header-seen
            end-if
        else
            if header-seen = 0
                perform validate-header
                move 1 to header-seen
            else
                add 1 to row-count
                if row-count <= max-rows
                    perform validate-data-row
                else
                    add 1 to schema-errors
                end-if
            end-if
        end-if
    end-if.

validate-header.
    if field-value(1) not = "Name"
        or field-value(2) not = "DexID"
        or field-value(3) not = "Status"
        or field-value(4) not = "Vibrance"
        or field-value(5) not = "Density"
        or field-value(6) not = "Harmony"
        or field-value(7) not = "Purity"
        or field-value(8) not = "CanonicalDisposition"
        or field-value(9) not = "SourceRow"
        or field-value(10) not = "ProposedVibrance"
        or field-value(11) not = "ProposedDensity"
        or field-value(12) not = "ProposedHarmony"
        or field-value(13) not = "ProposedPurity"
        or field-value(14) not = "ProposalGate"
        add 1 to schema-errors
    end-if.

validate-data-row.
    initialize stat-report-table
    move function upper-case(function trim(field-value(1)))
        to current-name
    move function trim(field-value(2)) to current-id
    move function trim(field-value(3)) to status-text
    move function trim(field-value(8)) to disposition-text

    if current-name = spaces
        add 1 to invalid-names
    else
        if function length(function trim(field-value(1))) > 256
            add 1 to invalid-names
        else
            perform varying row-index from 1 by 1
                until row-index > seen-count
                if current-name = seen-name(row-index)
                    add 1 to duplicate-names
                    exit perform
                end-if
            end-perform
        end-if
    end-if

    if status-text = spaces or disposition-text = spaces
        add 1 to invalid-states
    else
        evaluate status-text
            when "LOCKED CANON"
            when "MERGE-DUPLICATE"
            when "PENDING REVIEW"
            when "PERMANENT DEX"
            when "RETIRED"
            when "LEGACY-HISTORICAL"
            when "RENAME REQUIRED"
                continue
            when other
                add 1 to invalid-states
        end-evaluate
    end-if

    if status-text = "PENDING REVIEW"
        add 1 to pending-count
    end-if
    if status-text = "RETIRED" or status-text = "LEGACY-HISTORICAL"
        or disposition-text = "ARCHIVE; DO NOT SHIP"
        add 1 to legacy-retired-count
    end-if

    if current-id = spaces
        add 1 to unnumbered-count
    else
        add 1 to numbered-count
        perform validate-id
    end-if

    perform varying stat-index from 4 by 1 until stat-index > 7
        perform validate-stat
    end-perform

    move 0 to proposal-count proposal-changed
    perform varying stat-index from 10 by 1 until stat-index > 13
        if function trim(field-value(stat-index)) not = spaces
            add 1 to proposal-count
            perform validate-stat
            if function trim(field-value(stat-index))
                not = function trim(field-value(stat-index - 6))
                move 1 to proposal-changed
            end-if
        end-if
    end-perform
    move row-count to row-number-text
    display "ORIGINAL_STATS_" function trim(row-number-text) "="
        function trim(stat-report(4)) ","
        function trim(stat-report(5)) ","
        function trim(stat-report(6)) ","
        function trim(stat-report(7))
    display "PROPOSED_STATS_" function trim(row-number-text) "="
        function trim(stat-report(10)) ","
        function trim(stat-report(11)) ","
        function trim(stat-report(12)) ","
        function trim(stat-report(13))
    if proposal-count not = 0 and proposal-count not = 4
        add 1 to schema-errors
    end-if
    if proposal-count = 4 and proposal-changed = 1
        add 1 to proposed-unapplied-count
    end-if

    if current-name not = spaces and seen-count < max-rows
        add 1 to seen-count
        move current-name to seen-name(seen-count)
        if current-id not = spaces
            move current-id to seen-id(seen-count)
        end-if
    end-if.

validate-id.
    move 1 to valid-id
    if function length(function trim(field-value(2))) not = 7
        move 0 to valid-id
    else
        if current-id(1:4) not = "DEX-"
            move 0 to valid-id
        else
            if current-id(5:3) is not numeric
                move 0 to valid-id
            else
                if current-id(5:3) < "001" or current-id(5:3) > "125"
                    move 0 to valid-id
                end-if
            end-if
        end-if
    end-if

    if valid-id = 0
        add 1 to invalid-ids
    else
        perform varying row-index from 1 by 1 until row-index > seen-count
            if current-id = seen-id(row-index)
                add 1 to duplicate-ids
                exit perform
            end-if
        end-perform
    end-if.

validate-stat.
    move function trim(field-value(stat-index)) to stat-text
    move function length(function trim(field-value(stat-index)))
        to stat-length
    move 1 to valid-stat
    if stat-length < 1 or stat-length > 3
        move 0 to valid-stat
    else
        perform varying char-index from 1 by 1
            until char-index > stat-length
            if stat-text(char-index:1) < "0"
                or stat-text(char-index:1) > "9"
                move 0 to valid-stat
            end-if
        end-perform
    end-if

    if valid-stat = 0
        add 1 to invalid-attributes
        move "INVALID" to stat-report(stat-index)
    else
        move stat-text to stat-report(stat-index)
        compute numeric-stat = function numval(stat-text)
        if numeric-stat > 100
            add 1 to invalid-attributes
        end-if
    end-if.

display-report.
    move row-count to rows-text
    move numbered-count to numbered-text
    move unnumbered-count to unnumbered-text
    move pending-count to pending-text
    move legacy-retired-count to legacy-retired-text
    move proposed-unapplied-count to proposed-unapplied-text
    move duplicate-ids to duplicate-ids-text
    move invalid-ids to invalid-ids-text
    move duplicate-names to duplicate-names-text
    move invalid-names to invalid-names-text
    move invalid-attributes to invalid-attributes-text
    move invalid-states to invalid-states-text
    move csv-errors to csv-errors-text
    move schema-errors to schema-errors-text
    move errors-total to errors-text
    display "SCHEMA=ecokin-registry-v1"
    display "ROWS=" function trim(rows-text)
    display "NUMBERED=" function trim(numbered-text)
    display "UNNUMBERED=" function trim(unnumbered-text)
    display "PENDING_REVIEW=" function trim(pending-text)
    display "LEGACY_RETIRED=" function trim(legacy-retired-text)
    display "PROPOSED_UNAPPLIED=" function trim(proposed-unapplied-text)
    display "DUPLICATE_IDS=" function trim(duplicate-ids-text)
    display "INVALID_IDS=" function trim(invalid-ids-text)
    display "DUPLICATE_NAMES=" function trim(duplicate-names-text)
    display "INVALID_NAMES=" function trim(invalid-names-text)
    display "INVALID_ATTRIBUTES=" function trim(invalid-attributes-text)
    display "INVALID_STATES=" function trim(invalid-states-text)
    display "CSV_ERRORS=" function trim(csv-errors-text)
    display "SCHEMA_ERRORS=" function trim(schema-errors-text)
    display "ERRORS=" function trim(errors-text).
