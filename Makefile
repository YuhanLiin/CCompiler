devmain: main.c lexer.c array.c parser.c ast.c semantic.c codegen.c symbols.c symtable.c
	gcc -g main.c lexer.c array.c parser.c ast.c semantic.c codegen.c symbols.c symtable.c -o test/bin/main.exe

semantictest: test/semantictest.c lexer.c array.c parser.c ast.c semantic.c symbols.c symtable.c test/io.c test/utils.c
	gcc -g test/semantictest.c lexer.c array.c parser.c ast.c semantic.c symbols.c symtable.c -o semantictest.exe

lexertest: test/lexertest.c lexer.c array.c test/io.c test/utils.c
	gcc -g test/lexertest.c lexer.c array.c -o lexertest.exe

parsertest: test/parsertest.c lexer.c array.c parser.c ast.c test/io.c test/utils.c
	gcc -g test/parsertest.c lexer.c array.c parser.c ast.c -o parsertest.exe

arraytest: test/arraytest.c generics/gen_array.c generics/gen_array.h
	gcc -g test/arraytest.c -o arraytest.exe

maptest: test/maptest.c generics/gen_map.c generics/gen_map.h
	gcc -g test/maptest.c -o maptest.exe