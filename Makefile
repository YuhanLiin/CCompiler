c = gcc
basedir = -iquote C:\Users\linyu\MyCode\c\compiler

devmain: main.c lexer/lexer.c array.c parser/parser.c ast/ast.c semantics/semantics.c codegen/codegen.c scope/scope.c semantics/symtable.c codegen/address.c
	${c} ${basedir} -g main.c lexer/lexer.c array.c parser/parser.c ast/ast.c semantics/semantics.c codegen/codegen.c scope/scope.c semantics/symtable.c codegen/address.c -o test/bin/main.exe

semantictest: test/semantictest.c lexer/lexer.c array.c parser/parser.c ast/ast.c semantics/semantics.c scope/scope.c semantics/symtable.c test/io.c test/utils.c
	${c} ${basedir} -g test/semantictest.c lexer/lexer.c array.c parser/parser.c ast/ast.c semantics/semantics.c scope/scope.c semantics/symtable.c  -o semantictest.exe

lexertest: test/lexertest.c lexer/lexer.c array.c test/io.c test/utils.c
	${c} ${basedir} -g test/lexertest.c lexer/lexer.c array.c -o lexertest.exe

parsertest: test/parsertest.c lexer/lexer.c array.c parser/parser.c ast/ast.c test/io.c test/utils.c
	${c} ${basedir} -g test/parsertest.c lexer/lexer.c array.c parser/parser.c ast/ast.c -o parsertest.exe

arraytest: test/arraytest.c generics/gen_array.c generics/gen_array.h
	${c} ${basedir} -g test/arraytest.c -o arraytest.exe

maptest: test/maptest.c generics/gen_map.c generics/gen_map.h
	${c} ${basedir} -g test/maptest.c -o maptest.exe