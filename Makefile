bin/disassembler: src/disassembler.c
	clang -g -O0 -o bin/disassembler src/disassembler.c

bin/emulator: src/emulator.c
	clang -g -O0 -o bin/emulator src/emulator.c

clean:
	rm bin/*
