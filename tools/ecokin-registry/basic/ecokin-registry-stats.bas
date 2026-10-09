' Read-only Eco-Kin registry audit for the ecokin-registry-v1 CSV contract.
' Proposals are reported, never applied; this is not a canon approval tool.

Const MAX_ROWS As Integer = 1000
Const MAX_FIELDS As Integer = 14

Dim inputPath As String = Command(1)
If Len(Trim(inputPath)) = 0 Then
    Print "Usage: ecokin-registry-stats INPUT.csv"
    End 2
End If

Dim fileNum As Integer = FreeFile()
Open inputPath For Input As #fileNum
If Err <> 0 Then
    Print "ERROR: cannot read registry CSV"
    End 2
End If

Dim As String fields(1 To MAX_FIELDS)
Dim As String seenNames(1 To MAX_ROWS)
Dim As String seenIds(1 To MAX_ROWS)
Dim As String recordText, currentName, currentId, statusText, dispositionText
Dim As Integer rows = 0, numbered = 0, unnumbered = 0
Dim As Integer pendingReview = 0, legacyRetired = 0, proposedUnapplied = 0
Dim As Integer duplicateIds = 0, invalidIds = 0, duplicateNames = 0
Dim As Integer invalidNames = 0, invalidAttributes = 0, invalidStates = 0
Dim As Integer csvErrors = 0, schemaErrors = 0, errorsTotal = 0
Dim As Integer seenCount = 0, headerSeen = 0, quoteFound, malformed
Dim As Integer fieldIndex, fieldOffset, charIndex, fieldCount, rowIndex
Dim As Integer statIndex, proposalCount, proposalChanged, statValid
Dim As Double statValue

