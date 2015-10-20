
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 18 "parser/parser.y"

   extern int yylex ();
   #include <parser/parser.h>
   #include <iostream>
   #include <list>
   #include <stack>
   #define YYERROR_VERBOSE
   #define YYDEBUG 1

   //Required for yyerror below
   void yyerror(const char*);
   extern char *yytext;
   extern int yylineno;

   using namespace iegenlib;



/* Line 189 of yacc.c  */
#line 92 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LBRACE = 258,
     RBRACE = 259,
     LBRACKET = 260,
     RBRACKET = 261,
     LPAREN = 262,
     RPAREN = 263,
     COMMA = 264,
     COLON = 265,
     LT = 266,
     LTE = 267,
     GT = 268,
     GTE = 269,
     SEMI = 270,
     OR = 271,
     UNION = 272,
     INVERSE = 273,
     EXISTS = 274,
     EQ = 275,
     ARROW = 276,
     ID = 277,
     INT = 278,
     INVALID_ID = 279,
     DASH = 280,
     PLUS = 281,
     STAR = 282,
     UMINUS = 283,
     WAND = 284,
     AND = 285
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 222 of yacc.c  */
#line 36 "parser/parser.y"

   std::string*                           sval;
   int                                    ival;
   iegenlib::TupleDecl*                   tdecl;
   iegenlib::Environment*                 env;
   iegenlib::Set*                         set;
   iegenlib::Relation*                    relation;
   iegenlib::Conjunction*                 conj;
   std::list<iegenlib::Conjunction*>*     conjlist;
   iegenlib::Exp*                         exp;
   std::list<iegenlib::Exp*>*             explist;
   std::list<std::string>*                symlist;
   std::list<std::string>*                existslist;



