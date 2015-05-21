;************************************************************************
;*                                                                      *
;*      Copyright 1999 Concurrent Technologies, all rights reserved.    *
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
; BIGREALA: assembler implementation of Big Real mode.
;

; $Header: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/bit_load/BIGREALA.ASM,v 1.1 2013-09-04 07:57:07 chippisley Exp $
;
; $Log: BIGREALA.ASM,v $
; Revision 1.1  2013-09-04 07:57:07  chippisley
; Import files into new source repository.
;
; Revision 1.1  2008/04/18 13:41:07  swilson
; Place under CVS control.
;
; Revision 0.1  1999/11/12 11:17:52  stevew
; First RCS checkin V0.01-03
;
;
;


	.386p


_TEXT	segment byte public use16 'CODE'
_TEXT	ends


_TEXT	segment byte public use16 'CODE'

; GDT and pointer for cache flush
;

gdt_ptr     DW  16          ; 16 byte limit
gdt_base	DW  OFFSET gdt  ; lower 16-bits of base
gdt_upper	DW  00000h      ; upper 16-bits of base

gdt         DD  000000000h  ; NULL descriptor
			DD  000000000h  ;
			DD  00000FFFFh  ; 4GB (base 0) RW data
			DD  0009F9200h  ;


	ASSUME CS:_TEXT


;****************************************************************************
; _bGoBigReal: switch CPU to Big Real mode
;
; Interrupts are disabled during this function
;
; Input: none
;
; Output: AL - 0 if Ok or an error code
;
; Modifies : EAX, EBX
;

_bGoBigReal PROC FAR
	PUBLIC _bGoBigReal

	pushf						; don't want interrupts here
	cli							;

	push	bp   	       		; 'C' stack frame
	mov		bp, sp				;


	PUSH	DS					; save data segs
	PUSH	ES					;
	PUSH	FS  		        ;
	PUSH	GS					;

	xor		eax, eax			; get linear address of GDT structure
	mov		ax, cs				; messy, yes :-)
	shl		eax, 4				;
	xor		ebx, ebx            ;
	mov		bx, OFFSET cs:gdt   ;
	add		eax, ebx            ;

	mov		cs:[gdt_base], ax	; save GDT pointer
	shr		eax, 16				;
	mov		cs:[gdt_upper], ax	;


	MOV     EAX, CR0            ; enable protection
	OR      EAX, 000000001h     ;
	MOV     CR0, EAX            ;

	JMP     QflushP           	; switch to PROTECTED mode
QflushP:

	DB      066h                    ; force 32-bit load
	LGDT    FWORD PTR cs:[gdt_ptr]  ; point to ROM based GDT

	MOV		AX, 08h				; load segment descriptor with
	MOV		DS, AX				; 4GByte limit
	MOV		ES, AX              ;
	MOV		FS, AX              ;
	MOV		GS, AX              ;


	MOV     EAX, CR0            ; disable protection
	AND     EAX, 0FFFFFFFEh     ;
	MOV     CR0, EAX            ;

	JMP     SHORT QflushR       ; Return to REAL mode
QflushR:

	POP		GS                  ; restore original data segs.
	POP		FS                  ;
	POP		ES                  ;
	POP		DS                  ;

	in		al, 92h				; enable A20
	or		al, 02h             ;
	out		92h, al				;

	xor		al, al				; return Ok

	pop		bp					; restore stack frame
	popf						; restore interrupt status

	RET

_bGoBigReal ENDP



;****************************************************************************
; _dReadDword32: read the DWORD at a 32-bit address
;
; Input: [+6] 32-bit linear address
;
; Output: DX:AX the DWORD value
;
; Modifies : EAX, EBX, EDX
;

_dReadDword32 PROC FAR
	PUBLIC _dReadDword32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6]			; EBX is 32-bit offset
	MOV		EAX, DS:[EBX]		; read DWORD

	MOV		EDX, EAX			; get result in DX:AX for return
	SHR		EDX, 16				;

	POP		DS

	POP		BP
	RET

