' Echohearts standalone art-status intake counter (FreeBASIC).
' Reads only a manually exported text snapshot. Does not change game assets.
' Does not grant/verify Eco-Kin art canon or UE5.8 runtime status.

Dim As String inputPath = Command(1)
If Len(Trim(inputPath)) = 0 Then
    Print "Usage: echo-art-status INPUT_TEXT_FILE"
    End 2
End If

Dim As Integer fileNum = FreeFile()
If Open(inputPath For Input As #fileNum) <> 0 Then
    Print "ERROR: cannot read art-status input"
    End 2
End If

Dim As Integer activeCount = 0
Dim As Integer pendingCount = 0
Dim As Integer retiredCount = 0
Dim As Integer invalidCount = 0
Dim As String statusLine

While Not Eof(fileNum)
    Line Input #fileNum, statusLine
    Select Case Trim(statusLine)
        Case "ACTIVE_CANON_ART"
            activeCount += 1
        Case "PENDING_CANON_REVIEW"
            pendingCount += 1
        Case "REFERENCE_RETIRED_NEEDS_REDESIGN"
            retiredCount += 1
        Case Else
            invalidCount += 1
    End Select
Wend
Close #fileNum

Print "ACTIVE_CANON_ART=" & Trim(Str(activeCount))
Print "PENDING_CANON_REVIEW=" & Trim(Str(pendingCount))
Print "REFERENCE_RETIRED_NEEDS_REDESIGN=" & Trim(Str(retiredCount))
Print "INVALID=" & Trim(Str(invalidCount))

If invalidCount > 0 Then
    End 1
Else
    End 0
End If
