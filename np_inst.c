/* Copyright (C) 2022 The uOFW team
   See the file COPYING for copying permission.
*/

#include <common_imp.h>
#include <iofilemgr_kernel.h>
#include <sysmem_sysclib.h>

SCE_MODULE_INFO("sceNpInstall_Driver", SCE_MODULE_KERNEL | SCE_MODULE_KIRK_SEMAPHORE_LIB | SCE_MODULE_ATTR_EXCLUSIVE_LOAD
    | SCE_MODULE_ATTR_EXCLUSIVE_START, 1, 1);
SCE_MODULE_BOOTSTART("sceNpInstallInit");
SCE_MODULE_STOP("sceNpInstallEnd");
SCE_SDK_VERSION(SDK_VERSION);

// Defined structs

// uOFW's chkreg pullrequest #91: https://github.com/uofw/uofw/blob/2125bffc12b7341e1911fb3fc8dbfc4cceff04f7/include/openpsid_kernel.h
typedef struct {
	/* Unknown. On retail set to 0. */
	u16 unk0; // 0
	/* Company code. Set to 1. */
	u16 companyCode; // 2
	/* Product code. */
	u16 productCode; // 4
	/* Product sub code. */
	u16 productSubCode; // 6
	/* Upper two bit of PsFlags. */
	u8 psFlagsMajor : 2; // 8
	/* Factory code. */
	u8 factoryCode : 6; // 8
	u8 uniqueIdMajor : 2; // 9
	/* Lower six bit of the PsFlags. Contain the QA flag, if set. */
	u8 psFlagsMinor : 6; // 9
	u8 uniqueIdMinor[6]; // 10
} SceConsoleId; // size = 16

// Global vars
u8 g_buf[0x10];           // 0x000009C0
u64 g_destTick;           // 0x000009D0
SceConsoleId g_consoleId; // 

// Function prototypes
s32 sceNpDrmDecActivation(u32 *, u8 *);
s32 sceNpDrmVerifyAct(u32 *data);
s32 sceOpenPSIDGetPSID(SceConsoleId *consoleID, u32);
s32 scePcactAuth2BB(u32 *, u32*, u8 *);
s32 scePcactAuth1BB();
s32 sceRtc_driver_89FA4262(u64 *destTick, const u64 *srcTick, u64 numSecs);
s32 sceRtc_driver_CEEF238F(u64 *tick);

// Subroutine module_start - Address 0x00000000
s32 sceNpInstallInit(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
    return 0;
}

// Subroutine module_stop - Address 0x00000008
s32 sceNpInstallEnd(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
    return 0;
}

// Subroutine sub_0000016C - Address 0x0000016C
s32 sub_0000016C(s32 arg0, u32 *arg1, u32 *arg2, u32 *arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return 0;
}

// Subroutine sub_00000374 - Address 0x00000374
s32 sub_00000374(u32 *data, u32 size, u32 *arg)
{
    (void)data;
    (void)size;
    (void)arg;
    return 0;
}

// Subroutine sub_000005AC - Address 0x000005AC
s32 sub_000005AC(void)
{
    return 0;
}

// Subroutine sub_000004C4 - Address 0x000004C4
s32 sub_000004C4(u32 *addr) {
    (void)addr;
    return 0;
}

// Subroutine sceNpInstall_driver_0B039B36 - Address 0x00000088 - Aliases: sceNpInstall_user_0B039B36
s32 sceNpInstall_driver_0B039B36(u32 *data, u32 size, u32 *arg) __attribute__((alias("sceNpInstallActivation")));
s32 sceNpInstallActivation(u32 *data, u32 size, u32 *arg)
{
    s32 ret = SCE_ERROR_PRIV_REQUIRED;
    s32 oldK1 = pspShiftK1();
    
    if ((pspK1DynBufOk(data, size)) && (pspK1StaBufOk(arg, 0x40)))
        ret = sub_00000374(data, size, arg);
    
    pspSetK1(oldK1);
    return ret;
}

// Subroutine sceNpInstall_driver_5847D8C7 - Address 0x00000010 - Aliases: sceNpInstall_user_5847D8C7
s32 sceNpInstall_driver_5847D8C7(s32 arg0, u32 *arg1, u32 *arg2, u32 *arg3) __attribute__((alias("sceNpInstallGetChallenge")));
s32 sceNpInstallGetChallenge(s32 arg0, u32 *arg1, u32 *arg2, u32 *arg3)
{
    s32 ret = SCE_ERROR_PRIV_REQUIRED;
    s32 oldk1 = pspShiftK1();
    
    if ((pspK1StaBufOk(arg1, 0x20)) && (pspK1StaBufOk(arg2, 0x80)) && (pspK1StaBufOk(arg3, 0x40)))
        ret = sub_0000016C(arg0, arg1, arg2, arg3);
        
    pspSetK1(oldk1);
    return ret;
}


// Subroutine sceNpInstall_driver_7AE4C8BC - Address 0x00000140 - Aliases: sceNpInstall_user_7AE4C8BC
s32 sceNpInstall_driver_7AE4C8BC(void) __attribute__((alias("sceNpInstallDeactivation")));
s32 sceNpInstallDeactivation(void)
{
    s32 oldk1 = pspShiftK1();
    sub_000005AC();
    pspSetK1(oldk1);
    return 0;
}

// Subroutine sceNpInstall_driver_91F9D50D - Address 0x000000F4 - Aliases: sceNpInstall_user_91F9D50D
s32 sceNpInstall_driver_91F9D50D(u32 *addr) __attribute__((alias("sceNpInstallCheckActivation")));
s32 sceNpInstallCheckActivation(u32 *addr)
{
    s32 ret = SCE_ERROR_PRIV_REQUIRED;
    s32 oldk1 = pspShiftK1();
    
    if (pspK1StaBufOk(addr, 0x8))
        ret = sub_000004C4(addr);
    
    pspSetK1(oldk1);
    return ret;
}