_dReadDword32 ENDP



;****************************************************************************
; _wReadWord32: read the WORD at a 32-bit address
;
; Input: [+6] 32-bit linear address
;
; Output: AX the WORD value
;
; Modifies : AX, EBX
;

_wReadWord32 PROC FAR
	PUBLIC _wReadWord32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6]			; EBX is 32-bit offset
	MOV		AX, DS:[EBX]		; read WORD

	POP		DS

	POP		BP
	RET

_wReadWord32 ENDP



;****************************************************************************
; _bReadByte32: read the BYTE at a 32-bit address
;
; Input: [+6] 32-bit linear address
;
; Output: AL the BYTE value
;
; Modifies : AX, EBX
;

_bReadByte32 PROC FAR
	PUBLIC _bReadByte32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6]			; EBX is 32-bit offset
	MOV		AL, DS:[EBX]		; read BYTE

	POP		DS

	POP		BP
	RET

_bReadByte32 ENDP



;****************************************************************************
; _vWriteDword32: write a DWORD to a 32-bit address
;
; Input: [+6]  32-bit linear address
;		 [+10] DWORD data
;
; Output: none
;
; Modifies : EAX, EBX
;

_vWriteDword32 PROC FAR
	PUBLIC _vWriteDword32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6] 		; 32-bit offset
	MOV		EAX, [BP+10]		; data
	MOV		DS:[EBX], EAX		; write the data

	POP		DS

	POP		BP
	RET

_vWriteDword32 ENDP



;****************************************************************************
; _vWriteWord32: write a WORD to a 32-bit address
;
; Input: [+6]  32-bit linear address
;		 [+10] WORD data
;
; Output: none
;
; Modifies : AX, EBX
;

_vWriteWord32 PROC FAR
	PUBLIC _vWriteWord32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6] 		; 32-bit offset
	MOV		AX, [BP+10]			; data
	MOV		DS:[EBX], AX		; write the data

	POP		DS

	POP		BP
	RET

_vWriteWord32 ENDP



;****************************************************************************
; _vWriteByte32: write a BYTE to a 32-bit address
;
; Input: [+6]  32-bit linear address
;		 [+10] BYTE data
;
; Output: none
;
; Modifies : AX, EBX
;

_vWriteByte32 PROC FAR
	PUBLIC _vWriteByte32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;
	MOV		EBX, [BP+6] 		; 32-bit offset
	MOV		AL, [BP+10]			; data
	MOV		DS:[EBX], AL		; write the data

	POP		DS

	POP		BP
	RET

_vWriteByte32 ENDP



;****************************************************************************
; _vPatternDword32: write a pattern to a 32-bit address
;
; Input: [+6]  32-bit linear address
;		 [+10] DWORD data
;
; Output: none
;
; Modifies : EAX, EBX
;

_vPatternDword32 PROC FAR
	PUBLIC _vWriteDword32

	PUSH	BP
	MOV		BP, SP

	PUSH	ES
	PUSH	ESI

	XOR		AX, AX				; load ES with 0 base
	MOV		ES, AX				;
	MOV		ESI, [BP+6] 		; 32-bit address
	MOV		ECX, [BP+10]		; loop size


	XOR		EDX, EDX			;
	SHR		ECX, 2				; divide by 4

_loop:
	MOV		EAX, EDX			;
	NOT		EAX					;

	ADD		EDX, 4				;

	DB 067h						; 32-bit addressing
	STOSD						; store 32-bit quantity

	DB 066h						; 32-bit count
	LOOP	_loop				;

	POP		ESI
	POP		ES

	POP		BP
	RET

_vPatternDword32 ENDP



