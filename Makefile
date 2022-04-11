# Copyright (C) 2022 The uOFW team
# See the file COPYING for copying permission.

TARGET = np_inst
OBJS = np_inst.o

LIBS = -lThreadManForKernel -lSysclibForKernel -lIoFileMgrForKernel -lscePspNpDrm_driver -lscePcact_driver \
       -lsceRtc_driver -lsceOpenPSID_driver

include ../../lib/build.mak
