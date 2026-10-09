' Read-only Eco-Kin registry audit for the ecokin-registry-v1 CSV contract.
' Proposals are reported, never applied; this is not a canon approval tool.

Function SafeStat(ByVal rawValue As String) As String
    Dim safeIndex As Integer
    rawValue = Trim(rawValue)
    If Len(rawValue) = 0 Then Return ""
    If Len(rawValue) > 3 Then Return "INVALID"
    For safeIndex = 1 To Len(rawValue)
        If Mid(rawValue, safeIndex, 1) < "0" Or _
           Mid(rawValue, safeIndex, 1) > "9" Then Return "INVALID"
    Next
    Return rawValue
End Function

Function IsUnsignedInteger(ByVal rawValue As String, ByVal maxLength As Integer) As Integer
    Dim checkIndex As Integer
    rawValue = Trim(rawValue)
    If Len(rawValue) = 0 Or Len(rawValue) > maxLength Then Return 0
    For checkIndex = 1 To Len(rawValue)
        If Mid(rawValue, checkIndex, 1) < "0" Or _
           Mid(rawValue, checkIndex, 1) > "9" Then Return 0
    Next
    Return 1
End Function

Function IsSignedInteger(ByVal rawValue As String) As Integer
    Dim checkIndex As Integer
    rawValue = Trim(rawValue)
    If Len(rawValue) = 0 Or Len(rawValue) > 4 Then Return 0
    For checkIndex = 1 To Len(rawValue)
        If checkIndex <> 1 Or Mid(rawValue, checkIndex, 1) <> "-" Then
            If Mid(rawValue, checkIndex, 1) < "0" Or _
               Mid(rawValue, checkIndex, 1) > "9" Then Return 0
        End If
    Next
    If rawValue = "-" Then Return 0
    Return 1
End Function

Function FoldAscii(ByVal rawValue As String) As String
    Dim foldIndex As Integer
    Dim characterCode As Integer
    Dim folded As String = ""
    For foldIndex = 1 To Len(rawValue)
        characterCode = Asc(Mid(rawValue, foldIndex, 1))
        If characterCode >= 97 And characterCode <= 122 Then characterCode -= 32
        folded &= Chr(characterCode)
    Next
    Return folded
End Function

Function ReferenceName(ByVal rawValue As String) As String
    Dim annotation As Integer
    rawValue = FoldAscii(Trim(rawValue))
    annotation = InStr(rawValue, " (")
    If annotation > 0 Then rawValue = Left(rawValue, annotation - 1)
    Return Trim(rawValue)
End Function

Const MAX_ROWS As Integer = 1000
Const MAX_FIELDS As Integer = 19

Dim inputPath As String = Command(1)
If Len(Trim(inputPath)) = 0 Then
    Print "Usage: ecokin-registry-stats INPUT.csv"
    End 2
End If

Dim fileNum As Integer = FreeFile()
On Error Resume Next
Open inputPath For Input As #fileNum
If Err <> 0 Then
    Print "ERROR: cannot read registry CSV"
    End 2
End If
On Error Goto 0