;****************************************************************************
; _vCopyDword32: copy a data block in DWORDs
;
; Input: [+6]  32-bit linear source address
;		 [+10] 32-bit linear destination address
;		 [+14] 32-bit count of DWORDs
;
; Output: none
;
; Modifies : AX, ECX
;

_vCopyDword32 PROC FAR
	PUBLIC _vCopyDword32

	PUSH	BP
	MOV		BP, SP

	PUSHF
	CLI

	PUSH	DS
	PUSH	ES
	PUSH	ESI
	PUSH	EDI

	XOR		AX, AX				; load DS and ES with 0 base
	MOV		DS, AX				;
	MOV		ES, AX				;
	MOV		ESI, [BP+6]			; source 32-bit offset
	MOV		EDI, [BP+10]		; destination 32-bit offset
	MOV		ECX, [BP+14]		; DWORD transfer count

	DB 067h						; 32-bit addressing
	DB 066h						; 32-bit count
	REP		MOVSD				; move 32-bit quantity

	POP		EDI
	POP		ESI
	POP		ES
	POP		DS

	POPF

	POP		BP
	RET

_vCopyDword32 ENDP



;****************************************************************************
; _vSetDword32: set a region of memory to a fixed DWORD value
;
; Input: [+6]  32-bit linear source address
;		 [+10] DWORD set value
;		 [+14] 32-bit count of DWORDs
;
; Output: none
;
; Modifies : AX, ECX
;

_vSetDword32 PROC FAR
	PUBLIC _vSetDword32

	PUSH	BP
	MOV		BP, SP

	PUSH	ES
	PUSH	EDI

	XOR		AX, AX				; load ES with 0 base
	MOV		ES, AX				;
	MOV		EDI, [BP+6]			; desination 32-bit offset
	MOV		EAX, [BP+10]		; 32-bit set data
	MOV		ECX, [BP+14]		; DWORD set count

	DB 067h						; 32-bit addressing
	DB 066h						; 32-bit count
	REP		STOSD				; store 32-bit quantity

	POP		EDI
	POP		ES

	POP		BP
	RET

_vSetDword32 ENDP



;****************************************************************************
; _dInDword: read from a 32-bit I/O port
;
; Input: [+6] the 16-bit port address
;
; Output: DX:AX 32-bit port data
;
; Modifies : EAX, EDX
;

_dInDword PROC FAR
	PUBLIC _dInDword

	PUSH	BP
	MOV		BP, SP

	MOV		DX, [BP+6]
	IN		EAX, DX

	MOV		EDX, EAX
	SHR		EDX, 16

	POP		BP
	RET

_dInDword ENDP



;****************************************************************************
; _vOutDword: write to a 32-bit I/O port
;
; Input: [+6] the 16-bit port address
;		 [+8] the 32-bit data
;
; Output: none
;
; Modifies : EAX, DX
;

_vOutDword PROC FAR
	PUBLIC _vOutDword

	PUSH	BP
	MOV		BP, SP

	MOV		DX, [BP+6]
	MOV		EAX, [BP+8]
	OUT		DX, EAX

	POP		BP
	RET

_vOutDword ENDP



;****************************************************************************
; _wInWord: read from a 16-bit I/O port
;
; Input: [+6] the 16-bit port address
;
; Output: AX the 16-bit data
;
; Modifies : AX, DX
;

_wInWord PROC FAR
	PUBLIC _wInWord

	PUSH	BP
	MOV		BP, SP

	MOV		DX, [BP+6]
	IN		AX, DX

	POP		BP
	RET

_wInWord ENDP



;****************************************************************************
; _vOutWord: write to a 16-bit I/O port
;
; Input: [+6] the 16-bit port address
;		 [+8] the 16-bit data
;
; Output: none
;
; Modifies : AX, DX
;

_vOutWord PROC FAR
	PUBLIC _vOutWord

	PUSH	BP
	MOV		BP, SP

	MOV		DX, [BP+6]
	MOV		AX, [BP+8]
	OUT		DX, AX

	POP		BP
	RET

