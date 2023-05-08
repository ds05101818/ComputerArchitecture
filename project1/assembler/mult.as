	lw 0 2 mplier
	lw 0 3 mcand
    lw 0 5 temp
    lw 0 6 maxbit
loop nor 2 5 4
     beq 0 4 next
     beq 0 0 shift
next add 1 3 1
shift lw 0 4 neg
	add 6 4 6
    beq 0 6 done
    add 3 3 3
    add 5 5 5
	lw 0 4 pos
    add 5 4 5
    beq 0 0 loop
done halt
mcand .fill 32766
mplier .fill 12328
temp .fill -2
neg .fill -1 
pos .fill 1
maxbit .fill 15
