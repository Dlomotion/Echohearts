       >>SOURCE FORMAT FREE
identification division.
program-id. EchoArtStatus.

environment division.
input-output section.
file-control.
    select intake-file assign to dynamic input-path
        organization is line sequential
        file status is intake-file-status.

data division.
file section.
fd intake-file.
01 intake-record pic x(160).

working-storage section.
01 input-path pic x(512) value spaces.
01 intake-file-status pic xx.
01 reached-end pic 9 value 0.
01 active-count pic 9(8) value 0.
01 pending-count pic 9(8) value 0.
01 retired-count pic 9(8) value 0.
01 invalid-count pic 9(8) value 0.
01 active-text pic z(7)9.
01 pending-text pic z(7)9.
01 retired-text pic z(7)9.
01 invalid-text pic z(7)9.

procedure division.
main.
    accept input-path from command-line
    if function trim(input-path) = spaces
        display "Usage: echo-art-status INPUT_TEXT_FILE"
        move 2 to return-code
        goback
    end-if

    open input intake-file
    if intake-file-status not = "00"
        display "ERROR: cannot read art-status input"
        move 2 to return-code
        goback
    end-if

    perform until reached-end = 1
        read intake-file
            at end
                move 1 to reached-end
            not at end
                evaluate function trim(intake-record)
                    when "ACTIVE_CANON_ART"
                        add 1 to active-count
                    when "PENDING_CANON_REVIEW"
                        add 1 to pending-count
                    when "REFERENCE_RETIRED_NEEDS_REDESIGN"
                        add 1 to retired-count
                    when other
                        add 1 to invalid-count
                end-evaluate
        end-read
    end-perform

    close intake-file

    move active-count to active-text
    move pending-count to pending-text
    move retired-count to retired-text
    move invalid-count to invalid-text

    display "ACTIVE_CANON_ART=" function trim(active-text)
    display "PENDING_CANON_REVIEW=" function trim(pending-text)
    display "REFERENCE_RETIRED_NEEDS_REDESIGN="
        function trim(retired-text)
    display "INVALID=" function trim(invalid-text)

    if invalid-count > 0
        move 1 to return-code
    else
        move 0 to return-code
    end-if
    goback.
