; Based on code taken from http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
; 

!ifndef EstimatedSize_INCLUDED
!define EstimatedSize_INCLUDED

!include LogicLib.nsh

; Return on top of stack the total size of the selected (installed) sections, formated as DWORD
; Assumes no more than 256 sections are defined
Var GetInstalledSize.total
Function GetInstalledSize
    Push $0
    Push $1
    StrCpy $GetInstalledSize.total 0
    ${ForEach} $1 0 256 + 1
        ${if} ${SectionIsSelected} $1
            SectionGetSize $1 $0
            IntOp $GetInstalledSize.total $GetInstalledSize.total + $0
        ${Endif}
 
        ; Error flag is set when an out-of-bound section is referenced
        ${if} ${errors}
            ${break}
        ${Endif}
    ${Next}
    
    ClearErrors
    Pop $1
    Pop $0
    IntFmt $GetInstalledSize.total "0x%08X" $GetInstalledSize.total
    Push $GetInstalledSize.total
FunctionEnd

!endif # !EstimatedSize_INCLUDED