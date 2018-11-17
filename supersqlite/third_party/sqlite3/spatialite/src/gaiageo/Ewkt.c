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
#define YYNOCODE 117
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
#define ParseARG_SDECL  struct ewkt_data *p_data ;
#define ParseARG_PDECL , struct ewkt_data *p_data
#define ParseARG_FETCH  struct ewkt_data *p_data  = yypParser->p_data
#define ParseARG_STORE yypParser->p_data  = p_data
#define YYNSTATE 508
#define YYNRULE 199
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
    /*     0 */ 249, 336, 337, 338, 339, 340, 341, 342, 343, 344,
    /*    10 */ 345, 346, 347, 348, 349, 350, 351, 352, 353, 354,
    /*    20 */ 355, 356, 357, 358, 359, 360, 361, 362, 363, 364,
    /*    30 */ 365, 366, 163, 201, 224, 178, 202, 225, 179, 203,
    /*    40 */ 226, 180, 204, 227, 181, 205, 228, 182, 206, 229,
    /*    50 */ 508, 101, 250, 128, 371, 254, 66, 129, 19, 80,
    /*    60 */ 20, 95, 22, 124, 23, 141, 25, 158, 27, 232,
    /*    70 */ 250, 254, 708, 1, 13, 80, 19, 95, 20, 124,
    /*    80 */ 22, 141, 23, 158, 25, 247, 28, 103, 104, 371,
    /*    90 */ 105, 106, 63, 305, 111, 117, 305, 255, 21, 164,
    /*   100 */ 66, 165, 164, 166, 165, 167, 166, 169, 167, 177,
    /*   110 */ 169, 313, 183, 374, 313, 375, 376, 187, 14, 188,
    /*   120 */ 187, 189, 188, 190, 189, 192, 190, 200, 192, 321,
    /*   130 */ 207, 404, 321, 405, 406, 210, 62, 211, 210, 212,
    /*   140 */ 211, 213, 212, 215, 213, 223, 215, 254, 230, 3,
    /*   150 */ 24, 80, 104, 95, 26, 124, 71, 141, 111, 158,
    /*   160 */ 171, 240, 2, 172, 194, 163, 173, 195, 178, 174,
    /*   170 */ 196, 179, 175, 197, 180, 176, 198, 181, 217, 199,
    /*   180 */ 182, 218, 201, 125, 219, 202, 371, 220, 203, 224,
    /*   190 */ 221, 204, 225, 222, 205, 226, 108, 206, 227, 371,
    /*   200 */ 388, 228, 389, 390, 229, 284, 233, 242, 243, 244,
    /*   210 */ 245, 246, 234, 235, 236, 237, 238, 239, 31, 113,
    /*   220 */ 251, 252, 371, 253, 83, 63, 256, 258, 87, 260,
    /*   230 */ 91, 63, 265, 270, 103, 275, 257, 63, 106, 68,
    /*   240 */ 105, 78, 75, 75, 424, 117, 425, 426, 132, 119,
    /*   250 */ 135, 138, 371, 436, 64, 437, 438, 146, 69, 150,
    /*   260 */ 154, 65, 370, 68, 448, 70, 449, 450, 67, 184,
    /*   270 */ 68, 186, 209, 73, 74, 373, 33, 71, 71, 79,
    /*   280 */ 262, 75, 81, 66, 82, 66, 265, 66, 266, 267,
    /*   290 */ 85, 68, 270, 68, 68, 68, 71, 271, 272, 89,
    /*   300 */ 258, 71, 71, 71, 71, 275, 276, 75, 75, 277,
    /*   310 */ 93, 75, 75, 280, 281, 282, 66, 66, 66, 377,
    /*   320 */ 256, 99, 251, 378, 66, 68, 260, 68, 75, 252,
    /*   330 */ 253, 72, 75, 71, 407, 408, 371, 371, 380, 76,
    /*   340 */ 259, 77, 35, 36, 381, 38, 261, 383, 84, 263,
    /*   350 */ 385, 386, 39, 88, 268, 43, 47, 92, 96, 273,
    /*   360 */ 51, 98, 102, 278, 264, 86, 61, 283, 393, 269,
    /*   370 */ 287, 285, 90, 400, 286, 396, 107, 97, 274, 94,
    /*   380 */ 279, 399, 100, 289, 403, 108, 110, 413, 109, 288,
    /*   390 */ 112, 291, 113, 114, 116, 118, 115, 290, 119, 15,
    /*   400 */ 415, 419, 125, 120, 121, 123, 292, 122, 126, 130,
    /*   410 */ 133, 127, 417, 296, 55, 136, 293, 428, 297, 56,
    /*   420 */ 430, 131, 139, 298, 295, 57, 422, 294, 142, 148,
    /*   430 */ 144, 147, 301, 152, 134, 151, 155, 137, 156, 140,
    /*   440 */ 143, 159, 709, 145, 161, 16, 4, 168, 170, 149,
    /*   450 */ 5, 17, 709, 6, 83, 153, 709, 191, 193, 433,
    /*   460 */ 87, 709, 160, 157, 432, 299, 91, 162, 435, 374,
    /*   470 */ 388, 7, 132, 709, 709, 709, 300, 709, 8, 440,
    /*   480 */ 146, 404, 709, 18, 442, 214, 302, 445, 444, 216,
    /*   490 */ 424, 709, 303, 709, 709, 709, 447, 709, 709, 709,
    /*   500 */ 709, 436, 709, 709, 304, 709, 709, 375, 9, 709,
    /*   510 */ 135, 709, 389, 709, 709, 709, 709, 452, 453, 454,
    /*   520 */ 709, 455, 456, 709, 457, 185, 306, 312, 709, 150,
    /*   530 */ 307, 308, 309, 310, 425, 184, 10, 11, 709, 405,
    /*   540 */ 709, 709, 709, 311, 464, 709, 709, 709, 709, 709,
    /*   550 */ 709, 709, 437, 709, 376, 466, 390, 709, 335, 709,
    /*   560 */ 138, 709, 12, 709, 709, 367, 467, 154, 709, 29,
    /*   570 */ 468, 208, 469, 470, 471, 406, 426, 314, 315, 316,
    /*   580 */ 317, 318, 186, 319, 478, 368, 369, 372, 320, 379,
    /*   590 */ 438, 30, 382, 32, 34, 384, 37, 387, 391, 40,
    /*   600 */ 709, 392, 41, 42, 480, 481, 482, 394, 483, 44,
    /*   610 */ 58, 45, 484, 46, 395, 48, 397, 485, 59, 231,
    /*   620 */ 49, 50, 398, 52, 322, 401, 323, 53, 324, 54,
    /*   630 */ 402, 409, 493, 325, 326, 327, 209, 492, 410, 328,
    /*   640 */ 495, 496, 411, 412, 497, 498, 241, 499, 500, 501,
    /*   650 */ 407, 329, 414, 330, 408, 416, 418, 331, 332, 420,
    /*   660 */ 421, 423, 427, 429, 248, 431, 333, 434, 439, 334,
    /*   670 */ 441, 443, 446, 451, 458, 459, 460, 461, 462, 463,
    /*   680 */ 465, 472, 473, 474, 475, 476, 477, 479, 60, 709,
    /*   690 */ 486, 487, 488, 489, 709, 490, 491, 494, 502, 709,
    /*   700 */ 503, 504, 505, 506, 709, 507,
};

