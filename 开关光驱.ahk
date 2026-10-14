; 光驱开关工具 - 最终版
; 使用 \\.\CdRom0 设备路径，仅请求读权限，不触发 UAC

Gui, -MaximizeBox -MinimizeBox
Gui, Add, Button, x25 y18 w90 h30 gEjectDrive, 弹出光驱
Gui, Add, Button, x25 y63 w90 h30 gCloseDrive, 关闭光驱
Gui, Show, w140 h110, 光驱控制
Return

EjectDrive:
    EjectCdRom()
Return

CloseDrive:
    CloseCdRom()
Return

GuiClose:
ExitApp

; --- 弹出光驱 ---
EjectCdRom() {
    IOCTL_STORAGE_EJECT_MEDIA := 0x2D4808
    hDevice := DllCall("CreateFile"
        , "Str", "\\.\CdRom0"
        , "UInt", 0x80000000   ; 仅 GENERIC_READ
        , "UInt", 3            ; FILE_SHARE_READ | FILE_SHARE_WRITE
        , "Ptr", 0
        , "UInt", 3            ; OPEN_EXISTING
        , "UInt", 0
        , "Ptr", 0, "Ptr")
    if (hDevice = -1)
        return false
    result := DllCall("DeviceIoControl"
        , "Ptr", hDevice
        , "UInt", IOCTL_STORAGE_EJECT_MEDIA
        , "Ptr", 0, "UInt", 0
        , "Ptr", 0, "UInt", 0
        , "UInt*", 0
        , "Ptr", 0)
    DllCall("CloseHandle", "Ptr", hDevice)
    return result
}

; --- 关闭光驱 ---
CloseCdRom() {
    IOCTL_STORAGE_LOAD_MEDIA := 0x2D480C
    hDevice := DllCall("CreateFile"
        , "Str", "\\.\CdRom0"
        , "UInt", 0x80000000   ; 仅 GENERIC_READ
        , "UInt", 3
        , "Ptr", 0
        , "UInt", 3
        , "UInt", 0
        , "Ptr", 0, "Ptr")
    if (hDevice = -1)
        return false
    result := DllCall("DeviceIoControl"
        , "Ptr", hDevice
        , "UInt", IOCTL_STORAGE_LOAD_MEDIA
        , "Ptr", 0, "UInt", 0
        , "Ptr", 0, "UInt", 0
        , "UInt*", 0
        , "Ptr", 0)
    DllCall("CloseHandle", "Ptr", hDevice)
    return result
}