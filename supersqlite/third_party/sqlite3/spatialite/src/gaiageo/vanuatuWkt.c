/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>

/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
#define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 133
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE void *
typedef union
{
    int yyinit;
    ParseTOKENTYPE yy0;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 1000000
#endif
#define ParseARG_SDECL  struct vanuatu_data *p_data ;
#define ParseARG_PDECL , struct vanuatu_data *p_data
#define ParseARG_FETCH  struct vanuatu_data *p_data  = yypParser->p_data
#define ParseARG_STORE yypParser->p_data  = p_data
#define YYNSTATE 490
#define YYNRULE 201
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
#define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
    /*     0 */ 230, 316, 317, 318, 319, 320, 321, 322, 323, 324,
    /*    10 */ 325, 326, 327, 328, 329, 330, 331, 332, 333, 334,
    /*    20 */ 335, 336, 337, 338, 339, 340, 341, 342, 343, 344,
    /*    30 */ 345, 346, 347, 348, 490, 238, 231, 51, 93, 233,
    /*    40 */ 235, 237, 94, 46, 54, 57, 60, 63, 66, 72,
    /*    50 */ 78, 84, 90, 97, 105, 113, 122, 127, 132, 137,
    /*    60 */ 142, 147, 152, 157, 162, 179, 196, 213, 231, 231,
    /*    70 */ 101, 351, 350, 233, 47, 102, 54, 54, 234, 57,
    /*    80 */ 66, 66, 47, 72, 90, 90, 241, 97, 122, 122,
    /*    90 */ 47, 127, 142, 142, 233, 147, 177, 170, 58, 194,
    /*   100 */ 57, 235, 47, 235, 72, 692, 1, 60, 97, 60,
    /*   110 */ 59, 78, 127, 78, 47, 105, 147, 105, 253, 132,
    /*   120 */ 187, 132, 47, 152, 237, 152, 48, 211, 91, 204,
    /*   130 */ 63, 237, 254, 351, 84, 98, 47, 63, 113, 106,
    /*   140 */ 351, 84, 137, 114, 351, 113, 157, 232, 351, 137,
    /*   150 */ 228, 109, 46, 157, 49, 236, 110, 221, 49, 163,
    /*   160 */ 172, 173, 174, 175, 176, 164, 165, 166, 167, 168,
    /*   170 */ 169, 180, 189, 190, 191, 192, 193, 181, 182, 183,
    /*   180 */ 184, 185, 186, 353, 50, 197, 206, 207, 208, 209,
    /*   190 */ 210, 355, 198, 199, 200, 201, 202, 203, 18, 52,
    /*   200 */ 358, 20, 214, 223, 224, 225, 226, 227, 21, 53,
    /*   210 */ 215, 216, 217, 218, 219, 220, 239, 118, 55, 51,
    /*   220 */ 56, 46, 119, 46, 243, 46, 61, 49, 62, 49,
    /*   230 */ 245, 49, 51, 64, 65, 51, 51, 248, 249, 250,
    /*   240 */ 70, 357, 46, 46, 46, 46, 255, 76, 240, 258,
    /*   250 */ 47, 47, 49, 259, 260, 82, 49, 49, 49, 361,
    /*   260 */ 263, 264, 51, 51, 265, 88, 51, 51, 359, 23,
    /*   270 */ 242, 24, 26, 362, 27, 29, 67, 30, 69, 364,
    /*   280 */ 244, 73, 365, 34, 367, 246, 368, 79, 251, 370,
    /*   290 */ 75, 38, 81, 85, 256, 374, 68, 42, 247, 71,
    /*   300 */ 87, 261, 10, 373, 92, 74, 378, 252, 77, 267,
    /*   310 */ 377, 266, 95, 80, 382, 257, 83, 269, 381, 268,
    /*   320 */ 86, 91, 96, 262, 89, 386, 385, 11, 389, 99,
    /*   330 */ 391, 100, 270, 272, 271, 103, 98, 12, 104, 394,
    /*   340 */ 396, 107, 106, 108, 111, 274, 273, 13, 401, 277,
    /*   350 */ 120, 114, 123, 112, 275, 115, 125, 279, 399, 406,
    /*   360 */ 278, 116, 117, 276, 128, 404, 408, 130, 121, 133,
    /*   370 */ 280, 135, 138, 409, 140, 143, 145, 148, 153, 150,
    /*   380 */ 124, 126, 155, 158, 129, 131, 160, 2, 3, 4,
    /*   390 */ 134, 5, 136, 693, 693, 693, 693, 139, 693, 141,
    /*   400 */ 693, 693, 693, 144, 693, 146, 693, 6, 412, 7,
    /*   410 */ 411, 693, 149, 151, 693, 281, 415, 154, 414, 156,
    /*   420 */ 693, 693, 282, 693, 417, 159, 418, 161, 693, 693,
    /*   430 */ 693, 8, 283, 421, 420, 693, 284, 693, 423, 424,
    /*   440 */ 693, 693, 9, 693, 285, 693, 693, 427, 693, 426,
    /*   450 */ 693, 693, 286, 693, 429, 693, 693, 693, 693, 693,
    /*   460 */ 430, 693, 693, 693, 693, 287, 432, 693, 433, 693,
    /*   470 */ 434, 435, 693, 436, 437, 171, 315, 14, 438, 15,
    /*   480 */ 288, 289, 290, 291, 292, 293, 460, 178, 294, 445,
    /*   490 */ 349, 352, 16, 19, 354, 17, 693, 447, 360, 356,
    /*   500 */ 448, 693, 22, 363, 693, 449, 693, 450, 366, 25,
    /*   510 */ 451, 693, 452, 188, 453, 28, 295, 369, 296, 297,
    /*   520 */ 298, 299, 300, 195, 371, 31, 32, 33, 372, 375,
    /*   530 */ 35, 36, 37, 376, 379, 39, 301, 40, 462, 463,
    /*   540 */ 464, 465, 205, 466, 467, 41, 380, 468, 302, 303,
    /*   550 */ 304, 383, 212, 305, 43, 306, 384, 307, 44, 475,
    /*   560 */ 45, 387, 388, 390, 392, 308, 477, 393, 478, 395,
    /*   570 */ 479, 480, 397, 398, 481, 222, 482, 400, 402, 403,
    /*   580 */ 405, 483, 407, 410, 309, 413, 310, 416, 419, 422,
    /*   590 */ 425, 428, 311, 431, 439, 440, 229, 312, 313, 441,
    /*   600 */ 442, 443, 314, 444, 446, 454, 455, 456, 457, 458,
    /*   610 */ 459, 461, 469, 470, 471, 472, 473, 474, 476, 484,
    /*   620 */ 485, 486, 487, 488, 489,
};

static const YYCODETYPE yy_lookahead[] = {
    /*     0 */ 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
    /*    10 */ 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
    /*    20 */ 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
    /*    30 */ 67, 68, 69, 70, 0, 74, 2, 76, 71, 5,
    /*    40 */ 6, 7, 75, 76, 10, 11, 12, 13, 14, 15,
    /*    50 */ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    /*    60 */ 26, 27, 28, 29, 30, 31, 32, 33, 2, 2,
    /*    70 */ 72, 8, 76, 5, 76, 77, 10, 10, 72, 11,
    /*    80 */ 14, 14, 76, 15, 18, 18, 72, 19, 22, 22,
    /*    90 */ 76, 23, 26, 26, 5, 27, 30, 30, 72, 31,
    /*   100 */ 11, 6, 76, 6, 15, 35, 36, 12, 19, 12,
    /*   110 */ 72, 16, 23, 16, 76, 20, 27, 20, 72, 24,
    /*   120 */ 31, 24, 76, 28, 7, 28, 76, 32, 3, 32,
    /*   130 */ 13, 7, 72, 8, 17, 3, 76, 13, 21, 3,
    /*   140 */ 8, 17, 25, 3, 8, 21, 29, 71, 8, 25,
    /*   150 */ 33, 73, 76, 29, 76, 73, 78, 33, 76, 43,
    /*   160 */ 44, 45, 46, 47, 48, 43, 44, 45, 46, 47,
    /*   170 */ 48, 57, 58, 59, 60, 61, 62, 57, 58, 59,
    /*   180 */ 60, 61, 62, 76, 76, 50, 51, 52, 53, 54,
    /*   190 */ 55, 76, 50, 51, 52, 53, 54, 55, 3, 76,
    /*   200 */ 88, 9, 64, 65, 66, 67, 68, 69, 3, 76,
    /*   210 */ 64, 65, 66, 67, 68, 69, 71, 74, 71, 76,
    /*   220 */ 71, 76, 79, 76, 73, 76, 73, 76, 73, 76,
    /*   230 */ 74, 76, 76, 74, 74, 76, 76, 71, 71, 71,
    /*   240 */ 71, 76, 76, 76, 76, 76, 72, 72, 84, 73,
    /*   250 */ 76, 76, 76, 73, 73, 73, 76, 76, 76, 89,
    /*   260 */ 74, 74, 76, 76, 74, 74, 76, 76, 84, 9,
    /*   270 */ 85, 3, 9, 85, 3, 9, 3, 3, 9, 90,
    /*   280 */ 86, 3, 86, 3, 91, 87, 87, 3, 84, 92,
    /*   290 */ 9, 3, 9, 3, 85, 93, 96, 3, 97, 96,
    /*   300 */ 9, 86, 3, 97, 76, 98, 94, 99, 98, 76,
    /*   310 */ 99, 87, 9, 100, 95, 101, 100, 80, 101, 84,
    /*   320 */ 102, 3, 75, 103, 102, 104, 103, 3, 80, 76,
    /*   330 */ 105, 76, 76, 81, 85, 9, 3, 3, 77, 81,
    /*   340 */ 106, 76, 3, 76, 9, 86, 76, 3, 107, 87,
    /*   350 */ 9, 3, 3, 78, 82, 76, 9, 112, 82, 108,
    /*   360 */ 83, 76, 76, 76, 3, 83, 112, 9, 79, 3,
    /*   370 */ 113, 9, 3, 109, 9, 3, 9, 3, 3, 9,
    /*   380 */ 88, 88, 9, 3, 89, 89, 9, 3, 9, 3,
    /*   390 */ 90, 9, 90, 132, 132, 132, 132, 91, 132, 91,
    /*   400 */ 132, 132, 132, 92, 132, 92, 132, 3, 110, 9,
    /*   410 */ 113, 132, 93, 93, 132, 114, 111, 94, 114, 94,
    /*   420 */ 132, 132, 115, 132, 115, 95, 116, 95, 132, 132,
    /*   430 */ 132, 3, 120, 117, 120, 132, 121, 132, 121, 118,
    /*   440 */ 132, 132, 9, 132, 122, 132, 132, 119, 132, 122,
    /*   450 */ 132, 132, 123, 132, 123, 132, 132, 132, 132, 132,
    /*   460 */ 124, 132, 132, 132, 132, 128, 128, 132, 128, 132,
    /*   470 */ 128, 128, 132, 128, 128, 124, 1, 3, 128, 3,
    /*   480 */ 128, 128, 128, 128, 128, 128, 126, 124, 129, 125,
    /*   490 */ 4, 4, 3, 9, 4, 3, 132, 129, 4, 4,
    /*   500 */ 129, 132, 9, 4, 132, 129, 132, 129, 4, 9,
    /*   510 */ 129, 132, 129, 125, 129, 9, 129, 4, 129, 129,
    /*   520 */ 129, 129, 129, 125, 4, 9, 9, 9, 4, 4,
    /*   530 */ 9, 9, 9, 4, 4, 9, 130, 9, 130, 130,
    /*   540 */ 130, 130, 126, 130, 130, 9, 4, 130, 130, 130,
    /*   550 */ 130, 4, 126, 130, 9, 130, 4, 130, 9, 127,
    /*   560 */ 9, 4, 4, 4, 4, 131, 131, 4, 131, 4,
    /*   570 */ 131, 131, 4, 4, 131, 127, 131, 4, 4, 4,
    /*   580 */ 4, 131, 4, 4, 131, 4, 131, 4, 4, 4,
    /*   590 */ 4, 4, 131, 4, 4, 4, 127, 131, 131, 4,
    /*   600 */ 4, 4, 131, 4, 4, 4, 4, 4, 4, 4,
    /*   610 */ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    /*   620 */ 4, 4, 4, 4, 4,
};

