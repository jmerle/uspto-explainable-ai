grammar Whoosh;

// This ANTLR4 grammar only supports the subset of Whoosh query syntax that this project needs

WS: [ ]+ -> skip ;

CATEGORY: 'cpc' | 'ti' | 'ab' | 'clm' | 'detd' ;
TOKEN: [a-zA-Z0-9_./]+ ;
term: CATEGORY ':' TOKEN ;

expr: term #termExpr
    | '(' expr ')' #wrappedExpr
    | left=expr 'OR' right=expr #orExpr
    | left=expr 'AND' right=expr #andExpr
    | left=expr 'XOR' right=expr #xorExpr
    | 'NOT' right=expr #notExpr
    | left=expr right=expr #andExpr ;
