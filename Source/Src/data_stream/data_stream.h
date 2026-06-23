 /*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: data_stream.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __DATA_STREAM_H__
#define __DATA_STREAM_H__

void DATAST_Send_Kb_Break(U8_T currentHost,U8_T devinx, U8_T endpidx, U8_T reportId, U8_T reportLen);
U8_T DATAST_Move_Intr_Buf_Move(U8_T intt_id,U8_T *transmit_buf,U8_T currenthost);
#endif /* End of __DATA_STREAM_H__ */

/* End of data_stream.h */