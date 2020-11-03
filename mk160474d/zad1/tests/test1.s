#Testing:
# .global and .extern directives
# .skip, .byte, .word directives
# .end directive
# labels
# sections .data,.bss,.rodata,.text
# section header table
# symbol table 

.global a,a1
.extern b

.section .data
      .skip 3,0x7
 a: .byte 8, 0xF4, -53
	.skip 5
 a1: .word 0x1fc4,3218,-12
	
.section .bss
	.skip 2
 c: .skip 3,7
	
.end