#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_MAX 314
static const short yy_shift_ofst[] = {
    /*     0 */ -1, 34, 66, 67, 68, 89, 95, 97, 117, 124,
    /*    10 */ 125, 132, 136, 140, 63, 63, 63, 63, 63, 63,
    /*    20 */ 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    /*    30 */ 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    /*    40 */ 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    /*    50 */ 63, 63, 63, 63, 195, 192, 192, 205, 260, 260,
    /*    60 */ 268, 263, 263, 271, 266, 266, 273, 274, 269, 274,
    /*    70 */ 192, 269, 278, 280, 281, 280, 260, 281, 284, 288,
    /*    80 */ 283, 288, 263, 283, 290, 294, 291, 294, 266, 291,
    /*    90 */ 299, 63, 63, 192, 303, 318, 303, 324, 63, 63,
    /*   100 */ 63, 260, 326, 333, 326, 334, 63, 63, 63, 263,
    /*   110 */ 335, 339, 335, 344, 63, 63, 63, 63, 266, 341,
    /*   120 */ 348, 341, 349, 195, 347, 195, 347, 361, 205, 358,
    /*   130 */ 205, 358, 366, 268, 362, 268, 362, 369, 271, 365,
    /*   140 */ 271, 365, 372, 273, 367, 273, 367, 374, 278, 370,
    /*   150 */ 278, 370, 375, 284, 373, 284, 373, 380, 290, 377,
    /*   160 */ 290, 377, 384, 379, 379, 379, 379, 379, 379, 379,
    /*   170 */ 384, 379, 379, 379, 379, 379, 379, 384, 379, 386,
    /*   180 */ 382, 382, 382, 382, 382, 382, 382, 386, 382, 382,
    /*   190 */ 382, 382, 382, 382, 386, 382, 404, 400, 400, 400,
    /*   200 */ 400, 400, 400, 400, 404, 400, 400, 400, 400, 400,
    /*   210 */ 400, 404, 400, 428, 433, 433, 433, 433, 433, 433,
    /*   220 */ 433, 428, 433, 433, 433, 433, 433, 433, 428, 433,
    /*   230 */ 475, 474, 486, 476, 487, 489, 490, 492, 495, 484,
    /*   240 */ 494, 493, 499, 500, 504, 506, 513, 520, 516, 517,
    /*   250 */ 518, 524, 525, 521, 522, 523, 529, 530, 526, 528,
    /*   260 */ 536, 542, 547, 545, 549, 551, 552, 557, 558, 559,
    /*   270 */ 560, 563, 565, 568, 569, 573, 574, 575, 576, 578,
    /*   280 */ 579, 581, 583, 584, 585, 586, 587, 589, 590, 591,
    /*   290 */ 595, 596, 597, 599, 600, 601, 602, 603, 604, 605,
    /*   300 */ 606, 607, 608, 609, 610, 611, 612, 613, 614, 615,
    /*   310 */ 616, 617, 618, 619, 620,
};

#define YY_REDUCE_USE_DFLT (-40)
#define YY_REDUCE_MAX 229
static const short yy_reduce_ofst[] = {
    /*     0 */ 70, -37, 116, 122, 114, 120, 135, 142, 138, 146,
    /*    10 */ -33, -2, 78, 143, 76, 6, 82, -39, 145, 147,
    /*    20 */ 149, 14, 26, 38, 151, 153, 155, 156, 159, 160,
    /*    30 */ 166, 167, 168, 169, 46, 60, 174, 175, 176, 180,
    /*    40 */ 181, 182, 186, 187, 190, 191, -4, 50, 107, 108,
    /*    50 */ 115, 123, 133, 165, 112, 164, 184, 170, 185, 188,
    /*    60 */ 189, 194, 196, 193, 198, 199, 197, 200, 201, 203,
    /*    70 */ 204, 206, 202, 207, 208, 210, 209, 211, 212, 213,
    /*    80 */ 214, 216, 215, 217, 219, 218, 220, 222, 224, 223,
    /*    90 */ 221, 228, 233, 235, 237, 247, 248, 225, 253, 255,
    /*   100 */ 256, 249, 252, 261, 258, 234, 265, 267, 270, 259,
    /*   110 */ 272, 275, 276, 241, 279, 285, 286, 287, 262, 277,
    /*   120 */ 289, 282, 251, 292, 245, 293, 254, 264, 295, 257,
    /*   130 */ 296, 297, 298, 300, 301, 302, 304, 305, 306, 307,
    /*   140 */ 308, 309, 310, 311, 312, 313, 314, 316, 319, 315,
    /*   150 */ 320, 317, 321, 323, 322, 325, 327, 328, 330, 329,
    /*   160 */ 332, 331, 336, 337, 338, 340, 342, 343, 345, 346,
    /*   170 */ 351, 350, 352, 353, 354, 355, 356, 363, 357, 364,
    /*   180 */ 359, 368, 371, 376, 378, 381, 383, 388, 385, 387,
    /*   190 */ 389, 390, 391, 392, 398, 393, 360, 406, 408, 409,
    /*   200 */ 410, 411, 413, 414, 416, 417, 418, 419, 420, 423,
    /*   210 */ 425, 426, 427, 432, 434, 435, 437, 439, 440, 443,
    /*   220 */ 445, 448, 450, 453, 455, 461, 466, 467, 469, 471,
};

static const YYACTIONTYPE yy_default[] = {
    /*     0 */ 491, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*    10 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*    20 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*    30 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*    40 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*    50 */ 691, 691, 691, 691, 691, 547, 547, 691, 549, 549,
    /*    60 */ 691, 551, 551, 691, 553, 553, 691, 691, 572, 691,
    /*    70 */ 547, 572, 691, 691, 575, 691, 549, 575, 691, 691,
    /*    80 */ 578, 691, 551, 578, 691, 691, 581, 691, 553, 581,
    /*    90 */ 691, 691, 691, 547, 539, 691, 539, 691, 691, 691,
    /*   100 */ 691, 549, 541, 691, 541, 691, 691, 691, 691, 551,
    /*   110 */ 543, 691, 543, 691, 691, 691, 691, 691, 553, 545,
    /*   120 */ 691, 545, 691, 691, 600, 691, 600, 691, 691, 603,
    /*   130 */ 691, 603, 691, 691, 606, 691, 606, 691, 691, 609,
    /*   140 */ 691, 609, 691, 691, 616, 691, 616, 691, 691, 619,
    /*   150 */ 691, 619, 691, 691, 622, 691, 622, 691, 691, 625,
    /*   160 */ 691, 625, 691, 638, 638, 638, 638, 638, 638, 638,
    /*   170 */ 691, 638, 638, 638, 638, 638, 638, 691, 638, 691,
    /*   180 */ 653, 653, 653, 653, 653, 653, 653, 691, 653, 653,
    /*   190 */ 653, 653, 653, 653, 691, 653, 691, 668, 668, 668,
    /*   200 */ 668, 668, 668, 668, 691, 668, 668, 668, 668, 668,
    /*   210 */ 668, 691, 668, 691, 683, 683, 683, 683, 683, 683,
    /*   220 */ 683, 691, 683, 683, 683, 683, 683, 683, 691, 683,
    /*   230 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   240 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   250 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   260 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   270 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   280 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   290 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   300 */ 691, 691, 691, 691, 691, 691, 691, 691, 691, 691,
    /*   310 */ 691, 691, 691, 691, 691, 492, 493, 494, 495, 496,
    /*   320 */ 497, 498, 499, 500, 501, 502, 503, 504, 505, 506,
    /*   330 */ 507, 508, 509, 510, 511, 512, 513, 514, 515, 516,
    /*   340 */ 517, 518, 519, 520, 521, 522, 523, 524, 525, 526,
    /*   350 */ 534, 538, 527, 535, 528, 536, 529, 537, 555, 548,
    /*   360 */ 559, 556, 550, 560, 557, 552, 561, 558, 554, 562,
    /*   370 */ 563, 567, 571, 573, 564, 568, 574, 576, 565, 569,
    /*   380 */ 577, 579, 566, 570, 580, 582, 583, 530, 587, 540,
    /*   390 */ 591, 584, 531, 588, 542, 592, 585, 532, 589, 544,
    /*   400 */ 593, 586, 533, 590, 546, 594, 595, 599, 601, 596,
    /*   410 */ 602, 604, 597, 605, 607, 598, 608, 610, 611, 615,
    /*   420 */ 617, 612, 618, 620, 613, 621, 623, 614, 624, 626,
    /*   430 */ 627, 631, 639, 640, 641, 642, 643, 644, 645, 632,
    /*   440 */ 633, 634, 635, 636, 637, 628, 646, 654, 655, 656,
    /*   450 */ 657, 658, 659, 660, 647, 648, 649, 650, 651, 652,
    /*   460 */ 629, 661, 669, 670, 671, 672, 673, 674, 675, 662,
    /*   470 */ 663, 664, 665, 666, 667, 630, 676, 684, 685, 686,
    /*   480 */ 687, 688, 689, 690, 677, 678, 679, 680, 681, 682,
};

