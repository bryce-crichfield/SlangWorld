grammar Slap;

// Grammar Rules
program: (section)* EOF;
section: LABEL '{' (instruction)* '}';
instruction: OPCODE (argument)* ';';
argument: REGISTER | LABEL | NUMBER;

// Lexical Rules
OPCODE: 'halt' 'noop' 'add' 'jump' 'loadi';
LABEL: [a-zA-Z_][a-zA-Z0-9_]*;
REGISTER: '%'[0-9]+;
NUMBER
    : '0x' [0-9a-fA-F]+ 
    | '0b' [01]+
    | '0f' [0-9]+ '.' [0-9]+
    | '0i' [0-9]+
    ;

COMMENT : '//' ~[\r]* -> skip;
WHITESPACE : [ \t\r\n]+ -> skip;
