;************************************************************************
;*                                                                      *
;*      Copyright 2008 Concurrent Technologies, all rights reserved.    *
;*                                                                      *
;*      The program below is supplied by Concurrent Technologies        *
;*      on the understanding that no responsibility is assumed by       *
;*      Concurrent Technologies for any errors contained therein.       *
;*      Furthermore, Concurrent Technologies makes no commitment to     *
;*      update or keep current the program code, and reserves the       *
;*      right to change its specifications at any time.                 *
;*                                                                      *
;*      Concurrent Technologies assumes no responsibility either for    *
;*      the use of this code or for any infringements of the patent     *
;*      or other rights of third parties which may result from its use  *
;*                                                                      *
;************************************************************************

;
; BIT_CALL: assembler interface to BIT code
;

; $Header: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/bit_load/BIT_CALL.ASM,v 1.1 2013-09-04 07:57:07 chippisley Exp $
;
; $Log: BIT_CALL.ASM,v $
; Revision 1.1  2013-09-04 07:57:07  chippisley
; Import files into new source repository.
;
; Revision 1.1  2008/04/18 13:41:07  swilson
; Place under CVS control.
;
;
;


	.386p


_TEXT	segment byte public use16 'CODE'
_TEXT	ends


_TEXT	segment byte public use16 'CODE'

	ASSUME CS:_TEXT


;****************************************************************************
; _vCallFirstStage: call to BIT real-mode entry
;
; Input: [+6]  (UINT32) address of 1st-stage image
;        [+10] (UINT32) flags
;
; Output: none
;
; Modifies:
;

_vCallFirstStage PROC FAR
	PUBLIC _vCallFirstStage

	PUSH	BP
	MOV	BP, SP

	; Mask all possible interrupt sources

	CLI				; disable interrupts

	MOV	AL, 80h			; mask NMI
	OUT	70h, AL			;

	; Debugging ID

	MOV	AL, 0A0h		; debug code to Port-80
	OUT	80h, AL			;

	; Load calling registers

	MOV	EBX, [BP+6]		; EBX is physical address of image

	MOV	EDI, [BP+10]	; EDI contains flags

	; Put address of 1st-stage image on the stack

	MOV	EAX, EBX		; push destination segment
	SHR	EAX, 4			;
	AND	AX, 0F000h		; mask-out offset part
	PUSH	AX			;

	MOV	AX, BX			; push destination offset
	ADD	AX, 010h		; add offset to 1st-stage entry point
	PUSH	AX			;

	; Pass control to 1st-stage

	RET				; JUMP TO DESTINATION


_vCallFirstStage ENDP


_TEXT	ends

END
