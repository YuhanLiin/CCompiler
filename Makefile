lexertest: test/lexertest.c lexer.c array.c
	gcc -g test/lexertest.c lexer.c array.c -o lexertest.exe

arraytest: test/arraytest.c generics/gen_array.c generics/gen_array.h
	gcc -g test/arraytest.c