#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry
{
    YYACTIONTYPE stateno;	/* The state-number */
    YYCODETYPE major;		/* The major token value.  This is the code
				 ** number for the token at this stack level */
    YYMINORTYPE minor;		/* The user-supplied minor token value.  This
				 ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser
{
    int yyidx;			/* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
    int yyidxMax;		/* Maximum value of yyidx */
#endif
    int yyerrcnt;		/* Shifts left before out of the error */
      ParseARG_SDECL		/* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
    int yystksz;		/* Current side of the stack */
    yyStackEntry *yystack;	/* The parser's stack */
#else
      yyStackEntry yystack[YYSTACKDEPTH];	/* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void
ParseTrace (FILE * TraceFILE, char *zTracePrompt)
{
    yyTraceFILE = TraceFILE;
    yyTracePrompt = zTracePrompt;
    if (yyTraceFILE == 0)
	yyTracePrompt = 0;
    else if (yyTracePrompt == 0)
	yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = {
    "$", "VANUATU_NEWLINE", "VANUATU_POINT", "VANUATU_OPEN_BRACKET",
    "VANUATU_CLOSE_BRACKET", "VANUATU_POINT_M", "VANUATU_POINT_Z",
    "VANUATU_POINT_ZM",
    "VANUATU_NUM", "VANUATU_COMMA", "VANUATU_LINESTRING",
    "VANUATU_LINESTRING_M",
    "VANUATU_LINESTRING_Z", "VANUATU_LINESTRING_ZM", "VANUATU_POLYGON",
    "VANUATU_POLYGON_M",
    "VANUATU_POLYGON_Z", "VANUATU_POLYGON_ZM", "VANUATU_MULTIPOINT",
    "VANUATU_MULTIPOINT_M",
    "VANUATU_MULTIPOINT_Z", "VANUATU_MULTIPOINT_ZM", "VANUATU_MULTILINESTRING",
    "VANUATU_MULTILINESTRING_M",
    "VANUATU_MULTILINESTRING_Z", "VANUATU_MULTILINESTRING_ZM",
    "VANUATU_MULTIPOLYGON", "VANUATU_MULTIPOLYGON_M",
    "VANUATU_MULTIPOLYGON_Z", "VANUATU_MULTIPOLYGON_ZM",
    "VANUATU_GEOMETRYCOLLECTION", "VANUATU_GEOMETRYCOLLECTION_M",
    "VANUATU_GEOMETRYCOLLECTION_Z", "VANUATU_GEOMETRYCOLLECTION_ZM", "error",
    "main",
    "in", "state", "program", "geo_text",
    "geo_textz", "geo_textm", "geo_textzm", "point",
    "linestring", "polygon", "multipoint", "multilinestring",
    "multipolygon", "geocoll", "pointz", "linestringz",
    "polygonz", "multipointz", "multilinestringz", "multipolygonz",
    "geocollz", "pointm", "linestringm", "polygonm",
    "multipointm", "multilinestringm", "multipolygonm", "geocollm",
    "pointzm", "linestringzm", "polygonzm", "multipointzm",
    "multilinestringzm", "multipolygonzm", "geocollzm", "point_coordxy",
    "point_coordxym", "point_coordxyz", "point_coordxyzm", "point_brkt_coordxy",
    "coord", "point_brkt_coordxym", "point_brkt_coordxyz",
    "point_brkt_coordxyzm",
    "extra_brkt_pointsxy", "extra_brkt_pointsxym", "extra_brkt_pointsxyz",
    "extra_brkt_pointsxyzm",
    "extra_pointsxy", "extra_pointsxym", "extra_pointsxyz", "extra_pointsxyzm",
    "linestring_text", "linestring_textm", "linestring_textz",
    "linestring_textzm",
    "polygon_text", "polygon_textm", "polygon_textz", "polygon_textzm",
    "ring", "extra_rings", "ringm", "extra_ringsm",
    "ringz", "extra_ringsz", "ringzm", "extra_ringszm",
    "multipoint_text", "multipoint_textm", "multipoint_textz",
    "multipoint_textzm",
    "multilinestring_text", "multilinestring_textm", "multilinestring_textz",
    "multilinestring_textzm",
    "multilinestring_text2", "multilinestring_textm2", "multilinestring_textz2",
    "multilinestring_textzm2",
    "multipolygon_text", "multipolygon_textm", "multipolygon_textz",
    "multipolygon_textzm",
    "multipolygon_text2", "multipolygon_textm2", "multipolygon_textz2",
    "multipolygon_textzm2",
    "geocoll_text", "geocoll_textm", "geocoll_textz", "geocoll_textzm",
    "geocoll_text2", "geocoll_textm2", "geocoll_textz2", "geocoll_textzm2",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
    /*   0 */ "main ::= in",
    /*   1 */ "in ::=",
    /*   2 */ "in ::= in state VANUATU_NEWLINE",
    /*   3 */ "state ::= program",
    /*   4 */ "program ::= geo_text",
    /*   5 */ "program ::= geo_textz",
    /*   6 */ "program ::= geo_textm",
    /*   7 */ "program ::= geo_textzm",
    /*   8 */ "geo_text ::= point",
    /*   9 */ "geo_text ::= linestring",
    /*  10 */ "geo_text ::= polygon",
    /*  11 */ "geo_text ::= multipoint",
    /*  12 */ "geo_text ::= multilinestring",
    /*  13 */ "geo_text ::= multipolygon",
    /*  14 */ "geo_text ::= geocoll",
    /*  15 */ "geo_textz ::= pointz",
    /*  16 */ "geo_textz ::= linestringz",
    /*  17 */ "geo_textz ::= polygonz",
    /*  18 */ "geo_textz ::= multipointz",
    /*  19 */ "geo_textz ::= multilinestringz",
    /*  20 */ "geo_textz ::= multipolygonz",
    /*  21 */ "geo_textz ::= geocollz",
    /*  22 */ "geo_textm ::= pointm",
    /*  23 */ "geo_textm ::= linestringm",
    /*  24 */ "geo_textm ::= polygonm",
    /*  25 */ "geo_textm ::= multipointm",
    /*  26 */ "geo_textm ::= multilinestringm",
    /*  27 */ "geo_textm ::= multipolygonm",
    /*  28 */ "geo_textm ::= geocollm",
    /*  29 */ "geo_textzm ::= pointzm",
    /*  30 */ "geo_textzm ::= linestringzm",
    /*  31 */ "geo_textzm ::= polygonzm",
    /*  32 */ "geo_textzm ::= multipointzm",
    /*  33 */ "geo_textzm ::= multilinestringzm",
    /*  34 */ "geo_textzm ::= multipolygonzm",
    /*  35 */ "geo_textzm ::= geocollzm",
    /*  36 */
    "point ::= VANUATU_POINT VANUATU_OPEN_BRACKET point_coordxy VANUATU_CLOSE_BRACKET",
    /*  37 */
    "pointm ::= VANUATU_POINT_M VANUATU_OPEN_BRACKET point_coordxym VANUATU_CLOSE_BRACKET",
    /*  38 */
    "pointz ::= VANUATU_POINT_Z VANUATU_OPEN_BRACKET point_coordxyz VANUATU_CLOSE_BRACKET",
    /*  39 */
    "pointzm ::= VANUATU_POINT_ZM VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_CLOSE_BRACKET",
    /*  40 */
    "point_brkt_coordxy ::= VANUATU_OPEN_BRACKET coord coord VANUATU_CLOSE_BRACKET",
    /*  41 */
    "point_brkt_coordxym ::= VANUATU_OPEN_BRACKET coord coord coord VANUATU_CLOSE_BRACKET",
    /*  42 */
    "point_brkt_coordxyz ::= VANUATU_OPEN_BRACKET coord coord coord VANUATU_CLOSE_BRACKET",
    /*  43 */
    "point_brkt_coordxyzm ::= VANUATU_OPEN_BRACKET coord coord coord coord VANUATU_CLOSE_BRACKET",
    /*  44 */ "point_coordxy ::= coord coord",
    /*  45 */ "point_coordxym ::= coord coord coord",
    /*  46 */ "point_coordxyz ::= coord coord coord",
    /*  47 */ "point_coordxyzm ::= coord coord coord coord",
    /*  48 */ "coord ::= VANUATU_NUM",
    /*  49 */ "extra_brkt_pointsxy ::=",
    /*  50 */
    "extra_brkt_pointsxy ::= VANUATU_COMMA point_brkt_coordxy extra_brkt_pointsxy",
    /*  51 */ "extra_brkt_pointsxym ::=",
    /*  52 */
    "extra_brkt_pointsxym ::= VANUATU_COMMA point_brkt_coordxym extra_brkt_pointsxym",
    /*  53 */ "extra_brkt_pointsxyz ::=",
    /*  54 */
    "extra_brkt_pointsxyz ::= VANUATU_COMMA point_brkt_coordxyz extra_brkt_pointsxyz",
    /*  55 */ "extra_brkt_pointsxyzm ::=",
    /*  56 */
    "extra_brkt_pointsxyzm ::= VANUATU_COMMA point_brkt_coordxyzm extra_brkt_pointsxyzm",
    /*  57 */ "extra_pointsxy ::=",
    /*  58 */ "extra_pointsxy ::= VANUATU_COMMA point_coordxy extra_pointsxy",
    /*  59 */ "extra_pointsxym ::=",
    /*  60 */
    "extra_pointsxym ::= VANUATU_COMMA point_coordxym extra_pointsxym",
    /*  61 */ "extra_pointsxyz ::=",
    /*  62 */
    "extra_pointsxyz ::= VANUATU_COMMA point_coordxyz extra_pointsxyz",
    /*  63 */ "extra_pointsxyzm ::=",
    /*  64 */
    "extra_pointsxyzm ::= VANUATU_COMMA point_coordxyzm extra_pointsxyzm",
    /*  65 */ "linestring ::= VANUATU_LINESTRING linestring_text",
    /*  66 */ "linestringm ::= VANUATU_LINESTRING_M linestring_textm",
    /*  67 */ "linestringz ::= VANUATU_LINESTRING_Z linestring_textz",
    /*  68 */ "linestringzm ::= VANUATU_LINESTRING_ZM linestring_textzm",
    /*  69 */
    "linestring_text ::= VANUATU_OPEN_BRACKET point_coordxy VANUATU_COMMA point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET",
    /*  70 */
    "linestring_textm ::= VANUATU_OPEN_BRACKET point_coordxym VANUATU_COMMA point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET",
    /*  71 */
    "linestring_textz ::= VANUATU_OPEN_BRACKET point_coordxyz VANUATU_COMMA point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET",
    /*  72 */
    "linestring_textzm ::= VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_COMMA point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET",
    /*  73 */ "polygon ::= VANUATU_POLYGON polygon_text",
    /*  74 */ "polygonm ::= VANUATU_POLYGON_M polygon_textm",
    /*  75 */ "polygonz ::= VANUATU_POLYGON_Z polygon_textz",
    /*  76 */ "polygonzm ::= VANUATU_POLYGON_ZM polygon_textzm",
    /*  77 */
    "polygon_text ::= VANUATU_OPEN_BRACKET ring extra_rings VANUATU_CLOSE_BRACKET",
    /*  78 */
    "polygon_textm ::= VANUATU_OPEN_BRACKET ringm extra_ringsm VANUATU_CLOSE_BRACKET",
    /*  79 */
    "polygon_textz ::= VANUATU_OPEN_BRACKET ringz extra_ringsz VANUATU_CLOSE_BRACKET",
    /*  80 */
    "polygon_textzm ::= VANUATU_OPEN_BRACKET ringzm extra_ringszm VANUATU_CLOSE_BRACKET",
    /*  81 */
    "ring ::= VANUATU_OPEN_BRACKET point_coordxy VANUATU_COMMA point_coordxy VANUATU_COMMA point_coordxy VANUATU_COMMA point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET",
    /*  82 */ "extra_rings ::=",
    /*  83 */ "extra_rings ::= VANUATU_COMMA ring extra_rings",
    /*  84 */
    "ringm ::= VANUATU_OPEN_BRACKET point_coordxym VANUATU_COMMA point_coordxym VANUATU_COMMA point_coordxym VANUATU_COMMA point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET",
    /*  85 */ "extra_ringsm ::=",
    /*  86 */ "extra_ringsm ::= VANUATU_COMMA ringm extra_ringsm",
    /*  87 */
    "ringz ::= VANUATU_OPEN_BRACKET point_coordxyz VANUATU_COMMA point_coordxyz VANUATU_COMMA point_coordxyz VANUATU_COMMA point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET",
    /*  88 */ "extra_ringsz ::=",
    /*  89 */ "extra_ringsz ::= VANUATU_COMMA ringz extra_ringsz",
    /*  90 */
    "ringzm ::= VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_COMMA point_coordxyzm VANUATU_COMMA point_coordxyzm VANUATU_COMMA point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET",
    /*  91 */ "extra_ringszm ::=",
    /*  92 */ "extra_ringszm ::= VANUATU_COMMA ringzm extra_ringszm",
    /*  93 */ "multipoint ::= VANUATU_MULTIPOINT multipoint_text",
    /*  94 */ "multipointm ::= VANUATU_MULTIPOINT_M multipoint_textm",
    /*  95 */ "multipointz ::= VANUATU_MULTIPOINT_Z multipoint_textz",
    /*  96 */ "multipointzm ::= VANUATU_MULTIPOINT_ZM multipoint_textzm",
    /*  97 */
    "multipoint_text ::= VANUATU_OPEN_BRACKET point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET",
    /*  98 */
    "multipoint_textm ::= VANUATU_OPEN_BRACKET point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET",
    /*  99 */
    "multipoint_textz ::= VANUATU_OPEN_BRACKET point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET",
    /* 100 */
    "multipoint_textzm ::= VANUATU_OPEN_BRACKET point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET",
    /* 101 */
    "multipoint_text ::= VANUATU_OPEN_BRACKET point_brkt_coordxy extra_brkt_pointsxy VANUATU_CLOSE_BRACKET",
    /* 102 */
    "multipoint_textm ::= VANUATU_OPEN_BRACKET point_brkt_coordxym extra_brkt_pointsxym VANUATU_CLOSE_BRACKET",
    /* 103 */
    "multipoint_textz ::= VANUATU_OPEN_BRACKET point_brkt_coordxyz extra_brkt_pointsxyz VANUATU_CLOSE_BRACKET",
    /* 104 */
    "multipoint_textzm ::= VANUATU_OPEN_BRACKET point_brkt_coordxyzm extra_brkt_pointsxyzm VANUATU_CLOSE_BRACKET",
    /* 105 */
    "multilinestring ::= VANUATU_MULTILINESTRING multilinestring_text",
    /* 106 */
    "multilinestringm ::= VANUATU_MULTILINESTRING_M multilinestring_textm",
    /* 107 */
    "multilinestringz ::= VANUATU_MULTILINESTRING_Z multilinestring_textz",
    /* 108 */
    "multilinestringzm ::= VANUATU_MULTILINESTRING_ZM multilinestring_textzm",
    /* 109 */
    "multilinestring_text ::= VANUATU_OPEN_BRACKET linestring_text multilinestring_text2 VANUATU_CLOSE_BRACKET",
    /* 110 */ "multilinestring_text2 ::=",
    /* 111 */
    "multilinestring_text2 ::= VANUATU_COMMA linestring_text multilinestring_text2",
    /* 112 */
    "multilinestring_textm ::= VANUATU_OPEN_BRACKET linestring_textm multilinestring_textm2 VANUATU_CLOSE_BRACKET",
    /* 113 */ "multilinestring_textm2 ::=",
    /* 114 */
    "multilinestring_textm2 ::= VANUATU_COMMA linestring_textm multilinestring_textm2",
    /* 115 */
    "multilinestring_textz ::= VANUATU_OPEN_BRACKET linestring_textz multilinestring_textz2 VANUATU_CLOSE_BRACKET",
    /* 116 */ "multilinestring_textz2 ::=",
    /* 117 */
    "multilinestring_textz2 ::= VANUATU_COMMA linestring_textz multilinestring_textz2",
    /* 118 */
    "multilinestring_textzm ::= VANUATU_OPEN_BRACKET linestring_textzm multilinestring_textzm2 VANUATU_CLOSE_BRACKET",
    /* 119 */ "multilinestring_textzm2 ::=",
    /* 120 */
    "multilinestring_textzm2 ::= VANUATU_COMMA linestring_textzm multilinestring_textzm2",
    /* 121 */ "multipolygon ::= VANUATU_MULTIPOLYGON multipolygon_text",
    /* 122 */ "multipolygonm ::= VANUATU_MULTIPOLYGON_M multipolygon_textm",
    /* 123 */ "multipolygonz ::= VANUATU_MULTIPOLYGON_Z multipolygon_textz",
    /* 124 */ "multipolygonzm ::= VANUATU_MULTIPOLYGON_ZM multipolygon_textzm",
    /* 125 */
    "multipolygon_text ::= VANUATU_OPEN_BRACKET polygon_text multipolygon_text2 VANUATU_CLOSE_BRACKET",
    /* 126 */ "multipolygon_text2 ::=",
    /* 127 */
    "multipolygon_text2 ::= VANUATU_COMMA polygon_text multipolygon_text2",
    /* 128 */
    "multipolygon_textm ::= VANUATU_OPEN_BRACKET polygon_textm multipolygon_textm2 VANUATU_CLOSE_BRACKET",
    /* 129 */ "multipolygon_textm2 ::=",
    /* 130 */
    "multipolygon_textm2 ::= VANUATU_COMMA polygon_textm multipolygon_textm2",
    /* 131 */
    "multipolygon_textz ::= VANUATU_OPEN_BRACKET polygon_textz multipolygon_textz2 VANUATU_CLOSE_BRACKET",
    /* 132 */ "multipolygon_textz2 ::=",
    /* 133 */
    "multipolygon_textz2 ::= VANUATU_COMMA polygon_textz multipolygon_textz2",
    /* 134 */
    "multipolygon_textzm ::= VANUATU_OPEN_BRACKET polygon_textzm multipolygon_textzm2 VANUATU_CLOSE_BRACKET",
    /* 135 */ "multipolygon_textzm2 ::=",
    /* 136 */
    "multipolygon_textzm2 ::= VANUATU_COMMA polygon_textzm multipolygon_textzm2",
    /* 137 */ "geocoll ::= VANUATU_GEOMETRYCOLLECTION geocoll_text",
    /* 138 */ "geocollm ::= VANUATU_GEOMETRYCOLLECTION_M geocoll_textm",
    /* 139 */ "geocollz ::= VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz",
    /* 140 */ "geocollzm ::= VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm",
    /* 141 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET point geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 142 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET linestring geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 143 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET polygon geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 144 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET multipoint geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 145 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET multilinestring geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 146 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET multipolygon geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 147 */
    "geocoll_text ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION geocoll_text geocoll_text2 VANUATU_CLOSE_BRACKET",
    /* 148 */ "geocoll_text2 ::=",
    /* 149 */ "geocoll_text2 ::= VANUATU_COMMA point geocoll_text2",
    /* 150 */ "geocoll_text2 ::= VANUATU_COMMA linestring geocoll_text2",
    /* 151 */ "geocoll_text2 ::= VANUATU_COMMA polygon geocoll_text2",
    /* 152 */ "geocoll_text2 ::= VANUATU_COMMA multipoint geocoll_text2",
    /* 153 */ "geocoll_text2 ::= VANUATU_COMMA multilinestring geocoll_text2",
    /* 154 */ "geocoll_text2 ::= VANUATU_COMMA multipolygon geocoll_text2",
    /* 155 */
    "geocoll_text2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION geocoll_text geocoll_text2",
    /* 156 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET pointm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 157 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET linestringm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 158 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET polygonm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 159 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET multipointm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 160 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET multilinestringm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 161 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET multipolygonm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 162 */
    "geocoll_textm ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 VANUATU_CLOSE_BRACKET",
    /* 163 */ "geocoll_textm2 ::=",
    /* 164 */ "geocoll_textm2 ::= VANUATU_COMMA pointm geocoll_textm2",
    /* 165 */ "geocoll_textm2 ::= VANUATU_COMMA linestringm geocoll_textm2",
    /* 166 */ "geocoll_textm2 ::= VANUATU_COMMA polygonm geocoll_textm2",
    /* 167 */ "geocoll_textm2 ::= VANUATU_COMMA multipointm geocoll_textm2",
    /* 168 */
    "geocoll_textm2 ::= VANUATU_COMMA multilinestringm geocoll_textm2",
    /* 169 */ "geocoll_textm2 ::= VANUATU_COMMA multipolygonm geocoll_textm2",
    /* 170 */
    "geocoll_textm2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2",
    /* 171 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET pointz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 172 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET linestringz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 173 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET polygonz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 174 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET multipointz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 175 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET multilinestringz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 176 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET multipolygonz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 177 */
    "geocoll_textz ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz geocoll_textz2 VANUATU_CLOSE_BRACKET",
    /* 178 */ "geocoll_textz2 ::=",
    /* 179 */ "geocoll_textz2 ::= VANUATU_COMMA pointz geocoll_textz2",
    /* 180 */ "geocoll_textz2 ::= VANUATU_COMMA linestringz geocoll_textz2",
    /* 181 */ "geocoll_textz2 ::= VANUATU_COMMA polygonz geocoll_textz2",
    /* 182 */ "geocoll_textz2 ::= VANUATU_COMMA multipointz geocoll_textz2",
    /* 183 */
    "geocoll_textz2 ::= VANUATU_COMMA multilinestringz geocoll_textz2",
    /* 184 */ "geocoll_textz2 ::= VANUATU_COMMA multipolygonz geocoll_textz2",
    /* 185 */
    "geocoll_textz2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz geocoll_textz2",
    /* 186 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET pointzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 187 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET linestringzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 188 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET polygonzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 189 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET multipointzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 190 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET multilinestringzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 191 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET multipolygonzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 192 */
    "geocoll_textzm ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm geocoll_textzm2 VANUATU_CLOSE_BRACKET",
    /* 193 */ "geocoll_textzm2 ::=",
    /* 194 */ "geocoll_textzm2 ::= VANUATU_COMMA pointzm geocoll_textzm2",
    /* 195 */ "geocoll_textzm2 ::= VANUATU_COMMA linestringzm geocoll_textzm2",
    /* 196 */ "geocoll_textzm2 ::= VANUATU_COMMA polygonzm geocoll_textzm2",
    /* 197 */ "geocoll_textzm2 ::= VANUATU_COMMA multipointzm geocoll_textzm2",
    /* 198 */
    "geocoll_textzm2 ::= VANUATU_COMMA multilinestringzm geocoll_textzm2",
    /* 199 */
    "geocoll_textzm2 ::= VANUATU_COMMA multipolygonzm geocoll_textzm2",
    /* 200 */
    "geocoll_textzm2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm geocoll_textzm2",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void
yyGrowStack (yyParser * p)
{
    int newSize;
    yyStackEntry *pNew;

    newSize = p->yystksz * 2 + 100;
    pNew = realloc (p->yystack, newSize * sizeof (pNew[0]));
    if (pNew)
      {
	  p->yystack = pNew;
	  p->yystksz = newSize;
#ifndef NDEBUG
	  if (yyTraceFILE)
	    {
		fprintf (yyTraceFILE, "%sStack grows to %d entries!\n",
			 yyTracePrompt, p->yystksz);
	    }
#endif
      }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *
ParseAlloc (void *(*mallocProc) (size_t))
{
    yyParser *pParser;
    pParser = (yyParser *) (*mallocProc) ((size_t) sizeof (yyParser));
    if (pParser)
      {
	  pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
	  pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
	  pParser->yystack = NULL;
	  pParser->yystksz = 0;
	  yyGrowStack (pParser);
#endif
      }
    return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void
yy_destructor (yyParser * yypParser,	/* The parser */
	       YYCODETYPE yymajor,	/* Type code for object to destroy */
	       YYMINORTYPE * yypminor	/* The object to be destroyed */
    )
{
    ParseARG_FETCH;
    switch (yymajor)
      {
	  /* Here is inserted the actions which take place when a
	   ** terminal or non-terminal is destroyed.  This can happen
	   ** when the symbol is popped from the stack during a
	   ** reduce or during error processing or when a parser is 
	   ** being destroyed before it is finished parsing.
	   **
	   ** Note: during a reduce, the only symbols destroyed are those
	   ** which appear on the RHS of the rule, but which are not used
	   ** inside the C code.
	   */
      default:
	  break;		/* If no destructor action specified: do nothing */
      }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int
yy_pop_parser_stack (yyParser * pParser)
{
    YYCODETYPE yymajor;
    yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

    if (pParser->yyidx < 0)
	return 0;
#ifndef NDEBUG
    if (yyTraceFILE && pParser->yyidx >= 0)
      {
	  fprintf (yyTraceFILE, "%sPopping %s\n",
		   yyTracePrompt, yyTokenName[yytos->major]);
      }
#endif
    yymajor = yytos->major;
    yy_destructor (pParser, yymajor, &yytos->minor);
    pParser->yyidx--;
    return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void
ParseFree (void *p,		/* The parser to be deleted */
	   void (*freeProc) (void *)	/* Function used to reclaim memory */
    )
{
    yyParser *pParser = (yyParser *) p;
    if (pParser == 0)
	return;
    while (pParser->yyidx >= 0)
	yy_pop_parser_stack (pParser);
#if YYSTACKDEPTH<=0
    free (pParser->yystack);
#endif
    (*freeProc) ((void *) pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int
ParseStackPeak (void *p)
{
    yyParser *pParser = (yyParser *) p;
    return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int
yy_find_shift_action (yyParser * pParser,	/* The parser */
		      YYCODETYPE iLookAhead	/* The look-ahead token */
    )
{
    int i;
    int stateno = pParser->yystack[pParser->yyidx].stateno;

    if (stateno > YY_SHIFT_MAX
	|| (i = yy_shift_ofst[stateno]) == YY_SHIFT_USE_DFLT)
      {
	  return yy_default[stateno];
      }
    assert (iLookAhead != YYNOCODE);
    i += iLookAhead;
    if (i < 0 || i >= YY_SZ_ACTTAB || yy_lookahead[i] != iLookAhead)
      {
	  if (iLookAhead > 0)
	    {
#ifdef YYFALLBACK
		YYCODETYPE iFallback;	/* Fallback token */
		if (iLookAhead < sizeof (yyFallback) / sizeof (yyFallback[0])
		    && (iFallback = yyFallback[iLookAhead]) != 0)
		  {
#ifndef NDEBUG
		      if (yyTraceFILE)
			{
			    fprintf (yyTraceFILE, "%sFALLBACK %s => %s\n",
				     yyTracePrompt, yyTokenName[iLookAhead],
				     yyTokenName[iFallback]);
			}
#endif
		      return yy_find_shift_action (pParser, iFallback);
		  }
#endif
#ifdef YYWILDCARD
		{
		    int j = i - iLookAhead + YYWILDCARD;
		    if (j >= 0 && j < YY_SZ_ACTTAB
			&& yy_lookahead[j] == YYWILDCARD)
		      {
#ifndef NDEBUG
			  if (yyTraceFILE)
			    {
				fprintf (yyTraceFILE, "%sWILDCARD %s => %s\n",
					 yyTracePrompt, yyTokenName[iLookAhead],
					 yyTokenName[YYWILDCARD]);
			    }
#endif /* NDEBUG */
			  return yy_action[j];
		      }
		}
#endif /* YYWILDCARD */
	    }
	  return yy_default[stateno];
      }
    else
      {
	  return yy_action[i];
      }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int
yy_find_reduce_action (int stateno,	/* Current state number */
		       YYCODETYPE iLookAhead	/* The look-ahead token */
    )
{
    int i;
#ifdef YYERRORSYMBOL
    if (stateno > YY_REDUCE_MAX)
      {
	  return yy_default[stateno];
      }
#else
    assert (stateno <= YY_REDUCE_MAX);
#endif
    i = yy_reduce_ofst[stateno];
    assert (i != YY_REDUCE_USE_DFLT);
    assert (iLookAhead != YYNOCODE);
    i += iLookAhead;
#ifdef YYERRORSYMBOL
    if (i < 0 || i >= YY_SZ_ACTTAB || yy_lookahead[i] != iLookAhead)
      {
	  return yy_default[stateno];
      }
#else
    assert (i >= 0 && i < YY_SZ_ACTTAB);
    assert (yy_lookahead[i] == iLookAhead);
#endif
    return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void
yyStackOverflow (yyParser * yypParser, YYMINORTYPE * yypMinor)
{
    ParseARG_FETCH;
    yypParser->yyidx--;
#ifndef NDEBUG
    if (yyTraceFILE)
      {
	  fprintf (yyTraceFILE, "%sStack Overflow!\n", yyTracePrompt);
      }
#endif
    while (yypParser->yyidx >= 0)
	yy_pop_parser_stack (yypParser);
    /* Here code is inserted which will execute if the parser
     ** stack every overflows */

    spatialite_e ("Giving up.  Parser stack overflow\n");
    ParseARG_STORE;		/* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void
yy_shift (yyParser * yypParser,	/* The parser to be shifted */
	  int yyNewState,	/* The new state to shift in */
	  int yyMajor,		/* The major token to shift in */
	  YYMINORTYPE * yypMinor	/* Pointer to the minor token to shift in */
    )
{
    yyStackEntry *yytos;
    yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
    if (yypParser->yyidx > yypParser->yyidxMax)
      {
	  yypParser->yyidxMax = yypParser->yyidx;
      }
#endif
#if YYSTACKDEPTH>0
    if (yypParser->yyidx >= YYSTACKDEPTH)
      {
	  yyStackOverflow (yypParser, yypMinor);
	  return;
      }
#else
    if (yypParser->yyidx >= yypParser->yystksz)
      {
	  yyGrowStack (yypParser);
	  if (yypParser->yyidx >= yypParser->yystksz)
	    {
		yyStackOverflow (yypParser, yypMinor);
		return;
	    }
      }
#endif
    yytos = &yypParser->yystack[yypParser->yyidx];
    yytos->stateno = (YYACTIONTYPE) yyNewState;
    yytos->major = (YYCODETYPE) yyMajor;
    yytos->minor = *yypMinor;
#ifndef NDEBUG
    if (yyTraceFILE && yypParser->yyidx > 0)
      {
	  int i;
	  fprintf (yyTraceFILE, "%sShift %d\n", yyTracePrompt, yyNewState);
	  fprintf (yyTraceFILE, "%sStack:", yyTracePrompt);
	  for (i = 1; i <= yypParser->yyidx; i++)
	      fprintf (yyTraceFILE, " %s",
		       yyTokenName[yypParser->yystack[i].major]);
	  fprintf (yyTraceFILE, "\n");
      }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct
{
    YYCODETYPE lhs;		/* Symbol on the left-hand side of the rule */
    unsigned char nrhs;		/* Number of right-hand side symbols in the rule */
} yyRuleInfo[] =
{
    {
    35, 1},
    {
    36, 0},
    {
    36, 3},
    {
    37, 1},
    {
    38, 1},
    {
    38, 1},
    {
    38, 1},
    {
    38, 1},
    {
    39, 1},
    {
    39, 1},
    {
    39, 1},
    {
    39, 1},
    {
    39, 1},
    {
    39, 1},
    {
    39, 1},
    {
    40, 1},
    {
    40, 1},
    {
    40, 1},
    {
    40, 1},
    {
    40, 1},
    {
    40, 1},
    {
    40, 1},
    {
    41, 1},
    {
    41, 1},
    {
    41, 1},
    {
    41, 1},
    {
    41, 1},
    {
    41, 1},
    {
    41, 1},
    {
    42, 1},
    {
    42, 1},
    {
    42, 1},
    {
    42, 1},
    {
    42, 1},
    {
    42, 1},
    {
    42, 1},
    {
    43, 4},
    {
    57, 4},
    {
    50, 4},
    {
    64, 4},
    {
    75, 4},
    {
    77, 5},
    {
    78, 5},
    {
    79, 6},
    {
    71, 2},
    {
    72, 3},
    {
    73, 3},
    {
    74, 4},
    {
    76, 1},
    {
    80, 0},
    {
    80, 3},
    {
    81, 0},
    {
    81, 3},
    {
    82, 0},
    {
    82, 3},
    {
    83, 0},
    {
    83, 3},
    {
    84, 0},
    {
    84, 3},
    {
    85, 0},
    {
    85, 3},
    {
    86, 0},
    {
    86, 3},
    {
    87, 0},
    {
    87, 3},
    {
    44, 2},
    {
    58, 2},
    {
    51, 2},
    {
    65, 2},
    {
    88, 6},
    {
    89, 6},
    {
    90, 6},
    {
    91, 6},
    {
    45, 2},
    {
    59, 2},
    {
    52, 2},
    {
    66, 2},
    {
    92, 4},
    {
    93, 4},
    {
    94, 4},
    {
    95, 4},
    {
    96, 10},
    {
    97, 0},
    {
    97, 3},
    {
    98, 10},
    {
    99, 0},
    {
    99, 3},
    {
    100, 10},
    {
    101, 0},
    {
    101, 3},
    {
    102, 10},
    {
    103, 0},
    {
    103, 3},
    {
    46, 2},
    {
    60, 2},
    {
    53, 2},
    {
    67, 2},
    {
    104, 4},
    {
    105, 4},
    {
    106, 4},
    {
    107, 4},
    {
    104, 4},
    {
    105, 4},
    {
    106, 4},
    {
    107, 4},
    {
    47, 2},
    {
    61, 2},
    {
    54, 2},
    {
    68, 2},
    {
    108, 4},
    {
    112, 0},
    {
    112, 3},
    {
    109, 4},
    {
    113, 0},
    {
    113, 3},
    {
    110, 4},
    {
    114, 0},
    {
    114, 3},
    {
    111, 4},
    {
    115, 0},
    {
    115, 3},
    {
    48, 2},
    {
    62, 2},
    {
    55, 2},
    {
    69, 2},
    {
    116, 4},
    {
    120, 0},
    {
    120, 3},
    {
    117, 4},
    {
    121, 0},
    {
    121, 3},
    {
    118, 4},
    {
    122, 0},
    {
    122, 3},
    {
    119, 4},
    {
    123, 0},
    {
    123, 3},
    {
    49, 2},
    {
    63, 2},
    {
    56, 2},
    {
    70, 2},
    {
    124, 4},
    {
    124, 4},
    {
    124, 4},
    {
    124, 4},
    {
    124, 4},
    {
    124, 4},
    {
    124, 5},
    {
    128, 0},
    {
    128, 3},
    {
    128, 3},
    {
    128, 3},
    {
    128, 3},
    {
    128, 3},
    {
    128, 3},
    {
    128, 4},
    {
    125, 4},
    {
    125, 4},
    {
    125, 4},
    {
    125, 4},
    {
    125, 4},
    {
    125, 4},
    {
    125, 5},
    {
    129, 0},
    {
    129, 3},
    {
    129, 3},
    {
    129, 3},
    {
    129, 3},
    {
    129, 3},
    {
    129, 3},
    {
    129, 4},
    {
    126, 4},
    {
    126, 4},
    {
    126, 4},
    {
    126, 4},
    {
    126, 4},
    {
    126, 4},
    {
    126, 5},
    {
    130, 0},
    {
    130, 3},
    {
    130, 3},
    {
    130, 3},
    {
    130, 3},
    {
    130, 3},
    {
    130, 3},
    {
    130, 4},
    {
    127, 4},
    {
    127, 4},
    {
    127, 4},
    {
    127, 4},
    {
    127, 4},
    {
    127, 4},
    {
    127, 5},
    {
    131, 0},
    {
    131, 3},
    {
    131, 3},
    {
    131, 3},
    {
    131, 3},
    {
    131, 3},
    {
    131, 3},
    {
131, 4},};

static void yy_accept (yyParser *);	/* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void
yy_reduce (yyParser * yypParser,	/* The parser */
	   int yyruleno		/* Number of the rule by which to reduce */
    )
{
    int yygoto;			/* The next state */
    int yyact;			/* The next action */
    YYMINORTYPE yygotominor;	/* The LHS of the rule reduced */
    yyStackEntry *yymsp;	/* The top of the parser's stack */
    int yysize;			/* Amount to pop the stack */
    ParseARG_FETCH;
    yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
    if (yyTraceFILE && yyruleno >= 0
	&& yyruleno < (int) (sizeof (yyRuleName) / sizeof (yyRuleName[0])))
      {
	  fprintf (yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
		   yyRuleName[yyruleno]);
      }
#endif /* NDEBUG */

    /* Silence complaints from purify about yygotominor being uninitialized
     ** in some cases when it is copied into the stack after the following
     ** switch.  yygotominor is uninitialized when a rule reduces that does
     ** not set the value of its left-hand side nonterminal.  Leaving the
     ** value of the nonterminal uninitialized is utterly harmless as long
     ** as the value is never used.  So really the only thing this code
     ** accomplishes is to quieten purify.  
     **
     ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
     ** without this code, their parser segfaults.  I'm not sure what there
     ** parser is doing to make this happen.  This is the second bug report
     ** from wireshark this week.  Clearly they are stressing Lemon in ways
     ** that it has not been previously stressed...  (SQLite ticket #2172)
     */
    /*memset(&yygotominor, 0, sizeof(yygotominor)); */
    yygotominor = yyzerominor;


    switch (yyruleno)
      {
	  /* Beginning here are the reduction cases.  A typical example
	   ** follows:
	   **   case 0:
	   **  #line <lineno> <grammarfile>
	   **     { ... }           // User supplied code
	   **  #line <lineno> <thisfile>
	   **     break;
	   */
      case 8:			/* geo_text ::= point */
      case 9:			/* geo_text ::= linestring */
	  yytestcase (yyruleno == 9);
      case 10:			/* geo_text ::= polygon */
	  yytestcase (yyruleno == 10);
      case 11:			/* geo_text ::= multipoint */
	  yytestcase (yyruleno == 11);
      case 12:			/* geo_text ::= multilinestring */
	  yytestcase (yyruleno == 12);
      case 13:			/* geo_text ::= multipolygon */
	  yytestcase (yyruleno == 13);
      case 14:			/* geo_text ::= geocoll */
	  yytestcase (yyruleno == 14);
      case 15:			/* geo_textz ::= pointz */
	  yytestcase (yyruleno == 15);
      case 16:			/* geo_textz ::= linestringz */
	  yytestcase (yyruleno == 16);
      case 17:			/* geo_textz ::= polygonz */
	  yytestcase (yyruleno == 17);
      case 18:			/* geo_textz ::= multipointz */
	  yytestcase (yyruleno == 18);
      case 19:			/* geo_textz ::= multilinestringz */
	  yytestcase (yyruleno == 19);
      case 20:			/* geo_textz ::= multipolygonz */
	  yytestcase (yyruleno == 20);
      case 21:			/* geo_textz ::= geocollz */
	  yytestcase (yyruleno == 21);
      case 22:			/* geo_textm ::= pointm */
	  yytestcase (yyruleno == 22);
      case 23:			/* geo_textm ::= linestringm */
	  yytestcase (yyruleno == 23);
      case 24:			/* geo_textm ::= polygonm */
	  yytestcase (yyruleno == 24);
      case 25:			/* geo_textm ::= multipointm */
	  yytestcase (yyruleno == 25);
      case 26:			/* geo_textm ::= multilinestringm */
	  yytestcase (yyruleno == 26);
      case 27:			/* geo_textm ::= multipolygonm */
	  yytestcase (yyruleno == 27);
      case 28:			/* geo_textm ::= geocollm */
	  yytestcase (yyruleno == 28);
      case 29:			/* geo_textzm ::= pointzm */
	  yytestcase (yyruleno == 29);
      case 30:			/* geo_textzm ::= linestringzm */
	  yytestcase (yyruleno == 30);
      case 31:			/* geo_textzm ::= polygonzm */
	  yytestcase (yyruleno == 31);
      case 32:			/* geo_textzm ::= multipointzm */
	  yytestcase (yyruleno == 32);
      case 33:			/* geo_textzm ::= multilinestringzm */
	  yytestcase (yyruleno == 33);
      case 34:			/* geo_textzm ::= multipolygonzm */
	  yytestcase (yyruleno == 34);
      case 35:			/* geo_textzm ::= geocollzm */
	  yytestcase (yyruleno == 35);
	  {
	      p_data->result = yymsp[0].minor.yy0;
	  }
	  break;
      case 36:			/* point ::= VANUATU_POINT VANUATU_OPEN_BRACKET point_coordxy VANUATU_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  vanuatu_buildGeomFromPoint (p_data,
					      (gaiaPointPtr) yymsp[-1].minor.
					      yy0);
	  }
	  break;
      case 37:			/* pointm ::= VANUATU_POINT_M VANUATU_OPEN_BRACKET point_coordxym VANUATU_CLOSE_BRACKET */
      case 38:			/* pointz ::= VANUATU_POINT_Z VANUATU_OPEN_BRACKET point_coordxyz VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 38);
      case 39:			/* pointzm ::= VANUATU_POINT_ZM VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 39);
	  {
	      yygotominor.yy0 =
		  vanuatu_buildGeomFromPoint (p_data,
					      (gaiaPointPtr) yymsp[-1].minor.
					      yy0);
	  }
	  break;
      case 40:			/* point_brkt_coordxy ::= VANUATU_OPEN_BRACKET coord coord VANUATU_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xy (p_data,
					     (double *) yymsp[-2].minor.yy0,
					     (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 41:			/* point_brkt_coordxym ::= VANUATU_OPEN_BRACKET coord coord coord VANUATU_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xym (p_data,
					      (double *) yymsp[-3].minor.yy0,
					      (double *) yymsp[-2].minor.yy0,
					      (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 42:			/* point_brkt_coordxyz ::= VANUATU_OPEN_BRACKET coord coord coord VANUATU_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xyz (p_data,
					      (double *) yymsp[-3].minor.yy0,
					      (double *) yymsp[-2].minor.yy0,
					      (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 43:			/* point_brkt_coordxyzm ::= VANUATU_OPEN_BRACKET coord coord coord coord VANUATU_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xyzm (p_data,
					       (double *) yymsp[-4].minor.yy0,
					       (double *) yymsp[-3].minor.yy0,
					       (double *) yymsp[-2].minor.yy0,
					       (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 44:			/* point_coordxy ::= coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xy (p_data,
					     (double *) yymsp[-1].minor.yy0,
					     (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 45:			/* point_coordxym ::= coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xym (p_data,
					      (double *) yymsp[-2].minor.yy0,
					      (double *) yymsp[-1].minor.yy0,
					      (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 46:			/* point_coordxyz ::= coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xyz (p_data,
					      (double *) yymsp[-2].minor.yy0,
					      (double *) yymsp[-1].minor.yy0,
					      (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 47:			/* point_coordxyzm ::= coord coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) vanuatu_point_xyzm (p_data,
					       (double *) yymsp[-3].minor.yy0,
					       (double *) yymsp[-2].minor.yy0,
					       (double *) yymsp[-1].minor.yy0,
					       (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 48:			/* coord ::= VANUATU_NUM */
      case 93:			/* multipoint ::= VANUATU_MULTIPOINT multipoint_text */
	  yytestcase (yyruleno == 93);
      case 94:			/* multipointm ::= VANUATU_MULTIPOINT_M multipoint_textm */
	  yytestcase (yyruleno == 94);
      case 95:			/* multipointz ::= VANUATU_MULTIPOINT_Z multipoint_textz */
	  yytestcase (yyruleno == 95);
      case 96:			/* multipointzm ::= VANUATU_MULTIPOINT_ZM multipoint_textzm */
	  yytestcase (yyruleno == 96);
      case 105:		/* multilinestring ::= VANUATU_MULTILINESTRING multilinestring_text */
	  yytestcase (yyruleno == 105);
      case 106:		/* multilinestringm ::= VANUATU_MULTILINESTRING_M multilinestring_textm */
	  yytestcase (yyruleno == 106);
      case 107:		/* multilinestringz ::= VANUATU_MULTILINESTRING_Z multilinestring_textz */
	  yytestcase (yyruleno == 107);
      case 108:		/* multilinestringzm ::= VANUATU_MULTILINESTRING_ZM multilinestring_textzm */
	  yytestcase (yyruleno == 108);
      case 121:		/* multipolygon ::= VANUATU_MULTIPOLYGON multipolygon_text */
	  yytestcase (yyruleno == 121);
      case 122:		/* multipolygonm ::= VANUATU_MULTIPOLYGON_M multipolygon_textm */
	  yytestcase (yyruleno == 122);
      case 123:		/* multipolygonz ::= VANUATU_MULTIPOLYGON_Z multipolygon_textz */
	  yytestcase (yyruleno == 123);
      case 124:		/* multipolygonzm ::= VANUATU_MULTIPOLYGON_ZM multipolygon_textzm */
	  yytestcase (yyruleno == 124);
      case 137:		/* geocoll ::= VANUATU_GEOMETRYCOLLECTION geocoll_text */
	  yytestcase (yyruleno == 137);
      case 138:		/* geocollm ::= VANUATU_GEOMETRYCOLLECTION_M geocoll_textm */
	  yytestcase (yyruleno == 138);
      case 139:		/* geocollz ::= VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz */
	  yytestcase (yyruleno == 139);
      case 140:		/* geocollzm ::= VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm */
	  yytestcase (yyruleno == 140);
	  {
	      yygotominor.yy0 = yymsp[0].minor.yy0;
	  }
	  break;
      case 49:			/* extra_brkt_pointsxy ::= */
      case 51:			/* extra_brkt_pointsxym ::= */
	  yytestcase (yyruleno == 51);
      case 53:			/* extra_brkt_pointsxyz ::= */
	  yytestcase (yyruleno == 53);
      case 55:			/* extra_brkt_pointsxyzm ::= */
	  yytestcase (yyruleno == 55);
      case 57:			/* extra_pointsxy ::= */
	  yytestcase (yyruleno == 57);
      case 59:			/* extra_pointsxym ::= */
	  yytestcase (yyruleno == 59);
      case 61:			/* extra_pointsxyz ::= */
	  yytestcase (yyruleno == 61);
      case 63:			/* extra_pointsxyzm ::= */
	  yytestcase (yyruleno == 63);
      case 82:			/* extra_rings ::= */
	  yytestcase (yyruleno == 82);
      case 85:			/* extra_ringsm ::= */
	  yytestcase (yyruleno == 85);
      case 88:			/* extra_ringsz ::= */
	  yytestcase (yyruleno == 88);
      case 91:			/* extra_ringszm ::= */
	  yytestcase (yyruleno == 91);
      case 110:		/* multilinestring_text2 ::= */
	  yytestcase (yyruleno == 110);
      case 113:		/* multilinestring_textm2 ::= */
	  yytestcase (yyruleno == 113);
      case 116:		/* multilinestring_textz2 ::= */
	  yytestcase (yyruleno == 116);
      case 119:		/* multilinestring_textzm2 ::= */
	  yytestcase (yyruleno == 119);
      case 126:		/* multipolygon_text2 ::= */
	  yytestcase (yyruleno == 126);
      case 129:		/* multipolygon_textm2 ::= */
	  yytestcase (yyruleno == 129);
      case 132:		/* multipolygon_textz2 ::= */
	  yytestcase (yyruleno == 132);
      case 135:		/* multipolygon_textzm2 ::= */
	  yytestcase (yyruleno == 135);
      case 148:		/* geocoll_text2 ::= */
	  yytestcase (yyruleno == 148);
      case 163:		/* geocoll_textm2 ::= */
	  yytestcase (yyruleno == 163);
      case 178:		/* geocoll_textz2 ::= */
	  yytestcase (yyruleno == 178);
      case 193:		/* geocoll_textzm2 ::= */
	  yytestcase (yyruleno == 193);
	  {
	      yygotominor.yy0 = NULL;
	  }
	  break;
      case 50:			/* extra_brkt_pointsxy ::= VANUATU_COMMA point_brkt_coordxy extra_brkt_pointsxy */
      case 52:			/* extra_brkt_pointsxym ::= VANUATU_COMMA point_brkt_coordxym extra_brkt_pointsxym */
	  yytestcase (yyruleno == 52);
      case 54:			/* extra_brkt_pointsxyz ::= VANUATU_COMMA point_brkt_coordxyz extra_brkt_pointsxyz */
	  yytestcase (yyruleno == 54);
      case 56:			/* extra_brkt_pointsxyzm ::= VANUATU_COMMA point_brkt_coordxyzm extra_brkt_pointsxyzm */
	  yytestcase (yyruleno == 56);
      case 58:			/* extra_pointsxy ::= VANUATU_COMMA point_coordxy extra_pointsxy */
	  yytestcase (yyruleno == 58);
      case 60:			/* extra_pointsxym ::= VANUATU_COMMA point_coordxym extra_pointsxym */
	  yytestcase (yyruleno == 60);
      case 62:			/* extra_pointsxyz ::= VANUATU_COMMA point_coordxyz extra_pointsxyz */
	  yytestcase (yyruleno == 62);
      case 64:			/* extra_pointsxyzm ::= VANUATU_COMMA point_coordxyzm extra_pointsxyzm */
	  yytestcase (yyruleno == 64);
	  {
	      ((gaiaPointPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 65:			/* linestring ::= VANUATU_LINESTRING linestring_text */
      case 66:			/* linestringm ::= VANUATU_LINESTRING_M linestring_textm */
	  yytestcase (yyruleno == 66);
      case 67:			/* linestringz ::= VANUATU_LINESTRING_Z linestring_textz */
	  yytestcase (yyruleno == 67);
      case 68:			/* linestringzm ::= VANUATU_LINESTRING_ZM linestring_textzm */
	  yytestcase (yyruleno == 68);
	  {
	      yygotominor.yy0 =
		  vanuatu_buildGeomFromLinestring (p_data,
						   (gaiaLinestringPtr)
						   yymsp[0].minor.yy0);
	  }
	  break;
      case 69:			/* linestring_text ::= VANUATU_OPEN_BRACKET point_coordxy VANUATU_COMMA point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_linestring_xy (p_data,
						  (gaiaPointPtr)
						  yymsp[-4].minor.yy0);
	  }
	  break;
      case 70:			/* linestring_textm ::= VANUATU_OPEN_BRACKET point_coordxym VANUATU_COMMA point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_linestring_xym (p_data,
						   (gaiaPointPtr)
						   yymsp[-4].minor.yy0);
	  }
	  break;
      case 71:			/* linestring_textz ::= VANUATU_OPEN_BRACKET point_coordxyz VANUATU_COMMA point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_linestring_xyz (p_data,
						   (gaiaPointPtr)
						   yymsp[-4].minor.yy0);
	  }
	  break;
      case 72:			/* linestring_textzm ::= VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_COMMA point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_linestring_xyzm (p_data,
						    (gaiaPointPtr)
						    yymsp[-4].minor.yy0);
	  }
	  break;
      case 73:			/* polygon ::= VANUATU_POLYGON polygon_text */
      case 74:			/* polygonm ::= VANUATU_POLYGON_M polygon_textm */
	  yytestcase (yyruleno == 74);
      case 75:			/* polygonz ::= VANUATU_POLYGON_Z polygon_textz */
	  yytestcase (yyruleno == 75);
      case 76:			/* polygonzm ::= VANUATU_POLYGON_ZM polygon_textzm */
	  yytestcase (yyruleno == 76);
	  {
	      yygotominor.yy0 =
		  vanuatu_buildGeomFromPolygon (p_data,
						(gaiaPolygonPtr) yymsp[0].minor.
						yy0);
	  }
	  break;
      case 77:			/* polygon_text ::= VANUATU_OPEN_BRACKET ring extra_rings VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_polygon_xy (p_data,
					       (gaiaRingPtr) yymsp[-2].minor.
					       yy0);
	  }
	  break;
      case 78:			/* polygon_textm ::= VANUATU_OPEN_BRACKET ringm extra_ringsm VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_polygon_xym (p_data,
						(gaiaRingPtr) yymsp[-2].minor.
						yy0);
	  }
	  break;
      case 79:			/* polygon_textz ::= VANUATU_OPEN_BRACKET ringz extra_ringsz VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_polygon_xyz (p_data,
						(gaiaRingPtr) yymsp[-2].minor.
						yy0);
	  }
	  break;
      case 80:			/* polygon_textzm ::= VANUATU_OPEN_BRACKET ringzm extra_ringszm VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_polygon_xyzm (p_data,
						 (gaiaRingPtr) yymsp[-2].minor.
						 yy0);
	  }
	  break;
      case 81:			/* ring ::= VANUATU_OPEN_BRACKET point_coordxy VANUATU_COMMA point_coordxy VANUATU_COMMA point_coordxy VANUATU_COMMA point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-8].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-6].minor.yy0;
	      ((gaiaPointPtr) yymsp[-6].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-4].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_ring_xy (p_data,
					    (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 83:			/* extra_rings ::= VANUATU_COMMA ring extra_rings */
      case 86:			/* extra_ringsm ::= VANUATU_COMMA ringm extra_ringsm */
	  yytestcase (yyruleno == 86);
      case 89:			/* extra_ringsz ::= VANUATU_COMMA ringz extra_ringsz */
	  yytestcase (yyruleno == 89);
      case 92:			/* extra_ringszm ::= VANUATU_COMMA ringzm extra_ringszm */
	  yytestcase (yyruleno == 92);
	  {
	      ((gaiaRingPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 84:			/* ringm ::= VANUATU_OPEN_BRACKET point_coordxym VANUATU_COMMA point_coordxym VANUATU_COMMA point_coordxym VANUATU_COMMA point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-8].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-6].minor.yy0;
	      ((gaiaPointPtr) yymsp[-6].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-4].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_ring_xym (p_data,
					     (gaiaPointPtr) yymsp[-8].minor.
					     yy0);
	  }
	  break;
      case 87:			/* ringz ::= VANUATU_OPEN_BRACKET point_coordxyz VANUATU_COMMA point_coordxyz VANUATU_COMMA point_coordxyz VANUATU_COMMA point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-8].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-6].minor.yy0;
	      ((gaiaPointPtr) yymsp[-6].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-4].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_ring_xyz (p_data,
					     (gaiaPointPtr) yymsp[-8].minor.
					     yy0);
	  }
	  break;
      case 90:			/* ringzm ::= VANUATU_OPEN_BRACKET point_coordxyzm VANUATU_COMMA point_coordxyzm VANUATU_COMMA point_coordxyzm VANUATU_COMMA point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-8].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-6].minor.yy0;
	      ((gaiaPointPtr) yymsp[-6].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-4].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_ring_xyzm (p_data,
					      (gaiaPointPtr) yymsp[-8].minor.
					      yy0);
	  }
	  break;
      case 97:			/* multipoint_text ::= VANUATU_OPEN_BRACKET point_coordxy extra_pointsxy VANUATU_CLOSE_BRACKET */
      case 101:		/* multipoint_text ::= VANUATU_OPEN_BRACKET point_brkt_coordxy extra_brkt_pointsxy VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 101);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipoint_xy (p_data,
						  (gaiaPointPtr)
						  yymsp[-2].minor.yy0);
	  }
	  break;
      case 98:			/* multipoint_textm ::= VANUATU_OPEN_BRACKET point_coordxym extra_pointsxym VANUATU_CLOSE_BRACKET */
      case 102:		/* multipoint_textm ::= VANUATU_OPEN_BRACKET point_brkt_coordxym extra_brkt_pointsxym VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 102);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipoint_xym (p_data,
						   (gaiaPointPtr)
						   yymsp[-2].minor.yy0);
	  }
	  break;
      case 99:			/* multipoint_textz ::= VANUATU_OPEN_BRACKET point_coordxyz extra_pointsxyz VANUATU_CLOSE_BRACKET */
      case 103:		/* multipoint_textz ::= VANUATU_OPEN_BRACKET point_brkt_coordxyz extra_brkt_pointsxyz VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 103);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipoint_xyz (p_data,
						   (gaiaPointPtr)
						   yymsp[-2].minor.yy0);
	  }
	  break;
      case 100:		/* multipoint_textzm ::= VANUATU_OPEN_BRACKET point_coordxyzm extra_pointsxyzm VANUATU_CLOSE_BRACKET */
      case 104:		/* multipoint_textzm ::= VANUATU_OPEN_BRACKET point_brkt_coordxyzm extra_brkt_pointsxyzm VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 104);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipoint_xyzm (p_data,
						    (gaiaPointPtr)
						    yymsp[-2].minor.yy0);
	  }
	  break;
      case 109:		/* multilinestring_text ::= VANUATU_OPEN_BRACKET linestring_text multilinestring_text2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multilinestring_xy (p_data,
						       (gaiaLinestringPtr)
						       yymsp[-2].minor.yy0);
	  }
	  break;
      case 111:		/* multilinestring_text2 ::= VANUATU_COMMA linestring_text multilinestring_text2 */
      case 114:		/* multilinestring_textm2 ::= VANUATU_COMMA linestring_textm multilinestring_textm2 */
	  yytestcase (yyruleno == 114);
      case 117:		/* multilinestring_textz2 ::= VANUATU_COMMA linestring_textz multilinestring_textz2 */
	  yytestcase (yyruleno == 117);
      case 120:		/* multilinestring_textzm2 ::= VANUATU_COMMA linestring_textzm multilinestring_textzm2 */
	  yytestcase (yyruleno == 120);
	  {
	      ((gaiaLinestringPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 112:		/* multilinestring_textm ::= VANUATU_OPEN_BRACKET linestring_textm multilinestring_textm2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multilinestring_xym (p_data,
							(gaiaLinestringPtr)
							yymsp[-2].minor.yy0);
	  }
	  break;
      case 115:		/* multilinestring_textz ::= VANUATU_OPEN_BRACKET linestring_textz multilinestring_textz2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multilinestring_xyz (p_data,
							(gaiaLinestringPtr)
							yymsp[-2].minor.yy0);
	  }
	  break;
      case 118:		/* multilinestring_textzm ::= VANUATU_OPEN_BRACKET linestring_textzm multilinestring_textzm2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multilinestring_xyzm (p_data,
							 (gaiaLinestringPtr)
							 yymsp[-2].minor.yy0);
	  }
	  break;
      case 125:		/* multipolygon_text ::= VANUATU_OPEN_BRACKET polygon_text multipolygon_text2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipolygon_xy (p_data,
						    (gaiaPolygonPtr)
						    yymsp[-2].minor.yy0);
	  }
	  break;
      case 127:		/* multipolygon_text2 ::= VANUATU_COMMA polygon_text multipolygon_text2 */
      case 130:		/* multipolygon_textm2 ::= VANUATU_COMMA polygon_textm multipolygon_textm2 */
	  yytestcase (yyruleno == 130);
      case 133:		/* multipolygon_textz2 ::= VANUATU_COMMA polygon_textz multipolygon_textz2 */
	  yytestcase (yyruleno == 133);
      case 136:		/* multipolygon_textzm2 ::= VANUATU_COMMA polygon_textzm multipolygon_textzm2 */
	  yytestcase (yyruleno == 136);
	  {
	      ((gaiaPolygonPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 128:		/* multipolygon_textm ::= VANUATU_OPEN_BRACKET polygon_textm multipolygon_textm2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipolygon_xym (p_data,
						     (gaiaPolygonPtr)
						     yymsp[-2].minor.yy0);
	  }
	  break;
      case 131:		/* multipolygon_textz ::= VANUATU_OPEN_BRACKET polygon_textz multipolygon_textz2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipolygon_xyz (p_data,
						     (gaiaPolygonPtr)
						     yymsp[-2].minor.yy0);
	  }
	  break;
      case 134:		/* multipolygon_textzm ::= VANUATU_OPEN_BRACKET polygon_textzm multipolygon_textzm2 VANUATU_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_multipolygon_xyzm (p_data,
						      (gaiaPolygonPtr)
						      yymsp[-2].minor.yy0);
	  }
	  break;
      case 141:		/* geocoll_text ::= VANUATU_OPEN_BRACKET point geocoll_text2 VANUATU_CLOSE_BRACKET */
      case 142:		/* geocoll_text ::= VANUATU_OPEN_BRACKET linestring geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 142);
      case 143:		/* geocoll_text ::= VANUATU_OPEN_BRACKET polygon geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 143);
      case 144:		/* geocoll_text ::= VANUATU_OPEN_BRACKET multipoint geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 144);
      case 145:		/* geocoll_text ::= VANUATU_OPEN_BRACKET multilinestring geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 145);
      case 146:		/* geocoll_text ::= VANUATU_OPEN_BRACKET multipolygon geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 146);
      case 147:		/* geocoll_text ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION geocoll_text geocoll_text2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 147);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_geomColl_xy (p_data,
						(gaiaGeomCollPtr)
						yymsp[-2].minor.yy0);
	  }
	  break;
      case 149:		/* geocoll_text2 ::= VANUATU_COMMA point geocoll_text2 */
      case 150:		/* geocoll_text2 ::= VANUATU_COMMA linestring geocoll_text2 */
	  yytestcase (yyruleno == 150);
      case 151:		/* geocoll_text2 ::= VANUATU_COMMA polygon geocoll_text2 */
	  yytestcase (yyruleno == 151);
      case 152:		/* geocoll_text2 ::= VANUATU_COMMA multipoint geocoll_text2 */
	  yytestcase (yyruleno == 152);
      case 153:		/* geocoll_text2 ::= VANUATU_COMMA multilinestring geocoll_text2 */
	  yytestcase (yyruleno == 153);
      case 154:		/* geocoll_text2 ::= VANUATU_COMMA multipolygon geocoll_text2 */
	  yytestcase (yyruleno == 154);
      case 155:		/* geocoll_text2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION geocoll_text geocoll_text2 */
	  yytestcase (yyruleno == 155);
      case 164:		/* geocoll_textm2 ::= VANUATU_COMMA pointm geocoll_textm2 */
	  yytestcase (yyruleno == 164);
      case 165:		/* geocoll_textm2 ::= VANUATU_COMMA linestringm geocoll_textm2 */
	  yytestcase (yyruleno == 165);
      case 166:		/* geocoll_textm2 ::= VANUATU_COMMA polygonm geocoll_textm2 */
	  yytestcase (yyruleno == 166);
      case 167:		/* geocoll_textm2 ::= VANUATU_COMMA multipointm geocoll_textm2 */
	  yytestcase (yyruleno == 167);
      case 168:		/* geocoll_textm2 ::= VANUATU_COMMA multilinestringm geocoll_textm2 */
	  yytestcase (yyruleno == 168);
      case 169:		/* geocoll_textm2 ::= VANUATU_COMMA multipolygonm geocoll_textm2 */
	  yytestcase (yyruleno == 169);
      case 170:		/* geocoll_textm2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 */
	  yytestcase (yyruleno == 170);
      case 179:		/* geocoll_textz2 ::= VANUATU_COMMA pointz geocoll_textz2 */
	  yytestcase (yyruleno == 179);
      case 180:		/* geocoll_textz2 ::= VANUATU_COMMA linestringz geocoll_textz2 */
	  yytestcase (yyruleno == 180);
      case 181:		/* geocoll_textz2 ::= VANUATU_COMMA polygonz geocoll_textz2 */
	  yytestcase (yyruleno == 181);
      case 182:		/* geocoll_textz2 ::= VANUATU_COMMA multipointz geocoll_textz2 */
	  yytestcase (yyruleno == 182);
      case 183:		/* geocoll_textz2 ::= VANUATU_COMMA multilinestringz geocoll_textz2 */
	  yytestcase (yyruleno == 183);
      case 184:		/* geocoll_textz2 ::= VANUATU_COMMA multipolygonz geocoll_textz2 */
	  yytestcase (yyruleno == 184);
      case 185:		/* geocoll_textz2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz geocoll_textz2 */
	  yytestcase (yyruleno == 185);
      case 194:		/* geocoll_textzm2 ::= VANUATU_COMMA pointzm geocoll_textzm2 */
	  yytestcase (yyruleno == 194);
      case 195:		/* geocoll_textzm2 ::= VANUATU_COMMA linestringzm geocoll_textzm2 */
	  yytestcase (yyruleno == 195);
      case 196:		/* geocoll_textzm2 ::= VANUATU_COMMA polygonzm geocoll_textzm2 */
	  yytestcase (yyruleno == 196);
      case 197:		/* geocoll_textzm2 ::= VANUATU_COMMA multipointzm geocoll_textzm2 */
	  yytestcase (yyruleno == 197);
      case 198:		/* geocoll_textzm2 ::= VANUATU_COMMA multilinestringzm geocoll_textzm2 */
	  yytestcase (yyruleno == 198);
      case 199:		/* geocoll_textzm2 ::= VANUATU_COMMA multipolygonzm geocoll_textzm2 */
	  yytestcase (yyruleno == 199);
      case 200:		/* geocoll_textzm2 ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm geocoll_textzm2 */
	  yytestcase (yyruleno == 200);
	  {
	      ((gaiaGeomCollPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 156:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET pointm geocoll_textm2 VANUATU_CLOSE_BRACKET */
      case 157:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET linestringm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 157);
      case 158:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET polygonm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 158);
      case 159:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET multipointm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 159);
      case 160:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET multilinestringm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 160);
      case 161:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET multipolygonm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 161);
      case 162:		/* geocoll_textm ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 162);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_geomColl_xym (p_data,
						 (gaiaGeomCollPtr)
						 yymsp[-2].minor.yy0);
	  }
	  break;
      case 171:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET pointz geocoll_textz2 VANUATU_CLOSE_BRACKET */
      case 172:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET linestringz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 172);
      case 173:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET polygonz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 173);
      case 174:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET multipointz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 174);
      case 175:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET multilinestringz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 175);
      case 176:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET multipolygonz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 176);
      case 177:		/* geocoll_textz ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz geocoll_textz2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 177);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_geomColl_xyz (p_data,
						 (gaiaGeomCollPtr)
						 yymsp[-2].minor.yy0);
	  }
	  break;
      case 186:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET pointzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
      case 187:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET linestringzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 187);
      case 188:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET polygonzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 188);
      case 189:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET multipointzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 189);
      case 190:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET multilinestringzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 190);
      case 191:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET multipolygonzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 191);
      case 192:		/* geocoll_textzm ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm geocoll_textzm2 VANUATU_CLOSE_BRACKET */
	  yytestcase (yyruleno == 192);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) vanuatu_geomColl_xyzm (p_data,
						  (gaiaGeomCollPtr)
						  yymsp[-2].minor.yy0);
	  }
	  break;
      default:
	  /* (0) main ::= in */ yytestcase (yyruleno == 0);
	  /* (1) in ::= */ yytestcase (yyruleno == 1);
	  /* (2) in ::= in state VANUATU_NEWLINE */ yytestcase (yyruleno == 2);
	  /* (3) state ::= program */ yytestcase (yyruleno == 3);
	  /* (4) program ::= geo_text */ yytestcase (yyruleno == 4);
	  /* (5) program ::= geo_textz */ yytestcase (yyruleno == 5);
	  /* (6) program ::= geo_textm */ yytestcase (yyruleno == 6);
	  /* (7) program ::= geo_textzm */ yytestcase (yyruleno == 7);
	  break;
      };
    yygoto = yyRuleInfo[yyruleno].lhs;
    yysize = yyRuleInfo[yyruleno].nrhs;
    yypParser->yyidx -= yysize;
    yyact = yy_find_reduce_action (yymsp[-yysize].stateno, (YYCODETYPE) yygoto);
    if (yyact < YYNSTATE)
      {
#ifdef NDEBUG
	  /* If we are not debugging and the reduce action popped at least
	   ** one element off the stack, then we can push the new element back
	   ** onto the stack here, and skip the stack overflow test in yy_shift().
	   ** That gives a significant speed improvement. */
	  if (yysize)
	    {
		yypParser->yyidx++;
		yymsp -= yysize - 1;
		yymsp->stateno = (YYACTIONTYPE) yyact;
		yymsp->major = (YYCODETYPE) yygoto;
		yymsp->minor = yygotominor;
	    }
	  else
#endif
	    {
		yy_shift (yypParser, yyact, yygoto, &yygotominor);
	    }
      }
    else
      {
	  assert (yyact == YYNSTATE + YYNRULE + 1);
	  yy_accept (yypParser);
      }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void
yy_parse_failed (yyParser * yypParser	/* The parser */
    )
{
    ParseARG_FETCH;
#ifndef NDEBUG
    if (yyTraceFILE)
      {
	  fprintf (yyTraceFILE, "%sFail!\n", yyTracePrompt);
      }
#endif
    while (yypParser->yyidx >= 0)
	yy_pop_parser_stack (yypParser);
    /* Here code is inserted which will be executed whenever the
     ** parser fails */
    ParseARG_STORE;		/* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void
yy_syntax_error (yyParser * yypParser,	/* The parser */
		 int yymajor,	/* The major type of the error token */
		 YYMINORTYPE yyminor	/* The minor type of the error token */
    )
{
    ParseARG_FETCH;
#define TOKEN (yyminor.yy0)

/* 
** Sandro Furieri 2010 Apr 4
** when the LEMON parser encounters an error
** then this global variable is set 
*/
    p_data->vanuatu_parse_error = 1;
    p_data->result = NULL;
    ParseARG_STORE;		/* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void
yy_accept (yyParser * yypParser	/* The parser */
    )
{
    ParseARG_FETCH;
#ifndef NDEBUG
    if (yyTraceFILE)
      {
	  fprintf (yyTraceFILE, "%sAccept!\n", yyTracePrompt);
      }
#endif
    while (yypParser->yyidx >= 0)
	yy_pop_parser_stack (yypParser);
    /* Here code is inserted which will be executed whenever the
     ** parser accepts */
    ParseARG_STORE;		/* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void
Parse (void *yyp,		/* The parser */
       int yymajor,		/* The major token code number */
       ParseTOKENTYPE yyminor	/* The value for the token */
       ParseARG_PDECL		/* Optional %extra_argument parameter */
    )
{
    YYMINORTYPE yyminorunion;
    int yyact;			/* The parser action. */
    int yyendofinput;		/* True if we are at the end of input */
#ifdef YYERRORSYMBOL
    int yyerrorhit = 0;		/* True if yymajor has invoked an error */
#endif
    yyParser *yypParser;	/* The parser */

    /* (re)initialize the parser, if necessary */
    yypParser = (yyParser *) yyp;
    if (yypParser->yyidx < 0)
      {
#if YYSTACKDEPTH<=0
	  if (yypParser->yystksz <= 0)
	    {
		/*memset(&yyminorunion, 0, sizeof(yyminorunion)); */
		yyminorunion = yyzerominor;
		yyStackOverflow (yypParser, &yyminorunion);
		return;
	    }
#endif
	  yypParser->yyidx = 0;
	  yypParser->yyerrcnt = -1;
	  yypParser->yystack[0].stateno = 0;
	  yypParser->yystack[0].major = 0;
      }
    yyminorunion.yy0 = yyminor;
    yyendofinput = (yymajor == 0);
    ParseARG_STORE;

#ifndef NDEBUG
    if (yyTraceFILE)
      {
	  fprintf (yyTraceFILE, "%sInput %s\n", yyTracePrompt,
		   yyTokenName[yymajor]);
      }
#endif

    do
      {
	  yyact = yy_find_shift_action (yypParser, (YYCODETYPE) yymajor);
	  if (yyact < YYNSTATE)
	    {
		assert (!yyendofinput);	/* Impossible to shift the $ token */
		yy_shift (yypParser, yyact, yymajor, &yyminorunion);
		yypParser->yyerrcnt--;
		yymajor = YYNOCODE;
	    }
	  else if (yyact < YYNSTATE + YYNRULE)
	    {
		yy_reduce (yypParser, yyact - YYNSTATE);
	    }
	  else
	    {
		assert (yyact == YY_ERROR_ACTION);
#ifdef YYERRORSYMBOL
		int yymx;
#endif
#ifndef NDEBUG
		if (yyTraceFILE)
		  {
		      fprintf (yyTraceFILE, "%sSyntax Error!\n", yyTracePrompt);
		  }
#endif
#ifdef YYERRORSYMBOL
		/* A syntax error has occurred.
		 ** The response to an error depends upon whether or not the
		 ** grammar defines an error token "ERROR".  
		 **
		 ** This is what we do if the grammar does define ERROR:
		 **
		 **  * Call the %syntax_error function.
		 **
		 **  * Begin popping the stack until we enter a state where
		 **    it is legal to shift the error symbol, then shift
		 **    the error symbol.
		 **
		 **  * Set the error count to three.
		 **
		 **  * Begin accepting and shifting new tokens.  No new error
		 **    processing will occur until three tokens have been
		 **    shifted successfully.
		 **
		 */
		if (yypParser->yyerrcnt < 0)
		  {
		      yy_syntax_error (yypParser, yymajor, yyminorunion);
		  }
		yymx = yypParser->yystack[yypParser->yyidx].major;
		if (yymx == YYERRORSYMBOL || yyerrorhit)
		  {
#ifndef NDEBUG
		      if (yyTraceFILE)
			{
			    fprintf (yyTraceFILE, "%sDiscard input token %s\n",
				     yyTracePrompt, yyTokenName[yymajor]);
			}
#endif
		      yy_destructor (yypParser, (YYCODETYPE) yymajor,
				     &yyminorunion);
		      yymajor = YYNOCODE;
		  }
		else
		  {
		      while (yypParser->yyidx >= 0 &&
			     yymx != YYERRORSYMBOL &&
			     (yyact =
			      yy_find_reduce_action (yypParser->yystack
						     [yypParser->yyidx].stateno,
						     YYERRORSYMBOL)) >=
			     YYNSTATE)
			{
			    yy_pop_parser_stack (yypParser);
			}
		      if (yypParser->yyidx < 0 || yymajor == 0)
			{
			    yy_destructor (yypParser, (YYCODETYPE) yymajor,
					   &yyminorunion);
			    yy_parse_failed (yypParser);
			    yymajor = YYNOCODE;
			}
		      else if (yymx != YYERRORSYMBOL)
			{
			    YYMINORTYPE u2;
			    u2.YYERRSYMDT = 0;
			    yy_shift (yypParser, yyact, YYERRORSYMBOL, &u2);
			}
		  }
		yypParser->yyerrcnt = 3;
		yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
		/* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
		 ** do any kind of error recovery.  Instead, simply invoke the syntax
		 ** error routine and continue going as if nothing had happened.
		 **
		 ** Applications can set this macro (for example inside %include) if
		 ** they intend to abandon the parse upon the first syntax error seen.
		 */
		yy_syntax_error (yypParser, yymajor, yyminorunion);
		yy_destructor (yypParser, (YYCODETYPE) yymajor, &yyminorunion);
		yymajor = YYNOCODE;

#else /* YYERRORSYMBOL is not defined */
		/* This is what we do if the grammar does not define ERROR:
		 **
		 **  * Report an error message, and throw away the input token.
		 **
		 **  * If the input token is $, then fail the parse.
		 **
		 ** As before, subsequent error messages are suppressed until
		 ** three input tokens have been successfully shifted.
		 */
		if (yypParser->yyerrcnt <= 0)
		  {
		      yy_syntax_error (yypParser, yymajor, yyminorunion);
		  }
		yypParser->yyerrcnt = 3;
		yy_destructor (yypParser, (YYCODETYPE) yymajor, &yyminorunion);
		if (yyendofinput)
		  {
		      yy_parse_failed (yypParser);
		  }
		yymajor = YYNOCODE;
#endif
	    }
      }
    while (yymajor != YYNOCODE && yypParser->yyidx >= 0);
    return;
}
