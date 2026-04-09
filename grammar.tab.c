/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "grammar.y"

int yylex();
void yyerror(const char *s);

#line 76 "grammar.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "grammar.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_CONSTANT = 4,                   /* CONSTANT  */
  YYSYMBOL_STRING_LITERAL = 5,             /* STRING_LITERAL  */
  YYSYMBOL_SIZEOF = 6,                     /* SIZEOF  */
  YYSYMBOL_PTR_OP = 7,                     /* PTR_OP  */
  YYSYMBOL_INC_OP = 8,                     /* INC_OP  */
  YYSYMBOL_DEC_OP = 9,                     /* DEC_OP  */
  YYSYMBOL_LEFT_OP = 10,                   /* LEFT_OP  */
  YYSYMBOL_RIGHT_OP = 11,                  /* RIGHT_OP  */
  YYSYMBOL_LE_OP = 12,                     /* LE_OP  */
  YYSYMBOL_GE_OP = 13,                     /* GE_OP  */
  YYSYMBOL_EQ_OP = 14,                     /* EQ_OP  */
  YYSYMBOL_NE_OP = 15,                     /* NE_OP  */
  YYSYMBOL_AND_OP = 16,                    /* AND_OP  */
  YYSYMBOL_OR_OP = 17,                     /* OR_OP  */
  YYSYMBOL_MUL_ASSIGN = 18,                /* MUL_ASSIGN  */
  YYSYMBOL_DIV_ASSIGN = 19,                /* DIV_ASSIGN  */
  YYSYMBOL_MOD_ASSIGN = 20,                /* MOD_ASSIGN  */
  YYSYMBOL_ADD_ASSIGN = 21,                /* ADD_ASSIGN  */
  YYSYMBOL_SUB_ASSIGN = 22,                /* SUB_ASSIGN  */
  YYSYMBOL_LEFT_ASSIGN = 23,               /* LEFT_ASSIGN  */
  YYSYMBOL_RIGHT_ASSIGN = 24,              /* RIGHT_ASSIGN  */
  YYSYMBOL_AND_ASSIGN = 25,                /* AND_ASSIGN  */
  YYSYMBOL_XOR_ASSIGN = 26,                /* XOR_ASSIGN  */
  YYSYMBOL_OR_ASSIGN = 27,                 /* OR_ASSIGN  */
  YYSYMBOL_TYPE_NAME = 28,                 /* TYPE_NAME  */
  YYSYMBOL_TYPEDEF = 29,                   /* TYPEDEF  */
  YYSYMBOL_EXTERN = 30,                    /* EXTERN  */
  YYSYMBOL_STATIC = 31,                    /* STATIC  */
  YYSYMBOL_AUTO = 32,                      /* AUTO  */
  YYSYMBOL_REGISTER = 33,                  /* REGISTER  */
  YYSYMBOL_CHAR = 34,                      /* CHAR  */
  YYSYMBOL_SHORT = 35,                     /* SHORT  */
  YYSYMBOL_INT = 36,                       /* INT  */
  YYSYMBOL_LONG = 37,                      /* LONG  */
  YYSYMBOL_SIGNED = 38,                    /* SIGNED  */
  YYSYMBOL_UNSIGNED = 39,                  /* UNSIGNED  */
  YYSYMBOL_FLOAT = 40,                     /* FLOAT  */
  YYSYMBOL_DOUBLE = 41,                    /* DOUBLE  */
  YYSYMBOL_CONST = 42,                     /* CONST  */
  YYSYMBOL_VOLATILE = 43,                  /* VOLATILE  */
  YYSYMBOL_VOID = 44,                      /* VOID  */
  YYSYMBOL_STRUCT = 45,                    /* STRUCT  */
  YYSYMBOL_UNION = 46,                     /* UNION  */
  YYSYMBOL_ENUM = 47,                      /* ENUM  */
  YYSYMBOL_ELLIPSIS = 48,                  /* ELLIPSIS  */
  YYSYMBOL_CASE = 49,                      /* CASE  */
  YYSYMBOL_DEFAULT = 50,                   /* DEFAULT  */
  YYSYMBOL_IF = 51,                        /* IF  */
  YYSYMBOL_ELSE = 52,                      /* ELSE  */
  YYSYMBOL_SWITCH = 53,                    /* SWITCH  */
  YYSYMBOL_WHILE = 54,                     /* WHILE  */
  YYSYMBOL_DO = 55,                        /* DO  */
  YYSYMBOL_FOR = 56,                       /* FOR  */
  YYSYMBOL_GOTO = 57,                      /* GOTO  */
  YYSYMBOL_CONTINUE = 58,                  /* CONTINUE  */
  YYSYMBOL_BREAK = 59,                     /* BREAK  */
  YYSYMBOL_RETURN = 60,                    /* RETURN  */
  YYSYMBOL_61_ = 61,                       /* '('  */
  YYSYMBOL_62_ = 62,                       /* ')'  */
  YYSYMBOL_63_ = 63,                       /* '['  */
  YYSYMBOL_64_ = 64,                       /* ']'  */
  YYSYMBOL_65_ = 65,                       /* '.'  */
  YYSYMBOL_66_ = 66,                       /* ','  */
  YYSYMBOL_67_ = 67,                       /* '&'  */
  YYSYMBOL_68_ = 68,                       /* '*'  */
  YYSYMBOL_69_ = 69,                       /* '+'  */
  YYSYMBOL_70_ = 70,                       /* '-'  */
  YYSYMBOL_71_ = 71,                       /* '~'  */
  YYSYMBOL_72_ = 72,                       /* '!'  */
  YYSYMBOL_73_ = 73,                       /* '/'  */
  YYSYMBOL_74_ = 74,                       /* '%'  */
  YYSYMBOL_75_ = 75,                       /* '<'  */
  YYSYMBOL_76_ = 76,                       /* '>'  */
  YYSYMBOL_77_ = 77,                       /* '^'  */
  YYSYMBOL_78_ = 78,                       /* '|'  */
  YYSYMBOL_79_ = 79,                       /* '?'  */
  YYSYMBOL_80_ = 80,                       /* ':'  */
  YYSYMBOL_81_ = 81,                       /* '='  */
  YYSYMBOL_82_ = 82,                       /* ';'  */
  YYSYMBOL_83_ = 83,                       /* '{'  */
  YYSYMBOL_84_ = 84,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 85,                  /* $accept  */
  YYSYMBOL_primary_expression = 86,        /* primary_expression  */
  YYSYMBOL_postfix_expression = 87,        /* postfix_expression  */
  YYSYMBOL_argument_expression_list = 88,  /* argument_expression_list  */
  YYSYMBOL_unary_expression = 89,          /* unary_expression  */
  YYSYMBOL_unary_operator = 90,            /* unary_operator  */
  YYSYMBOL_cast_expression = 91,           /* cast_expression  */
  YYSYMBOL_multiplicative_expression = 92, /* multiplicative_expression  */
  YYSYMBOL_additive_expression = 93,       /* additive_expression  */
  YYSYMBOL_shift_expression = 94,          /* shift_expression  */
  YYSYMBOL_relational_expression = 95,     /* relational_expression  */
  YYSYMBOL_equality_expression = 96,       /* equality_expression  */
  YYSYMBOL_and_expression = 97,            /* and_expression  */
  YYSYMBOL_exclusive_or_expression = 98,   /* exclusive_or_expression  */
  YYSYMBOL_inclusive_or_expression = 99,   /* inclusive_or_expression  */
  YYSYMBOL_logical_and_expression = 100,   /* logical_and_expression  */
  YYSYMBOL_logical_or_expression = 101,    /* logical_or_expression  */
  YYSYMBOL_conditional_expression = 102,   /* conditional_expression  */
  YYSYMBOL_assignment_expression = 103,    /* assignment_expression  */
  YYSYMBOL_assignment_operator = 104,      /* assignment_operator  */
  YYSYMBOL_expression = 105,               /* expression  */
  YYSYMBOL_constant_expression = 106,      /* constant_expression  */
  YYSYMBOL_declaration = 107,              /* declaration  */
  YYSYMBOL_declaration_specifiers = 108,   /* declaration_specifiers  */
  YYSYMBOL_init_declarator_list = 109,     /* init_declarator_list  */
  YYSYMBOL_init_declarator = 110,          /* init_declarator  */
  YYSYMBOL_storage_class_specifier = 111,  /* storage_class_specifier  */
  YYSYMBOL_type_specifier = 112,           /* type_specifier  */
  YYSYMBOL_struct_or_union_specifier = 113, /* struct_or_union_specifier  */
  YYSYMBOL_struct_or_union = 114,          /* struct_or_union  */
  YYSYMBOL_struct_declaration_list = 115,  /* struct_declaration_list  */
  YYSYMBOL_struct_declaration = 116,       /* struct_declaration  */
  YYSYMBOL_specifier_qualifier_list = 117, /* specifier_qualifier_list  */
  YYSYMBOL_struct_declarator_list = 118,   /* struct_declarator_list  */
  YYSYMBOL_struct_declarator = 119,        /* struct_declarator  */
  YYSYMBOL_enum_specifier = 120,           /* enum_specifier  */
  YYSYMBOL_enumerator_list = 121,          /* enumerator_list  */
  YYSYMBOL_enumerator = 122,               /* enumerator  */
  YYSYMBOL_type_qualifier = 123,           /* type_qualifier  */
  YYSYMBOL_declarator = 124,               /* declarator  */
  YYSYMBOL_direct_declarator = 125,        /* direct_declarator  */
  YYSYMBOL_pointer = 126,                  /* pointer  */
  YYSYMBOL_type_qualifier_list = 127,      /* type_qualifier_list  */
  YYSYMBOL_parameter_type_list = 128,      /* parameter_type_list  */
  YYSYMBOL_parameter_list = 129,           /* parameter_list  */
  YYSYMBOL_parameter_declaration = 130,    /* parameter_declaration  */
  YYSYMBOL_identifier_list = 131,          /* identifier_list  */
  YYSYMBOL_type_name = 132,                /* type_name  */
  YYSYMBOL_abstract_declarator = 133,      /* abstract_declarator  */
  YYSYMBOL_direct_abstract_declarator = 134, /* direct_abstract_declarator  */
  YYSYMBOL_initializer = 135,              /* initializer  */
  YYSYMBOL_initializer_list = 136,         /* initializer_list  */
  YYSYMBOL_statement = 137,                /* statement  */
  YYSYMBOL_labeled_statement = 138,        /* labeled_statement  */
  YYSYMBOL_compound_statement = 139,       /* compound_statement  */
  YYSYMBOL_declaration_list = 140,         /* declaration_list  */
  YYSYMBOL_statement_list = 141,           /* statement_list  */
  YYSYMBOL_expression_statement = 142,     /* expression_statement  */
  YYSYMBOL_selection_statement = 143,      /* selection_statement  */
  YYSYMBOL_iteration_statement = 144,      /* iteration_statement  */
  YYSYMBOL_jump_statement = 145,           /* jump_statement  */
  YYSYMBOL_translation_unit = 146,         /* translation_unit  */
  YYSYMBOL_external_declaration = 147,     /* external_declaration  */
  YYSYMBOL_function_definition = 148       /* function_definition  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  70
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2205

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  85
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  263
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  478

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   315


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    72,     2,     2,     2,    74,    67,     2,
      61,    62,    68,    69,    66,    70,    65,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    80,    82,
      75,    81,    76,    79,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    83,    78,    84,    71,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    22,    22,    23,    24,    25,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    41,    42,    43,    47,    48,
      49,    50,    51,    52,    56,    57,    58,    59,    60,    61,
      65,    66,    70,    71,    72,    73,    74,    75,    76,    80,
      81,    82,    83,    84,    88,    89,    90,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   106,   107,   108,   109,
     110,   114,   115,   119,   120,   124,   125,   129,   130,   134,
     135,   139,   140,   141,   142,   143,   147,   148,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   166,
     167,   171,   175,   176,   177,   178,   182,   183,   184,   185,
     186,   187,   191,   192,   193,   197,   198,   199,   203,   204,
     205,   206,   207,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   226,   227,   228,   229,   230,
     234,   235,   239,   240,   244,   245,   249,   250,   251,   252,
     256,   257,   258,   262,   263,   264,   265,   269,   270,   271,
     272,   273,   277,   278,   279,   283,   284,   285,   289,   290,
     294,   295,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   312,   313,   314,   315,   316,   320,   321,   326,
     327,   328,   332,   333,   334,   338,   339,   340,   344,   345,
     349,   350,   354,   355,   356,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   375,   376,   377,
     378,   382,   383,   384,   388,   389,   390,   391,   392,   393,
     397,   398,   399,   403,   404,   405,   406,   407,   411,   412,
     416,   417,   421,   422,   426,   427,   428,   429,   430,   431,
     435,   436,   437,   438,   439,   440,   444,   445,   446,   447,
     448,   449,   453,   454,   458,   459,   463,   464,   465,   466,
     467,   468,   469,   470
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER",
  "CONSTANT", "STRING_LITERAL", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP",
  "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP",
  "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN",
  "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN",
  "OR_ASSIGN", "TYPE_NAME", "TYPEDEF", "EXTERN", "STATIC", "AUTO",
  "REGISTER", "CHAR", "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED",
  "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "STRUCT", "UNION",
  "ENUM", "ELLIPSIS", "CASE", "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE",
  "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN", "'('", "')'", "'['",
  "']'", "'.'", "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'",
  "'%'", "'<'", "'>'", "'^'", "'|'", "'?'", "':'", "'='", "';'", "'{'",
  "'}'", "$accept", "primary_expression", "postfix_expression",
  "argument_expression_list", "unary_expression", "unary_operator",
  "cast_expression", "multiplicative_expression", "additive_expression",
  "shift_expression", "relational_expression", "equality_expression",
  "and_expression", "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_or_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "constant_expression", "declaration",
  "declaration_specifiers", "init_declarator_list", "init_declarator",
  "storage_class_specifier", "type_specifier", "struct_or_union_specifier",
  "struct_or_union", "struct_declaration_list", "struct_declaration",
  "specifier_qualifier_list", "struct_declarator_list",
  "struct_declarator", "enum_specifier", "enumerator_list", "enumerator",
  "type_qualifier", "declarator", "direct_declarator", "pointer",
  "type_qualifier_list", "parameter_type_list", "parameter_list",
  "parameter_declaration", "identifier_list", "type_name",
  "abstract_declarator", "direct_abstract_declarator", "initializer",
  "initializer_list", "statement", "labeled_statement",
  "compound_statement", "declaration_list", "statement_list",
  "expression_statement", "selection_statement", "iteration_statement",
  "jump_statement", "translation_unit", "external_declaration",
  "function_definition", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-317)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-193)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1687,  1950,  -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,
    -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,
    -317,  -317,     4,   223,   209,  -317,    16,  2138,  2138,  -317,
       5,  -317,  2138,   738,    99,    42,  1532,  -317,  -317,   144,
     389,  -317,   152,  -317,  2006,   -77,   220,    29,   133,   130,
    -317,  -317,    26,   197,  -317,   165,  -317,  1984,  -317,  -317,
      69,  2056,  -317,   167,  -317,  2006,  1868,   935,   213,    99,
    -317,  -317,   230,   824,   202,  -317,  -317,  1443,  1475,  1475,
     304,   234,   242,   260,   295,   641,   301,   327,   285,   297,
     819,   923,  -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,
      44,  1435,  1487,  -317,    27,   271,   401,    37,   549,   315,
     314,   326,   399,    -6,  -317,  -317,    90,  -317,  -317,  -317,
     461,   533,  -317,  -317,  -317,  -317,    93,   356,  -317,  -317,
     240,    -1,   372,    33,  -317,  -317,  -317,  -317,  -317,  -317,
     226,   785,  -317,  -317,   226,   304,  -317,   785,  -317,  2006,
    2076,   160,  2158,   440,  -317,   205,  2158,  -317,  -317,   -20,
    -317,  -317,  1440,   409,   397,  -317,   -18,   646,  -317,  -317,
    -317,  -317,   424,  -317,  1054,  1054,  1091,  1091,  1487,  1126,
    1126,  1487,  1487,  1054,  1054,  1138,  -317,  -317,   677,   923,
    -317,  1163,  -317,  -317,   429,   677,  1175,  1210,   304,  1320,
     481,  1247,   479,  -317,  -317,  1363,  -317,   166,  1683,   135,
     243,   503,   540,  -317,  -317,   970,   304,   563,  -317,  -317,
    -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,   304,
    -317,  1487,  1487,  1487,  1126,  1126,  1054,  1054,  1054,  1054,
    1054,  1054,  1091,  1091,  1259,  1259,  1259,  1259,  1259,  1282,
     304,  -317,  1395,  -317,   569,  -317,  -317,   149,    84,   258,
     304,  -317,   304,   258,  -317,    15,  -317,   802,  -317,  -317,
    -317,   485,  -317,  -317,   218,   512,  -317,  -317,  -317,   486,
    -317,  -317,    71,   304,   172,  -317,   489,  -317,  -317,  2096,
    1593,  1007,  -317,   254,  -317,   391,  -317,  2117,  -317,   568,
    -317,   278,   401,   401,  1311,    37,    37,  -317,   287,    27,
      27,  -317,  -317,   401,   401,  1512,   187,  -317,   514,   677,
    -317,  1698,   257,  1723,   270,   277,   518,   519,  1344,  1294,
    -317,  -317,  -317,  -317,  -317,  1640,   462,  -317,  1487,  -317,
    1766,  -317,   291,  -317,   496,  -317,  -317,  -317,  -317,  -317,
      27,    27,   271,   271,   401,   401,   401,   401,    37,    37,
    1589,   549,   315,   314,   326,   399,  1537,   215,  -317,  -317,
    -317,  -317,   500,  -317,  -317,  -317,  -317,   846,  -317,   156,
    -317,  -317,  -317,   208,   304,  -317,   208,  -317,   304,  -317,
    -317,   319,  -317,   523,   535,  1791,  -317,   531,   391,  1915,
    1019,  -317,  -317,  -317,   304,   304,  -317,  -317,   677,   677,
     677,   677,   677,   304,   304,   677,  1042,   532,  -317,   304,
    -317,   304,  -317,   304,   304,   785,  -317,   605,  -317,   -23,
    -317,  -317,  -317,  -317,  -317,  -317,  -317,  -317,   334,  -317,
     537,  1804,  -317,   543,  -317,  -317,   560,   579,  -317,  -317,
    -317,   339,   340,  -317,   677,   341,  -317,  -317,  -317,  -317,
    -317,  -317,  -317,  -317,  -317,  -317,  -317,   677,   677,   539,
     550,  -317,   677,  -317,  -317,  -317,  -317,  -317
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,   162,   124,   108,   109,   110,   111,   112,   114,
     115,   116,   117,   120,   121,   118,   119,   158,   159,   113,
     130,   131,     0,     0,     0,   255,     0,    96,    98,   122,
       0,   123,   100,     0,   161,     0,     0,   252,   254,     0,
       0,   228,     0,   263,     0,   151,     0,     0,     0,     0,
     177,   174,   173,     0,    92,     0,   102,   105,    97,    99,
     127,     0,   101,     0,   261,     0,     0,     0,     0,   160,
       1,   253,     0,     0,     2,     3,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,    29,   223,     7,
      18,    30,     0,    32,    39,    44,    47,    56,    61,    63,
      65,    67,    69,    71,    76,    89,     0,   230,   214,   215,
       0,     0,   216,   217,   218,   219,     0,   105,   229,   260,
       0,     0,   155,     0,   152,   171,   163,   176,   178,   175,
       0,     0,    95,   258,     0,     0,    93,     0,   257,     0,
       0,     0,   137,     0,   132,     0,   139,   262,   259,     0,
     188,   170,     0,     0,   179,   182,     0,     0,     2,   165,
      30,    91,     0,   169,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   232,   227,     0,     0,
      22,     0,    19,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   247,   248,     0,   249,     0,     0,     0,
     190,     0,     0,    13,    14,     0,     0,     0,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    78,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   233,     0,   225,     0,   224,   231,     0,     0,     0,
       0,   149,     0,     0,   147,     0,   104,     0,   207,   107,
     103,     0,   106,   256,     0,     0,   135,   129,   136,     0,
     126,   133,     0,     0,     0,   140,   143,   138,   168,     0,
       0,     0,   185,     0,   186,   193,   166,     0,   167,     0,
     164,     0,    54,    55,     0,    59,    60,    36,     0,    42,
      43,    37,    38,    52,    53,     0,     0,   220,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,   251,   250,     6,     5,     0,   192,   191,     0,    12,
       0,     9,     0,    15,     0,    11,    77,    33,    34,    35,
      40,    41,    45,    46,    50,    51,    48,    49,    57,    58,
       0,    62,    64,    66,    68,    70,     0,     0,    90,   226,
     150,   148,     0,   154,   157,   156,   153,     0,   211,     0,
      94,   128,   125,     0,     0,   144,     0,   134,     0,   181,
     184,     0,   201,     0,     0,     0,   196,     0,   194,     0,
       0,   180,   183,   189,     0,     0,    23,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    31,     0,
      10,     0,     8,     0,     0,     0,   210,     0,   208,     0,
     142,   146,   141,   145,   202,   195,   198,   197,     0,   203,
       0,     0,   199,     0,    73,    75,   237,   234,   239,   236,
     240,     0,     0,   245,     0,     0,    17,    16,    74,    72,
     213,   209,   212,   205,   204,   206,   200,     0,     0,     0,
       0,   242,     0,   238,   235,   244,   241,   243
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -317,  -317,  -317,  -317,   -55,  -317,   -89,  -120,   290,   578,
    -161,   371,   388,   402,   387,   395,  -317,   -46,   -11,  -317,
     -50,   -66,    20,     0,  -317,   269,  -317,   112,  -317,  -317,
     501,  -144,   -81,  -317,  -316,  -317,   505,   192,    11,     3,
     -33,     9,  -317,   -62,  -317,  -140,  -317,   465,  -132,  -262,
    -129,  -317,   -84,  -317,    53,    22,   536,  -196,  -317,  -317,
    -317,  -317,   619,  -317
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    99,   100,   342,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   229,
     116,   172,    41,    42,    55,    56,    27,    28,    29,    30,
     153,   154,   155,   284,   285,    31,   133,   134,    32,   127,
      34,    35,    52,   393,   164,   165,   166,   211,   394,   295,
     269,   379,   117,   118,   119,    44,   121,   122,   123,   124,
     125,    36,    37,    38
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      26,   200,    69,    33,   163,   329,   130,    45,    60,   281,
     210,   248,   170,   230,   194,   305,   306,    53,   272,     2,
      25,   171,   190,   192,   193,   170,    48,    58,    59,    57,
     294,   398,    62,    51,   171,    50,    26,   256,    39,    33,
     207,   209,   288,    68,   298,     2,   289,   170,   299,   238,
     239,   212,   213,   214,    43,    65,    25,   384,   137,   309,
     310,   139,   120,   138,   128,   259,   162,   430,    17,    18,
     432,   278,   156,   249,   398,   287,    39,    23,   337,   149,
     260,   358,   359,   261,    24,   128,    64,    46,    61,   307,
      39,   135,   311,   312,    24,   231,   141,   129,    54,   263,
     232,   233,   156,    23,   317,   215,   143,   216,   210,   217,
     148,   320,   240,   241,   350,   351,   157,   264,   158,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     268,   281,    39,   416,   271,   316,   268,   383,   378,   209,
     128,   209,   347,   348,   349,    72,   322,   324,   325,   390,
     263,   384,   150,   126,    39,     2,   250,   402,   286,   140,
      66,   156,    67,   156,   156,   292,   344,   156,   371,   128,
     256,   293,   251,   152,   141,   142,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   374,   136,   375,   334,    24,   367,
     156,   250,   273,   152,   343,   170,   282,   170,     2,   429,
      49,     2,  -172,    23,   171,   259,   171,   385,   346,   336,
      24,   131,   427,   132,    47,   397,     2,   265,   170,     2,
     260,   144,   250,   370,    54,   407,   170,   171,   386,   368,
     428,   257,   276,   132,   277,   171,   145,   146,   332,   418,
      40,    17,    18,   250,   387,    68,   268,     2,    39,   372,
      69,   132,   152,   140,   152,   152,    23,   405,   152,    23,
    -172,  -172,  -172,    24,    39,  -172,    24,    24,   141,   142,
      40,   250,   188,   170,    23,   283,   156,    23,   283,   162,
     162,    24,   173,    48,    24,   424,   460,   162,   462,   293,
     276,   152,   381,   196,   335,   167,   291,   168,    75,    76,
      77,    24,    78,    79,   195,   290,  -192,   291,   431,   409,
    -192,   197,   433,   250,   446,   447,   448,   449,   450,   170,
     202,   453,   411,   170,   443,   162,   250,   440,   171,   412,
     234,   235,   171,   250,   336,   170,   178,   179,   180,   170,
     170,   181,   182,   420,   171,   178,   198,   421,   444,   445,
     181,   182,   201,   451,   452,    91,   455,   203,   170,   170,
     471,    92,    93,    94,    95,    96,    97,   458,   459,   204,
      39,   135,   244,   473,   474,   289,   286,   152,   477,   286,
      73,   245,    74,    75,    76,    77,   463,    78,    79,   162,
     289,   469,   470,   472,   246,   250,   250,   250,   456,   266,
     457,   236,   237,   270,   268,   247,   268,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,   147,    80,    81,
      82,   279,    83,    84,    85,    86,    87,    88,    89,    90,
      91,   373,   399,   262,   400,   376,    92,    93,    94,    95,
      96,    97,   252,   297,    74,    75,    76,    77,     3,    78,
      79,   296,    40,    98,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,   300,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,   319,
      80,    81,    82,   279,    83,    84,    85,    86,    87,    88,
      89,    90,    91,   335,   280,   291,   352,   353,    92,    93,
      94,    95,    96,    97,   252,   327,    74,    75,    76,    77,
       3,    78,    79,   339,    40,   253,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     422,   330,   250,   242,   243,   338,   345,   380,   276,   388,
     252,   403,    74,    75,    76,    77,   406,    78,    79,   413,
     414,   260,    80,    81,    82,   434,    83,    84,    85,    86,
      87,    88,    89,    90,    91,   437,   382,   435,   289,   464,
      92,    93,    94,    95,    96,    97,   167,   466,   168,    75,
      76,    77,   467,    78,    79,   361,    40,   255,    80,    81,
      82,   475,    83,    84,    85,    86,    87,    88,    89,    90,
      91,   468,   476,   362,   364,   258,    92,    93,    94,    95,
      96,    97,   199,   365,    74,    75,    76,    77,   363,    78,
      79,   275,    40,   369,   318,    71,   254,     0,   174,   175,
     176,   177,     0,     0,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,   252,     0,
      74,    75,    76,    77,     0,    78,    79,     0,   267,   461,
      80,    81,    82,     0,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,   178,   179,   180,     0,     0,   181,
     182,   183,   184,     0,    40,   185,    80,    81,    82,     0,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    63,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
       0,     0,   302,   303,     0,     0,     0,     0,     0,     0,
      40,   313,   314,     0,     0,     0,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,   167,     0,   168,    75,
      76,    77,     0,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,   377,     0,   168,    75,    76,    77,     0,
      78,    79,     0,     0,     0,     0,   354,   355,   356,   357,
     205,    40,   168,    75,    76,    77,     0,    78,    79,     0,
       0,     0,     0,     0,     0,     0,   174,   175,   176,   177,
       0,     0,     0,     0,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,   174,   175,
     176,   177,     0,    91,     0,     0,     0,     0,   267,    92,
      93,    94,    95,    96,    97,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,   267,    92,    93,    94,    95,
      96,    97,   178,   179,   180,     0,     0,   181,   182,   183,
     184,   206,     0,   185,     0,     0,   186,     0,   187,     0,
       0,     0,   425,     0,   178,   179,   180,     0,     0,   181,
     182,   183,   184,     0,   208,   185,   168,    75,    76,    77,
     426,    78,    79,     0,     0,     0,   167,     0,   168,    75,
      76,    77,     0,    78,    79,     0,     0,     0,     0,     0,
       0,     3,     0,     0,     0,     0,     0,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,   340,     0,   168,    75,    76,    77,     0,    78,    79,
       0,     0,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    91,     0,     0,   169,
       0,     0,    92,    93,    94,    95,    96,    97,   395,     0,
     168,    75,    76,    77,     0,    78,    79,     0,     0,     0,
     441,     0,   168,    75,    76,    77,     0,    78,    79,     0,
       0,    91,   341,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,   167,     0,   168,    75,    76,    77,     0,
      78,    79,     0,     0,     0,   301,     0,   168,    75,    76,
      77,     0,    78,    79,     0,     0,     0,     0,    91,     0,
       0,   396,     0,     0,    92,    93,    94,    95,    96,    97,
      91,     0,     0,   442,     0,     0,    92,    93,    94,    95,
      96,    97,   304,     0,   168,    75,    76,    77,     0,    78,
      79,     0,     0,    91,   454,     0,     0,     0,     0,    92,
      93,    94,    95,    96,    97,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,   308,     0,   168,
      75,    76,    77,     0,    78,    79,     0,     0,     0,   315,
       0,   168,    75,    76,    77,     0,    78,    79,     0,     0,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,   208,     0,   168,    75,    76,    77,
       0,    78,    79,     0,     0,     0,   321,     0,   168,    75,
      76,    77,     0,    78,    79,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,   323,     0,   168,    75,    76,    77,     0,    78,    79,
       0,     0,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,   328,     0,
     168,    75,    76,    77,     0,    78,    79,     0,     0,     0,
     360,     0,   168,    75,    76,    77,     0,    78,    79,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,   366,     0,   168,    75,    76,    77,     0,
      78,    79,     0,     0,     0,   252,     0,   168,    75,    76,
      77,     0,    78,    79,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
      91,     0,     0,   174,   175,     0,    92,    93,    94,    95,
      96,    97,   174,   175,   176,   177,     0,     0,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,    96,    97,    91,   174,   175,   176,   177,
       0,    92,    93,    94,    95,    96,    97,     0,     0,     0,
       0,     0,     0,     0,   326,   174,   175,   176,   177,   178,
     179,   180,     0,     0,   181,   182,   183,   184,   178,   179,
     180,     0,     0,   181,   182,   183,   184,     0,     0,   185,
       0,     0,   186,     0,     0,     0,   415,   174,   175,   176,
     177,     0,   178,   179,   180,     0,     0,   181,   182,   183,
     184,     0,     0,   185,     0,     0,   186,     0,     0,     0,
       0,   178,   179,   180,     0,     0,   181,   182,   183,   184,
       0,    68,   185,     2,     0,   331,   168,    75,    76,    77,
       0,    78,    79,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   178,   179,   180,     0,     0,   181,   182,
     183,   184,     0,     0,   185,     0,     0,   186,   168,    75,
      76,    77,     0,    78,    79,     0,     0,     0,     0,     0,
     168,    75,    76,    77,     0,    78,    79,     0,     0,     0,
       0,   290,  -187,   291,   189,     0,  -187,     0,    24,     0,
      92,    93,    94,    95,    96,    97,   228,     0,     0,     0,
       0,     0,     0,     0,   174,   175,   176,   177,     0,     0,
       0,     0,    70,     1,     0,     2,   191,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    91,   174,
     175,   176,   177,     0,    92,    93,    94,    95,    96,    97,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     178,   179,   180,     0,     0,   181,   182,   183,   184,     0,
       0,   185,   404,    23,   391,     0,     2,     0,     0,     0,
      24,   174,   175,   176,   177,   178,   179,   180,     0,     0,
     181,   182,   183,   184,     0,     0,   185,   423,     0,     0,
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,   417,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   290,   392,   291,   178,   179,   180,
       0,    24,   181,   182,   183,   184,     0,     0,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,     1,     0,
       2,     0,     0,     0,     0,   174,   175,   176,   177,     0,
       0,   335,   392,   291,     0,     0,     0,     0,    24,     0,
     174,   175,   176,   177,     0,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,   174,   175,   176,   177,     0,
       0,     0,     0,     0,     0,   333,     0,     0,    23,     0,
       0,   178,   179,   180,     0,    24,   181,   182,   183,   184,
     408,     0,   185,     0,     0,     0,   178,   179,   180,     0,
       0,   181,   182,   183,   184,     0,     0,   185,   174,   175,
     176,   177,     0,     0,     0,   410,     0,     0,     0,     0,
       0,   178,   179,   180,     0,     0,   181,   182,   183,   184,
       0,     0,   185,   174,   175,   176,   177,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   174,   175,   176,   177,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   419,     0,   178,   179,   180,     0,     0,   181,
     182,   183,   184,     0,     0,   185,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   436,     0,     0,     0,   178,
     179,   180,     0,     0,   181,   182,   183,   184,   465,   159,
     185,   160,   178,   179,   180,     0,     0,   181,   182,   183,
     184,     0,     0,   185,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,   438,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     161,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   439,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,    40,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,     0,     0,   151,     0,     0,
       0,     0,     0,     0,     0,   147,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   274,     0,     0,
       0,     0,     0,     0,     3,     0,     0,     0,     0,    40,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     3,     0,     0,     0,     0,     0,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,   389,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,   401,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     3,     0,     0,     0,
       0,     0,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22
};

static const yytype_int16 yycheck[] =
{
       0,    85,    35,     0,    66,   201,    83,     3,     3,   153,
      91,    17,    67,   102,    80,   176,   177,     1,   147,     3,
       0,    67,    77,    78,    79,    80,    23,    27,    28,    26,
     162,   293,    32,    24,    80,    24,    36,   121,    61,    36,
      90,    91,    62,     1,    62,     3,    66,   102,    66,    12,
      13,     7,     8,     9,     1,    33,    36,    80,    49,   179,
     180,    52,    40,    52,    44,    66,    66,   383,    42,    43,
     386,   152,    61,    79,   336,   156,    61,    61,   210,    57,
      81,   242,   243,    84,    68,    65,    33,    83,    83,   178,
      61,    62,   181,   182,    68,    68,    81,    44,    82,    66,
      73,    74,    91,    61,   188,    61,    53,    63,   189,    65,
      57,   195,    75,    76,   234,   235,    63,    84,    65,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     141,   275,    61,   329,   145,   185,   147,    66,   267,   189,
     120,   191,   231,   232,   233,     1,   196,   197,   198,   289,
      66,    80,    83,     1,    61,     3,    66,   297,   155,    66,
      61,   150,    63,   152,   153,   162,   216,   156,    84,   149,
     254,   162,    82,    61,    81,    82,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   260,    62,   262,    62,    68,   249,
     189,    66,   149,    91,   215,   260,     1,   262,     3,     1,
       1,     3,     3,    61,   260,    66,   262,   283,   229,   210,
      68,     1,    66,     3,     1,   291,     3,     1,   283,     3,
      81,    66,    66,    84,    82,   319,   291,   283,    66,   250,
      84,     1,    82,     3,    84,   291,    81,    82,    82,   338,
      83,    42,    43,    66,    82,     1,   267,     3,    61,     1,
     293,     3,   150,    66,   152,   153,    61,    80,   156,    61,
      61,    62,    63,    68,    61,    66,    68,    68,    81,    82,
      83,    66,    80,   338,    61,    80,   275,    61,    80,   289,
     290,    68,    62,   290,    68,    80,   425,   297,   427,   290,
      82,   189,    84,    61,    61,     1,    63,     3,     4,     5,
       6,    68,     8,     9,    80,    61,    62,    63,   384,    62,
      66,    61,   388,    66,   408,   409,   410,   411,   412,   384,
       3,   415,    62,   388,   400,   335,    66,   399,   384,    62,
      69,    70,   388,    66,   335,   400,    68,    69,    70,   404,
     405,    73,    74,    62,   400,    68,    61,    66,   404,   405,
      73,    74,    61,   413,   414,    61,   416,    82,   423,   424,
     454,    67,    68,    69,    70,    71,    72,   423,   424,    82,
      61,    62,    67,   467,   468,    66,   383,   275,   472,   386,
       1,    77,     3,     4,     5,     6,    62,     8,     9,   399,
      66,    62,    62,    62,    78,    66,    66,    66,   419,   140,
     421,    10,    11,   144,   425,    16,   427,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    81,    49,    50,
      51,     1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,   259,    61,    81,    63,   263,    67,    68,    69,    70,
      71,    72,     1,    66,     3,     4,     5,     6,    28,     8,
       9,    62,    83,    84,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    64,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    80,
      49,    50,    51,     1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    61,    84,    63,   236,   237,    67,    68,
      69,    70,    71,    72,     1,    54,     3,     4,     5,     6,
      28,     8,     9,     3,    83,    84,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      64,    82,    66,    14,    15,    62,     3,    82,    82,    80,
       1,     3,     3,     4,     5,     6,    62,     8,     9,    61,
      61,    81,    49,    50,    51,    62,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    64,    84,    62,    66,    62,
      67,    68,    69,    70,    71,    72,     1,    64,     3,     4,
       5,     6,    52,     8,     9,   244,    83,    84,    49,    50,
      51,    82,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    52,    82,   245,   247,   130,    67,    68,    69,    70,
      71,    72,     1,   248,     3,     4,     5,     6,   246,     8,
       9,   150,    83,    84,   189,    36,   120,    -1,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    -1,    83,    84,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    -1,    -1,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    68,    69,    70,    -1,    -1,    73,
      74,    75,    76,    -1,    83,    79,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     1,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    -1,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      83,   183,   184,    -1,    -1,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,   238,   239,   240,   241,
       1,    83,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    12,    13,
      14,    15,    -1,    61,    -1,    -1,    -1,    -1,    83,    67,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,
      61,    -1,    -1,    -1,    -1,    83,    67,    68,    69,    70,
      71,    72,    68,    69,    70,    -1,    -1,    73,    74,    75,
      76,    82,    -1,    79,    -1,    -1,    82,    -1,    84,    -1,
      -1,    -1,    66,    -1,    68,    69,    70,    -1,    -1,    73,
      74,    75,    76,    -1,     1,    79,     3,     4,     5,     6,
      84,     8,     9,    -1,    -1,    -1,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    61,    -1,    -1,    64,
      -1,    -1,    67,    68,    69,    70,    71,    72,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
       1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    61,    62,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,     1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    64,    -1,    -1,    67,    68,    69,    70,    71,    72,
      61,    -1,    -1,    64,    -1,    -1,    67,    68,    69,    70,
      71,    72,     1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    61,    62,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    61,    -1,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,     1,    -1,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,     1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,     1,    -1,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    61,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,     1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    61,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
       1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    61,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,     1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      61,    -1,    -1,    12,    13,    -1,    67,    68,    69,    70,
      71,    72,    12,    13,    14,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    61,    12,    13,    14,    15,
      -1,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    12,    13,    14,    15,    68,
      69,    70,    -1,    -1,    73,    74,    75,    76,    68,    69,
      70,    -1,    -1,    73,    74,    75,    76,    -1,    -1,    79,
      -1,    -1,    82,    -1,    -1,    -1,    62,    12,    13,    14,
      15,    -1,    68,    69,    70,    -1,    -1,    73,    74,    75,
      76,    -1,    -1,    79,    -1,    -1,    82,    -1,    -1,    -1,
      -1,    68,    69,    70,    -1,    -1,    73,    74,    75,    76,
      -1,     1,    79,     3,    -1,    82,     3,     4,     5,     6,
      -1,     8,     9,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    68,    69,    70,    -1,    -1,    73,    74,
      75,    76,    -1,    -1,    79,    -1,    -1,    82,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    61,    62,    63,    61,    -1,    66,    -1,    68,    -1,
      67,    68,    69,    70,    71,    72,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,     0,     1,    -1,     3,    61,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    61,    12,
      13,    14,    15,    -1,    67,    68,    69,    70,    71,    72,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      68,    69,    70,    -1,    -1,    73,    74,    75,    76,    -1,
      -1,    79,    80,    61,     1,    -1,     3,    -1,    -1,    -1,
      68,    12,    13,    14,    15,    68,    69,    70,    -1,    -1,
      73,    74,    75,    76,    -1,    -1,    79,    80,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    62,    63,    68,    69,    70,
      -1,    68,    73,    74,    75,    76,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     1,    -1,
       3,    -1,    -1,    -1,    -1,    12,    13,    14,    15,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,
      12,    13,    14,    15,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    12,    13,    14,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,    61,    -1,
      -1,    68,    69,    70,    -1,    68,    73,    74,    75,    76,
      62,    -1,    79,    -1,    -1,    -1,    68,    69,    70,    -1,
      -1,    73,    74,    75,    76,    -1,    -1,    79,    12,    13,
      14,    15,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    -1,    -1,    73,    74,    75,    76,
      -1,    -1,    79,    12,    13,    14,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    68,    69,    70,    -1,    -1,    73,
      74,    75,    76,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    68,
      69,    70,    -1,    -1,    73,    74,    75,    76,    64,     1,
      79,     3,    68,    69,    70,    -1,    -1,    73,    74,    75,
      76,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    83,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    83,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    28,    -1,    -1,    -1,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    28,    -1,    -1,    -1,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    61,    68,   107,   108,   111,   112,   113,
     114,   120,   123,   124,   125,   126,   146,   147,   148,    61,
      83,   107,   108,   139,   140,     3,    83,     1,   124,     1,
     123,   126,   127,     1,    82,   109,   110,   124,   108,   108,
       3,    83,   108,     1,   139,   140,    61,    63,     1,   125,
       0,   147,     1,     1,     3,     4,     5,     6,     8,     9,
      49,    50,    51,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    67,    68,    69,    70,    71,    72,    84,    86,
      87,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   105,   137,   138,   139,
     140,   141,   142,   143,   144,   145,     1,   124,   107,   139,
      83,     1,     3,   121,   122,    62,    62,   126,   123,   126,
      66,    81,    82,   139,    66,    81,    82,    81,   139,   140,
      83,     1,   112,   115,   116,   117,   123,   139,   139,     1,
       3,    62,   108,   128,   129,   130,   131,     1,     3,    64,
      89,   102,   106,    62,    12,    13,    14,    15,    68,    69,
      70,    73,    74,    75,    76,    79,    82,    84,    80,    61,
      89,    61,    89,    89,   106,    80,    61,    61,    61,     1,
     137,    61,     3,    82,    82,     1,    82,   105,     1,   105,
     117,   132,     7,     8,     9,    61,    63,    65,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    81,   104,
      91,    68,    73,    74,    69,    70,    10,    11,    12,    13,
      75,    76,    14,    15,    67,    77,    78,    16,    17,    79,
      66,    82,     1,    84,   141,    84,   137,     1,   121,    66,
      81,    84,    81,    66,    84,     1,   110,    83,   103,   135,
     110,   103,   135,   139,     1,   115,    82,    84,   117,     1,
      84,   116,     1,    80,   118,   119,   124,   117,    62,    66,
      61,    63,   124,   126,   133,   134,    62,    66,    62,    66,
      64,     1,    94,    94,     1,    95,    95,    91,     1,    92,
      92,    91,    91,    94,    94,     1,   105,   137,   132,    80,
     137,     1,   105,     1,   105,   105,    54,    54,     1,   142,
      82,    82,    82,    62,    62,    61,   126,   133,    62,     3,
       1,    62,    88,   103,   105,     3,   103,    91,    91,    91,
      92,    92,    93,    93,    94,    94,    94,    94,    95,    95,
       1,    96,    97,    98,    99,   100,     1,   105,   103,    84,
      84,    84,     1,   122,   106,   106,   122,     1,   135,   136,
      82,    84,    84,    66,    80,   106,    66,    82,    80,    48,
     130,     1,    62,   128,   133,     1,    64,   106,   134,    61,
      63,    48,   130,     3,    80,    80,    62,   137,    62,    62,
      62,    62,    62,    61,    61,    62,   142,     1,    91,    66,
      62,    66,    64,    80,    80,    66,    84,    66,    84,     1,
     119,   106,   119,   106,    62,    62,    64,    64,     1,    62,
     128,     1,    64,   106,   102,   102,   137,   137,   137,   137,
     137,   105,   105,   137,    62,   105,   103,   103,   102,   102,
     135,    84,   135,    62,    62,    64,    64,    52,    52,    62,
      62,   137,    62,   137,   137,    82,    82,   137
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    85,    86,    86,    86,    86,    86,    87,    87,    87,
      87,    87,    87,    87,    87,    88,    88,    88,    89,    89,
      89,    89,    89,    89,    90,    90,    90,    90,    90,    90,
      91,    91,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    93,    93,    93,    94,    94,    94,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      96,    97,    97,    98,    98,    99,    99,   100,   100,   101,
     101,   102,   102,   102,   102,   102,   103,   103,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   105,
     105,   106,   107,   107,   107,   107,   108,   108,   108,   108,
     108,   108,   109,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   113,   113,   113,   113,   113,
     114,   114,   115,   115,   116,   116,   117,   117,   117,   117,
     118,   118,   118,   119,   119,   119,   119,   120,   120,   120,
     120,   120,   121,   121,   121,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   126,   126,   126,   126,   126,   127,   127,   128,
     128,   128,   129,   129,   129,   130,   130,   130,   131,   131,
     132,   132,   133,   133,   133,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   135,   135,   135,
     135,   136,   136,   136,   137,   137,   137,   137,   137,   137,
     138,   138,   138,   139,   139,   139,   139,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   143,   143,   143,   143,
     144,   144,   144,   144,   144,   144,   145,   145,   145,   145,
     145,   145,   146,   146,   147,   147,   148,   148,   148,   148,
     148,   148,   148,   148
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     3,     1,     4,     3,
       4,     3,     3,     2,     2,     1,     3,     3,     1,     2,
       2,     2,     2,     4,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     5,     5,     5,     5,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     2,     3,     5,     3,     1,     2,     1,     2,
       1,     2,     1,     3,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     5,     4,     2,     5,     4,
       1,     1,     1,     2,     3,     2,     2,     1,     2,     1,
       1,     3,     3,     1,     2,     3,     3,     4,     5,     4,
       5,     2,     1,     3,     3,     1,     3,     3,     1,     1,
       2,     1,     1,     3,     4,     3,     4,     4,     4,     4,
       3,     3,     1,     2,     2,     3,     3,     1,     2,     1,
       3,     3,     1,     3,     3,     2,     2,     1,     1,     3,
       1,     2,     1,     1,     2,     3,     2,     3,     3,     3,
       4,     2,     3,     3,     4,     4,     4,     1,     3,     4,
       3,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       3,     4,     3,     2,     3,     3,     4,     3,     1,     2,
       1,     2,     2,     2,     5,     7,     5,     5,     7,     5,
       5,     7,     6,     7,     7,     5,     3,     2,     2,     2,
       3,     3,     1,     2,     1,     1,     4,     3,     3,     3,
       3,     2,     3,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {

#line 2174 "grammar.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 473 "grammar.y"

#include <stdio.h>
#include <string.h>
#include "main.h"
#define RESET       "\033[0m"
#define KNRM        "\x1B[0m"
#define KRED        "\x1B[31m"
#define KGRN        "\x1B[32m"
#define KYEL        "\x1B[33m"
#define KBLU        "\x1B[34m"
#define KMAG        "\x1B[35m"
#define KCYN        "\x1B[36m"
#define KWHT        "\x1B[37m"
#define BOLDBLACK   "\033[1m\033[30m"
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

extern int column;
extern int yylineno;
extern int yyleng;
extern char* yytext;
extern char* line_string;
extern void fill_line();
extern void clear_line_string();
extern char* concat(char *s1,char *s2);

extern char* current_file[MAX_INCL];
extern int current_file_i;

int recovering()
  {
  //YYABORT;
  return 0;
  }


void yyerror(const char *s)
{
  /* error_tpye/error_message? */
  fflush(stdout);
  char* original_line_string;
  original_line_string = concat("", line_string);
  //printf("original string:\n%s\n",original_line_string);
  fill_line();
  //printf("\n%*s\n%*s\n", column, "^", column, s);

  char* underline;
  int i;
  underline =malloc(1);
  strcpy(underline,"");
  char *squiggly = malloc(2*sizeof(char));
  squiggly[0] = '~';
  squiggly[1] = '\0';
  for(i=1;i<yyleng;++i)
    {
    char* temp;
    temp = concat(underline,squiggly);
    free(underline);
    underline=temp;
    }
  printf(
        BOLDMAGENTA "\n%s:"           RESET
        BOLDBLACK   "\nl:%d c:%d"     RESET
                    "::= %s at "      RESET
        BOLDMAGENTA "%s\n"            RESET
                    "\n%s"            RESET
        BOLDRED     "\n%*s"           RESET
        BOLDRED     "%s\n"            RESET
        , current_file[current_file_i]
        , (yylineno-1), column
        , s
        , yytext
        , line_string
        , (column-yyleng+1), "^"
        , underline
        );
  free(underline);
  free(line_string);
  line_string = concat("", original_line_string);
  free(original_line_string);
}