static const YYCODETYPE yy_lookahead[] = {
    /*     0 */ 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    /*    10 */ 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    /*    20 */ 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    /*    30 */ 53, 54, 27, 28, 29, 30, 31, 32, 33, 34,
    /*    40 */ 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    /*    50 */ 0, 3, 2, 57, 6, 5, 60, 61, 8, 9,
    /*    60 */ 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    /*    70 */ 2, 5, 21, 22, 3, 9, 8, 11, 10, 13,
    /*    80 */ 12, 15, 14, 17, 16, 19, 18, 55, 56, 6,
    /*    90 */ 58, 59, 60, 2, 62, 63, 2, 57, 3, 8,
    /*   100 */ 60, 10, 8, 12, 10, 14, 12, 16, 14, 18,
    /*   110 */ 16, 2, 18, 72, 2, 74, 75, 8, 3, 10,
    /*   120 */ 8, 12, 10, 14, 12, 16, 14, 18, 16, 2,
    /*   130 */ 18, 88, 2, 90, 91, 8, 60, 10, 8, 12,
    /*   140 */ 10, 14, 12, 16, 14, 18, 16, 5, 18, 3,
    /*   150 */ 3, 9, 56, 11, 3, 13, 60, 15, 62, 17,
    /*   160 */ 27, 19, 3, 30, 28, 27, 33, 31, 30, 36,
    /*   170 */ 34, 33, 39, 37, 36, 42, 40, 39, 29, 43,
    /*   180 */ 42, 32, 28, 3, 35, 31, 6, 38, 34, 29,
    /*   190 */ 41, 37, 32, 44, 40, 35, 3, 43, 38, 6,
    /*   200 */ 76, 41, 78, 79, 44, 60, 48, 49, 50, 51,
    /*   210 */ 52, 53, 48, 49, 50, 51, 52, 53, 7, 3,
    /*   220 */ 55, 56, 6, 58, 80, 60, 55, 56, 84, 58,
    /*   230 */ 86, 60, 55, 56, 55, 58, 68, 60, 59, 60,
    /*   240 */ 58, 58, 60, 60, 92, 63, 94, 95, 72, 3,
    /*   250 */ 74, 75, 6, 100, 60, 102, 103, 76, 55, 78,
    /*   260 */ 79, 60, 60, 60, 108, 55, 110, 111, 60, 108,
    /*   270 */ 60, 110, 111, 56, 56, 60, 7, 60, 60, 58,
    /*   280 */ 57, 60, 57, 60, 57, 60, 55, 60, 55, 55,
    /*   290 */ 55, 60, 56, 60, 60, 60, 60, 56, 56, 56,
    /*   300 */ 56, 60, 60, 60, 60, 58, 58, 60, 60, 58,
    /*   310 */ 58, 60, 60, 57, 57, 57, 60, 60, 60, 60,
    /*   320 */ 55, 57, 55, 68, 60, 60, 58, 60, 60, 56,
    /*   330 */ 58, 60, 60, 60, 4, 4, 6, 6, 60, 60,
    /*   340 */ 70, 60, 7, 3, 70, 7, 71, 71, 7, 69,
    /*   350 */ 73, 69, 3, 7, 68, 3, 3, 7, 3, 70,
    /*   360 */ 3, 7, 60, 71, 81, 80, 60, 69, 81, 85,
    /*   370 */ 71, 68, 84, 77, 70, 85, 7, 82, 87, 86,
    /*   380 */ 83, 87, 82, 64, 83, 3, 59, 64, 60, 60,
    /*   390 */ 7, 66, 3, 60, 62, 7, 60, 60, 3, 3,
    /*   400 */ 66, 89, 3, 60, 60, 63, 67, 60, 60, 7,
    /*   410 */ 7, 60, 67, 96, 3, 7, 60, 96, 98, 3,
    /*   420 */ 98, 61, 7, 99, 65, 3, 65, 69, 3, 3,
    /*   430 */ 7, 7, 106, 3, 72, 7, 7, 74, 3, 75,
    /*   440 */ 73, 3, 116, 73, 7, 3, 7, 3, 3, 76,
    /*   450 */ 3, 3, 116, 7, 80, 78, 116, 3, 3, 93,
    /*   460 */ 84, 116, 77, 79, 99, 97, 86, 77, 97, 72,
    /*   470 */ 76, 3, 72, 116, 116, 116, 104, 116, 7, 104,
    /*   480 */ 76, 88, 116, 3, 106, 3, 107, 101, 107, 3,
    /*   490 */ 92, 116, 105, 116, 116, 116, 105, 116, 116, 116,
    /*   500 */ 116, 100, 116, 116, 112, 116, 116, 74, 3, 116,
    /*   510 */ 74, 116, 78, 116, 116, 116, 116, 112, 112, 112,
    /*   520 */ 116, 112, 112, 116, 112, 108, 112, 114, 116, 78,
    /*   530 */ 112, 112, 112, 112, 94, 108, 3, 7, 116, 90,
    /*   540 */ 116, 116, 116, 112, 112, 116, 116, 116, 116, 116,
    /*   550 */ 116, 116, 102, 116, 75, 114, 79, 116, 1, 116,
    /*   560 */ 75, 116, 3, 116, 116, 4, 114, 79, 116, 3,
    /*   570 */ 114, 110, 114, 114, 114, 91, 95, 114, 114, 114,
    /*   580 */ 114, 114, 110, 114, 114, 4, 4, 4, 115, 4,
    /*   590 */ 103, 7, 4, 7, 7, 4, 7, 4, 4, 7,
    /*   600 */ 116, 4, 7, 7, 115, 115, 115, 4, 115, 7,
    /*   610 */ 3, 7, 115, 7, 4, 7, 4, 115, 3, 111,
    /*   620 */ 7, 7, 4, 7, 115, 4, 115, 7, 115, 7,
    /*   630 */ 4, 4, 109, 115, 115, 115, 111, 115, 4, 113,
    /*   640 */ 113, 113, 4, 4, 113, 113, 109, 113, 113, 113,
    /*   650 */ 4, 113, 4, 113, 4, 4, 4, 113, 113, 4,
    /*   660 */ 4, 4, 4, 4, 109, 4, 113, 4, 4, 113,
    /*   670 */ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    /*   680 */ 4, 4, 4, 4, 4, 4, 4, 4, 3, 116,
    /*   690 */ 4, 4, 4, 4, 116, 4, 4, 4, 4, 116,
    /*   700 */ 4, 4, 4, 4, 116, 4,
};

#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_MAX 334
static const short yy_shift_ofst[] = {
    /*     0 */ -1, 50, 68, 48, 91, 94, 109, 112, 127, 130,
    /*    10 */ 66, 142, 83, 83, 83, 180, 193, 216, 246, 71,
    /*    20 */ 95, 115, 146, 147, 71, 151, 95, 159, 159, 83,
    /*    30 */ 83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
    /*    40 */ 83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
    /*    50 */ 83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
    /*    60 */ 83, 330, 331, 83, 83, 83, 83, 83, 83, 211,
    /*    70 */ 211, 83, 83, 269, 269, 83, 83, 83, 335, 335,
    /*    80 */ 340, 338, 338, 341, 349, 211, 341, 346, 352, 269,
    /*    90 */ 346, 350, 353, 335, 350, 355, 357, 354, 357, 338,
    /*   100 */ 354, 83, 83, 211, 269, 335, 369, 382, 83, 83,
    /*   110 */ 369, 383, 389, 83, 83, 83, 383, 388, 395, 83,
    /*   120 */ 83, 83, 83, 388, 396, 83, 83, 83, 338, 402,
    /*   130 */ 399, 402, 403, 411, 403, 408, 416, 408, 415, 422,
    /*   140 */ 415, 425, 340, 423, 340, 423, 424, 426, 349, 424,
    /*   150 */ 428, 430, 352, 428, 429, 435, 353, 429, 438, 355,
    /*   160 */ 437, 355, 437, 439, 411, 426, 442, 444, 411, 445,
    /*   170 */ 426, 439, 439, 439, 439, 439, 439, 447, 439, 439,
    /*   180 */ 439, 439, 439, 447, 439, 439, 446, 416, 430, 448,
    /*   190 */ 454, 416, 455, 430, 446, 446, 446, 446, 446, 446,
    /*   200 */ 468, 446, 446, 446, 446, 446, 446, 468, 446, 471,
    /*   210 */ 422, 435, 480, 482, 422, 486, 435, 471, 471, 471,
    /*   220 */ 471, 471, 471, 505, 471, 471, 471, 471, 471, 471,
    /*   230 */ 505, 471, 533, 530, 530, 530, 530, 530, 530, 530,
    /*   240 */ 533, 530, 530, 530, 530, 530, 530, 533, 530, 557,
    /*   250 */ 559, 561, 581, 582, 566, 583, 584, 585, 586, 588,
    /*   260 */ 587, 591, 589, 593, 594, 592, 595, 596, 597, 603,
    /*   270 */ 602, 604, 606, 610, 612, 608, 613, 614, 618, 621,
    /*   280 */ 616, 620, 622, 626, 627, 634, 638, 639, 646, 648,
    /*   290 */ 650, 651, 652, 655, 656, 657, 658, 659, 661, 663,
    /*   300 */ 664, 666, 667, 668, 669, 607, 670, 671, 672, 673,
    /*   310 */ 674, 675, 676, 615, 677, 678, 679, 680, 681, 682,
    /*   320 */ 683, 685, 686, 687, 688, 689, 691, 692, 693, 694,
    /*   330 */ 696, 697, 698, 699, 701,
};

#define YY_REDUCE_USE_DFLT (-24)
#define YY_REDUCE_MAX 248
static const short yy_reduce_ofst[] = {
    /*     0 */ 51, -23, 5, 32, 133, 138, 136, 154, 149, 160,
    /*    10 */ 158, 164, 165, 171, 177, -4, 179, 96, 182, 41,
    /*    20 */ 124, 144, 43, 152, 176, 153, 181, 156, 161, 40,
    /*    30 */ 203, 210, 217, 218, 183, 221, 223, 225, 227, 231,
    /*    40 */ 233, 234, 235, 236, 241, 242, 243, 247, 248, 251,
    /*    50 */ 252, 256, 257, 258, 264, 265, 244, 268, 267, 273,
    /*    60 */ 272, 76, 145, 194, 201, 202, 208, 215, 259, 168,
    /*    70 */ 255, 271, 278, 270, 274, 279, 281, 202, 275, 276,
    /*    80 */ 277, 280, 282, 283, 285, 286, 287, 284, 288, 289,
    /*    90 */ 290, 291, 293, 292, 294, 296, 295, 297, 300, 298,
    /*   100 */ 301, 302, 306, 303, 304, 299, 319, 327, 328, 329,
    /*   110 */ 323, 325, 332, 333, 336, 337, 334, 339, 342, 343,
    /*   120 */ 344, 347, 145, 345, 312, 348, 351, 356, 358, 359,
    /*   130 */ 360, 361, 317, 362, 321, 320, 363, 322, 324, 364,
    /*   140 */ 365, 366, 367, 368, 370, 371, 372, 373, 374, 375,
    /*   150 */ 326, 377, 376, 378, 379, 384, 380, 381, 386, 385,
    /*   160 */ 387, 390, 391, 392, 397, 394, 393, 398, 400, 401,
    /*   170 */ 404, 405, 406, 407, 409, 410, 412, 417, 414, 418,
    /*   180 */ 419, 420, 421, 427, 431, 432, 413, 433, 434, 449,
    /*   190 */ 440, 436, 450, 451, 441, 452, 456, 458, 459, 460,
    /*   200 */ 461, 463, 464, 465, 466, 467, 469, 472, 470, 473,
    /*   210 */ 479, 477, 484, 481, 485, 487, 488, 489, 490, 491,
    /*   220 */ 493, 497, 502, 508, 509, 511, 513, 518, 519, 520,
    /*   230 */ 525, 522, 523, 526, 527, 528, 531, 532, 534, 535,
    /*   240 */ 537, 536, 538, 540, 544, 545, 553, 555, 556,
};

