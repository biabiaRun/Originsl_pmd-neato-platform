; Copyright (C) 2018 pmdtechnologies ag
;
; THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
; KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
; PARTICULAR PURPOSE.
; 

!ifndef DumpLog_INCLUDED
!define DumpLog_INCLUDED

!ifndef LVM_GETITEMCOUNT
!define LVM_GETITEMCOUNT 0x1004
!endif
!ifndef LVM_GETITEMTEXTA
!define LVM_GETITEMTEXTA 0x102D
!endif

Function DumpLog # Written by KiCHiK
  Exch $5
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $6

  FindWindow $0 "#32770" "" $HWNDPARENT
  GetDlgItem $0 $0 1016
  StrCmp $0 0 error
  FileOpen $5 $5 "w"
  StrCmp $5 0 error
    SendMessage $0 ${LVM_GETITEMCOUNT} 0 0 $6
    System::StrAlloc ${NSIS_MAX_STRLEN}
    Pop $3
    StrCpy $2 0
    System::Call "*(i, i, i, i, i, i, i, i, i) p \
      (0, 0, 0, 0, 0, r3, ${NSIS_MAX_STRLEN}) .r1"
    loop: StrCmp $2 $6 done
      System::Call "User32::SendMessageA(p, i, p, p) i \
        ($0, ${LVM_GETITEMTEXTA}, $2, r1)"
      System::Call "*$3(&t${NSIS_MAX_STRLEN} .r4)"
      FileWrite $5 "$4$\r$\n"
      IntOp $2 $2 + 1
      Goto loop
    done:
      FileClose $5
      System::Free $1
      System::Free $3
      Goto exit
  error:
    MessageBox MB_OK error
  exit:
    Pop $6
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
    Exch $5
FunctionEnd

!endif # !DumpLog_INCLUDED
