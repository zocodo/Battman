.section __TEXT,__text,regular,pure_instructions
.globl _regAppleSMCNotification
.p2align 2

; x0: cb
_regAppleSMCNotification:
	sub sp,sp,#48
	stp x29,x30,[sp,#32]
	stp x20,x21,[sp,#16]
	mov x20,x0
	adrp x0,AppleSMCStr@PAGE
	add x0,x0,AppleSMCStr@PAGEOFF
	bl _IOServiceMatching
	mov x1,x0
	mov x0,xzr
	bl _IOServiceGetMatchingService
	mov x21,x0
	mov x0,xzr
	bl _IONotificationPortCreate
	str x0,[sp]
	mov x0,xzr
	mov x1,xzr
	bl _dispatch_get_global_queue
	mov x1,x0
	ldr x0,[sp]
	bl _IONotificationPortSetDispatchQueue
	ldr x0,[sp]
	mov x1,x21
	adrp x2,generalInt.str@PAGE
	add x2,x2,generalInt.str@PAGEOFF
	mov x3,x20
	mov x4,xzr
	mov x5,sp
	bl _IOServiceAddInterestNotification
	mov x0,x21
	bl _IOObjectRelease
	ldp x20,x21,[sp,#16]
	ldp x29,x30,[sp,#32]
	ldr x0,[sp]
	add sp,sp,#48
	ret

.section __TEXT,__cstring,cstring_literals
AppleSMCStr:
.asciz "AppleSMC"
generalInt.str:
.asciz "IOGeneralInterest"
