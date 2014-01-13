# ======================================================================
#   Copyright (C) 2010 Texas Instruments Incorporated
#
#   All rights reserved. Property of Texas Instruments Incorporated.
#   Restricted rights to use, duplicate or disclose this code are
#   granted through contract.
#
#   The program may not be used without the written permission
#   of Texas Instruments Incorporated or against the terms and conditions
#   stipulated in the agreement under which this program has been
#   supplied.
# ====================================================================
#
#  ----------------------------------------------------------------------------
#  Revision History
#
#
#  SEP xx,2002    REF=ORG			Swamy MC
#      Original version.
#  ----------------------------------------------------------------------------

#PROJROOT = $(ROOTDIR)/src

include $(PROJROOT)/make/start.mk

# Do not change above "include" line(s)

# Arguments to tools, will move to make system once finalized.

CFLAGS         = 
CDEFS          = DEBUG

EXEC_ARGS      = 
ST_LIB_ARGS    = 
SH_LIB_ARGS    = 

# Define this macro if target runs in kernel mode
#__KERNEL__ = 1

# Target name and extension
# static library        (ST_LIB): filename.a
# shared library soname (SH_LIB): filename.so.maj_ver.min_ver
# executable            (EXEC)  : filename.out

TARGETNAME  =


# TARGETTYPE must be EXEC, ST_LIB or SH_LIB in upper case.
#TARGETTYPE  = ST_LIB
TARGETTYPE  =

# For shared object library, soname is filename.so.maj_ver
SH_SONAME = 

# Folders in which gmake will run before building current target

SUBMODULES  = \
omx_sample \
omx_video_dec \
omx_h264_enc \
omx_h264svc_enc \
omx_mpeg4_enc \
 
#video/omx_proxy_component/test 

# Filename must not begin with '.', '/' or '\'

SOURCES     =

# Search path for include files

INCLUDES    =

# Libraries needed for linking.

ST_LIBS        =
SH_LIBS        =

# Search path for library (and linker command) files.
# Current folder and target folder are included by default.

LIBINCLUDES =


# Do not change below "include" line(s)

include $(PROJROOT)/make/build.mk

