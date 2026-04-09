%{
int yylex();
void yyerror(const char *s);
%}

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%token MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN SUB_ASSIGN

%token EXTERN STATIC AUTO REGISTER
%token CHAR INT FLOAT VOID
%token ALIAS STRUCT UNION ENUM

%token DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN THROUGH MAPPING

%start translation_unit
%define parse.error verbose
%%

// --- EXPRESSIONS ---

primary_expression
  : IDENTIFIER
  | CONSTANT
  | STRING_LITERAL
  | '(' expression ')'
  ;

postfix_expression
  : primary_expression
  | postfix_expression '[' expression ']'
  | postfix_expression '(' ')'
  | postfix_expression '(' expression_list ')'
  | postfix_expression '.' IDENTIFIER
  ;

expression_list
  : expression
  | expression_list ',' expression
  ;

unary_expression
  : postfix_expression
  | INC_OP unary_expression
  | DEC_OP unary_expression
  | unary_operator cast_expression
  | SIZEOF unary_expression
  | SIZEOF '(' type_name ')'
  ;

unary_operator
  : '&'
  | '*'
  | '+'
  | '-'
  | '!'
  ;

multiplicative_expression
  : unary_expression
  | multiplicative_expression '*' unary_expression
  | multiplicative_expression '/' unary_expression
  | multiplicative_expression '%' unary_expression
  ;

additive_expression
  : multiplicative_expression
  | additive_expression '+' multiplicative_expression
  | additive_expression '-' multiplicative_expression
  ;

relational_expression
  : additive_expression
  | relational_expression '<' additive_expression
  | relational_expression '>' additive_expression
  | relational_expression LE_OP additive_expression
  | relational_expression GE_OP additive_expression
  ;

equality_expression
  : relational_expression
  | equality_expression EQ_OP relational_expression
  | equality_expression NE_OP relational_expression
  ;

logical_and_expression
  : equality_expression
  | logical_and_expression AND_OP equality_expression
  ;

logical_or_expression
  : logical_and_expression
  | logical_or_expression OR_OP logical_and_expression
  ;

expression
  : logical_or_expression
  ;

// --- STATEMENTS ---

variable_declaration
  : IDENTIFIER COLON_ASSIGN expression
  ;

assignment_statement
  : unary_expression assignment_operator expression
  ;

assignment_operator
  : '='
  | MUL_ASSIGN
  | DIV_ASSIGN
  | ADD_ASSIGN
  | SUB_ASSIGN
  ;

iteration_statement
  : WHILE expression block
  | FOR variable_declaration ';' expression ';' assignment_statement block
  ;

selection_statement
  : SWITCH expression '{' selection_case_list '}'
  ;

selection_case
  : expression block
  ;

selection_case_list
  : selection_case
  | selection_case_list selection_case
  ;

statement
  : ';'
  | expression ';'
  | assignment_statement ';'
  | iteration_statement
  | selection_statement
  ;

statement_list
  : statement
  | statement_list statement
  ;

block
  : '{' statement_list '}'
  ;

// --- GLOBALS ---

function
  : FUNC IDENTIFIER LPAREN RPAREN block
  | FUNC IDENTIFIER LPAREN RPAREN type block
  ;

function_parameter
  : IDENTIFIER type
  ;

function_parameter_list
  : function_parameter
  | function_parameter_list ',' function_parameter
  ;


