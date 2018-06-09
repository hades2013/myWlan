#ifndef _FTM_BT_PERSIST_H_
#define _FTM_BT_PERSIST_H_

/*==========================================================================

                     BT persist NV items access source file

Description
  Read/Write APIs for retreiving NV items from persist memory.

# Copyright (c) 2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/27/11   rrr     Moved persist related API for c/c++ compatibility, needed
                   for random BD address to be persistent across target
                   reboots.
==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "ftm_bt_common.h"
#include <string.h>

#ifdef BT_NV_SUPPORT
#define FTM_BT_CMD_NV_READ 0xB
#define FTM_BT_CMD_NV_WRITE 0xC
#endif

/*===========================================================================
FUNCTION   ftm_bt_send_nv_read_cmd

DESCRIPTION
 Helper Routine to process the nv read command

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  FALSE = failure, else TRUE

SIDE EFFECTS
  None

===========================================================================*/
boolean ftm_bt_send_nv_read_cmd
(
  uint8 * cmd_buf,   /* pointer to Cmd */
  uint16 cmd_len     /* Cmd length */
);

/*===========================================================================
FUNCTION   ftm_bt_send_nv_write_cmd

DESCRIPTION
 Helper Routine to process the nv write command

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  FALSE = failure, else TRUE

SIDE EFFECTS
  None

===========================================================================*/
boolean ftm_bt_send_nv_write_cmd
(
  uint8 * cmd_buf,   /* pointer to Cmd */
  uint16 cmd_len     /* Cmd length */
);

#ifdef __cplusplus
}
#endif

#endif /* _FTM_BT_PERSIST_H_ */

