bin/disassembler : src/disassembler.c
	clang -o bin/disassembler src/disassembler.c

clean:
	rm bin/disassembler