Dim As String fields(1 To MAX_FIELDS)
Dim As String seenNames(1 To MAX_ROWS)
Dim As String seenIds(1 To MAX_ROWS)
Dim As String seenMergeRefs(1 To MAX_ROWS)
Dim As String seenFormRefs(1 To MAX_ROWS)
Dim As String seenSourceRows(1 To MAX_ROWS)
Dim As String recordText, currentName, currentId, statusText, dispositionText
Dim As String sourceRowText, idStatusText, proposalGate, referenceText
Dim As Integer rows = 0, numbered = 0, unnumbered = 0
Dim As Integer pendingReview = 0, legacyRetired = 0, archiveOnly = 0
Dim As Integer proposedUnapplied = 0
Dim As Integer duplicateIds = 0, invalidIds = 0, duplicateNames = 0
Dim As Integer invalidNames = 0, invalidAttributes = 0, invalidStates = 0
Dim As Integer provenanceErrors = 0, proposalErrors = 0, referenceErrors = 0
Dim As Integer csvErrors = 0, schemaErrors = 0, errorsTotal = 0
Dim As Integer seenCount = 0, headerSeen = 0, quoteFound, malformed
Dim As Integer fieldIndex, fieldOffset, charIndex, fieldCount, rowIndex
Dim As Integer statIndex, proposalChanged, statValid
Dim As Integer searchIndex, referenceFound
Dim As Double statValue
Dim As Double originalTotal, proposedTotal, proposalDelta

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
           fields(7) <> "Purity" Or fields(8) <> "SourceRow" Or _
           fields(9) <> "ReviewFlags" Or fields(10) <> "SuggestedMergeTarget" Or _
           fields(11) <> "FormBaseCandidate" Or fields(12) <> "CanonicalDisposition" Or _
           fields(13) <> "IDStatus" Or fields(14) <> "ProposedVibrance" Or _
           fields(15) <> "ProposedDensity" Or fields(16) <> "ProposedHarmony" Or _
           fields(17) <> "ProposedPurity" Or fields(18) <> "ProposalDelta" Or _
           fields(19) <> "ProposalGate" Then
            schemaErrors += 1
        End If
        headerSeen = 1
    Else
        rows += 1
        If rows > MAX_ROWS Then
            schemaErrors += 1
        Else
            currentName = FoldAscii(Trim(fields(1)))
            currentId = Trim(fields(2))
            statusText = Trim(fields(3))
            sourceRowText = Trim(fields(8))
            dispositionText = Trim(fields(12))
            idStatusText = Trim(fields(13))
            proposalGate = Trim(fields(19))

            Print "ORIGINAL_STATS_" & Trim(Str(rows)) & "=" & _
                SafeStat(fields(4)) & "," & SafeStat(fields(5)) & "," & _
                SafeStat(fields(6)) & "," & SafeStat(fields(7))
            Print "PROPOSED_STATS_" & Trim(Str(rows)) & "=" & _
                SafeStat(fields(14)) & "," & SafeStat(fields(15)) & "," & _
                SafeStat(fields(16)) & "," & SafeStat(fields(17))

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
                Case Else
                    invalidStates += 1
            End Select
            Select Case dispositionText
                Case "INTAKE; CANON PROMOTION REQUIRED", "CORE SLOT; PENDING REVIEW", _
                     "CORE SLOT; EXISTING PERMANENT LABEL", "ARCHIVE; DO NOT SHIP", _
                     "INTAKE; RENAME REVIEW", "CORE SLOT; HISTORICAL STATUS CONFLICT", _
                     "LOCKED CORE IDENTITY; STAT TUNING PENDING"
                Case Else
                    invalidStates += 1
            End Select

            If statusText = "PENDING REVIEW" Then pendingReview += 1
            If statusText = "RETIRED" Or statusText = "LEGACY-HISTORICAL" Or _
               dispositionText = "ARCHIVE; DO NOT SHIP" Then legacyRetired += 1
            If statusText = "RETIRED" Or statusText = "LEGACY-HISTORICAL" Or _
               statusText = "MERGE-DUPLICATE" Or statusText = "RENAME REQUIRED" Then
                archiveOnly += 1
            End If
            If (statusText = "MERGE-DUPLICATE" Or statusText = "RENAME REQUIRED") And _
               Len(Trim(fields(9))) = 0 Then invalidStates += 1

            If Len(currentId) = 0 Then
                unnumbered += 1
                If idStatusText <> "NO PRODUCTION ID — DO NOT ASSIGN" Then invalidStates += 1
                If statusText = "PERMANENT DEX" Or statusText = "LOCKED CANON" Then
                    invalidStates += 1
                End If
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
                If idStatusText <> "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED" Then
                    invalidStates += 1
                End If
            End If

            statValid = IsUnsignedInteger(sourceRowText, 9)
            If statValid = 0 Or Val(sourceRowText) <= 0 Then
                provenanceErrors += 1
            Else
                For rowIndex = 1 To seenCount
                    If Val(sourceRowText) = Val(seenSourceRows(rowIndex)) Then
                        provenanceErrors += 1
                        Exit For
                    End If
                Next
            End If

            originalTotal = 0
            proposedTotal = 0
            For statIndex = 4 To 7
                statValue = Val(Trim(fields(statIndex)))
                statValid = IsUnsignedInteger(fields(statIndex), 3)
                If statValid = 0 Then
                    invalidAttributes += 1
                Else
                    originalTotal += statValue
                    If statValue > 100 Then invalidAttributes += 1
                End If
            Next

            proposalChanged = 0
            For statIndex = 14 To 17
                statValue = Val(Trim(fields(statIndex)))
                statValid = IsUnsignedInteger(fields(statIndex), 3)
                If statValid = 0 Then
                    invalidAttributes += 1
                Else
                    proposedTotal += statValue
                    If statValue > 100 Then invalidAttributes += 1
                End If
                If Val(fields(statIndex)) <> Val(fields(statIndex - 10)) Then
                    proposalChanged = 1
                End If
            Next
            If IsSignedInteger(fields(18)) = 0 Then
                proposalErrors += 1
            Else
                proposalDelta = Val(fields(18))
                Select Case proposalGate
                    Case "NO NUMERIC CHANGE PROPOSED"
                        If proposalChanged <> 0 Or proposalDelta <> 0 Then proposalErrors += 1
                    Case "DRAFT CAP SUGGESTION; NOT APPROVED"
                        proposedUnapplied += 1
                        If proposedTotal - originalTotal <> proposalDelta Or _
                           proposedTotal = originalTotal Then proposalErrors += 1
                    Case Else
                        proposalErrors += 1
                End Select
            End If

            If seenCount < MAX_ROWS Then
                seenCount += 1
                seenNames(seenCount) = currentName
                If Len(currentId) > 0 Then seenIds(seenCount) = currentId
                seenSourceRows(seenCount) = sourceRowText
                seenMergeRefs(seenCount) = Trim(fields(10))
                seenFormRefs(seenCount) = Trim(fields(11))
            End If
        End If
    End If