_vOutWord ENDP



;****************************************************************************
; _vRmwSet32: perform read-modify-write to set bits at a 32-bit address
;
; Input: [+6] the 32-bit address
;		 [+10] the 32-bit SET mask
;
; Output: none
;
; Modifies : EAX, EBX
;

_vRmwSet32 PROC FAR
	PUBLIC _vRmwSet32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;

	MOV		EBX, [BP+6]			; 32-bit offset
	MOV		EAX, [BP+10]		; data pattern; bit=1 for set
	LOCK						; RMW cycle to set bits
	OR		DS:[EBX], EAX		;

	POP		DS

	POP		BP
	RET

_vRmwSet32 ENDP



;****************************************************************************
; _vRmwClear32: perform read-modify-write to clear bits at a 32-bit address
;
; Input: [+6] the 32-bit address
;		 [+10] the 32-bit CLEAR mask
;
; Output: none
;
; Modifies : EAX, EBX
;

_vRmwClear32 PROC FAR
	PUBLIC _vRmwSet32

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	XOR		AX, AX				; load DS with 0 base
	MOV		DS, AX				;

	MOV		EBX, [BP+6]			; 32-bit offset
	MOV		EAX, [BP+10]		; data pattern; bit=0 for clear
	LOCK						; RMW cycle to clear bits
	AND		DS:[EBX], EAX		;

	POP		DS

	POP		BP
	RET

_vRmwClear32 ENDP


;****************************************************************************
; _vReadMsr: read an MSR register
;
; Input: [+6]  32-bit register number
;        [+10] far pointer to MSBs
;        [+14] far pointer to LSBs
;
; Output: none
;
; Modifies : EAX, BX, ECX, EDX
;

_vReadMsr PROC FAR
	PUBLIC _vReadMsr

	PUSH	BP
	MOV		BP, SP

	PUSH	DS

	MOV		ECX, [BP+6]			; ECX is MSR register number
	DB		0Fh, 32h			; RDMSR (result in EDX:EAX)

	LDS		BX, [BP+10]			; DS:BX is pointer for MSBs
	MOV		[BX], EDX
	LDS		BX, [BP+14]			; DS:BX is pointer for LSBs
	MOV		[BX], EAX

	POP		DS

	POP		BP
	RET

_vReadMsr ENDP


;****************************************************************************
; _vWriteMsr: write to an MSR register
;
; Input: [+6]  32-bit register number
;        [+10] 32-bit MSBs
;        [+14] 32-bit LSBs
;
; Output: none
;
; Modifies : EAX, ECX, EDX
;

_vWriteMsr PROC FAR
	PUBLIC _vWriteMsr

	PUSH	BP
	MOV		BP, SP

	MOV		EAX, CR0			;
	OR		EAX, 60000000h		; disable caching
	MOV		CR0, EAX			;

	DB		0Fh, 09h			; WBINVD


	MOV		ECX, [BP+6]			; ECX is MSR register number
	MOV		EDX, [BP+10]		; EAX contains MSBs
	MOV		EAX, [BP+14]		; EDX contains LSBs

	DB		0Fh, 30h			; WRMSR (value in EDX:EAX)

	MOV		EAX, CR0			;
	AND		EAX, 9FFFFFFFh		; enable caching
	MOV		CR0, EAX			;

	POP		BP
	RET

_vWriteMsr ENDP


;****************************************************************************
; _bGetCacheState: report CR0 cache state
;
; Input: none
;
; Output: 3 = disabled, 0 = enabled
;
; Modifies : EAX
;

_bGetCacheState PROC FAR
	PUBLIC _bGetCacheState

	MOV		EAX, CR0
	AND		EAX, 60000000h
	SHR		EAX, 29

	RET

_bGetCacheState ENDP


;****************************************************************************
; _vCacheEnable: enable cache via CR0
;
; Input: none
;
; Output: none
;
; Modifies : EAX
;

