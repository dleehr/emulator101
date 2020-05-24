bin/disassembler : src/disassembler.c
	clang -g -O0 -o bin/disassembler src/disassembler.c

clean:
	rm bin/disassembler
