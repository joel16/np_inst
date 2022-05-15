/* Copyright (C) 2022 The uOFW team
   See the file COPYING for copying permission.
*/

#include <common_imp.h>
#include <iofilemgr_kernel.h>
#include <sysmem_sysclib.h>
#include <threadman_kernel.h>

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
} SceConsoleId; // size = 0x10

// act.dat structure based on https://wiki.henkaku.xyz/vita/Act.dat
typedef struct {
    u32 activationType; 
    u32 version;
    u32 accountID[2];
} __attribute__((packed)) AccountInfo; // size = 0x10

typedef struct {
    u8 unk1[0x40];
    SceConsoleId consoleId;
    u8 unk3[0x10];
    u8 unk4[0x10];
} __attribute__((packed)) AccountDataEnc; // size = 0x70

typedef struct {
    u8 rsaSignature[0x100];
    u8 unkSignature[0x40];
    u8 ecdsaSignature[0x28];
} __attribute__((packed)) AccountDataSig; // size = 0x168

typedef struct {
    AccountInfo info;                  // 0
    u8 primaryKeyTable[0x800];         // 0x10
    AccountDataEnc encData;            // 0x810
    u8 secondaryTable[0x650];          // 0x880
    AccountDataSig sigInfo;            // 0xED0
} __attribute__((packed)) AccountData; // 0x1038

// Global vars
AccountInfo *g_info;      // 0x000009C0
u64 g_destTick;           // 0x000009D0
SceConsoleId g_consoleId; //
SceUID g_fd;              // 0x000009D8

// Function prototypes
s32 sceKernelTerminateThread(SceUID thid);
s32 sceNpDrmDecActivation(u32 *, AccountInfo *);
s32 sceNpDrmVerifyAct(u32 *);
s32 sceOpenPSIDGetPSID(SceConsoleId *consoleID, u32);
s32 scePcactAuth1BB(u32, SceConsoleId, void *, void *, u32, u32);
s32 scePcactAuth2BB(u32 *, u32*, AccountInfo *);
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
s32 sub_00000374(u32 *data, u32 size, u32 *arg2)
{
    s32 ret = 0;
    
    if ((!data) || (size != 0x1090) || (!arg2))
        return 0x80550980;
        
    if (((ret = scePcactAuth2BB(data, arg2, g_info)) >= 0) && ((ret = sceNpDrmDecActivation(data + 0x50, g_info)) >= 0)
        && ((ret = sceNpDrmVerifyAct(data + 0x50)) >= 0)) {
            SceUID fd = sceIoOpen("flash2:/act.dat", 0x04000602, 0x1B6);
            
            if (fd >= 0) {
                ret = sceIoWrite(fd, data + 0x50, 0x1038);
                
                if (ret != 0x1038) {
                    if (ret >= 0)
                        ret = 0x80550981; // Read some bytes but incorrect data
                }
            }
            
            sceIoClose(fd);
    }
    else
        sceIoRemove("flash2:/act.dat");
        
    memset(data + 0x50, 0, 0x1040);
    memset(arg2, 0, 0x40);
    memset(g_info, 0, 0x10);
    return ret;
}

s32 removeActivation(SceSize args __attribute__((unused)), void *argp __attribute__((unused))) {
    g_fd = sceIoRemove("flash2:/act.dat");
    return 0;
}

// Subroutine sub_000005AC - Address 0x000005AC
s32 sub_000005AC(void)
{
    SceUID thread = sceKernelCreateThread("SceNpDeactivation", removeActivation, 0x20, 0x4000, 0, NULL);
    
    if (thread >= 0) {
        g_fd = 0;
        s32 status = sceKernelStartThread(thread, 0, NULL);
        
        if (status >= 0) {
            SceUInt timeout = 0;
            status = sceKernelWaitThreadEnd(thread, &timeout);
            
            if (status >= 0)
                status = g_fd; // if sceKernelWaitThreadEnd succeeds set status to g_fd and return it
        }
        
        sceKernelTerminateThread(thread);
        sceKernelDeleteThread(thread);
        return status;
    }
    
    return thread;
}

// Subroutine sub_000004C4 - Address 0x000004C4
s32 sub_000004C4(u32 *addr) {
    s32 ret = 0x80550980;
    AccountData act;
    
    if (!addr)
        return ret;
        
    SceUID fd = sceIoOpen("flash2:/act.dat", 0x04000001, 0);
    if (fd >= 0) {
        // Return bytes read (16) if success
        ret = sceIoRead(fd, &act, 0x10);
        
        if (ret != 0x10) {
            if (ret >= 0)
                ret = 0x80550981; // Read some bytes but incorrect data
            else
                ret = 0x80550982; // Did not read any bytes
        }
        else {
            addr[0] = act.info.accountID[0];
            addr[1] = act.info.accountID[1];
        }
        
        sceIoClose(fd);
    }
    
    return ret;
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
