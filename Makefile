lexertest: test/lexertest.c lexer.c array.c test/io.c test/utils.c
	gcc -g test/lexertest.c lexer.c array.c -o lexertest.exe

parsertest: test/parsertest.c lexer.c array.c parser.c ast.c test/io.c test/utils.c
	gcc -g test/parsertest.c lexer.c array.c parser.c ast.c -o parsertest.exe

arraytest: test/arraytest.c generics/gen_array.c generics/gen_array.h
	gcc -g test/arraytest.c