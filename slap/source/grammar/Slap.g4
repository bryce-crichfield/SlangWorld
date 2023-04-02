grammar Slap;

// Grammar Rules
program: (sectionDeclaration | nativeDeclaration)* EOF;
sectionDeclaration: sectionSpecifier '{' (instruction)* '}';
nativeDeclaration: 'native' sectionSpecifier HEX_NUMBER ';';
sectionSpecifier: '@' '(' LABEL ')';

// Instructions
instruction: instruction_body ';';
instruction_body
    : instructionHalt
    | instructionNoop
    | instructionLoadi
    | instructionLoadr
    | instructionLoadm
    | instructionDrop
    | instructionStorer
    | instructionStorem
    | instructionDup
    | instructionSwap
    | instructionRot
    | instructionAdd
    | instructionSub
    | instructionMul
    | instructionDiv
    | instructionMod
    | instructionAddf
    | instructionSubf
    | instructionMulf
    | instructionDivf
    | instructionModf
    | instructionAlloc
    | instructionFree
    | instructionJmp
    | instructionJne
    | instructionJeq
    | instructionCall
    | instructionRet
    | instructionCalln
    | instructionFtoi
    | instructionItof
    | instructionItoc
    ;

instructionHalt: 'halt' wholeNumber?;
instructionNoop: 'noop';
instructionLoadi: 'loadi' anyNumber;
instructionLoadr: 'loadr' wholeNumber;
instructionLoadm: 'loadm' wholeNumber;
instructionDrop: 'drop';
instructionStorer: 'storer' wholeNumber;
instructionStorem: 'storem' wholeNumber;
instructionDup: 'dup';
instructionSwap: 'swap';
instructionRot: 'rot';
instructionAdd: 'add';
instructionSub: 'sub';
instructionMul: 'mul';
instructionDiv: 'div';
instructionMod: 'mod';
instructionAddf: 'addf';
instructionSubf: 'subf';
instructionMulf: 'mulf';
instructionDivf: 'divf';
instructionModf: 'modf';
instructionAlloc: 'alloc' wholeNumber;
instructionFree: 'free';
instructionJmp: 'jmp' sectionSpecifier;
instructionJne: 'jne' sectionSpecifier;
instructionJeq: 'jeq' sectionSpecifier;
instructionCall: 'call' sectionSpecifier;
instructionRet: 'ret';
instructionCalln: 'calln' sectionSpecifier;
instructionFtoi: 'ftoi';
instructionItof: 'itof';
instructionItoc: 'itoc';

anyNumber: floatingNumber | wholeNumber;
wholeNumber: INT_NUMBER | BIN_NUMBER | HEX_NUMBER;
floatingNumber: FLOAT_NUMBER;

LABEL: [a-zA-Z_][a-zA-Z0-9_]*;

HEX_NUMBER: '0x' [0-9a-fA-F]+;
BIN_NUMBER: '0b' [01]+;
FLOAT_NUMBER: '0f' [0-9]+ '.' [0-9]+;
INT_NUMBER: '0i' [0-9]+;

BLOCK_COMMENT : '/*' .*? '*/' -> channel(HIDDEN);
LINE_COMMENT : '//' ~[\r\n]* -> channel(HIDDEN);

WHITESPACE : [ \t\r\n]+ -> skip;
