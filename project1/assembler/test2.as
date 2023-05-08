		lw	0	1	one		loop start condition
		lw	0	2	five	loop finish condition
		add	0	1	3		i value
start	beq	3	2	exit
		add	4	3	4		accumulate
		add	1	3	3		increment i
		beq	0	0	start
exit	sw	0	4	accum
		halt
one		.fill	1
five	.fill   5
accum	.fill	0
