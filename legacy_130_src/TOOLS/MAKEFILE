
default: dircomp2.c
	gcc -O6 -mpentium -Wall -s dircomp2.c -o dircomp2.exe

asm:
	gcc -O6 -mpentium -Wall -g dircomp2.c -o tmp.exe
	objdump  -dS tmp.exe --debugging --no-show-raw-insn > dircomp2.s
	del tmp.exe

