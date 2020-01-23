uo: LOAD "2"
.set OVO -123
.set BOLOVO 0x1234123412
.set IPO 5
.set JIHO 20
lul:
STOR "0x0000000711"
lel:
LOAD "1"
.align 1
.wfill 10 -2147483648
.org 230
tops:
ADD "123"
ted:
SUB "0x00000000F4"
MUL "431"
.align 3
MUL "0x00000000F4"
.align 1
.word 0x4B21F65482
.align 1
.word 10
.word 0x4B21F65482
.word tops
.word -10
.word 0x00AB000012
.word ted
.word OVO
.word BOLOVO
STORA "lel"
STORA "tops"
STORA "ted"
ADD "tops"
