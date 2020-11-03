#Testing:
#//Featurers from test2.s:
# .global and .extern directives
# .skip, .byte, .word directives
# .end directive
# labels
# sections
# section header table
# symbol table 
# user defined sections
# referening symbols -->relocation tables(only R_16 type - absolute)
# backpatching
#//New features:
#instructions
#relocation tables(+ R_16_PC type - pc relative)
#equ directive && tns table


.global a,a1
.extern b

.section .data
    .skip 3,0x7
 a: .byte 8, 0xF4, -53
	.skip 5
 a1: .word 0x1fc4,3218,-12, .data
	.word c
	#new features
	.equ f, 3+a
	#//

.section .bss
	.skip 2
 c: .skip 3,7
	
.section .moja_sekcija
 #new features
	.equ g, a+c-d + e - 0x01 
 #//
	.word a,b,c
	.byte 0xD3
	.word d,e
	.skip 1
	.global d
 d: .byte 0xF8
 e: .word 0x4CCF
#new features
	mov %r1,(%r2)
	sub $0x33FF,0x43(%r4)
 
 .section .text 
	int 0x55
 h: add $b, f(%r7) #pc rel
	movb %r3h,%r2
	cmp %r2,%r3
	jeq *h(%pc) #pc rel
	jne end
end: halt

#//

.end