_vEnableCache PROC FAR
	PUBLIC _vEnableCache

	MOV		EAX, CR0
	AND		EAX, NOT 60000000h
	MOV		CR0, EAX

	RET

_vEnableCache ENDP


;****************************************************************************
; _vDisableCache: disable cache via CR0
;
; Input: none
;
; Output: none
;
; Modifies : EAX
;

_vDisableCache PROC FAR
	PUBLIC _vDisableCache

	DB		0Fh, 09h

	MOV		EAX, CR0
	OR		EAX, 60000000h
	MOV		CR0, EAX

	RET

_vDisableCache ENDP


;****************************************************************************
; _dCacheState: get CR0 cache bits
;
; Input: none
;
; Output: DX:AX = CR0 cache bits
;
; Modifies : EAX, EDX
;

_dCacheState PROC FAR
	PUBLIC _dCacheState

	MOV		EAX, CR0
	AND		EAX, 60000000h
	MOV		EDX, EAX
	SHR		EDX, 16

	RET

_dCacheState ENDP


;****************************************************************************
; _vInitMemR:
;
; Input: [+6]  32-bit start address
;        [+10] 32-bit length
;
; Output: none
;
; Modifies: AX, ECX
;

_vInitMemR PROC FAR
	PUBLIC _vInitMemR

	PUSH	DS
	PUSH	ESI

	MOV		AX, 0
	MOV		DS, AX

	MOV		ESI, 000400000h
	MOV		ECX, 00FC00000h		; 1-GB - 4-MB bytes
	SHR		ECX, 2				; convert to DWORDs

	CLI
	DB		067h				; 32-bit addressing
	REP		LODSD				;
	STI

	POP		ESI
	POP		DS
	RET

_vInitMemR ENDP


;****************************************************************************
; _vInitMemW:
;
; Input: [+6]  32-bit start address
;        [+10] 32-bit length
;
; Output: none
;
; Modifies :
;

_vInitMemW PROC FAR
	PUBLIC _vInitMemW

	PUSH	DS
	PUSH	ESI

	MOV		AX, 0
	MOV		ES, AX

	MOV		EDI, 000400000h
	MOV		ECX, 00FC00000h		; 1-GB - 4-MB bytes
	SHR		ECX, 2				; convert to DWORDs

	CLI
	DB		067h				; 32-bit addressing
	REP		STOSD				;
	STI

	POP		ESI
	POP		DS
	RET

_vInitMemW ENDP


;****************************************************************************
; _vMemRead:
;
; Input: none
;
; Output: none
;
; Modifies: AX, ECX
;

_vMemRead PROC FAR
	PUBLIC _vMemRead

	PUSH	BP
	MOV		BP, SP

	PUSH	DS
	PUSH	ESI

	MOV		AX, 0
	MOV		DS, AX

	MOV		ESI, [BP+6]			; start address
	MOV		ECX, [BP+10]		; length
	SHR		ECX, 2				; convert to DWORDs

	CLI
	DB		067h				; 32-bit addressing
	REP		LODSD				;
	STI

	POP		ESI
	POP		DS

	POP		BP
	RET

_vMemRead ENDP


;****************************************************************************
; _vMemWrite:
;
; Input: none
;
; Output: none
;
; Modifies :
;

_vMemWrite PROC FAR
	PUBLIC _vMemWrite

	PUSH	BP
	MOV		BP, SP

	PUSH	DS
	PUSH	ESI

	MOV		AX, 0
	MOV		ES, AX

	MOV		EDI, [BP+6]			; start address
	MOV		ECX, [BP+10]		; length
	SHR		ECX, 2				; convert to DWORDs

	CLI
	DB		067h				; 32-bit addressing
	REP		STOSD				;
	STI

	POP		ESI
	POP		DS

	POP		BP
	RET

_vMemWrite ENDP


_TEXT	ends

END