Wend
Close #fileNum
If headerSeen = 0 Then schemaErrors += 1

For rowIndex = 1 To seenCount
    For statIndex = 1 To 2
        If statIndex = 1 Then
            referenceText = ReferenceName(seenMergeRefs(rowIndex))
        Else
            referenceText = ReferenceName(seenFormRefs(rowIndex))
        End If
        If Len(referenceText) > 0 Then
            referenceFound = 0
            For searchIndex = 1 To seenCount
                If referenceText = seenNames(searchIndex) Then
                    referenceFound = 1
                    Exit For
                End If
            Next
            If referenceFound = 0 Then referenceErrors += 1
        End If
    Next
Next

errorsTotal = duplicateIds + invalidIds + duplicateNames + invalidNames + _
    invalidAttributes + invalidStates + provenanceErrors + proposalErrors + _
    referenceErrors + csvErrors + schemaErrors

Print "SCHEMA=ecokin-registry-v1"
Print "ROWS=" & Trim(Str(rows))
Print "NUMBERED=" & Trim(Str(numbered))
Print "UNNUMBERED=" & Trim(Str(unnumbered))
Print "PENDING_REVIEW=" & Trim(Str(pendingReview))
Print "LEGACY_RETIRED=" & Trim(Str(legacyRetired))
Print "ARCHIVE_ONLY=" & Trim(Str(archiveOnly))
Print "PROPOSED_UNAPPLIED=" & Trim(Str(proposedUnapplied))
Print "DUPLICATE_IDS=" & Trim(Str(duplicateIds))
Print "INVALID_IDS=" & Trim(Str(invalidIds))
Print "DUPLICATE_NAMES=" & Trim(Str(duplicateNames))
Print "INVALID_NAMES=" & Trim(Str(invalidNames))
Print "INVALID_ATTRIBUTES=" & Trim(Str(invalidAttributes))
Print "INVALID_STATES=" & Trim(Str(invalidStates))
Print "PROVENANCE_ERRORS=" & Trim(Str(provenanceErrors))
Print "PROPOSAL_ERRORS=" & Trim(Str(proposalErrors))
Print "REFERENCE_ERRORS=" & Trim(Str(referenceErrors))
Print "CSV_ERRORS=" & Trim(Str(csvErrors))
Print "SCHEMA_ERRORS=" & Trim(Str(schemaErrors))
Print "ERRORS=" & Trim(Str(errorsTotal))

If errorsTotal > 0 Then
    End 1
Else
    End 0
End If
