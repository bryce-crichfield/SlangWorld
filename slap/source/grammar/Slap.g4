grammar Slap;

// Grammar Rules
program: (section)* EOF;
section: LABEL '{' (instruction)* '}';
instruction: OPCODE (argument)? ';';
argument: REGISTER | LABEL | NUMBER;

// Lexical Rules
OPCODE
    : 'halt' 
    | 'noop' 
    | 'loadi'
    | 'loadr'
    | 'loadm'
    | 'drop'
    | 'storer'
    | 'storem'
    | 'dup'
    | 'swap'
    | 'rot'
    | 'add'
    | 'sub'
    | 'mul'
    | 'div'
    | 'mod'
    | 'addf'
    | 'subf'
    | 'mulf'
    | 'divf'
    | 'modf'
    | 'alloc'
    | 'free'
    | 'jmp'
    | 'jne'
    | 'jeq'
    | 'call'
    | 'ret'
    | 'ftoi'
    | 'itof'
    ;
    
LABEL: [a-zA-Z_][a-zA-Z0-9_]*;
REGISTER: '%'[0-9]+;
NUMBER
    : '0x' [0-9a-fA-F]+ 
    | '0b' [01]+
    | '0f' [0-9]+ '.' [0-9]+
    | '0i' [0-9]+
    ;

COMMENT : '//' ~[\r\n]* -> skip;
WHITESPACE : [ \t\r\n]+ -> skip;
