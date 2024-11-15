%{
#include <stdio.h>
%}

%%
[;]{2}.*         { printf("COMMENT\n"); }

([1-9][0-9]*|[0]+)[b][1-9][0-9]*  { printf("VALUEF\n"); }

and              { printf("KW_AND\n"); }
or               { printf("KW_OR\n"); }
not              { printf("KW_NOT\n"); }
equal            { printf("KW_EQUAL\n"); }
less             { printf("KW_LESS\n"); }
nil              { printf("KW_NIL\n"); }
list             { printf("KW_LIST\n"); }
append           { printf("KW_APPEND\n"); }
concat           { printf("KW_CONCAT\n"); }
set              { printf("KW_SET\n"); }
def              { printf("KW_DEF\n"); }
for              { printf("KW_FOR\n"); }
if               { printf("KW_IF\n"); }
exit             { printf("KW_EXIT\n"); }
load             { printf("KW_LOAD\n"); }
display          { printf("KW_DISPLAY\n"); }
true             { printf("KW_TRUE\n"); }
false            { printf("KW_FALSE\n"); }

"+"              { printf("OP_PLUS\n"); }
"-"              { printf("OP_MINUS\n"); }
"*"              { printf("OP_MULT\n"); }
"/"              { printf("OP_DIV\n"); }
"("              { printf("OP_OP\n"); }
")"              { printf("OP_CP\n"); }
","              { printf("OP_COMMA\n"); }

[a-zA-Z][a-zA-Z0-9]* { printf("IDENTIFIER\n"); }
[ \t\r\n]          {}
[0-9][a-zA-Z0-9]* { printf("SYNTAX_ERROR, %s  cannot be tokenized\n", yytext); exit(1);}
[!-'.:-@\-`{-~] { printf("SYNTAX_ERROR, %s  cannot be tokenized\n", yytext); exit(1);}
%%

int yywrap() {}

int main(int argc, char **argv)
{
    if(1 < argc) {
        yyin = fopen(argv[1], "r");
        yyrestart(yyin);
        while(!feof(yyin)) {
            yylex();
        }
        fclose(yyin);
    }
    yyin = stdin;
    yyrestart(yyin);
    while(1) {
        printf("\nWrite a line for lexical analyze: ");
        yylex();
    }
    return 0;
}