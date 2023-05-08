		lw		0	1	target	load target address to reg1
		jalr	1	2			store pc+1 to reg2 then branch to address in reg1
		lw		0	2	fail	if failed execute here
		noop
succ	halt					if success execute here
		lw		0	2	fail
		halt
fail	.fill	-1
target	.fill	succ