static const YYACTIONTYPE yy_default[] = {
    /*     0 */ 509, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    10 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    20 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    30 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    40 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    50 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*    60 */ 707, 707, 707, 707, 550, 552, 707, 707, 707, 563,
    /*    70 */ 563, 707, 707, 567, 567, 707, 707, 707, 569, 569,
    /*    80 */ 707, 565, 565, 588, 707, 563, 588, 594, 707, 567,
    /*    90 */ 594, 597, 707, 569, 597, 707, 707, 591, 707, 565,
    /*   100 */ 591, 707, 707, 563, 567, 569, 555, 707, 707, 707,
    /*   110 */ 555, 559, 707, 707, 707, 707, 559, 561, 707, 707,
    /*   120 */ 707, 707, 707, 561, 707, 707, 707, 707, 565, 557,
    /*   130 */ 707, 557, 616, 707, 616, 622, 707, 622, 625, 707,
    /*   140 */ 625, 707, 707, 619, 707, 619, 632, 707, 707, 632,
    /*   150 */ 638, 707, 707, 638, 641, 707, 707, 641, 707, 707,
    /*   160 */ 635, 707, 635, 654, 707, 707, 707, 707, 707, 707,
    /*   170 */ 707, 654, 654, 654, 654, 654, 654, 707, 654, 654,
    /*   180 */ 654, 654, 654, 707, 654, 654, 684, 707, 707, 707,
    /*   190 */ 707, 707, 707, 707, 684, 684, 684, 684, 684, 684,
    /*   200 */ 707, 684, 684, 684, 684, 684, 684, 707, 684, 699,
    /*   210 */ 707, 707, 707, 707, 707, 707, 707, 699, 699, 699,
    /*   220 */ 699, 699, 699, 707, 699, 699, 699, 699, 699, 699,
    /*   230 */ 707, 699, 707, 669, 669, 669, 669, 669, 669, 669,
    /*   240 */ 707, 669, 669, 669, 669, 669, 669, 707, 669, 707,
    /*   250 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   260 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   270 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   280 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   290 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   300 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   310 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   320 */ 707, 707, 707, 707, 707, 707, 707, 707, 707, 707,
    /*   330 */ 707, 707, 707, 707, 707, 510, 511, 512, 513, 514,
    /*   340 */ 515, 516, 517, 518, 519, 520, 521, 522, 523, 524,
    /*   350 */ 525, 526, 527, 528, 529, 530, 531, 532, 533, 534,
    /*   360 */ 535, 536, 537, 538, 539, 540, 541, 542, 543, 545,
    /*   370 */ 553, 554, 544, 551, 571, 573, 574, 550, 564, 575,
    /*   380 */ 552, 568, 577, 570, 578, 572, 566, 576, 579, 581,
    /*   390 */ 582, 583, 587, 589, 585, 593, 595, 586, 596, 598,
    /*   400 */ 580, 584, 590, 592, 599, 601, 602, 546, 548, 549,
    /*   410 */ 603, 605, 606, 556, 607, 560, 609, 562, 610, 600,
    /*   420 */ 547, 604, 558, 608, 611, 613, 614, 615, 617, 621,
    /*   430 */ 623, 624, 626, 612, 618, 620, 627, 629, 630, 631,
    /*   440 */ 633, 637, 639, 640, 642, 628, 634, 636, 643, 645,
    /*   450 */ 646, 647, 655, 656, 657, 658, 659, 660, 648, 649,
    /*   460 */ 650, 651, 652, 653, 661, 683, 685, 686, 687, 688,
    /*   470 */ 689, 690, 677, 678, 679, 680, 681, 682, 691, 698,
    /*   480 */ 700, 701, 702, 703, 704, 705, 692, 693, 694, 695,
    /*   490 */ 696, 697, 706, 644, 662, 670, 671, 672, 673, 674,
    /*   500 */ 675, 676, 663, 664, 665, 666, 667, 668,
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
    "$", "EWKT_NEWLINE", "EWKT_POINT", "EWKT_OPEN_BRACKET",
    "EWKT_CLOSE_BRACKET", "EWKT_POINT_M", "EWKT_NUM", "EWKT_COMMA",
    "EWKT_LINESTRING", "EWKT_LINESTRING_M", "EWKT_POLYGON", "EWKT_POLYGON_M",
    "EWKT_MULTIPOINT", "EWKT_MULTIPOINT_M", "EWKT_MULTILINESTRING",
    "EWKT_MULTILINESTRING_M",
    "EWKT_MULTIPOLYGON", "EWKT_MULTIPOLYGON_M", "EWKT_GEOMETRYCOLLECTION",
    "EWKT_GEOMETRYCOLLECTION_M",
    "error", "main", "in", "state",
    "program", "geo_text", "geo_textm", "point",
    "pointz", "pointzm", "linestring", "linestringz",
    "linestringzm", "polygon", "polygonz", "polygonzm",
    "multipoint", "multipointz", "multipointzm", "multilinestring",
    "multilinestringz", "multilinestringzm", "multipolygon", "multipolygonz",
    "multipolygonzm", "geocoll", "geocollz", "geocollzm",
    "pointm", "linestringm", "polygonm", "multipointm",
    "multilinestringm", "multipolygonm", "geocollm", "point_coordxy",
    "point_coordxyz", "point_coordxym", "point_coordxyzm", "point_brkt_coordxy",
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
    /*   2 */ "in ::= in state EWKT_NEWLINE",
    /*   3 */ "state ::= program",
    /*   4 */ "program ::= geo_text",
    /*   5 */ "program ::= geo_textm",
    /*   6 */ "geo_text ::= point",
    /*   7 */ "geo_text ::= pointz",
    /*   8 */ "geo_text ::= pointzm",
    /*   9 */ "geo_text ::= linestring",
    /*  10 */ "geo_text ::= linestringz",
    /*  11 */ "geo_text ::= linestringzm",
    /*  12 */ "geo_text ::= polygon",
    /*  13 */ "geo_text ::= polygonz",
    /*  14 */ "geo_text ::= polygonzm",
    /*  15 */ "geo_text ::= multipoint",
    /*  16 */ "geo_text ::= multipointz",
    /*  17 */ "geo_text ::= multipointzm",
    /*  18 */ "geo_text ::= multilinestring",
    /*  19 */ "geo_text ::= multilinestringz",
    /*  20 */ "geo_text ::= multilinestringzm",
    /*  21 */ "geo_text ::= multipolygon",
    /*  22 */ "geo_text ::= multipolygonz",
    /*  23 */ "geo_text ::= multipolygonzm",
    /*  24 */ "geo_text ::= geocoll",
    /*  25 */ "geo_text ::= geocollz",
    /*  26 */ "geo_text ::= geocollzm",
    /*  27 */ "geo_textm ::= pointm",
    /*  28 */ "geo_textm ::= linestringm",
    /*  29 */ "geo_textm ::= polygonm",
    /*  30 */ "geo_textm ::= multipointm",
    /*  31 */ "geo_textm ::= multilinestringm",
    /*  32 */ "geo_textm ::= multipolygonm",
    /*  33 */ "geo_textm ::= geocollm",
    /*  34 */
    "point ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxy EWKT_CLOSE_BRACKET",
    /*  35 */
    "pointz ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyz EWKT_CLOSE_BRACKET",
    /*  36 */
    "pointm ::= EWKT_POINT_M EWKT_OPEN_BRACKET point_coordxym EWKT_CLOSE_BRACKET",
    /*  37 */
    "pointzm ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyzm EWKT_CLOSE_BRACKET",
    /*  38 */
    "point_brkt_coordxy ::= EWKT_OPEN_BRACKET coord coord EWKT_CLOSE_BRACKET",
    /*  39 */
    "point_brkt_coordxym ::= EWKT_OPEN_BRACKET coord coord coord EWKT_CLOSE_BRACKET",
    /*  40 */
    "point_brkt_coordxyz ::= EWKT_OPEN_BRACKET coord coord coord EWKT_CLOSE_BRACKET",
    /*  41 */
    "point_brkt_coordxyzm ::= EWKT_OPEN_BRACKET coord coord coord coord EWKT_CLOSE_BRACKET",
    /*  42 */ "point_coordxy ::= coord coord",
    /*  43 */ "point_coordxym ::= coord coord coord",
    /*  44 */ "point_coordxyz ::= coord coord coord",
    /*  45 */ "point_coordxyzm ::= coord coord coord coord",
    /*  46 */ "coord ::= EWKT_NUM",
    /*  47 */ "extra_brkt_pointsxy ::=",
    /*  48 */
    "extra_brkt_pointsxy ::= EWKT_COMMA point_brkt_coordxy extra_brkt_pointsxy",
    /*  49 */ "extra_brkt_pointsxym ::=",
    /*  50 */
    "extra_brkt_pointsxym ::= EWKT_COMMA point_brkt_coordxym extra_brkt_pointsxym",
    /*  51 */ "extra_brkt_pointsxyz ::=",
    /*  52 */
    "extra_brkt_pointsxyz ::= EWKT_COMMA point_brkt_coordxyz extra_brkt_pointsxyz",
    /*  53 */ "extra_brkt_pointsxyzm ::=",
    /*  54 */
    "extra_brkt_pointsxyzm ::= EWKT_COMMA point_brkt_coordxyzm extra_brkt_pointsxyzm",
    /*  55 */ "extra_pointsxy ::=",
    /*  56 */ "extra_pointsxy ::= EWKT_COMMA point_coordxy extra_pointsxy",
    /*  57 */ "extra_pointsxym ::=",
    /*  58 */ "extra_pointsxym ::= EWKT_COMMA point_coordxym extra_pointsxym",
    /*  59 */ "extra_pointsxyz ::=",
    /*  60 */ "extra_pointsxyz ::= EWKT_COMMA point_coordxyz extra_pointsxyz",
    /*  61 */ "extra_pointsxyzm ::=",
    /*  62 */
    "extra_pointsxyzm ::= EWKT_COMMA point_coordxyzm extra_pointsxyzm",
    /*  63 */ "linestring ::= EWKT_LINESTRING linestring_text",
    /*  64 */ "linestringm ::= EWKT_LINESTRING_M linestring_textm",
    /*  65 */ "linestringz ::= EWKT_LINESTRING linestring_textz",
    /*  66 */ "linestringzm ::= EWKT_LINESTRING linestring_textzm",
    /*  67 */
    "linestring_text ::= EWKT_OPEN_BRACKET point_coordxy EWKT_COMMA point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET",
    /*  68 */
    "linestring_textm ::= EWKT_OPEN_BRACKET point_coordxym EWKT_COMMA point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET",
    /*  69 */
    "linestring_textz ::= EWKT_OPEN_BRACKET point_coordxyz EWKT_COMMA point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET",
    /*  70 */
    "linestring_textzm ::= EWKT_OPEN_BRACKET point_coordxyzm EWKT_COMMA point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET",
    /*  71 */ "polygon ::= EWKT_POLYGON polygon_text",
    /*  72 */ "polygonm ::= EWKT_POLYGON_M polygon_textm",
    /*  73 */ "polygonz ::= EWKT_POLYGON polygon_textz",
    /*  74 */ "polygonzm ::= EWKT_POLYGON polygon_textzm",
    /*  75 */
    "polygon_text ::= EWKT_OPEN_BRACKET ring extra_rings EWKT_CLOSE_BRACKET",
    /*  76 */
    "polygon_textm ::= EWKT_OPEN_BRACKET ringm extra_ringsm EWKT_CLOSE_BRACKET",
    /*  77 */
    "polygon_textz ::= EWKT_OPEN_BRACKET ringz extra_ringsz EWKT_CLOSE_BRACKET",
    /*  78 */
    "polygon_textzm ::= EWKT_OPEN_BRACKET ringzm extra_ringszm EWKT_CLOSE_BRACKET",
    /*  79 */
    "ring ::= EWKT_OPEN_BRACKET point_coordxy EWKT_COMMA point_coordxy EWKT_COMMA point_coordxy EWKT_COMMA point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET",
    /*  80 */ "extra_rings ::=",
    /*  81 */ "extra_rings ::= EWKT_COMMA ring extra_rings",
    /*  82 */
    "ringm ::= EWKT_OPEN_BRACKET point_coordxym EWKT_COMMA point_coordxym EWKT_COMMA point_coordxym EWKT_COMMA point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET",
    /*  83 */ "extra_ringsm ::=",
    /*  84 */ "extra_ringsm ::= EWKT_COMMA ringm extra_ringsm",
    /*  85 */
    "ringz ::= EWKT_OPEN_BRACKET point_coordxyz EWKT_COMMA point_coordxyz EWKT_COMMA point_coordxyz EWKT_COMMA point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET",
    /*  86 */ "extra_ringsz ::=",
    /*  87 */ "extra_ringsz ::= EWKT_COMMA ringz extra_ringsz",
    /*  88 */
    "ringzm ::= EWKT_OPEN_BRACKET point_coordxyzm EWKT_COMMA point_coordxyzm EWKT_COMMA point_coordxyzm EWKT_COMMA point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET",
    /*  89 */ "extra_ringszm ::=",
    /*  90 */ "extra_ringszm ::= EWKT_COMMA ringzm extra_ringszm",
    /*  91 */ "multipoint ::= EWKT_MULTIPOINT multipoint_text",
    /*  92 */ "multipointm ::= EWKT_MULTIPOINT_M multipoint_textm",
    /*  93 */ "multipointz ::= EWKT_MULTIPOINT multipoint_textz",
    /*  94 */ "multipointzm ::= EWKT_MULTIPOINT multipoint_textzm",
    /*  95 */
    "multipoint_text ::= EWKT_OPEN_BRACKET point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET",
    /*  96 */
    "multipoint_textm ::= EWKT_OPEN_BRACKET point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET",
    /*  97 */
    "multipoint_textz ::= EWKT_OPEN_BRACKET point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET",
    /*  98 */
    "multipoint_textzm ::= EWKT_OPEN_BRACKET point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET",
    /*  99 */
    "multipoint_text ::= EWKT_OPEN_BRACKET point_brkt_coordxy extra_brkt_pointsxy EWKT_CLOSE_BRACKET",
    /* 100 */
    "multipoint_textm ::= EWKT_OPEN_BRACKET point_brkt_coordxym extra_brkt_pointsxym EWKT_CLOSE_BRACKET",
    /* 101 */
    "multipoint_textz ::= EWKT_OPEN_BRACKET point_brkt_coordxyz extra_brkt_pointsxyz EWKT_CLOSE_BRACKET",
    /* 102 */
    "multipoint_textzm ::= EWKT_OPEN_BRACKET point_brkt_coordxyzm extra_brkt_pointsxyzm EWKT_CLOSE_BRACKET",
    /* 103 */ "multilinestring ::= EWKT_MULTILINESTRING multilinestring_text",
    /* 104 */
    "multilinestringm ::= EWKT_MULTILINESTRING_M multilinestring_textm",
    /* 105 */ "multilinestringz ::= EWKT_MULTILINESTRING multilinestring_textz",
    /* 106 */
    "multilinestringzm ::= EWKT_MULTILINESTRING multilinestring_textzm",
    /* 107 */
    "multilinestring_text ::= EWKT_OPEN_BRACKET linestring_text multilinestring_text2 EWKT_CLOSE_BRACKET",
    /* 108 */ "multilinestring_text2 ::=",
    /* 109 */
    "multilinestring_text2 ::= EWKT_COMMA linestring_text multilinestring_text2",
    /* 110 */
    "multilinestring_textm ::= EWKT_OPEN_BRACKET linestring_textm multilinestring_textm2 EWKT_CLOSE_BRACKET",
    /* 111 */ "multilinestring_textm2 ::=",
    /* 112 */
    "multilinestring_textm2 ::= EWKT_COMMA linestring_textm multilinestring_textm2",
    /* 113 */
    "multilinestring_textz ::= EWKT_OPEN_BRACKET linestring_textz multilinestring_textz2 EWKT_CLOSE_BRACKET",
    /* 114 */ "multilinestring_textz2 ::=",
    /* 115 */
    "multilinestring_textz2 ::= EWKT_COMMA linestring_textz multilinestring_textz2",
    /* 116 */
    "multilinestring_textzm ::= EWKT_OPEN_BRACKET linestring_textzm multilinestring_textzm2 EWKT_CLOSE_BRACKET",
    /* 117 */ "multilinestring_textzm2 ::=",
    /* 118 */
    "multilinestring_textzm2 ::= EWKT_COMMA linestring_textzm multilinestring_textzm2",
    /* 119 */ "multipolygon ::= EWKT_MULTIPOLYGON multipolygon_text",
    /* 120 */ "multipolygonm ::= EWKT_MULTIPOLYGON_M multipolygon_textm",
    /* 121 */ "multipolygonz ::= EWKT_MULTIPOLYGON multipolygon_textz",
    /* 122 */ "multipolygonzm ::= EWKT_MULTIPOLYGON multipolygon_textzm",
    /* 123 */
    "multipolygon_text ::= EWKT_OPEN_BRACKET polygon_text multipolygon_text2 EWKT_CLOSE_BRACKET",
    /* 124 */ "multipolygon_text2 ::=",
    /* 125 */
    "multipolygon_text2 ::= EWKT_COMMA polygon_text multipolygon_text2",
    /* 126 */
    "multipolygon_textm ::= EWKT_OPEN_BRACKET polygon_textm multipolygon_textm2 EWKT_CLOSE_BRACKET",
    /* 127 */ "multipolygon_textm2 ::=",
    /* 128 */
    "multipolygon_textm2 ::= EWKT_COMMA polygon_textm multipolygon_textm2",
    /* 129 */
    "multipolygon_textz ::= EWKT_OPEN_BRACKET polygon_textz multipolygon_textz2 EWKT_CLOSE_BRACKET",
    /* 130 */ "multipolygon_textz2 ::=",
    /* 131 */
    "multipolygon_textz2 ::= EWKT_COMMA polygon_textz multipolygon_textz2",
    /* 132 */
    "multipolygon_textzm ::= EWKT_OPEN_BRACKET polygon_textzm multipolygon_textzm2 EWKT_CLOSE_BRACKET",
    /* 133 */ "multipolygon_textzm2 ::=",
    /* 134 */
    "multipolygon_textzm2 ::= EWKT_COMMA polygon_textzm multipolygon_textzm2",
    /* 135 */ "geocoll ::= EWKT_GEOMETRYCOLLECTION geocoll_text",
    /* 136 */ "geocollm ::= EWKT_GEOMETRYCOLLECTION_M geocoll_textm",
    /* 137 */ "geocollz ::= EWKT_GEOMETRYCOLLECTION geocoll_textz",
    /* 138 */ "geocollzm ::= EWKT_GEOMETRYCOLLECTION geocoll_textzm",
    /* 139 */
    "geocoll_text ::= EWKT_OPEN_BRACKET point geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 140 */
    "geocoll_text ::= EWKT_OPEN_BRACKET linestring geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 141 */
    "geocoll_text ::= EWKT_OPEN_BRACKET polygon geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 142 */
    "geocoll_text ::= EWKT_OPEN_BRACKET multipoint geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 143 */
    "geocoll_text ::= EWKT_OPEN_BRACKET multilinestring geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 144 */
    "geocoll_text ::= EWKT_OPEN_BRACKET multipolygon geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 145 */
    "geocoll_text ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_text geocoll_text2 EWKT_CLOSE_BRACKET",
    /* 146 */ "geocoll_text2 ::=",
    /* 147 */ "geocoll_text2 ::= EWKT_COMMA point geocoll_text2",
    /* 148 */ "geocoll_text2 ::= EWKT_COMMA linestring geocoll_text2",
    /* 149 */ "geocoll_text2 ::= EWKT_COMMA polygon geocoll_text2",
    /* 150 */ "geocoll_text2 ::= EWKT_COMMA multipoint geocoll_text2",
    /* 151 */ "geocoll_text2 ::= EWKT_COMMA multilinestring geocoll_text2",
    /* 152 */ "geocoll_text2 ::= EWKT_COMMA multipolygon geocoll_text2",
    /* 153 */
    "geocoll_text2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_text geocoll_text2",
    /* 154 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET pointm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 155 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET linestringm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 156 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET polygonm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 157 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET multipointm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 158 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET multilinestringm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 159 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET multipolygonm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 160 */
    "geocoll_textm ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 EWKT_CLOSE_BRACKET",
    /* 161 */ "geocoll_textm2 ::=",
    /* 162 */ "geocoll_textm2 ::= EWKT_COMMA pointm geocoll_textm2",
    /* 163 */ "geocoll_textm2 ::= EWKT_COMMA linestringm geocoll_textm2",
    /* 164 */ "geocoll_textm2 ::= EWKT_COMMA polygonm geocoll_textm2",
    /* 165 */ "geocoll_textm2 ::= EWKT_COMMA multipointm geocoll_textm2",
    /* 166 */ "geocoll_textm2 ::= EWKT_COMMA multilinestringm geocoll_textm2",
    /* 167 */ "geocoll_textm2 ::= EWKT_COMMA multipolygonm geocoll_textm2",
    /* 168 */
    "geocoll_textm2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2",
    /* 169 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET pointz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 170 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET linestringz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 171 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET polygonz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 172 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET multipointz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 173 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET multilinestringz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 174 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET multipolygonz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 175 */
    "geocoll_textz ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textz geocoll_textz2 EWKT_CLOSE_BRACKET",
    /* 176 */ "geocoll_textz2 ::=",
    /* 177 */ "geocoll_textz2 ::= EWKT_COMMA pointz geocoll_textz2",
    /* 178 */ "geocoll_textz2 ::= EWKT_COMMA linestringz geocoll_textz2",
    /* 179 */ "geocoll_textz2 ::= EWKT_COMMA polygonz geocoll_textz2",
    /* 180 */ "geocoll_textz2 ::= EWKT_COMMA multipointz geocoll_textz2",
    /* 181 */ "geocoll_textz2 ::= EWKT_COMMA multilinestringz geocoll_textz2",
    /* 182 */ "geocoll_textz2 ::= EWKT_COMMA multipolygonz geocoll_textz2",
    /* 183 */
    "geocoll_textz2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textz geocoll_textz2",
    /* 184 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET pointzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 185 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET linestringzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 186 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET polygonzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 187 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET multipointzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 188 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET multilinestringzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 189 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET multipolygonzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 190 */
    "geocoll_textzm ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textzm geocoll_textzm2 EWKT_CLOSE_BRACKET",
    /* 191 */ "geocoll_textzm2 ::=",
    /* 192 */ "geocoll_textzm2 ::= EWKT_COMMA pointzm geocoll_textzm2",
    /* 193 */ "geocoll_textzm2 ::= EWKT_COMMA linestringzm geocoll_textzm2",
    /* 194 */ "geocoll_textzm2 ::= EWKT_COMMA polygonzm geocoll_textzm2",
    /* 195 */ "geocoll_textzm2 ::= EWKT_COMMA multipointzm geocoll_textzm2",
    /* 196 */
    "geocoll_textzm2 ::= EWKT_COMMA multilinestringzm geocoll_textzm2",
    /* 197 */ "geocoll_textzm2 ::= EWKT_COMMA multipolygonzm geocoll_textzm2",
    /* 198 */
    "geocoll_textzm2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textzm geocoll_textzm2",
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
    21, 1},
    {
    22, 0},
    {
    22, 3},
    {
    23, 1},
    {
    24, 1},
    {
    24, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    25, 1},
    {
    26, 1},
    {
    26, 1},
    {
    26, 1},
    {
    26, 1},
    {
    26, 1},
    {
    26, 1},
    {
    26, 1},
    {
    27, 4},
    {
    28, 4},
    {
    48, 4},
    {
    29, 4},
    {
    59, 4},
    {
    61, 5},
    {
    62, 5},
    {
    63, 6},
    {
    55, 2},
    {
    57, 3},
    {
    56, 3},
    {
    58, 4},
    {
    60, 1},
    {
    64, 0},
    {
    64, 3},
    {
    65, 0},
    {
    65, 3},
    {
    66, 0},
    {
    66, 3},
    {
    67, 0},
    {
    67, 3},
    {
    68, 0},
    {
    68, 3},
    {
    69, 0},
    {
    69, 3},
    {
    70, 0},
    {
    70, 3},
    {
    71, 0},
    {
    71, 3},
    {
    30, 2},
    {
    49, 2},
    {
    31, 2},
    {
    32, 2},
    {
    72, 6},
    {
    73, 6},
    {
    74, 6},
    {
    75, 6},
    {
    33, 2},
    {
    50, 2},
    {
    34, 2},
    {
    35, 2},
    {
    76, 4},
    {
    77, 4},
    {
    78, 4},
    {
    79, 4},
    {
    80, 10},
    {
    81, 0},
    {
    81, 3},
    {
    82, 10},
    {
    83, 0},
    {
    83, 3},
    {
    84, 10},
    {
    85, 0},
    {
    85, 3},
    {
    86, 10},
    {
    87, 0},
    {
    87, 3},
    {
    36, 2},
    {
    51, 2},
    {
    37, 2},
    {
    38, 2},
    {
    88, 4},
    {
    89, 4},
    {
    90, 4},
    {
    91, 4},
    {
    88, 4},
    {
    89, 4},
    {
    90, 4},
    {
    91, 4},
    {
    39, 2},
    {
    52, 2},
    {
    40, 2},
    {
    41, 2},
    {
    92, 4},
    {
    96, 0},
    {
    96, 3},
    {
    93, 4},
    {
    97, 0},
    {
    97, 3},
    {
    94, 4},
    {
    98, 0},
    {
    98, 3},
    {
    95, 4},
    {
    99, 0},
    {
    99, 3},
    {
    42, 2},
    {
    53, 2},
    {
    43, 2},
    {
    44, 2},
    {
    100, 4},
    {
    104, 0},
    {
    104, 3},
    {
    101, 4},
    {
    105, 0},
    {
    105, 3},
    {
    102, 4},
    {
    106, 0},
    {
    106, 3},
    {
    103, 4},
    {
    107, 0},
    {
    107, 3},
    {
    45, 2},
    {
    54, 2},
    {
    46, 2},
    {
    47, 2},
    {
    108, 4},
    {
    108, 4},
    {
    108, 4},
    {
    108, 4},
    {
    108, 4},
    {
    108, 4},
    {
    108, 5},
    {
    112, 0},
    {
    112, 3},
    {
    112, 3},
    {
    112, 3},
    {
    112, 3},
    {
    112, 3},
    {
    112, 3},
    {
    112, 4},
    {
    109, 4},
    {
    109, 4},
    {
    109, 4},
    {
    109, 4},
    {
    109, 4},
    {
    109, 4},
    {
    109, 5},
    {
    113, 0},
    {
    113, 3},
    {
    113, 3},
    {
    113, 3},
    {
    113, 3},
    {
    113, 3},
    {
    113, 3},
    {
    113, 4},
    {
    110, 4},
    {
    110, 4},
    {
    110, 4},
    {
    110, 4},
    {
    110, 4},
    {
    110, 4},
    {
    110, 5},
    {
    114, 0},
    {
    114, 3},
    {
    114, 3},
    {
    114, 3},
    {
    114, 3},
    {
    114, 3},
    {
    114, 3},
    {
    114, 4},
    {
    111, 4},
    {
    111, 4},
    {
    111, 4},
    {
    111, 4},
    {
    111, 4},
    {
    111, 4},
    {
    111, 5},
    {
    115, 0},
    {
    115, 3},
    {
    115, 3},
    {
    115, 3},
    {
    115, 3},
    {
    115, 3},
    {
    115, 3},
    {
115, 4},};

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
      case 6:			/* geo_text ::= point */
      case 7:			/* geo_text ::= pointz */
	  yytestcase (yyruleno == 7);
      case 8:			/* geo_text ::= pointzm */
	  yytestcase (yyruleno == 8);
      case 9:			/* geo_text ::= linestring */
	  yytestcase (yyruleno == 9);
      case 10:			/* geo_text ::= linestringz */
	  yytestcase (yyruleno == 10);
      case 11:			/* geo_text ::= linestringzm */
	  yytestcase (yyruleno == 11);
      case 12:			/* geo_text ::= polygon */
	  yytestcase (yyruleno == 12);
      case 13:			/* geo_text ::= polygonz */
	  yytestcase (yyruleno == 13);
      case 14:			/* geo_text ::= polygonzm */
	  yytestcase (yyruleno == 14);
      case 15:			/* geo_text ::= multipoint */
	  yytestcase (yyruleno == 15);
      case 16:			/* geo_text ::= multipointz */
	  yytestcase (yyruleno == 16);
      case 17:			/* geo_text ::= multipointzm */
	  yytestcase (yyruleno == 17);
      case 18:			/* geo_text ::= multilinestring */
	  yytestcase (yyruleno == 18);
      case 19:			/* geo_text ::= multilinestringz */
	  yytestcase (yyruleno == 19);
      case 20:			/* geo_text ::= multilinestringzm */
	  yytestcase (yyruleno == 20);
      case 21:			/* geo_text ::= multipolygon */
	  yytestcase (yyruleno == 21);
      case 22:			/* geo_text ::= multipolygonz */
	  yytestcase (yyruleno == 22);
      case 23:			/* geo_text ::= multipolygonzm */
	  yytestcase (yyruleno == 23);
      case 24:			/* geo_text ::= geocoll */
	  yytestcase (yyruleno == 24);
      case 25:			/* geo_text ::= geocollz */
	  yytestcase (yyruleno == 25);
      case 26:			/* geo_text ::= geocollzm */
	  yytestcase (yyruleno == 26);
      case 27:			/* geo_textm ::= pointm */
	  yytestcase (yyruleno == 27);
      case 28:			/* geo_textm ::= linestringm */
	  yytestcase (yyruleno == 28);
      case 29:			/* geo_textm ::= polygonm */
	  yytestcase (yyruleno == 29);
      case 30:			/* geo_textm ::= multipointm */
	  yytestcase (yyruleno == 30);
      case 31:			/* geo_textm ::= multilinestringm */
	  yytestcase (yyruleno == 31);
      case 32:			/* geo_textm ::= multipolygonm */
	  yytestcase (yyruleno == 32);
      case 33:			/* geo_textm ::= geocollm */
	  yytestcase (yyruleno == 33);
	  {
	      p_data->result = yymsp[0].minor.yy0;
	  }
	  break;
      case 34:			/* point ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxy EWKT_CLOSE_BRACKET */
      case 35:			/* pointz ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyz EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 35);
      case 37:			/* pointzm ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyzm EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 37);
	  {
	      yygotominor.yy0 =
		  ewkt_buildGeomFromPoint (p_data,
					   (gaiaPointPtr) yymsp[-1].minor.yy0);
	  }
	  break;
      case 36:			/* pointm ::= EWKT_POINT_M EWKT_OPEN_BRACKET point_coordxym EWKT_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  ewkt_buildGeomFromPoint (p_data,
					   (gaiaPointPtr) yymsp[-1].minor.yy0);
	  }
	  break;
      case 38:			/* point_brkt_coordxy ::= EWKT_OPEN_BRACKET coord coord EWKT_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xy (p_data,
					  (double *) yymsp[-2].minor.yy0,
					  (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 39:			/* point_brkt_coordxym ::= EWKT_OPEN_BRACKET coord coord coord EWKT_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xym (p_data,
					   (double *) yymsp[-3].minor.yy0,
					   (double *) yymsp[-2].minor.yy0,
					   (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 40:			/* point_brkt_coordxyz ::= EWKT_OPEN_BRACKET coord coord coord EWKT_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xyz (p_data,
					   (double *) yymsp[-3].minor.yy0,
					   (double *) yymsp[-2].minor.yy0,
					   (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 41:			/* point_brkt_coordxyzm ::= EWKT_OPEN_BRACKET coord coord coord coord EWKT_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xyzm (p_data,
					    (double *) yymsp[-4].minor.yy0,
					    (double *) yymsp[-3].minor.yy0,
					    (double *) yymsp[-2].minor.yy0,
					    (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 42:			/* point_coordxy ::= coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xy (p_data,
					  (double *) yymsp[-1].minor.yy0,
					  (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 43:			/* point_coordxym ::= coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xym (p_data,
					   (double *) yymsp[-2].minor.yy0,
					   (double *) yymsp[-1].minor.yy0,
					   (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 44:			/* point_coordxyz ::= coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xyz (p_data,
					   (double *) yymsp[-2].minor.yy0,
					   (double *) yymsp[-1].minor.yy0,
					   (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 45:			/* point_coordxyzm ::= coord coord coord coord */
	  {
	      yygotominor.yy0 =
		  (void *) ewkt_point_xyzm (p_data,
					    (double *) yymsp[-3].minor.yy0,
					    (double *) yymsp[-2].minor.yy0,
					    (double *) yymsp[-1].minor.yy0,
					    (double *) yymsp[0].minor.yy0);
	  }
	  break;
      case 46:			/* coord ::= EWKT_NUM */
      case 91:			/* multipoint ::= EWKT_MULTIPOINT multipoint_text */
	  yytestcase (yyruleno == 91);
      case 92:			/* multipointm ::= EWKT_MULTIPOINT_M multipoint_textm */
	  yytestcase (yyruleno == 92);
      case 93:			/* multipointz ::= EWKT_MULTIPOINT multipoint_textz */
	  yytestcase (yyruleno == 93);
      case 94:			/* multipointzm ::= EWKT_MULTIPOINT multipoint_textzm */
	  yytestcase (yyruleno == 94);
      case 103:		/* multilinestring ::= EWKT_MULTILINESTRING multilinestring_text */
	  yytestcase (yyruleno == 103);
      case 104:		/* multilinestringm ::= EWKT_MULTILINESTRING_M multilinestring_textm */
	  yytestcase (yyruleno == 104);
      case 105:		/* multilinestringz ::= EWKT_MULTILINESTRING multilinestring_textz */
	  yytestcase (yyruleno == 105);
      case 106:		/* multilinestringzm ::= EWKT_MULTILINESTRING multilinestring_textzm */
	  yytestcase (yyruleno == 106);
      case 119:		/* multipolygon ::= EWKT_MULTIPOLYGON multipolygon_text */
	  yytestcase (yyruleno == 119);
      case 120:		/* multipolygonm ::= EWKT_MULTIPOLYGON_M multipolygon_textm */
	  yytestcase (yyruleno == 120);
      case 121:		/* multipolygonz ::= EWKT_MULTIPOLYGON multipolygon_textz */
	  yytestcase (yyruleno == 121);
      case 122:		/* multipolygonzm ::= EWKT_MULTIPOLYGON multipolygon_textzm */
	  yytestcase (yyruleno == 122);
      case 135:		/* geocoll ::= EWKT_GEOMETRYCOLLECTION geocoll_text */
	  yytestcase (yyruleno == 135);
      case 136:		/* geocollm ::= EWKT_GEOMETRYCOLLECTION_M geocoll_textm */
	  yytestcase (yyruleno == 136);
      case 137:		/* geocollz ::= EWKT_GEOMETRYCOLLECTION geocoll_textz */
	  yytestcase (yyruleno == 137);
      case 138:		/* geocollzm ::= EWKT_GEOMETRYCOLLECTION geocoll_textzm */
	  yytestcase (yyruleno == 138);
	  {
	      yygotominor.yy0 = yymsp[0].minor.yy0;
	  }
	  break;
      case 47:			/* extra_brkt_pointsxy ::= */
      case 49:			/* extra_brkt_pointsxym ::= */
	  yytestcase (yyruleno == 49);
      case 51:			/* extra_brkt_pointsxyz ::= */
	  yytestcase (yyruleno == 51);
      case 53:			/* extra_brkt_pointsxyzm ::= */
	  yytestcase (yyruleno == 53);
      case 55:			/* extra_pointsxy ::= */
	  yytestcase (yyruleno == 55);
      case 57:			/* extra_pointsxym ::= */
	  yytestcase (yyruleno == 57);
      case 59:			/* extra_pointsxyz ::= */
	  yytestcase (yyruleno == 59);
      case 61:			/* extra_pointsxyzm ::= */
	  yytestcase (yyruleno == 61);
      case 80:			/* extra_rings ::= */
	  yytestcase (yyruleno == 80);
      case 83:			/* extra_ringsm ::= */
	  yytestcase (yyruleno == 83);
      case 86:			/* extra_ringsz ::= */
	  yytestcase (yyruleno == 86);
      case 89:			/* extra_ringszm ::= */
	  yytestcase (yyruleno == 89);
      case 108:		/* multilinestring_text2 ::= */
	  yytestcase (yyruleno == 108);
      case 111:		/* multilinestring_textm2 ::= */
	  yytestcase (yyruleno == 111);
      case 114:		/* multilinestring_textz2 ::= */
	  yytestcase (yyruleno == 114);
      case 117:		/* multilinestring_textzm2 ::= */
	  yytestcase (yyruleno == 117);
      case 124:		/* multipolygon_text2 ::= */
	  yytestcase (yyruleno == 124);
      case 127:		/* multipolygon_textm2 ::= */
	  yytestcase (yyruleno == 127);
      case 130:		/* multipolygon_textz2 ::= */
	  yytestcase (yyruleno == 130);
      case 133:		/* multipolygon_textzm2 ::= */
	  yytestcase (yyruleno == 133);
      case 146:		/* geocoll_text2 ::= */
	  yytestcase (yyruleno == 146);
      case 161:		/* geocoll_textm2 ::= */
	  yytestcase (yyruleno == 161);
      case 176:		/* geocoll_textz2 ::= */
	  yytestcase (yyruleno == 176);
      case 191:		/* geocoll_textzm2 ::= */
	  yytestcase (yyruleno == 191);
	  {
	      yygotominor.yy0 = NULL;
	  }
	  break;
      case 48:			/* extra_brkt_pointsxy ::= EWKT_COMMA point_brkt_coordxy extra_brkt_pointsxy */
      case 50:			/* extra_brkt_pointsxym ::= EWKT_COMMA point_brkt_coordxym extra_brkt_pointsxym */
	  yytestcase (yyruleno == 50);
      case 52:			/* extra_brkt_pointsxyz ::= EWKT_COMMA point_brkt_coordxyz extra_brkt_pointsxyz */
	  yytestcase (yyruleno == 52);
      case 54:			/* extra_brkt_pointsxyzm ::= EWKT_COMMA point_brkt_coordxyzm extra_brkt_pointsxyzm */
	  yytestcase (yyruleno == 54);
      case 56:			/* extra_pointsxy ::= EWKT_COMMA point_coordxy extra_pointsxy */
	  yytestcase (yyruleno == 56);
      case 58:			/* extra_pointsxym ::= EWKT_COMMA point_coordxym extra_pointsxym */
	  yytestcase (yyruleno == 58);
      case 60:			/* extra_pointsxyz ::= EWKT_COMMA point_coordxyz extra_pointsxyz */
	  yytestcase (yyruleno == 60);
      case 62:			/* extra_pointsxyzm ::= EWKT_COMMA point_coordxyzm extra_pointsxyzm */
	  yytestcase (yyruleno == 62);
	  {
	      ((gaiaPointPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 63:			/* linestring ::= EWKT_LINESTRING linestring_text */
      case 64:			/* linestringm ::= EWKT_LINESTRING_M linestring_textm */
	  yytestcase (yyruleno == 64);
      case 65:			/* linestringz ::= EWKT_LINESTRING linestring_textz */
	  yytestcase (yyruleno == 65);
      case 66:			/* linestringzm ::= EWKT_LINESTRING linestring_textzm */
	  yytestcase (yyruleno == 66);
	  {
	      yygotominor.yy0 =
		  ewkt_buildGeomFromLinestring (p_data,
						(gaiaLinestringPtr)
						yymsp[0].minor.yy0);
	  }
	  break;
      case 67:			/* linestring_text ::= EWKT_OPEN_BRACKET point_coordxy EWKT_COMMA point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_linestring_xy (p_data,
					       (gaiaPointPtr) yymsp[-4].minor.
					       yy0);
	  }
	  break;
      case 68:			/* linestring_textm ::= EWKT_OPEN_BRACKET point_coordxym EWKT_COMMA point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_linestring_xym (p_data,
						(gaiaPointPtr) yymsp[-4].minor.
						yy0);
	  }
	  break;
      case 69:			/* linestring_textz ::= EWKT_OPEN_BRACKET point_coordxyz EWKT_COMMA point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_linestring_xyz (p_data,
						(gaiaPointPtr) yymsp[-4].minor.
						yy0);
	  }
	  break;
      case 70:			/* linestring_textzm ::= EWKT_OPEN_BRACKET point_coordxyzm EWKT_COMMA point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_linestring_xyzm (p_data,
						 (gaiaPointPtr) yymsp[-4].minor.
						 yy0);
	  }
	  break;
      case 71:			/* polygon ::= EWKT_POLYGON polygon_text */
      case 72:			/* polygonm ::= EWKT_POLYGON_M polygon_textm */
	  yytestcase (yyruleno == 72);
      case 73:			/* polygonz ::= EWKT_POLYGON polygon_textz */
	  yytestcase (yyruleno == 73);
      case 74:			/* polygonzm ::= EWKT_POLYGON polygon_textzm */
	  yytestcase (yyruleno == 74);
	  {
	      yygotominor.yy0 =
		  ewkt_buildGeomFromPolygon (p_data,
					     (gaiaPolygonPtr) yymsp[0].minor.
					     yy0);
	  }
	  break;
      case 75:			/* polygon_text ::= EWKT_OPEN_BRACKET ring extra_rings EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_polygon_xy (p_data,
					    (gaiaRingPtr) yymsp[-2].minor.yy0);
	  }
	  break;
      case 76:			/* polygon_textm ::= EWKT_OPEN_BRACKET ringm extra_ringsm EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_polygon_xym (p_data,
					     (gaiaRingPtr) yymsp[-2].minor.yy0);
	  }
	  break;
      case 77:			/* polygon_textz ::= EWKT_OPEN_BRACKET ringz extra_ringsz EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_polygon_xyz (p_data,
					     (gaiaRingPtr) yymsp[-2].minor.yy0);
	  }
	  break;
      case 78:			/* polygon_textzm ::= EWKT_OPEN_BRACKET ringzm extra_ringszm EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_polygon_xyzm (p_data,
					      (gaiaRingPtr) yymsp[-2].minor.
					      yy0);
	  }
	  break;
      case 79:			/* ring ::= EWKT_OPEN_BRACKET point_coordxy EWKT_COMMA point_coordxy EWKT_COMMA point_coordxy EWKT_COMMA point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET */
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
		  (void *) ewkt_ring_xy (p_data,
					 (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 81:			/* extra_rings ::= EWKT_COMMA ring extra_rings */
      case 84:			/* extra_ringsm ::= EWKT_COMMA ringm extra_ringsm */
	  yytestcase (yyruleno == 84);
      case 87:			/* extra_ringsz ::= EWKT_COMMA ringz extra_ringsz */
	  yytestcase (yyruleno == 87);
      case 90:			/* extra_ringszm ::= EWKT_COMMA ringzm extra_ringszm */
	  yytestcase (yyruleno == 90);
	  {
	      ((gaiaRingPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 82:			/* ringm ::= EWKT_OPEN_BRACKET point_coordxym EWKT_COMMA point_coordxym EWKT_COMMA point_coordxym EWKT_COMMA point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET */
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
		  (void *) ewkt_ring_xym (p_data,
					  (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 85:			/* ringz ::= EWKT_OPEN_BRACKET point_coordxyz EWKT_COMMA point_coordxyz EWKT_COMMA point_coordxyz EWKT_COMMA point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET */
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
		  (void *) ewkt_ring_xyz (p_data,
					  (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 88:			/* ringzm ::= EWKT_OPEN_BRACKET point_coordxyzm EWKT_COMMA point_coordxyzm EWKT_COMMA point_coordxyzm EWKT_COMMA point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET */
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
		  (void *) ewkt_ring_xyzm (p_data,
					   (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 95:			/* multipoint_text ::= EWKT_OPEN_BRACKET point_coordxy extra_pointsxy EWKT_CLOSE_BRACKET */
      case 99:			/* multipoint_text ::= EWKT_OPEN_BRACKET point_brkt_coordxy extra_brkt_pointsxy EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 99);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipoint_xy (p_data,
					       (gaiaPointPtr) yymsp[-2].minor.
					       yy0);
	  }
	  break;
      case 96:			/* multipoint_textm ::= EWKT_OPEN_BRACKET point_coordxym extra_pointsxym EWKT_CLOSE_BRACKET */
      case 100:		/* multipoint_textm ::= EWKT_OPEN_BRACKET point_brkt_coordxym extra_brkt_pointsxym EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 100);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipoint_xym (p_data,
						(gaiaPointPtr) yymsp[-2].minor.
						yy0);
	  }
	  break;
      case 97:			/* multipoint_textz ::= EWKT_OPEN_BRACKET point_coordxyz extra_pointsxyz EWKT_CLOSE_BRACKET */
      case 101:		/* multipoint_textz ::= EWKT_OPEN_BRACKET point_brkt_coordxyz extra_brkt_pointsxyz EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 101);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipoint_xyz (p_data,
						(gaiaPointPtr) yymsp[-2].minor.
						yy0);
	  }
	  break;
      case 98:			/* multipoint_textzm ::= EWKT_OPEN_BRACKET point_coordxyzm extra_pointsxyzm EWKT_CLOSE_BRACKET */
      case 102:		/* multipoint_textzm ::= EWKT_OPEN_BRACKET point_brkt_coordxyzm extra_brkt_pointsxyzm EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 102);
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipoint_xyzm (p_data,
						 (gaiaPointPtr) yymsp[-2].minor.
						 yy0);
	  }
	  break;
      case 107:		/* multilinestring_text ::= EWKT_OPEN_BRACKET linestring_text multilinestring_text2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multilinestring_xy (p_data,
						    (gaiaLinestringPtr)
						    yymsp[-2].minor.yy0);
	  }
	  break;
      case 109:		/* multilinestring_text2 ::= EWKT_COMMA linestring_text multilinestring_text2 */
      case 112:		/* multilinestring_textm2 ::= EWKT_COMMA linestring_textm multilinestring_textm2 */
	  yytestcase (yyruleno == 112);
      case 115:		/* multilinestring_textz2 ::= EWKT_COMMA linestring_textz multilinestring_textz2 */
	  yytestcase (yyruleno == 115);
      case 118:		/* multilinestring_textzm2 ::= EWKT_COMMA linestring_textzm multilinestring_textzm2 */
	  yytestcase (yyruleno == 118);
	  {
	      ((gaiaLinestringPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 110:		/* multilinestring_textm ::= EWKT_OPEN_BRACKET linestring_textm multilinestring_textm2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multilinestring_xym (p_data,
						     (gaiaLinestringPtr)
						     yymsp[-2].minor.yy0);
	  }
	  break;
      case 113:		/* multilinestring_textz ::= EWKT_OPEN_BRACKET linestring_textz multilinestring_textz2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multilinestring_xyz (p_data,
						     (gaiaLinestringPtr)
						     yymsp[-2].minor.yy0);
	  }
	  break;
      case 116:		/* multilinestring_textzm ::= EWKT_OPEN_BRACKET linestring_textzm multilinestring_textzm2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multilinestring_xyzm (p_data,
						      (gaiaLinestringPtr)
						      yymsp[-2].minor.yy0);
	  }
	  break;
      case 123:		/* multipolygon_text ::= EWKT_OPEN_BRACKET polygon_text multipolygon_text2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipolygon_xy (p_data,
						 (gaiaPolygonPtr)
						 yymsp[-2].minor.yy0);
	  }
	  break;
      case 125:		/* multipolygon_text2 ::= EWKT_COMMA polygon_text multipolygon_text2 */
      case 128:		/* multipolygon_textm2 ::= EWKT_COMMA polygon_textm multipolygon_textm2 */
	  yytestcase (yyruleno == 128);
      case 131:		/* multipolygon_textz2 ::= EWKT_COMMA polygon_textz multipolygon_textz2 */
	  yytestcase (yyruleno == 131);
      case 134:		/* multipolygon_textzm2 ::= EWKT_COMMA polygon_textzm multipolygon_textzm2 */
	  yytestcase (yyruleno == 134);
	  {
	      ((gaiaPolygonPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 126:		/* multipolygon_textm ::= EWKT_OPEN_BRACKET polygon_textm multipolygon_textm2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipolygon_xym (p_data,
						  (gaiaPolygonPtr)
						  yymsp[-2].minor.yy0);
	  }
	  break;
      case 129:		/* multipolygon_textz ::= EWKT_OPEN_BRACKET polygon_textz multipolygon_textz2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipolygon_xyz (p_data,
						  (gaiaPolygonPtr)
						  yymsp[-2].minor.yy0);
	  }
	  break;
      case 132:		/* multipolygon_textzm ::= EWKT_OPEN_BRACKET polygon_textzm multipolygon_textzm2 EWKT_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_multipolygon_xyzm (p_data,
						   (gaiaPolygonPtr)
						   yymsp[-2].minor.yy0);
	  }
	  break;
      case 139:		/* geocoll_text ::= EWKT_OPEN_BRACKET point geocoll_text2 EWKT_CLOSE_BRACKET */
      case 140:		/* geocoll_text ::= EWKT_OPEN_BRACKET linestring geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 140);
      case 141:		/* geocoll_text ::= EWKT_OPEN_BRACKET polygon geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 141);
      case 142:		/* geocoll_text ::= EWKT_OPEN_BRACKET multipoint geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 142);
      case 143:		/* geocoll_text ::= EWKT_OPEN_BRACKET multilinestring geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 143);
      case 144:		/* geocoll_text ::= EWKT_OPEN_BRACKET multipolygon geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 144);
      case 145:		/* geocoll_text ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_text geocoll_text2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 145);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_geomColl_xy (p_data,
					     (gaiaGeomCollPtr) yymsp[-2].minor.
					     yy0);
	  }
	  break;
      case 147:		/* geocoll_text2 ::= EWKT_COMMA point geocoll_text2 */
      case 148:		/* geocoll_text2 ::= EWKT_COMMA linestring geocoll_text2 */
	  yytestcase (yyruleno == 148);
      case 149:		/* geocoll_text2 ::= EWKT_COMMA polygon geocoll_text2 */
	  yytestcase (yyruleno == 149);
      case 150:		/* geocoll_text2 ::= EWKT_COMMA multipoint geocoll_text2 */
	  yytestcase (yyruleno == 150);
      case 151:		/* geocoll_text2 ::= EWKT_COMMA multilinestring geocoll_text2 */
	  yytestcase (yyruleno == 151);
      case 152:		/* geocoll_text2 ::= EWKT_COMMA multipolygon geocoll_text2 */
	  yytestcase (yyruleno == 152);
      case 153:		/* geocoll_text2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_text geocoll_text2 */
	  yytestcase (yyruleno == 153);
      case 162:		/* geocoll_textm2 ::= EWKT_COMMA pointm geocoll_textm2 */
	  yytestcase (yyruleno == 162);
      case 163:		/* geocoll_textm2 ::= EWKT_COMMA linestringm geocoll_textm2 */
	  yytestcase (yyruleno == 163);
      case 164:		/* geocoll_textm2 ::= EWKT_COMMA polygonm geocoll_textm2 */
	  yytestcase (yyruleno == 164);
      case 165:		/* geocoll_textm2 ::= EWKT_COMMA multipointm geocoll_textm2 */
	  yytestcase (yyruleno == 165);
      case 166:		/* geocoll_textm2 ::= EWKT_COMMA multilinestringm geocoll_textm2 */
	  yytestcase (yyruleno == 166);
      case 167:		/* geocoll_textm2 ::= EWKT_COMMA multipolygonm geocoll_textm2 */
	  yytestcase (yyruleno == 167);
      case 168:		/* geocoll_textm2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 */
	  yytestcase (yyruleno == 168);
      case 177:		/* geocoll_textz2 ::= EWKT_COMMA pointz geocoll_textz2 */
	  yytestcase (yyruleno == 177);
      case 178:		/* geocoll_textz2 ::= EWKT_COMMA linestringz geocoll_textz2 */
	  yytestcase (yyruleno == 178);
      case 179:		/* geocoll_textz2 ::= EWKT_COMMA polygonz geocoll_textz2 */
	  yytestcase (yyruleno == 179);
      case 180:		/* geocoll_textz2 ::= EWKT_COMMA multipointz geocoll_textz2 */
	  yytestcase (yyruleno == 180);
      case 181:		/* geocoll_textz2 ::= EWKT_COMMA multilinestringz geocoll_textz2 */
	  yytestcase (yyruleno == 181);
      case 182:		/* geocoll_textz2 ::= EWKT_COMMA multipolygonz geocoll_textz2 */
	  yytestcase (yyruleno == 182);
      case 183:		/* geocoll_textz2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textz geocoll_textz2 */
	  yytestcase (yyruleno == 183);
      case 192:		/* geocoll_textzm2 ::= EWKT_COMMA pointzm geocoll_textzm2 */
	  yytestcase (yyruleno == 192);
      case 193:		/* geocoll_textzm2 ::= EWKT_COMMA linestringzm geocoll_textzm2 */
	  yytestcase (yyruleno == 193);
      case 194:		/* geocoll_textzm2 ::= EWKT_COMMA polygonzm geocoll_textzm2 */
	  yytestcase (yyruleno == 194);
      case 195:		/* geocoll_textzm2 ::= EWKT_COMMA multipointzm geocoll_textzm2 */
	  yytestcase (yyruleno == 195);
      case 196:		/* geocoll_textzm2 ::= EWKT_COMMA multilinestringzm geocoll_textzm2 */
	  yytestcase (yyruleno == 196);
      case 197:		/* geocoll_textzm2 ::= EWKT_COMMA multipolygonzm geocoll_textzm2 */
	  yytestcase (yyruleno == 197);
      case 198:		/* geocoll_textzm2 ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textzm geocoll_textzm2 */
	  yytestcase (yyruleno == 198);
	  {
	      ((gaiaGeomCollPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 154:		/* geocoll_textm ::= EWKT_OPEN_BRACKET pointm geocoll_textm2 EWKT_CLOSE_BRACKET */
      case 155:		/* geocoll_textm ::= EWKT_OPEN_BRACKET linestringm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 155);
      case 156:		/* geocoll_textm ::= EWKT_OPEN_BRACKET polygonm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 156);
      case 157:		/* geocoll_textm ::= EWKT_OPEN_BRACKET multipointm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 157);
      case 158:		/* geocoll_textm ::= EWKT_OPEN_BRACKET multilinestringm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 158);
      case 159:		/* geocoll_textm ::= EWKT_OPEN_BRACKET multipolygonm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 159);
      case 160:		/* geocoll_textm ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION_M geocoll_textm geocoll_textm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 160);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_geomColl_xym (p_data,
					      (gaiaGeomCollPtr) yymsp[-2].minor.
					      yy0);
	  }
	  break;
      case 169:		/* geocoll_textz ::= EWKT_OPEN_BRACKET pointz geocoll_textz2 EWKT_CLOSE_BRACKET */
      case 170:		/* geocoll_textz ::= EWKT_OPEN_BRACKET linestringz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 170);
      case 171:		/* geocoll_textz ::= EWKT_OPEN_BRACKET polygonz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 171);
      case 172:		/* geocoll_textz ::= EWKT_OPEN_BRACKET multipointz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 172);
      case 173:		/* geocoll_textz ::= EWKT_OPEN_BRACKET multilinestringz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 173);
      case 174:		/* geocoll_textz ::= EWKT_OPEN_BRACKET multipolygonz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 174);
      case 175:		/* geocoll_textz ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textz geocoll_textz2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 175);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_geomColl_xyz (p_data,
					      (gaiaGeomCollPtr) yymsp[-2].minor.
					      yy0);
	  }
	  break;
      case 184:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET pointzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
      case 185:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET linestringzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 185);
      case 186:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET polygonzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 186);
      case 187:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET multipointzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 187);
      case 188:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET multilinestringzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 188);
      case 189:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET multipolygonzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 189);
      case 190:		/* geocoll_textzm ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textzm geocoll_textzm2 EWKT_CLOSE_BRACKET */
	  yytestcase (yyruleno == 190);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) ewkt_geomColl_xyzm (p_data,
					       (gaiaGeomCollPtr)
					       yymsp[-2].minor.yy0);
	  }
	  break;
      default:
	  /* (0) main ::= in */ yytestcase (yyruleno == 0);
	  /* (1) in ::= */ yytestcase (yyruleno == 1);
	  /* (2) in ::= in state EWKT_NEWLINE */ yytestcase (yyruleno == 2);
	  /* (3) state ::= program */ yytestcase (yyruleno == 3);
	  /* (4) program ::= geo_text */ yytestcase (yyruleno == 4);
	  /* (5) program ::= geo_textm */ yytestcase (yyruleno == 5);
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
** when the LEMON parser encounters an error
** then this global variable is set 
*/
    p_data->ewkt_parse_error = 1;
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