While Not Eof(fileNum)
    Line Input #fileNum, recordText
    For fieldIndex = 1 To MAX_FIELDS
        fields(fieldIndex) = ""
    Next
    fieldIndex = 1
    fieldOffset = 0
    quoteFound = 0
    malformed = 0

    If Len(recordText) > 4096 Then
        malformed = 1
    Else
        For charIndex = 1 To Len(recordText)
            Select Case Mid(recordText, charIndex, 1)
                Case Chr(34)
                    quoteFound = 1
                Case ","
                    If fieldIndex < MAX_FIELDS Then
                        fieldIndex += 1
                        fieldOffset = 0
                    Else
                        malformed = 1
                    End If
                Case Else
                    fieldOffset += 1
                    If fieldOffset <= 256 Then
                        fields(fieldIndex) &= Mid(recordText, charIndex, 1)
                    Else
                        malformed = 1
                    End If
            End Select
        Next
    End If
    fieldCount = fieldIndex

    If quoteFound <> 0 Then
        csvErrors += 1
        If headerSeen = 0 Then
            headerSeen = 1
        End If
    ElseIf malformed <> 0 Or fieldCount <> MAX_FIELDS Then
        schemaErrors += 1
        If headerSeen = 0 Then
            headerSeen = 1
        End If
    ElseIf headerSeen = 0 Then
        If fields(1) <> "Name" Or fields(2) <> "DexID" Or _
           fields(3) <> "Status" Or fields(4) <> "Vibrance" Or _
           fields(5) <> "Density" Or fields(6) <> "Harmony" Or _
           fields(7) <> "Purity" Or fields(8) <> "CanonicalDisposition" Or _
           fields(9) <> "SourceRow" Or fields(10) <> "ProposedVibrance" Or _
           fields(11) <> "ProposedDensity" Or fields(12) <> "ProposedHarmony" Or _
           fields(13) <> "ProposedPurity" Or fields(14) <> "ProposalGate" Then
            schemaErrors += 1
        End If
        headerSeen = 1
    Else
        rows += 1
        If rows > MAX_ROWS Then
            schemaErrors += 1
        Else
            currentName = UCase(Trim(fields(1)))
            currentId = Trim(fields(2))
            statusText = Trim(fields(3))
            dispositionText = Trim(fields(8))

            If Len(currentName) = 0 Then
                invalidNames += 1
            Else
                For rowIndex = 1 To seenCount
                    If currentName = seenNames(rowIndex) Then
                        duplicateNames += 1
                        Exit For
                    End If
                Next
            End If

            Select Case statusText
                Case "LOCKED CANON", "MERGE-DUPLICATE", "PENDING REVIEW", _
                     "PERMANENT DEX", "RETIRED", "LEGACY-HISTORICAL", _
                     "RENAME REQUIRED"
                    If Len(dispositionText) = 0 Then invalidStates += 1
                Case Else
                    invalidStates += 1
            End Select

            If statusText = "PENDING REVIEW" Then pendingReview += 1
            If statusText = "RETIRED" Or statusText = "LEGACY-HISTORICAL" Or _
               dispositionText = "ARCHIVE; DO NOT SHIP" Then legacyRetired += 1

            If Len(currentId) = 0 Then
                unnumbered += 1
            Else
                numbered += 1
                statValid = 1
                If Len(currentId) <> 7 Or Left(currentId, 4) <> "DEX-" Then
                    statValid = 0
                ElseIf Val(Mid(currentId, 5, 3)) < 1 Or _
                       Val(Mid(currentId, 5, 3)) > 125 Or _
                       Str(Val(Mid(currentId, 5, 3))) = "" Then
                    statValid = 0
                Else
                    For charIndex = 5 To 7
                        If Mid(currentId, charIndex, 1) < "0" Or _
                           Mid(currentId, charIndex, 1) > "9" Then statValid = 0
                    Next
                End If
                If statValid = 0 Then
                    invalidIds += 1
                Else
                    For rowIndex = 1 To seenCount
                        If currentId = seenIds(rowIndex) Then
                            duplicateIds += 1
                            Exit For
                        End If
                    Next
                End If
            End If

            For statIndex = 4 To 7
                statValue = Val(Trim(fields(statIndex)))
                statValid = Len(Trim(fields(statIndex))) > 0 And _
                    Len(Trim(fields(statIndex))) <= 3
                For charIndex = 1 To Len(Trim(fields(statIndex)))
                    If Mid(Trim(fields(statIndex)), charIndex, 1) < "0" Or _
                       Mid(Trim(fields(statIndex)), charIndex, 1) > "9" Then statValid = 0
                Next
                If statValid = 0 Or statValue < 0 Or statValue > 100 Then
                    invalidAttributes += 1
                End If
            Next

            proposalCount = 0
            proposalChanged = 0
            For statIndex = 10 To 13
                If Len(Trim(fields(statIndex))) > 0 Then
                    proposalCount += 1
                    statValue = Val(Trim(fields(statIndex)))
                    statValid = Len(Trim(fields(statIndex))) <= 3
                    For charIndex = 1 To Len(Trim(fields(statIndex)))
                        If Mid(Trim(fields(statIndex)), charIndex, 1) < "0" Or _
                           Mid(Trim(fields(statIndex)), charIndex, 1) > "9" Then statValid = 0
                    Next
                    If statValid = 0 Or statValue < 0 Or statValue > 100 Then
                        invalidAttributes += 1
                    End If
                    If Trim(fields(statIndex)) <> Trim(fields(statIndex - 6)) Then
                        proposalChanged = 1
                    End If
                End If
            Next
            If proposalCount <> 0 And proposalCount <> 4 Then schemaErrors += 1
            If proposalCount = 4 And proposalChanged <> 0 Then proposedUnapplied += 1

            If Len(currentName) > 0 And seenCount < MAX_ROWS Then
                seenCount += 1
                seenNames(seenCount) = currentName
                If Len(currentId) > 0 Then seenIds(seenCount) = currentId
            End If
        End If
    End If
Wend
Close #fileNum
If headerSeen = 0 Then schemaErrors += 1

errorsTotal = duplicateIds + invalidIds + duplicateNames + invalidNames + _
    invalidAttributes + invalidStates + csvErrors + schemaErrors

Print "SCHEMA=ecokin-registry-v1"
Print "ROWS=" & Trim(Str(rows))
Print "NUMBERED=" & Trim(Str(numbered))
Print "UNNUMBERED=" & Trim(Str(unnumbered))
Print "PENDING_REVIEW=" & Trim(Str(pendingReview))
Print "LEGACY_RETIRED=" & Trim(Str(legacyRetired))
Print "PROPOSED_UNAPPLIED=" & Trim(Str(proposedUnapplied))
Print "DUPLICATE_IDS=" & Trim(Str(duplicateIds))
Print "INVALID_IDS=" & Trim(Str(invalidIds))
Print "DUPLICATE_NAMES=" & Trim(Str(duplicateNames))
Print "INVALID_NAMES=" & Trim(Str(invalidNames))
Print "INVALID_ATTRIBUTES=" & Trim(Str(invalidAttributes))
Print "INVALID_STATES=" & Trim(Str(invalidStates))
Print "CSV_ERRORS=" & Trim(Str(csvErrors))
Print "SCHEMA_ERRORS=" & Trim(Str(schemaErrors))
Print "ERRORS=" & Trim(Str(errorsTotal))

If errorsTotal > 0 Then
    End 1
Else
    End 0
End If