/* Line 222 of yacc.c  */
#line 175 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 187 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  14
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   129

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  31
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNRULES -- Number of states.  */
#define YYNSTATES  141

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   285

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    24,
      27,    34,    37,    44,    46,    50,    55,    57,    61,    68,
      72,    74,    77,    81,    83,    88,    92,    97,    98,   100,
     104,   107,   108,   112,   116,   118,   122,   124,   126,   128,
     131,   135,   136,   138,   142,   146,   148,   150,   152,   154,
     156,   160,   164,   168,   172,   176,   178,   180,   182,   184,
     186,   188,   192,   194,   197,   201,   205,   208,   212,   216,
     218,   220,   222,   224,   229,   237,   242,   246
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      32,     0,    -1,    33,    -1,    34,    -1,    36,    -1,    35,
      -1,    37,    -1,     1,    -1,    22,     7,     8,    20,    18,
      22,     7,     8,    -1,    46,    38,    -1,    46,     3,    43,
      15,    42,     4,    -1,    46,    40,    -1,    46,     3,    45,
      15,    44,     4,    -1,    39,    -1,    38,    17,    39,    -1,
       3,    48,    52,     4,    -1,    41,    -1,    40,    17,    41,
      -1,     3,    48,    21,    48,    52,     4,    -1,    43,    15,
      42,    -1,    43,    -1,    48,    52,    -1,    45,    15,    44,
      -1,    45,    -1,    48,    21,    48,    52,    -1,    48,    21,
       1,    -1,     5,    47,     6,    21,    -1,    -1,    22,    -1,
      22,     9,    47,    -1,     5,     6,    -1,    -1,     5,    49,
       6,    -1,     5,    49,     1,    -1,    50,    -1,    49,     9,
      50,    -1,    51,    -1,    23,    -1,    22,    -1,    10,    53,
      -1,    10,    53,     1,    -1,    -1,    54,    -1,    53,    30,
      54,    -1,    53,    29,    54,    -1,    55,    -1,    58,    -1,
      59,    -1,    56,    -1,    57,    -1,    60,    20,    60,    -1,
      60,    11,    60,    -1,    60,    12,    60,    -1,    60,    13,
      60,    -1,    60,    14,    60,    -1,    62,    -1,    66,    -1,
      63,    -1,    64,    -1,    65,    -1,    61,    -1,     7,    69,
       8,    -1,    23,    -1,    25,    60,    -1,    60,    26,    60,
      -1,    60,    25,    60,    -1,    23,    66,    -1,    23,    27,
      66,    -1,    66,    27,    23,    -1,    67,    -1,    68,    -1,
      61,    -1,    22,    -1,    51,     7,    69,     8,    -1,    51,
       7,    69,     8,     5,    23,     6,    -1,    51,     7,    69,
       1,    -1,    69,     9,    60,    -1,    60,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   144,   144,   147,   150,   153,   156,   159,   163,   171,
     184,   201,   216,   233,   241,   247,   267,   275,   281,   304,
     310,   318,   339,   345,   353,   375,   383,   387,   389,   397,
     404,   410,   414,   417,   425,   429,   436,   442,   448,   453,
     456,   467,   470,   477,   483,   489,   492,   495,   498,   501,
     504,   512,   521,   529,   538,   546,   549,   552,   555,   558,
     561,   564,   585,   592,   598,   604,   611,   617,   623,   629,
     632,   635,   638,   653,   667,   681,   691,   697
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LBRACE", "RBRACE", "LBRACKET",
  "RBRACKET", "LPAREN", "RPAREN", "COMMA", "COLON", "LT", "LTE", "GT",
  "GTE", "SEMI", "OR", "UNION", "INVERSE", "EXISTS", "EQ", "ARROW", "ID",
  "INT", "INVALID_ID", "DASH", "PLUS", "STAR", "UMINUS", "WAND", "AND",
  "$accept", "Start", "environment", "omega_set", "isl_set",
  "omega_relation", "isl_relation", "conjunct_list", "single_conjunct",
  "rel_conjunct_list", "rel_single_conjunct", "semi_conjunct_list",
  "semi_conjunct", "semi_rel_conjunct_list", "semi_rel_conjunct",
  "in_symbolic", "symbol_list", "tuple_decl_with_brackets", "tuple_decl",
  "tuple_elem", "variable_id", "optional_constraints", "constraint_list",
  "constraint", "constraint_eq", "constraint_lt", "constraint_lte",
  "constraint_gt", "constraint_gte", "expression", "expression_tuple",
  "expression_int", "expression_unop", "expression_binop",
  "expression_int_mult", "expression_simple", "expression_id",
  "expression_func", "expression_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    31,    32,    32,    32,    32,    32,    32,    33,    34,
      35,    36,    37,    38,    38,    39,    40,    40,    41,    42,
      42,    43,    44,    44,    45,    45,    46,    46,    47,    47,
      48,    48,    48,    48,    49,    49,    50,    50,    51,    52,
      52,    52,    53,    53,    53,    54,    54,    54,    54,    54,
      55,    56,    57,    58,    59,    60,    60,    60,    60,    60,
      60,    61,    62,    63,    64,    64,    65,    65,    65,    66,
      66,    66,    67,    68,    68,    68,    69,    69
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     8,     2,
       6,     2,     6,     1,     3,     4,     1,     3,     6,     3,
       1,     2,     3,     1,     4,     3,     4,     0,     1,     3,
       2,     0,     3,     3,     1,     3,     1,     1,     1,     2,
       3,     0,     1,     3,     3,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     3,     3,     2,     3,     3,     1,
       1,     1,     1,     4,     7,     4,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     7,     0,     0,     0,     2,     3,     5,     4,     6,
       0,    28,     0,     0,     1,    31,     9,    13,    11,    16,
       0,     0,     0,     0,     0,     0,    41,     0,     0,    29,
      26,     0,    30,    38,    37,     0,    34,    36,    31,    31,
       0,     0,    21,    31,    14,    31,    17,     0,    33,    32,
       0,     0,    20,    41,     0,    23,     0,     0,    72,    62,
       0,     0,     0,    42,    45,    48,    49,    46,    47,     0,
      60,    55,    57,    58,    59,    56,    69,    70,    25,    41,
      15,    41,     0,     0,    35,    10,    31,    21,    12,    31,
       0,    77,     0,     0,    71,    66,    63,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
       0,    31,     0,    19,    22,    41,    61,     0,    67,     0,
      44,    43,    51,    52,    53,    54,    50,    65,    64,    68,
      18,    41,     8,    24,    76,    75,    73,     0,     0,     0,
      74
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,     7,     8,     9,    16,    17,    18,
      19,    51,    52,    54,    55,    10,    12,    53,    35,    36,
      61,    42,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    92
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -57
static const yytype_int8 yypact[] =
{
       9,   -57,   -11,     6,    43,   -57,   -57,   -57,   -57,   -57,
      32,    36,    55,    65,   -57,    72,    61,   -57,    68,   -57,
     -11,    66,    70,    35,    71,    73,    34,    88,    89,   -57,
     -57,    75,   -57,   -57,   -57,    16,   -57,   -57,    72,    72,
      31,    79,    91,    72,   -57,    72,   -57,    76,   -57,   -57,
      -7,    93,    84,    90,    97,    87,    82,    31,    98,    12,
      31,    99,     3,   -57,   -57,   -57,   -57,   -57,   -57,    56,
      77,   -57,   -57,   -57,   -57,    80,   -57,   -57,   -57,    90,
     -57,    90,    92,   101,   -57,   -57,    72,   -57,   -57,    72,
      79,    -5,    51,     1,   -57,   -57,   -57,    31,   -57,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    86,   106,
      91,    72,   103,   -57,   -57,    90,   -57,    31,   -57,    63,
     -57,   -57,    -5,    -5,    -5,    -5,    -5,   -57,   -57,   -57,
     -57,    90,   -57,   -57,    -5,   -57,   107,   106,    94,   108,
     -57
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,    95,   -57,
      96,    29,   104,    27,   105,   -57,   109,   -15,   -57,    78,
     -14,   -52,   -57,   -34,   -57,   -57,   -57,   -57,   -57,   -55,
     -56,   -57,   -57,   -57,   -57,   -53,   -57,   -57,    21
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -72
static const yytype_int16 yytable[] =
{
      26,    87,    91,    94,    98,    96,    95,   -39,    57,    37,
       1,    11,   -27,    13,     2,    33,    34,    48,   -39,    57,
     106,   107,    49,    58,    56,    50,    79,   109,    81,   110,
      82,     3,    99,   100,    58,    15,    37,    94,    57,    93,
     118,    32,    91,    14,    40,    20,   122,   123,   124,   125,
     126,   127,   128,    58,    59,    41,    60,    33,    34,   116,
     117,    21,   134,   133,   135,   120,   121,   101,   102,   103,
     104,   136,   117,    22,    56,   115,   105,    23,    27,   137,
      78,   106,   107,   -31,    23,    28,    38,    30,    39,   -31,
      31,    43,    45,    47,   -31,    80,   131,    85,    83,    86,
      40,    88,    89,    90,   -71,   -38,    97,   108,   112,   129,
     130,   132,   138,   111,   140,   113,   114,   139,   119,    24,
      25,     0,    44,     0,    46,     0,     0,     0,    84,    29
};

static const yytype_int16 yycheck[] =
{
      15,    53,    57,    59,     1,    60,    59,     4,     7,    23,
       1,    22,     3,     7,     5,    22,    23,     1,    15,     7,
      25,    26,     6,    22,    39,     9,    41,    79,    43,    81,
      45,    22,    29,    30,    22,     3,    50,    93,     7,    27,
      93,     6,    97,     0,    10,     9,   101,   102,   103,   104,
     105,   106,   107,    22,    23,    21,    25,    22,    23,     8,
       9,     6,   117,   115,     1,    99,   100,    11,    12,    13,
      14,     8,     9,     8,    89,    90,    20,     5,    17,   131,
       1,    25,    26,     4,     5,    17,    15,    21,    15,    10,
      20,     3,     3,    18,    15,     4,   111,     4,    22,    15,
      10,     4,    15,    21,    27,     7,     7,    27,     7,    23,
       4,     8,     5,    21,     6,    86,    89,    23,    97,    15,
      15,    -1,    27,    -1,    28,    -1,    -1,    -1,    50,    20
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     5,    22,    32,    33,    34,    35,    36,    37,
      46,    22,    47,     7,     0,     3,    38,    39,    40,    41,
       9,     6,     8,     5,    43,    45,    48,    17,    17,    47,
      21,    20,     6,    22,    23,    49,    50,    51,    15,    15,
      10,    21,    52,     3,    39,     3,    41,    18,     1,     6,
       9,    42,    43,    48,    44,    45,    48,     7,    22,    23,
      25,    51,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     1,    48,
       4,    48,    48,    22,    50,     4,    15,    52,     4,    15,
      21,    60,    69,    27,    61,    66,    60,     7,     1,    29,
      30,    11,    12,    13,    14,    20,    25,    26,    27,    52,
      52,    21,     7,    42,    44,    48,     8,     9,    66,    69,
      54,    54,    60,    60,    60,    60,    60,    60,    60,    23,
       4,    48,     8,    52,    60,     1,     8,    52,     5,    23,
       6
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
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



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
	    /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 22: /* "ID" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->sval));};

/* Line 1000 of yacc.c  */
#line 1211 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 23: /* "INT" */

/* Line 1000 of yacc.c  */
#line 133 "parser/parser.y"
	{};

/* Line 1000 of yacc.c  */
#line 1220 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 24: /* "INVALID_ID" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->sval));};

/* Line 1000 of yacc.c  */
#line 1229 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 33: /* "environment" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->env));};

/* Line 1000 of yacc.c  */
#line 1238 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 34: /* "omega_set" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->set));};

/* Line 1000 of yacc.c  */
#line 1247 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 35: /* "isl_set" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->set));};

/* Line 1000 of yacc.c  */
#line 1256 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 36: /* "omega_relation" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->relation));};

/* Line 1000 of yacc.c  */
#line 1265 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 37: /* "isl_relation" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->relation));};

/* Line 1000 of yacc.c  */
#line 1274 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 38: /* "conjunct_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conjlist));};

/* Line 1000 of yacc.c  */
#line 1283 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 39: /* "single_conjunct" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conj));};

/* Line 1000 of yacc.c  */
#line 1292 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 40: /* "rel_conjunct_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conjlist));};

/* Line 1000 of yacc.c  */
#line 1301 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 41: /* "rel_single_conjunct" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conj));};

/* Line 1000 of yacc.c  */
#line 1310 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 42: /* "semi_conjunct_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conjlist));};

/* Line 1000 of yacc.c  */
#line 1319 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 43: /* "semi_conjunct" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conj));};

/* Line 1000 of yacc.c  */
#line 1328 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 44: /* "semi_rel_conjunct_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conjlist));};

/* Line 1000 of yacc.c  */
#line 1337 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 45: /* "semi_rel_conjunct" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->conj));};

/* Line 1000 of yacc.c  */
#line 1346 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 46: /* "in_symbolic" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->symlist));};

/* Line 1000 of yacc.c  */
#line 1355 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 47: /* "symbol_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->symlist));};

/* Line 1000 of yacc.c  */
#line 1364 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 48: /* "tuple_decl_with_brackets" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->tdecl));};

/* Line 1000 of yacc.c  */
#line 1373 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 49: /* "tuple_decl" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->tdecl));};

/* Line 1000 of yacc.c  */
#line 1382 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 50: /* "tuple_elem" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->tdecl));};

/* Line 1000 of yacc.c  */
#line 1391 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 51: /* "variable_id" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->sval));};

/* Line 1000 of yacc.c  */
#line 1400 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 52: /* "optional_constraints" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->explist));};

/* Line 1000 of yacc.c  */
#line 1409 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 53: /* "constraint_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->explist));};

/* Line 1000 of yacc.c  */
#line 1418 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 54: /* "constraint" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1427 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 55: /* "constraint_eq" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1436 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 56: /* "constraint_lt" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1445 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 57: /* "constraint_lte" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1454 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 58: /* "constraint_gt" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1463 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 59: /* "constraint_gte" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1472 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 60: /* "expression" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1481 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 61: /* "expression_tuple" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1490 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 62: /* "expression_int" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1499 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 63: /* "expression_unop" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1508 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 64: /* "expression_binop" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1517 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 65: /* "expression_int_mult" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1526 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 66: /* "expression_simple" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1535 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 67: /* "expression_id" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1544 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 68: /* "expression_func" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->exp));};

/* Line 1000 of yacc.c  */
#line 1553 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;
      case 69: /* "expression_list" */

/* Line 1000 of yacc.c  */
#line 134 "parser/parser.y"
	{delete ((yyvaluep->explist));};

/* Line 1000 of yacc.c  */
#line 1562 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
	break;

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 145 "parser/parser.y"
    {parser::set_parse_env_result((yyvsp[(1) - (1)].env));;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 148 "parser/parser.y"
    {parser::set_parse_set_result((yyvsp[(1) - (1)].set));;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 151 "parser/parser.y"
    {parser::set_parse_relation_result((yyvsp[(1) - (1)].relation)); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 154 "parser/parser.y"
    {parser::set_parse_set_result((yyvsp[(1) - (1)].set));;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 157 "parser/parser.y"
    {parser::set_parse_relation_result((yyvsp[(1) - (1)].relation));;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 160 "parser/parser.y"
    {yyclearin;;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 164 "parser/parser.y"
    { Environment* env = new Environment();
           env->setInverse(*(yyvsp[(1) - (8)].sval),*(yyvsp[(6) - (8)].sval));
           delete (yyvsp[(1) - (8)].sval);
           delete (yyvsp[(6) - (8)].sval);
           (yyval.env) = env;
         ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 172 "parser/parser.y"
    {
            std::list<Conjunction*>::iterator it=(yyvsp[(2) - (2)].conjlist)->begin();
            std::list<Conjunction*>::iterator end=(yyvsp[(2) - (2)].conjlist)->end();
            Set* set = new Set((*it)->arity());
            for (; it!=end; ++it) {
               set->addConjunction(*it);
            }
            (yyval.set) = set;
            delete (yyvsp[(2) - (2)].conjlist);
            delete (yyvsp[(1) - (2)].symlist);
         ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 185 "parser/parser.y"
    {
            Set* set = new Set((yyvsp[(3) - (6)].conj)->arity());
            set->addConjunction((yyvsp[(3) - (6)].conj));

            std::list<Conjunction*>::iterator it=(yyvsp[(5) - (6)].conjlist)->begin();
            std::list<Conjunction*>::iterator end=(yyvsp[(5) - (6)].conjlist)->end();
            for (; it!=end; ++it) {
               set->addConjunction(*it);
            }
            (yyval.set) = set;
            delete (yyvsp[(5) - (6)].conjlist);
            delete (yyvsp[(1) - (6)].symlist);
         ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 202 "parser/parser.y"
    {
            std::list<Conjunction*>::iterator it=(yyvsp[(2) - (2)].conjlist)->begin();
            std::list<Conjunction*>::iterator end=(yyvsp[(2) - (2)].conjlist)->end();
            int inarity = (*it)->inarity();
            int outarity = (*it)->arity()-inarity;
            Relation* relation = new Relation(inarity,outarity);
            for (; it!=end; ++it) {
               relation->addConjunction(*it);
            }
            (yyval.relation) = relation;
            delete (yyvsp[(2) - (2)].conjlist);
            delete (yyvsp[(1) - (2)].symlist);
         ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 217 "parser/parser.y"
    {
            int inarity = (yyvsp[(3) - (6)].conj)->inarity();
            int outarity = (yyvsp[(3) - (6)].conj)->arity()-inarity;
            Relation* relation = new Relation(inarity,outarity);
            relation->addConjunction((yyvsp[(3) - (6)].conj));

            std::list<Conjunction*>::iterator it=(yyvsp[(5) - (6)].conjlist)->begin();
            std::list<Conjunction*>::iterator end=(yyvsp[(5) - (6)].conjlist)->end();
            for (; it!=end; ++it) {
               relation->addConjunction(*it);
            }
            (yyval.relation) = relation;
            delete (yyvsp[(5) - (6)].conjlist);
            delete (yyvsp[(1) - (6)].symlist);
         ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 234 "parser/parser.y"
    {
            std::list<Conjunction*>* conlist=
               new std::list<Conjunction*>();
            conlist->push_back((yyvsp[(1) - (1)].conj));
            (yyval.conjlist) = conlist;
         ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 242 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].conjlist)->push_back((yyvsp[(3) - (3)].conj));
            (yyval.conjlist)=(yyvsp[(1) - (3)].conjlist);
         ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 248 "parser/parser.y"
    {
            Conjunction* conj = new Conjunction((yyvsp[(2) - (4)].tdecl)->size());
            conj->setTupleDecl( *((yyvsp[(2) - (4)].tdecl)) );
            delete (yyvsp[(2) - (4)].tdecl);
            
            std::list<Exp*>::iterator it2 = (yyvsp[(3) - (4)].explist)->begin();
            std::list<Exp*>::iterator end2 = (yyvsp[(3) - (4)].explist)->end();
            for (; it2 != end2; ++it2) {
               if ((*it2)->isEquality())
                  { conj->addEquality(*it2); }
               else if ((*it2)->isInequality())
                  { conj->addInequality(*it2); }
            }
            
            conj->substituteTupleDecl(); 
            (yyval.conj) = conj;
            delete (yyvsp[(3) - (4)].explist);
         ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 268 "parser/parser.y"
    {
            std::list<Conjunction*>* conlist=
               new std::list<Conjunction*>();
            conlist->push_back((yyvsp[(1) - (1)].conj));
            (yyval.conjlist) = conlist;
         ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 276 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].conjlist)->push_back((yyvsp[(3) - (3)].conj));
            (yyval.conjlist)=(yyvsp[(1) - (3)].conjlist);
         ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 282 "parser/parser.y"
    {
            int arity = (yyvsp[(2) - (6)].tdecl)->size() + (yyvsp[(4) - (6)].tdecl)->size(), inarity = (yyvsp[(2) - (6)].tdecl)->size();
            Conjunction* conj = new Conjunction(arity, inarity);
            conj->setTupleDecl( *((yyvsp[(2) - (6)].tdecl)), *((yyvsp[(4) - (6)].tdecl)) );
            delete (yyvsp[(2) - (6)].tdecl);
            delete (yyvsp[(4) - (6)].tdecl);
            
            std::list<Exp*>::iterator it2 = (yyvsp[(5) - (6)].explist)->begin();
            std::list<Exp*>::iterator end2 = (yyvsp[(5) - (6)].explist)->end();
            for (; it2 != end2; ++it2) {
               if ((*it2)->isEquality())
                  { conj->addEquality(*it2); }
               else if ((*it2)->isInequality())
                  { conj->addInequality(*it2); }
            }
            
            conj->substituteTupleDecl();
            (yyval.conj) = conj;

            delete (yyvsp[(5) - (6)].explist);
         ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 305 "parser/parser.y"
    { 
            (yyvsp[(3) - (3)].conjlist)->push_back((yyvsp[(1) - (3)].conj)); 
            (yyval.conjlist) = (yyvsp[(3) - (3)].conjlist); 
         ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 311 "parser/parser.y"
    {
            std::list<Conjunction*>* conlist = 
               new std::list<Conjunction*>();
            conlist->push_back((yyvsp[(1) - (1)].conj));
            (yyval.conjlist) = conlist;
         ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 319 "parser/parser.y"
    {
            Conjunction* conj = new Conjunction((yyvsp[(1) - (2)].tdecl)->size());
            conj->setTupleDecl(*((yyvsp[(1) - (2)].tdecl)));
            delete (yyvsp[(1) - (2)].tdecl);

            std::list<Exp*>::iterator it2 = (yyvsp[(2) - (2)].explist)->begin();
            std::list<Exp*>::iterator end2 = (yyvsp[(2) - (2)].explist)->end();
            for (; it2 != end2; ++it2) {
               if ((*it2)->isEquality())
                  { conj->addEquality(*it2); }
               else if ((*it2)->isInequality())
                  { conj->addInequality(*it2); }
            }
            
            conj->substituteTupleDecl(); 
            (yyval.conj) = conj;
            delete (yyvsp[(2) - (2)].explist);

         ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 340 "parser/parser.y"
    { 
            (yyvsp[(3) - (3)].conjlist)->push_back((yyvsp[(1) - (3)].conj)); 
            (yyval.conjlist) = (yyvsp[(3) - (3)].conjlist); 
         ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 346 "parser/parser.y"
    {
            std::list<Conjunction*>* conlist = 
               new std::list<Conjunction*>();
            conlist->push_back((yyvsp[(1) - (1)].conj));
            (yyval.conjlist) = conlist;
         ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 354 "parser/parser.y"
    {
            int arity = (yyvsp[(1) - (4)].tdecl)->size() + (yyvsp[(3) - (4)].tdecl)->size(), inarity = (yyvsp[(3) - (4)].tdecl)->size();
            Conjunction* conj = new Conjunction(arity, inarity);
            conj->setTupleDecl(*((yyvsp[(1) - (4)].tdecl)),*((yyvsp[(3) - (4)].tdecl)));
            delete (yyvsp[(1) - (4)].tdecl);
            delete (yyvsp[(3) - (4)].tdecl);

            std::list<Exp*>::iterator it2 = (yyvsp[(4) - (4)].explist)->begin();
            std::list<Exp*>::iterator end2 = (yyvsp[(4) - (4)].explist)->end();
            for (; it2 != end2; ++it2) {
               if ((*it2)->isEquality())
                  { conj->addEquality(*it2); }
               else if ((*it2)->isInequality())
                  { conj->addInequality(*it2); }
            }
            
            conj->substituteTupleDecl();
            (yyval.conj) = conj;
            delete (yyvsp[(4) - (4)].explist);

         ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 376 "parser/parser.y"
    {
            delete (yyvsp[(1) - (3)].tdecl);
            (yyval.conj) =  new Conjunction(0,0);
            yyclearin;
      ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 384 "parser/parser.y"
    {(yyval.symlist) = (yyvsp[(2) - (4)].symlist);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 387 "parser/parser.y"
    {(yyval.symlist) = new std::list<std::string>();;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 390 "parser/parser.y"
    {
            std::list<std::string>* symbols = new std::list<std::string>();
            symbols->push_back(*(yyvsp[(1) - (1)].sval));
            (yyval.symlist) = symbols;
            delete (yyvsp[(1) - (1)].sval);
         ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 398 "parser/parser.y"
    {
            (yyvsp[(3) - (3)].symlist)->push_back(*(yyvsp[(1) - (3)].sval));
            (yyval.symlist) = (yyvsp[(3) - (3)].symlist);
            delete (yyvsp[(1) - (3)].sval);
         ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 405 "parser/parser.y"
    { 
            (yyval.tdecl) = new TupleDecl();
         ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 410 "parser/parser.y"
    {
            (yyval.tdecl) = new TupleDecl();
         ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 415 "parser/parser.y"
    { (yyval.tdecl)= (yyvsp[(2) - (3)].tdecl); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 418 "parser/parser.y"
    {
            delete (yyvsp[(2) - (3)].tdecl);
            (yyval.tdecl) = new TupleDecl(0);
            yyclearin;
         ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 426 "parser/parser.y"
    {
            (yyval.tdecl) = (yyvsp[(1) - (1)].tdecl);
         ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 430 "parser/parser.y"
    {
            (yyval.tdecl) = (yyvsp[(1) - (3)].tdecl);
            (yyval.tdecl)->concat(*(yyvsp[(3) - (3)].tdecl));
            delete (yyvsp[(3) - (3)].tdecl);
         ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 437 "parser/parser.y"
    { (yyval.tdecl) = new TupleDecl(1);
           (yyval.tdecl)->setTupleElem(0, *(yyvsp[(1) - (1)].sval));
           delete (yyvsp[(1) - (1)].sval);
         ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 443 "parser/parser.y"
    { (yyval.tdecl) = new TupleDecl(1);
           (yyval.tdecl)->setTupleElem(0, (yyvsp[(1) - (1)].ival));
         ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 449 "parser/parser.y"
    {(yyval.sval) = (yyvsp[(1) - (1)].sval);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 454 "parser/parser.y"
    {(yyval.explist) = (yyvsp[(2) - (2)].explist);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 457 "parser/parser.y"
    {
            for (std::list<Exp*>::iterator it=(yyvsp[(2) - (3)].explist)->begin(); it!=(yyvsp[(2) - (3)].explist)->end(); ++it)
            { delete *it; }

            delete (yyvsp[(2) - (3)].explist);
            (yyval.explist) = new std::list<Exp*>();
            yyclearin;
         ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 467 "parser/parser.y"
    {(yyval.explist) = new std::list<Exp*>();;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 471 "parser/parser.y"
    {
            std::list<Exp*>* clist = new std::list<Exp*>();
            clist->push_back((yyvsp[(1) - (1)].exp));
            (yyval.explist) = clist;
         ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 478 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].explist)->push_back((yyvsp[(3) - (3)].exp));
            (yyval.explist) = (yyvsp[(1) - (3)].explist);
         ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 484 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].explist)->push_back((yyvsp[(3) - (3)].exp));
            (yyval.explist) = (yyvsp[(1) - (3)].explist);
         ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 490 "parser/parser.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 493 "parser/parser.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 496 "parser/parser.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 499 "parser/parser.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 502 "parser/parser.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 505 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].exp)->setEquality();
            (yyvsp[(3) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(1) - (3)].exp)->addExp((yyvsp[(3) - (3)].exp));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 513 "parser/parser.y"
    {
            (yyvsp[(3) - (3)].exp)->setInequality();
            (yyvsp[(1) - (3)].exp)->addTerm(new Term(1));
            (yyvsp[(1) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(3) - (3)].exp)->addExp((yyvsp[(1) - (3)].exp));
            (yyval.exp) = (yyvsp[(3) - (3)].exp);
         ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 522 "parser/parser.y"
    {
            (yyvsp[(3) - (3)].exp)->setInequality();
            (yyvsp[(1) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(3) - (3)].exp)->addExp((yyvsp[(1) - (3)].exp));
            (yyval.exp) = (yyvsp[(3) - (3)].exp);
         ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 530 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].exp)->setInequality();
            (yyvsp[(3) - (3)].exp)->addTerm(new Term(1));
            (yyvsp[(3) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(1) - (3)].exp)->addExp((yyvsp[(3) - (3)].exp));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 539 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].exp)->setInequality();
            (yyvsp[(3) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(1) - (3)].exp)->addExp((yyvsp[(3) - (3)].exp));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 547 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 550 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 553 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 556 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 559 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 562 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 565 "parser/parser.y"
    {
            // If there is only one expression then just pass
            // that up, otherwise create a TupleExpTerm and put that
            // in an Exp.
            if ((yyvsp[(2) - (3)].explist)->size() == 1) {
                (yyval.exp) = (yyvsp[(2) - (3)].explist)->front();
            } else {
                TupleExpTerm* tuple = new TupleExpTerm((yyvsp[(2) - (3)].explist)->size());
                unsigned int count = 0;
                for (std::list<Exp*>::iterator i=(yyvsp[(2) - (3)].explist)->begin(); i != (yyvsp[(2) - (3)].explist)->end();
                        ++i) {
                    tuple->setExpElem(count++,*i);
                }
                Exp* exp = new Exp();
                exp -> addTerm(tuple);
                (yyval.exp) = exp;
            }
            delete (yyvsp[(2) - (3)].explist);
         ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 586 "parser/parser.y"
    { 
            Exp* exp = new Exp();
            exp->addTerm(new Term((yyvsp[(1) - (1)].ival)));
            (yyval.exp) = exp; 
         ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 593 "parser/parser.y"
    {
            (yyvsp[(2) - (2)].exp)->multiplyBy(-1);
            (yyval.exp) = (yyvsp[(2) - (2)].exp);
         ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 599 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].exp)->addExp((yyvsp[(3) - (3)].exp));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 605 "parser/parser.y"
    {
            (yyvsp[(3) - (3)].exp)->multiplyBy(-1);
            (yyvsp[(1) - (3)].exp)->addExp((yyvsp[(3) - (3)].exp));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 612 "parser/parser.y"
    {
            (yyvsp[(2) - (2)].exp)->multiplyBy((yyvsp[(1) - (2)].ival));
            (yyval.exp) = (yyvsp[(2) - (2)].exp);
         ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 618 "parser/parser.y"
    {
            (yyvsp[(3) - (3)].exp)->multiplyBy((yyvsp[(1) - (3)].ival));
            (yyval.exp) = (yyvsp[(3) - (3)].exp);
         ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 624 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].exp)->multiplyBy((yyvsp[(3) - (3)].ival));
            (yyval.exp) = (yyvsp[(1) - (3)].exp);
         ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 630 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 633 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 636 "parser/parser.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 639 "parser/parser.y"
    {
            Exp* exp = new Exp();
            exp->addTerm(new VarTerm(*(yyvsp[(1) - (1)].sval)));
            (yyval.exp) = exp;
            delete (yyvsp[(1) - (1)].sval);
         ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 654 "parser/parser.y"
    {
            UFCallTerm* uf_call = new UFCallTerm(*(yyvsp[(1) - (4)].sval), (yyvsp[(3) - (4)].explist)->size());
            unsigned int count = 0;
            for (std::list<Exp*>::iterator i=(yyvsp[(3) - (4)].explist)->begin(); i != (yyvsp[(3) - (4)].explist)->end(); ++i) {
                uf_call->setParamExp(count++,*i);
            }
            Exp* exp = new Exp();
            exp -> addTerm(uf_call);
            (yyval.exp) = exp;
            delete (yyvsp[(1) - (4)].sval);
            delete (yyvsp[(3) - (4)].explist);
         ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 668 "parser/parser.y"
    {
            UFCallTerm* uf_call = new UFCallTerm(*(yyvsp[(1) - (7)].sval), (yyvsp[(3) - (7)].explist)->size(), (yyvsp[(6) - (7)].ival));
            unsigned int count = 0;
            for (std::list<Exp*>::iterator i=(yyvsp[(3) - (7)].explist)->begin(); i != (yyvsp[(3) - (7)].explist)->end(); ++i) {
                uf_call->setParamExp(count++,*i);
            }
            Exp* exp = new Exp();
            exp -> addTerm(uf_call);
            (yyval.exp) = exp;
            delete (yyvsp[(3) - (7)].explist);
            delete (yyvsp[(1) - (7)].sval);
         ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 682 "parser/parser.y"
    {
            for (std::list<Exp*>::iterator it=(yyvsp[(3) - (4)].explist)->begin(); it!=(yyvsp[(3) - (4)].explist)->end(); ++it)
            { delete *it; }
            (yyval.exp) = new Exp();
            delete (yyvsp[(1) - (4)].sval);
            delete (yyvsp[(3) - (4)].explist);
            yyclearin;
         ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 692 "parser/parser.y"
    {
            (yyvsp[(1) - (3)].explist)->push_back((yyvsp[(3) - (3)].exp));
            (yyval.explist) = (yyvsp[(1) - (3)].explist);
         ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 698 "parser/parser.y"
    {
            std::list<Exp*>* exp_list = new std::list<Exp*>();
            exp_list->push_back((yyvsp[(1) - (1)].exp));
            (yyval.explist) = exp_list;
         ;}
    break;



/* Line 1455 of yacc.c  */
#line 2717 "/Users/mstrout/SVNWorkDirs/IEGenProject/trunk/IEGenLib/src/parser/gen_parser.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 705 "parser/parser.y"



/*! yyerror is called when an error occurs in parsing. It prints localion of the
error as well as why the error occured to standard error. When an error occurs
we set the parse_error;
@param const char pointer
*/
#include <iostream>
#include <typeinfo>
#include <sstream>
void yyerror (const char *s)
{
   std::stringstream EM;
   EM << s << " at '" << yytext << "', line " << yylineno << std::endl;
   //sets the parse error flag to true
   parser::set_parse_error(EM.str());
   parser::clearAll();
   yyclearin;
}


