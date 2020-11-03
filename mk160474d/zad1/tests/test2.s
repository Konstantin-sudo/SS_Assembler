#Testing:
#//Featurers from test1.s:
# .global and .extern directives
# .skip, .byte, .word directives
# .end directive
# labels
# sections
# section header table
# symbol table 
#//New features:
# user defined sections
# referening symbols -->relocation tables(only R_16 type - absolute)
# backpatching

.global a,a1
.extern b

.section .data
    .skip 3,0x7
 a: .byte 8, 0xF4, -53
	.skip 5

 a1: .word 0x1fc4,3218,-12
#new features:
	.word .data
	.word c
#//
.section .bss
	.skip 2
 c: .skip 3,7
	
#new features:
.section .moja_sekcija
	.word a,b,c
	.byte 0xD3
	.word d,e
	.skip 1
	.global d
 d: .byte 0xF8
 e: .word 0x4CCF
#//
.end