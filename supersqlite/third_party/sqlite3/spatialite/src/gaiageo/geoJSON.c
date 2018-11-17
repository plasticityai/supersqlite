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
#define YYNOCODE 84
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
#define ParseARG_SDECL  struct geoJson_data *p_data ;
#define ParseARG_PDECL , struct geoJson_data *p_data
#define ParseARG_FETCH  struct geoJson_data *p_data  = yypParser->p_data
#define ParseARG_STORE yypParser->p_data  = p_data
#define YYNSTATE 679
#define YYNRULE 159
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
    /*     0 */ 184, 533, 534, 535, 536, 537, 538, 539, 540, 541,
    /*    10 */ 542, 543, 544, 545, 546, 547, 548, 187, 139, 189,
    /*    20 */ 146, 147, 148, 190, 155, 156, 679, 104, 185, 112,
    /*    30 */ 242, 287, 334, 373, 412, 451, 139, 195, 146, 147,
    /*    40 */ 140, 201, 141, 142, 512, 149, 88, 150, 151, 157,
    /*    50 */ 148, 551, 155, 156, 496, 188, 243, 193, 254, 509,
    /*    60 */ 204, 262, 455, 288, 335, 301, 340, 471, 309, 348,
    /*    70 */ 507, 374, 413, 379, 418, 457, 387, 426, 482, 191,
    /*    80 */ 498, 480, 452, 567, 568, 839, 1, 169, 461, 465,
    /*    90 */ 93, 486, 490, 501, 504, 238, 240, 211, 199, 207,
    /*   100 */ 86, 201, 200, 208, 101, 244, 245, 215, 205, 217,
    /*   110 */ 553, 216, 223, 219, 227, 99, 201, 220, 228, 246,
    /*   120 */ 229, 19, 256, 250, 263, 275, 201, 260, 261, 95,
    /*   130 */ 30, 269, 31, 265, 266, 201, 273, 274, 277, 278,
    /*   140 */ 281, 285, 286, 42, 201, 292, 289, 290, 303, 297,
    /*   150 */ 307, 308, 201, 310, 322, 312, 313, 96, 316, 328,
    /*   160 */ 320, 321, 201, 201, 324, 325, 332, 333, 206, 120,
    /*   170 */ 209, 336, 337, 121, 342, 192, 349, 361, 201, 53,
    /*   180 */ 346, 347, 351, 352, 355, 64, 359, 360, 201, 363,
    /*   190 */ 364, 367, 375, 376, 2, 201, 381, 371, 372, 394,
    /*   200 */ 201, 122, 126, 201, 385, 386, 388, 400, 390, 391,
    /*   210 */ 414, 415, 398, 399, 402, 403, 406, 410, 411, 420,
    /*   220 */ 201, 130, 134, 201, 424, 425, 427, 439, 433, 429,
    /*   230 */ 430, 218, 201, 221, 437, 438, 441, 442, 445, 477,
    /*   240 */ 478, 202, 201, 449, 450, 473, 460, 468, 493, 201,
    /*   250 */ 485, 464, 489, 508, 520, 514, 510, 511, 524, 201,
    /*   260 */ 518, 519, 522, 523, 526, 203, 530, 531, 201, 264,
    /*   270 */ 276, 267, 279, 521, 311, 323, 314, 326, 350, 362,
    /*   280 */ 353, 365, 389, 401, 392, 404, 428, 440, 431, 443,
    /*   290 */ 556, 247, 248, 97, 99, 251, 249, 571, 102, 105,
    /*   300 */ 252, 253, 103, 292, 106, 573, 291, 295, 113, 293,
    /*   310 */ 294, 111, 123, 114, 110, 589, 125, 124, 296, 297,
    /*   320 */ 119, 127, 298, 299, 377, 840, 118, 620, 592, 128,
    /*   330 */ 246, 131, 300, 132, 338, 135, 339, 250, 136, 4,
    /*   340 */ 3, 5, 129, 532, 6, 158, 186, 133, 8, 552,
    /*   350 */ 104, 378, 159, 549, 460, 622, 137, 550, 112, 87,
    /*   360 */ 416, 194, 9, 10, 197, 464, 636, 196, 417, 468,
    /*   370 */ 485, 638, 198, 453, 554, 555, 489, 454, 89, 90,
    /*   380 */ 651, 652, 493, 91, 653, 11, 170, 469, 12, 470,
    /*   390 */ 557, 558, 210, 213, 479, 13, 662, 663, 664, 214,
    /*   400 */ 212, 14, 494, 559, 560, 678, 171, 495, 15, 561,
    /*   410 */ 562, 222, 225, 16, 17, 226, 224, 563, 230, 564,
    /*   420 */ 231, 236, 235, 233, 20, 232, 7, 234, 456, 239,
    /*   430 */ 840, 565, 241, 237, 566, 569, 160, 18, 840, 551,
    /*   440 */ 570, 92, 94, 98, 100, 572, 88, 255, 840, 21,
    /*   450 */ 258, 840, 574, 257, 259, 22, 575, 270, 576, 172,
    /*   460 */ 23, 268, 577, 578, 271, 25, 840, 272, 26, 24,
    /*   470 */ 579, 580, 173, 581, 280, 840, 582, 840, 283, 27,
    /*   480 */ 284, 28, 282, 583, 32, 29, 161, 584, 840, 840,
    /*   490 */ 585, 586, 107, 108, 109, 587, 304, 840, 840, 115,
    /*   500 */ 116, 117, 588, 590, 302, 33, 840, 591, 305, 840,
    /*   510 */ 306, 840, 593, 594, 174, 34, 36, 35, 840, 39,
    /*   520 */ 315, 476, 595, 596, 318, 37, 75, 840, 317, 840,
    /*   530 */ 319, 329, 597, 598, 599, 175, 40, 38, 327, 600,
    /*   540 */ 840, 330, 840, 331, 41, 601, 602, 603, 162, 840,
    /*   550 */ 840, 341, 604, 840, 840, 840, 605, 606, 43, 344,
    /*   560 */ 343, 840, 44, 356, 345, 45, 840, 607, 608, 176,
    /*   570 */ 46, 358, 840, 609, 354, 840, 610, 357, 48, 47,
    /*   580 */ 611, 49, 840, 612, 177, 366, 840, 613, 614, 840,
    /*   590 */ 840, 369, 50, 370, 163, 368, 51, 615, 52, 840,
    /*   600 */ 840, 616, 840, 617, 618, 840, 380, 840, 619, 621,
    /*   610 */ 382, 383, 840, 178, 54, 55, 384, 840, 623, 56,
    /*   620 */ 624, 57, 625, 393, 626, 840, 840, 59, 396, 840,
    /*   630 */ 58, 397, 395, 627, 840, 179, 628, 60, 405, 629,
    /*   640 */ 630, 840, 840, 62, 408, 840, 61, 409, 407, 631,
    /*   650 */ 840, 164, 632, 63, 840, 633, 634, 419, 840, 840,
    /*   660 */ 481, 635, 840, 637, 65, 423, 840, 421, 422, 66,
    /*   670 */ 840, 81, 639, 67, 640, 840, 180, 68, 69, 840,
    /*   680 */ 840, 432, 641, 642, 840, 435, 70, 840, 840, 434,
    /*   690 */ 436, 840, 643, 181, 644, 71, 517, 840, 645, 444,
    /*   700 */ 840, 646, 840, 183, 72, 447, 840, 446, 647, 448,
    /*   710 */ 73, 840, 674, 648, 165, 138, 840, 649, 166, 143,
    /*   720 */ 840, 650, 458, 84, 840, 459, 462, 654, 840, 463,
    /*   730 */ 144, 655, 840, 466, 145, 467, 840, 840, 840, 656,
    /*   740 */ 472, 840, 840, 657, 840, 658, 474, 475, 529, 840,
    /*   750 */ 74, 840, 659, 85, 660, 167, 665, 840, 661, 483,
    /*   760 */ 152, 840, 484, 487, 153, 840, 488, 154, 840, 666,
    /*   770 */ 491, 840, 492, 840, 497, 667, 840, 168, 840, 668,
    /*   780 */ 840, 669, 499, 670, 76, 500, 840, 840, 502, 77,
    /*   790 */ 503, 840, 505, 840, 78, 79, 506, 840, 840, 182,
    /*   800 */ 80, 513, 840, 671, 840, 840, 82, 516, 840, 840,
    /*   810 */ 840, 515, 672, 840, 840, 673, 83, 840, 840, 527,
    /*   820 */ 525, 675, 840, 528, 840, 840, 840, 840, 676, 677,
};

static const YYCODETYPE yy_lookahead[] = {
    /*     0 */ 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    /*    10 */ 38, 39, 40, 41, 42, 43, 44, 5, 75, 45,
    /*    20 */ 77, 78, 79, 49, 81, 82, 0, 59, 2, 61,
    /*    30 */ 18, 19, 20, 21, 22, 23, 75, 46, 77, 78,
    /*    40 */ 75, 50, 77, 78, 9, 79, 6, 81, 82, 23,
    /*    50 */ 79, 11, 81, 82, 2, 7, 7, 9, 9, 24,
    /*    60 */ 12, 12, 2, 7, 7, 9, 9, 9, 12, 12,
    /*    70 */ 12, 7, 7, 9, 9, 5, 12, 12, 5, 50,
    /*    80 */ 5, 2, 24, 16, 17, 26, 27, 50, 18, 19,
    /*    90 */ 10, 18, 19, 18, 19, 51, 52, 46, 45, 45,
    /*   100 */ 10, 50, 49, 49, 49, 55, 56, 45, 47, 48,
    /*   110 */ 15, 49, 46, 45, 45, 10, 50, 49, 49, 45,
    /*   120 */ 2, 10, 46, 49, 47, 48, 50, 55, 56, 45,
    /*   130 */ 10, 46, 10, 55, 56, 50, 55, 56, 55, 56,
    /*   140 */ 46, 55, 56, 10, 50, 45, 57, 58, 46, 49,
    /*   150 */ 57, 58, 50, 47, 48, 57, 58, 6, 46, 46,
    /*   160 */ 57, 58, 50, 50, 57, 58, 57, 58, 7, 45,
    /*   170 */ 9, 63, 64, 49, 46, 50, 47, 48, 50, 10,
    /*   180 */ 63, 64, 63, 64, 46, 10, 63, 64, 50, 63,
    /*   190 */ 64, 46, 65, 66, 10, 50, 46, 63, 64, 46,
    /*   200 */ 50, 55, 56, 50, 65, 66, 47, 48, 65, 66,
    /*   210 */ 69, 70, 65, 66, 65, 66, 46, 65, 66, 46,
    /*   220 */ 50, 57, 58, 50, 69, 70, 47, 48, 46, 69,
    /*   230 */ 70, 7, 50, 9, 69, 70, 69, 70, 46, 73,
    /*   240 */ 74, 50, 50, 69, 70, 46, 45, 57, 58, 50,
    /*   250 */ 49, 55, 56, 47, 48, 46, 73, 74, 9, 50,
    /*   260 */ 73, 74, 73, 74, 46, 50, 73, 74, 50, 7,
    /*   270 */ 7, 9, 9, 24, 7, 7, 9, 9, 7, 7,
    /*   280 */ 9, 9, 7, 7, 9, 9, 7, 7, 9, 9,
    /*   290 */ 50, 50, 50, 45, 10, 50, 53, 53, 6, 6,
    /*   300 */ 50, 54, 49, 45, 10, 54, 60, 53, 6, 45,
    /*   310 */ 45, 59, 6, 10, 45, 60, 55, 10, 62, 49,
    /*   320 */ 61, 6, 49, 49, 67, 83, 49, 67, 62, 10,
    /*   330 */ 45, 6, 54, 10, 53, 6, 54, 49, 10, 6,
    /*   340 */ 10, 6, 56, 1, 10, 4, 3, 57, 4, 11,
    /*   350 */ 59, 68, 6, 8, 45, 68, 58, 8, 61, 6,
    /*   360 */ 71, 4, 10, 4, 6, 55, 71, 11, 72, 57,
    /*   370 */ 49, 72, 7, 73, 8, 8, 56, 76, 6, 6,
    /*   380 */ 76, 76, 58, 6, 76, 4, 6, 76, 4, 76,
    /*   390 */ 8, 8, 4, 6, 80, 10, 80, 80, 80, 7,
    /*   400 */ 11, 4, 80, 8, 8, 74, 6, 80, 4, 8,
    /*   410 */ 8, 4, 6, 10, 4, 7, 11, 8, 3, 8,
    /*   420 */ 4, 2, 4, 6, 10, 13, 4, 14, 3, 8,
    /*   430 */ 83, 8, 8, 13, 8, 8, 6, 4, 83, 11,
    /*   440 */ 8, 6, 6, 6, 6, 11, 6, 4, 83, 4,
    /*   450 */ 6, 83, 11, 11, 7, 4, 8, 11, 8, 6,
    /*   460 */ 4, 4, 8, 8, 6, 4, 83, 7, 4, 10,
    /*   470 */ 8, 8, 6, 8, 4, 83, 8, 83, 6, 10,
    /*   480 */ 7, 4, 11, 8, 10, 4, 6, 8, 83, 83,
    /*   490 */ 8, 8, 6, 6, 6, 11, 11, 83, 83, 6,
    /*   500 */ 6, 6, 11, 11, 4, 4, 83, 11, 6, 83,
    /*   510 */ 7, 83, 8, 8, 6, 4, 10, 4, 83, 10,
    /*   520 */ 4, 24, 8, 8, 6, 4, 4, 83, 11, 83,
    /*   530 */ 7, 11, 8, 8, 8, 6, 4, 4, 4, 8,
    /*   540 */ 83, 6, 83, 7, 4, 8, 8, 8, 6, 83,
    /*   550 */ 83, 4, 8, 83, 83, 83, 11, 11, 10, 6,
    /*   560 */ 11, 83, 4, 11, 7, 4, 83, 8, 8, 6,
    /*   570 */ 4, 7, 83, 8, 4, 83, 8, 6, 4, 10,
    /*   580 */ 8, 4, 83, 8, 6, 4, 83, 8, 8, 83,
    /*   590 */ 83, 6, 10, 7, 6, 11, 4, 8, 4, 83,
    /*   600 */ 83, 8, 83, 8, 8, 83, 4, 83, 11, 11,
    /*   610 */ 11, 6, 83, 6, 10, 4, 7, 83, 8, 4,
    /*   620 */ 8, 4, 8, 4, 8, 83, 83, 4, 6, 83,
    /*   630 */ 10, 7, 11, 8, 83, 6, 8, 4, 4, 8,
    /*   640 */ 8, 83, 83, 4, 6, 83, 10, 7, 11, 8,
    /*   650 */ 83, 6, 8, 4, 83, 8, 8, 4, 83, 83,
    /*   660 */ 3, 11, 83, 11, 10, 7, 83, 11, 6, 4,
    /*   670 */ 83, 10, 8, 4, 8, 83, 6, 4, 10, 83,
    /*   680 */ 83, 4, 8, 8, 83, 6, 4, 83, 83, 11,
    /*   690 */ 7, 83, 8, 6, 8, 4, 24, 83, 8, 4,
    /*   700 */ 83, 8, 83, 6, 10, 6, 83, 11, 8, 7,
    /*   710 */ 4, 83, 8, 8, 6, 4, 83, 8, 4, 4,
    /*   720 */ 83, 11, 6, 10, 83, 7, 6, 8, 83, 7,
    /*   730 */ 4, 8, 83, 6, 4, 7, 83, 83, 83, 8,
    /*   740 */ 4, 83, 83, 11, 83, 11, 11, 6, 24, 83,
    /*   750 */ 10, 83, 8, 4, 8, 4, 8, 83, 11, 6,
    /*   760 */ 4, 83, 7, 6, 4, 83, 7, 4, 83, 8,
    /*   770 */ 6, 83, 7, 83, 3, 8, 83, 4, 83, 11,
    /*   780 */ 83, 11, 6, 8, 4, 7, 83, 83, 6, 4,
    /*   790 */ 7, 83, 6, 83, 4, 4, 7, 83, 83, 6,
    /*   800 */ 4, 4, 83, 8, 83, 83, 4, 6, 83, 83,
    /*   810 */ 83, 11, 8, 83, 83, 8, 4, 83, 83, 11,
    /*   820 */ 4, 8, 83, 6, 83, 83, 83, 83, 8, 8,
};

#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_MAX 531
static const short yy_shift_ofst[] = {
    /*     0 */ -1, 26, 52, 60, 60, 79, 79, 67, 90, 95,
    /*    10 */ 90, 118, 90, 95, 90, 90, 95, 90, 111, 90,
    /*    20 */ 95, 111, 118, 111, 95, 111, 111, 95, 111, 120,
    /*    30 */ 122, 90, 95, 120, 118, 120, 95, 120, 120, 95,
    /*    40 */ 120, 133, 90, 95, 133, 118, 133, 95, 133, 133,
    /*    50 */ 95, 133, 169, 111, 95, 169, 118, 169, 95, 169,
    /*    60 */ 169, 95, 169, 175, 120, 95, 175, 118, 175, 95,
    /*    70 */ 175, 175, 95, 175, 95, 184, 90, 111, 120, 118,
    /*    80 */ 184, 95, 184, 184, 95, 184, 95, 95, 95, 95,
    /*    90 */ 95, 95, 80, 95, 95, 151, 80, 151, 284, 95,
    /*   100 */ 95, 292, 105, 292, 293, 294, 80, 80, 80, 80,
    /*   110 */ 151, 293, 302, 303, 105, 105, 105, 105, 292, 302,
    /*   120 */ 151, 292, 306, 307, 80, 306, 315, 319, 105, 315,
    /*   130 */ 325, 323, 294, 325, 329, 328, 303, 329, 330, 333,
    /*   140 */ 333, 333, 333, 80, 307, 323, 333, 333, 335, 335,
    /*   150 */ 335, 335, 105, 319, 328, 335, 335, 334, 12, 48,
    /*   160 */ 49, 56, 57, 64, 65, 58, 70, 73, 75, 40,
    /*   170 */ 161, 224, 262, 263, 267, 268, 271, 272, 275, 276,
    /*   180 */ 279, 280, 35, 249, 342, 343, 341, 346, 344, 345,
    /*   190 */ 349, 353, 338, 357, 352, 356, 358, 365, 359, 366,
    /*   200 */ 367, 372, 373, 377, 381, 380, 384, 382, 383, 388,
    /*   210 */ 385, 389, 387, 392, 397, 395, 396, 400, 404, 401,
    /*   220 */ 402, 407, 403, 405, 406, 408, 410, 409, 411, 415,
    /*   230 */ 416, 412, 417, 413, 418, 419, 420, 422, 421, 423,
    /*   240 */ 424, 426, 430, 433, 427, 432, 435, 436, 428, 434,
    /*   250 */ 437, 438, 440, 441, 443, 414, 442, 444, 447, 445,
    /*   260 */ 448, 450, 451, 453, 456, 454, 455, 457, 459, 446,
    /*   270 */ 458, 460, 461, 462, 463, 466, 464, 465, 468, 470,
    /*   280 */ 469, 471, 472, 473, 477, 475, 479, 480, 481, 482,
    /*   290 */ 483, 484, 486, 487, 488, 491, 492, 493, 494, 495,
    /*   300 */ 496, 500, 474, 485, 502, 503, 501, 504, 505, 511,
    /*   310 */ 508, 513, 514, 515, 516, 506, 517, 518, 523, 521,
    /*   320 */ 524, 525, 529, 533, 526, 531, 534, 509, 520, 535,
    /*   330 */ 536, 532, 537, 538, 542, 540, 539, 544, 545, 546,
    /*   340 */ 547, 548, 549, 553, 557, 558, 559, 560, 561, 563,
    /*   350 */ 566, 565, 568, 570, 569, 552, 571, 564, 574, 572,
    /*   360 */ 575, 578, 577, 579, 580, 581, 582, 584, 585, 586,
    /*   370 */ 592, 589, 593, 588, 594, 595, 596, 597, 598, 602,
    /*   380 */ 604, 599, 605, 609, 611, 610, 612, 615, 607, 617,
    /*   390 */ 614, 616, 619, 620, 621, 622, 624, 623, 625, 628,
    /*   400 */ 629, 633, 631, 632, 634, 636, 637, 638, 640, 639,
    /*   410 */ 641, 644, 645, 649, 647, 648, 650, 652, 653, 654,
    /*   420 */ 656, 662, 658, 665, 664, 666, 669, 670, 673, 674,
    /*   430 */ 675, 677, 668, 678, 679, 683, 682, 684, 686, 687,
    /*   440 */ 691, 690, 693, 695, 694, 696, 699, 702, 706, 700,
    /*   450 */ 705, 708, 711, 709, 710, 425, 714, 716, 718, 715,
    /*   460 */ 719, 720, 722, 726, 723, 727, 728, 730, 731, 732,
    /*   470 */ 734, 736, 740, 735, 741, 497, 522, 744, 746, 747,
    /*   480 */ 657, 751, 753, 755, 756, 748, 757, 759, 760, 761,
    /*   490 */ 764, 765, 763, 767, 768, 770, 771, 773, 776, 778,
    /*   500 */ 780, 782, 783, 785, 786, 789, 790, 791, 793, 796,
    /*   510 */ 775, 795, 797, 661, 800, 801, 672, 802, 804, 807,
    /*   520 */ 697, 812, 704, 813, 816, 713, 808, 817, 724, 749,
    /*   530 */ 820, 821,
};

#define YY_REDUCE_USE_DFLT (-58)
#define YY_REDUCE_MAX 157
static const short yy_reduce_ofst[] = {
    /*     0 */ 59, -28, -57, -39, -35, -34, -29, 44, -26, -9,
    /*    10 */ 53, 61, 54, 51, 62, 68, 66, 69, 50, 74,
    /*    20 */ 76, 72, 77, 78, 85, 81, 83, 94, 86, 89,
    /*    30 */ -32, 100, 102, 93, 106, 98, 112, 103, 107, 113,
    /*    40 */ 109, 108, 124, 128, 117, 129, 119, 138, 123, 126,
    /*    50 */ 145, 134, 127, 146, 150, 139, 159, 143, 153, 147,
    /*    60 */ 149, 170, 152, 141, 164, 173, 155, 179, 160, 182,
    /*    70 */ 165, 167, 192, 174, 199, 166, 201, 196, 190, 206,
    /*    80 */ 183, 209, 187, 189, 218, 193, 29, 37, 125, 191,
    /*    90 */ 215, 240, 84, 241, 242, 243, 248, 244, 55, 245,
    /*   100 */ 250, 247, 253, 251, 246, 252, 258, 264, 265, 269,
    /*   110 */ 254, 255, 256, 259, 270, 273, 274, 277, 278, 266,
    /*   120 */ 281, 282, 257, 261, 285, 260, 283, 286, 288, 287,
    /*   130 */ 289, 290, 291, 295, 296, 298, 297, 299, 300, 301,
    /*   140 */ 304, 305, 308, 309, 310, 312, 311, 313, 314, 316,
    /*   150 */ 317, 318, 321, 320, 324, 322, 327, 331,
};

static const YYACTIONTYPE yy_default[] = {
    /*     0 */ 680, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    10 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    20 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    30 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    40 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    50 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    60 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    70 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    80 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*    90 */ 838, 838, 838, 838, 838, 718, 838, 718, 838, 838,
    /*   100 */ 838, 720, 838, 720, 751, 838, 838, 838, 838, 838,
    /*   110 */ 718, 751, 754, 838, 838, 838, 838, 838, 720, 754,
    /*   120 */ 718, 720, 783, 838, 838, 783, 786, 838, 838, 786,
    /*   130 */ 801, 838, 838, 801, 804, 838, 838, 804, 838, 821,
    /*   140 */ 821, 821, 821, 838, 838, 838, 821, 821, 828, 828,
    /*   150 */ 828, 828, 838, 838, 838, 828, 828, 838, 838, 838,
    /*   160 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   170 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   180 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   190 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   200 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   210 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   220 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   230 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   240 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   250 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   260 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   270 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   280 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   290 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   300 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   310 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   320 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   330 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   340 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   350 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   360 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   370 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   380 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   390 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   400 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   410 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   420 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   430 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   440 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   450 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   460 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   470 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   480 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   490 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   500 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   510 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   520 */ 838, 838, 838, 838, 838, 838, 838, 838, 838, 838,
    /*   530 */ 838, 838, 681, 682, 683, 684, 685, 686, 687, 688,
    /*   540 */ 689, 690, 691, 692, 693, 694, 695, 696, 697, 698,
    /*   550 */ 704, 713, 714, 715, 699, 705, 710, 700, 706, 702,
    /*   560 */ 708, 701, 707, 703, 709, 711, 712, 716, 717, 722,
    /*   570 */ 728, 719, 734, 721, 735, 723, 729, 724, 730, 726,
    /*   580 */ 732, 725, 731, 727, 733, 736, 742, 748, 750, 752,
    /*   590 */ 749, 753, 755, 737, 743, 738, 744, 740, 746, 739,
    /*   600 */ 745, 741, 747, 756, 762, 768, 769, 757, 763, 758,
    /*   610 */ 764, 760, 766, 759, 765, 761, 767, 770, 776, 782,
    /*   620 */ 784, 785, 787, 771, 777, 772, 778, 774, 780, 773,
    /*   630 */ 779, 775, 781, 788, 794, 800, 802, 803, 805, 789,
    /*   640 */ 795, 790, 796, 792, 798, 791, 797, 793, 799, 806,
    /*   650 */ 818, 822, 823, 824, 832, 834, 836, 819, 820, 807,
    /*   660 */ 813, 825, 829, 830, 831, 833, 835, 837, 826, 827,
    /*   670 */ 808, 814, 810, 816, 809, 815, 811, 817, 812,
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
    "$", "GEOJSON_NEWLINE", "GEOJSON_OPEN_BRACE", "GEOJSON_TYPE",
    "GEOJSON_COLON", "GEOJSON_POINT", "GEOJSON_COMMA", "GEOJSON_COORDS",
    "GEOJSON_CLOSE_BRACE", "GEOJSON_BBOX", "GEOJSON_OPEN_BRACKET",
    "GEOJSON_CLOSE_BRACKET",
    "GEOJSON_CRS", "GEOJSON_NAME", "GEOJSON_PROPS", "GEOJSON_NUM",
    "GEOJSON_SHORT_SRID", "GEOJSON_LONG_SRID", "GEOJSON_LINESTRING",
    "GEOJSON_POLYGON",
    "GEOJSON_MULTIPOINT", "GEOJSON_MULTILINESTRING", "GEOJSON_MULTIPOLYGON",
    "GEOJSON_GEOMETRYCOLLECTION",
    "GEOJSON_GEOMS", "error", "main", "in",
    "state", "program", "geo_text", "point",
    "pointz", "linestring", "linestringz", "polygon",
    "polygonz", "multipoint", "multipointz", "multilinestring",
    "multilinestringz", "multipolygon", "multipolygonz", "geocoll",
    "geocollz", "point_coordxy", "bbox", "short_crs",
    "long_crs", "point_coordxyz", "coord", "short_srid",
    "long_srid", "extra_pointsxy", "extra_pointsxyz", "linestring_text",
    "linestring_textz", "polygon_text", "polygon_textz", "ring",
    "extra_rings", "ringz", "extra_ringsz", "multipoint_text",
    "multipoint_textz", "multilinestring_text", "multilinestring_textz",
    "multilinestring_text2",
    "multilinestring_textz2", "multipolygon_text", "multipolygon_textz",
    "multipolygon_text2",
    "multipolygon_textz2", "geocoll_text", "geocoll_textz", "coll_point",
    "geocoll_text2", "coll_linestring", "coll_polygon", "coll_pointz",
    "geocoll_textz2", "coll_linestringz", "coll_polygonz",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
    /*   0 */ "main ::= in",
    /*   1 */ "in ::=",
    /*   2 */ "in ::= in state GEOJSON_NEWLINE",
    /*   3 */ "state ::= program",
    /*   4 */ "program ::= geo_text",
    /*   5 */ "geo_text ::= point",
    /*   6 */ "geo_text ::= pointz",
    /*   7 */ "geo_text ::= linestring",
    /*   8 */ "geo_text ::= linestringz",
    /*   9 */ "geo_text ::= polygon",
    /*  10 */ "geo_text ::= polygonz",
    /*  11 */ "geo_text ::= multipoint",
    /*  12 */ "geo_text ::= multipointz",
    /*  13 */ "geo_text ::= multilinestring",
    /*  14 */ "geo_text ::= multilinestringz",
    /*  15 */ "geo_text ::= multipolygon",
    /*  16 */ "geo_text ::= multipolygonz",
    /*  17 */ "geo_text ::= geocoll",
    /*  18 */ "geo_text ::= geocollz",
    /*  19 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  20 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  21 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  22 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  23 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  24 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /*  25 */
    "pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  26 */
    "pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  27 */
    "pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  28 */
    "pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  29 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  30 */
    "point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /*  31 */
    "bbox ::= coord GEOJSON_COMMA coord GEOJSON_COMMA coord GEOJSON_COMMA coord",
    /*  32 */
    "short_crs ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON short_srid GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE",
    /*  33 */
    "long_crs ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON long_srid GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE",
    /*  34 */
    "point_coordxy ::= GEOJSON_OPEN_BRACKET coord GEOJSON_COMMA coord GEOJSON_CLOSE_BRACKET",
    /*  35 */
    "point_coordxyz ::= GEOJSON_OPEN_BRACKET coord GEOJSON_COMMA coord GEOJSON_COMMA coord GEOJSON_CLOSE_BRACKET",
    /*  36 */ "coord ::= GEOJSON_NUM",
    /*  37 */ "short_srid ::= GEOJSON_SHORT_SRID",
    /*  38 */ "long_srid ::= GEOJSON_LONG_SRID",
    /*  39 */ "extra_pointsxy ::=",
    /*  40 */ "extra_pointsxy ::= GEOJSON_COMMA point_coordxy extra_pointsxy",
    /*  41 */ "extra_pointsxyz ::=",
    /*  42 */
    "extra_pointsxyz ::= GEOJSON_COMMA point_coordxyz extra_pointsxyz",
    /*  43 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  44 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  45 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  46 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  47 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  48 */
    "linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /*  49 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  50 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  51 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  52 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  53 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  54 */
    "linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /*  55 */
    "linestring_text ::= GEOJSON_OPEN_BRACKET point_coordxy GEOJSON_COMMA point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET",
    /*  56 */
    "linestring_textz ::= GEOJSON_OPEN_BRACKET point_coordxyz GEOJSON_COMMA point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET",
    /*  57 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  58 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  59 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  60 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  61 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  62 */
    "polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /*  63 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  64 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  65 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  66 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  67 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  68 */
    "polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
    /*  69 */
    "polygon_text ::= GEOJSON_OPEN_BRACKET ring extra_rings GEOJSON_CLOSE_BRACKET",
    /*  70 */
    "polygon_textz ::= GEOJSON_OPEN_BRACKET ringz extra_ringsz GEOJSON_CLOSE_BRACKET",
    /*  71 */
    "ring ::= GEOJSON_OPEN_BRACKET point_coordxy GEOJSON_COMMA point_coordxy GEOJSON_COMMA point_coordxy GEOJSON_COMMA point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET",
    /*  72 */ "extra_rings ::=",
    /*  73 */ "extra_rings ::= GEOJSON_COMMA ring extra_rings",
    /*  74 */
    "ringz ::= GEOJSON_OPEN_BRACKET point_coordxyz GEOJSON_COMMA point_coordxyz GEOJSON_COMMA point_coordxyz GEOJSON_COMMA point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET",
    /*  75 */ "extra_ringsz ::=",
    /*  76 */ "extra_ringsz ::= GEOJSON_COMMA ringz extra_ringsz",
    /*  77 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  78 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  79 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  80 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  81 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  82 */
    "multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE",
    /*  83 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  84 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  85 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  86 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  87 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  88 */
    "multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE",
    /*  89 */
    "multipoint_text ::= GEOJSON_OPEN_BRACKET point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET",
    /*  90 */
    "multipoint_textz ::= GEOJSON_OPEN_BRACKET point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET",
    /*  91 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  92 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  93 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  94 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  95 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  96 */
    "multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE",
    /*  97 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /*  98 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /*  99 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /* 100 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /* 101 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /* 102 */
    "multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE",
    /* 103 */
    "multilinestring_text ::= GEOJSON_OPEN_BRACKET linestring_text multilinestring_text2 GEOJSON_CLOSE_BRACKET",
    /* 104 */ "multilinestring_text2 ::=",
    /* 105 */
    "multilinestring_text2 ::= GEOJSON_COMMA linestring_text multilinestring_text2",
    /* 106 */
    "multilinestring_textz ::= GEOJSON_OPEN_BRACKET linestring_textz multilinestring_textz2 GEOJSON_CLOSE_BRACKET",
    /* 107 */ "multilinestring_textz2 ::=",
    /* 108 */
    "multilinestring_textz2 ::= GEOJSON_COMMA linestring_textz multilinestring_textz2",
    /* 109 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 110 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 111 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 112 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 113 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 114 */
    "multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE",
    /* 115 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 116 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 117 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 118 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 119 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 120 */
    "multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE",
    /* 121 */
    "multipolygon_text ::= GEOJSON_OPEN_BRACKET polygon_text multipolygon_text2 GEOJSON_CLOSE_BRACKET",
    /* 122 */ "multipolygon_text2 ::=",
    /* 123 */
    "multipolygon_text2 ::= GEOJSON_COMMA polygon_text multipolygon_text2",
    /* 124 */
    "multipolygon_textz ::= GEOJSON_OPEN_BRACKET polygon_textz multipolygon_textz2 GEOJSON_CLOSE_BRACKET",
    /* 125 */ "multipolygon_textz2 ::=",
    /* 126 */
    "multipolygon_textz2 ::= GEOJSON_COMMA polygon_textz multipolygon_textz2",
    /* 127 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 128 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 129 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 130 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 131 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 132 */
    "geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE",
    /* 133 */ "geocollz ::= GEOJSON_GEOMETRYCOLLECTION geocoll_textz",
    /* 134 */
    "geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE",
    /* 135 */
    "geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE",
    /* 136 */
    "geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE",
    /* 137 */
    "geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE",
    /* 138 */
    "geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE",
    /* 139 */
    "geocoll_text ::= GEOJSON_OPEN_BRACKET coll_point geocoll_text2 GEOJSON_CLOSE_BRACKET",
    /* 140 */
    "geocoll_text ::= GEOJSON_OPEN_BRACKET coll_linestring geocoll_text2 GEOJSON_CLOSE_BRACKET",
    /* 141 */
    "geocoll_text ::= GEOJSON_OPEN_BRACKET coll_polygon geocoll_text2 GEOJSON_CLOSE_BRACKET",
    /* 142 */ "geocoll_text2 ::=",
    /* 143 */ "geocoll_text2 ::= GEOJSON_COMMA coll_point geocoll_text2",
    /* 144 */ "geocoll_text2 ::= GEOJSON_COMMA coll_linestring geocoll_text2",
    /* 145 */ "geocoll_text2 ::= GEOJSON_COMMA coll_polygon geocoll_text2",
    /* 146 */
    "geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_pointz geocoll_textz2 GEOJSON_CLOSE_BRACKET",
    /* 147 */
    "geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_linestringz geocoll_textz2 GEOJSON_CLOSE_BRACKET",
    /* 148 */
    "geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_polygonz geocoll_textz2 GEOJSON_CLOSE_BRACKET",
    /* 149 */ "geocoll_textz2 ::=",
    /* 150 */ "geocoll_textz2 ::= GEOJSON_COMMA coll_pointz geocoll_textz2",
    /* 151 */
    "geocoll_textz2 ::= GEOJSON_COMMA coll_linestringz geocoll_textz2",
    /* 152 */ "geocoll_textz2 ::= GEOJSON_COMMA coll_polygonz geocoll_textz2",
    /* 153 */
    "coll_point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE",
    /* 154 */
    "coll_pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE",
    /* 155 */
    "coll_linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE",
    /* 156 */
    "coll_linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE",
    /* 157 */
    "coll_polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE",
    /* 158 */
    "coll_polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE",
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
    26, 1},
    {
    27, 0},
    {
    27, 3},
    {
    28, 1},
    {
    29, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    30, 1},
    {
    31, 9},
    {
    31, 15},
    {
    31, 13},
    {
    31, 13},
    {
    31, 19},
    {
    31, 19},
    {
    32, 9},
    {
    32, 15},
    {
    32, 13},
    {
    32, 13},
    {
    31, 19},
    {
    31, 19},
    {
    46, 7},
    {
    47, 13},
    {
    48, 13},
    {
    45, 5},
    {
    49, 7},
    {
    50, 1},
    {
    51, 1},
    {
    52, 1},
    {
    53, 0},
    {
    53, 3},
    {
    54, 0},
    {
    54, 3},
    {
    33, 9},
    {
    33, 15},
    {
    33, 13},
    {
    33, 13},
    {
    33, 19},
    {
    33, 19},
    {
    34, 9},
    {
    34, 15},
    {
    34, 13},
    {
    34, 13},
    {
    34, 19},
    {
    34, 19},
    {
    55, 6},
    {
    56, 6},
    {
    35, 9},
    {
    35, 15},
    {
    35, 13},
    {
    35, 13},
    {
    35, 19},
    {
    35, 19},
    {
    36, 9},
    {
    36, 15},
    {
    36, 13},
    {
    36, 13},
    {
    36, 19},
    {
    36, 19},
    {
    57, 4},
    {
    58, 4},
    {
    59, 10},
    {
    60, 0},
    {
    60, 3},
    {
    61, 10},
    {
    62, 0},
    {
    62, 3},
    {
    37, 9},
    {
    37, 15},
    {
    37, 13},
    {
    37, 13},
    {
    37, 19},
    {
    37, 19},
    {
    38, 9},
    {
    38, 15},
    {
    38, 13},
    {
    38, 13},
    {
    38, 19},
    {
    38, 19},
    {
    63, 4},
    {
    64, 4},
    {
    39, 9},
    {
    39, 15},
    {
    39, 13},
    {
    39, 13},
    {
    39, 19},
    {
    39, 19},
    {
    40, 9},
    {
    40, 15},
    {
    40, 13},
    {
    40, 13},
    {
    40, 19},
    {
    40, 19},
    {
    65, 4},
    {
    67, 0},
    {
    67, 3},
    {
    66, 4},
    {
    68, 0},
    {
    68, 3},
    {
    41, 9},
    {
    41, 15},
    {
    41, 13},
    {
    41, 13},
    {
    41, 19},
    {
    41, 19},
    {
    42, 9},
    {
    42, 15},
    {
    42, 13},
    {
    42, 13},
    {
    42, 19},
    {
    42, 19},
    {
    69, 4},
    {
    71, 0},
    {
    71, 3},
    {
    70, 4},
    {
    72, 0},
    {
    72, 3},
    {
    43, 9},
    {
    43, 15},
    {
    43, 13},
    {
    43, 13},
    {
    43, 19},
    {
    43, 19},
    {
    44, 2},
    {
    44, 15},
    {
    44, 13},
    {
    44, 13},
    {
    44, 19},
    {
    44, 19},
    {
    73, 4},
    {
    73, 4},
    {
    73, 4},
    {
    76, 0},
    {
    76, 3},
    {
    76, 3},
    {
    76, 3},
    {
    74, 4},
    {
    74, 4},
    {
    74, 4},
    {
    80, 0},
    {
    80, 3},
    {
    80, 3},
    {
    80, 3},
    {
    75, 9},
    {
    79, 9},
    {
    77, 9},
    {
    81, 9},
    {
    78, 9},
    {
82, 9},};

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
      case 5:			/* geo_text ::= point */
      case 6:			/* geo_text ::= pointz */
	  yytestcase (yyruleno == 6);
      case 7:			/* geo_text ::= linestring */
	  yytestcase (yyruleno == 7);
      case 8:			/* geo_text ::= linestringz */
	  yytestcase (yyruleno == 8);
      case 9:			/* geo_text ::= polygon */
	  yytestcase (yyruleno == 9);
      case 10:			/* geo_text ::= polygonz */
	  yytestcase (yyruleno == 10);
      case 11:			/* geo_text ::= multipoint */
	  yytestcase (yyruleno == 11);
      case 12:			/* geo_text ::= multipointz */
	  yytestcase (yyruleno == 12);
      case 13:			/* geo_text ::= multilinestring */
	  yytestcase (yyruleno == 13);
      case 14:			/* geo_text ::= multilinestringz */
	  yytestcase (yyruleno == 14);
      case 15:			/* geo_text ::= multipolygon */
	  yytestcase (yyruleno == 15);
      case 16:			/* geo_text ::= multipolygonz */
	  yytestcase (yyruleno == 16);
      case 17:			/* geo_text ::= geocoll */
	  yytestcase (yyruleno == 17);
      case 18:			/* geo_text ::= geocollz */
	  yytestcase (yyruleno == 18);
	  {
	      p_data->result = yymsp[0].minor.yy0;
	  }
	  break;
      case 19:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
      case 20:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 20);
      case 25:			/* pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 25);
      case 26:			/* pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 26);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPoint (p_data,
					      (gaiaPointPtr) yymsp[-1].minor.
					      yy0);
	  }
	  break;
      case 21:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
      case 22:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 22);
      case 27:			/* pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 27);
      case 28:			/* pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 28);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPointSrid (p_data,
						  (gaiaPointPtr)
						  yymsp[-1].minor.yy0,
						  (int *) yymsp[-5].minor.yy0);
	  }
	  break;
      case 23:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
      case 24:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 24);
      case 29:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 29);
      case 30:			/* point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 30);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPointSrid (p_data,
						  (gaiaPointPtr)
						  yymsp[-1].minor.yy0,
						  (int *) yymsp[-11].minor.yy0);
	  }
	  break;
      case 32:			/* short_crs ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON short_srid GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE */
      case 33:			/* long_crs ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON long_srid GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 33);
	  {
	      yygotominor.yy0 = yymsp[-2].minor.yy0;
	  }
	  break;
      case 34:			/* point_coordxy ::= GEOJSON_OPEN_BRACKET coord GEOJSON_COMMA coord GEOJSON_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) geoJSON_point_xy (p_data,
					     (double *) yymsp[-3].minor.yy0,
					     (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 35:			/* point_coordxyz ::= GEOJSON_OPEN_BRACKET coord GEOJSON_COMMA coord GEOJSON_COMMA coord GEOJSON_CLOSE_BRACKET */
	  {
	      yygotominor.yy0 =
		  (void *) geoJSON_point_xyz (p_data,
					      (double *) yymsp[-5].minor.yy0,
					      (double *) yymsp[-3].minor.yy0,
					      (double *) yymsp[-1].minor.yy0);
	  }
	  break;
      case 36:			/* coord ::= GEOJSON_NUM */
      case 37:			/* short_srid ::= GEOJSON_SHORT_SRID */
	  yytestcase (yyruleno == 37);
      case 38:			/* long_srid ::= GEOJSON_LONG_SRID */
	  yytestcase (yyruleno == 38);
      case 133:		/* geocollz ::= GEOJSON_GEOMETRYCOLLECTION geocoll_textz */
	  yytestcase (yyruleno == 133);
	  {
	      yygotominor.yy0 = yymsp[0].minor.yy0;
	  }
	  break;
      case 39:			/* extra_pointsxy ::= */
      case 41:			/* extra_pointsxyz ::= */
	  yytestcase (yyruleno == 41);
      case 72:			/* extra_rings ::= */
	  yytestcase (yyruleno == 72);
      case 75:			/* extra_ringsz ::= */
	  yytestcase (yyruleno == 75);
      case 104:		/* multilinestring_text2 ::= */
	  yytestcase (yyruleno == 104);
      case 107:		/* multilinestring_textz2 ::= */
	  yytestcase (yyruleno == 107);
      case 122:		/* multipolygon_text2 ::= */
	  yytestcase (yyruleno == 122);
      case 125:		/* multipolygon_textz2 ::= */
	  yytestcase (yyruleno == 125);
      case 142:		/* geocoll_text2 ::= */
	  yytestcase (yyruleno == 142);
      case 149:		/* geocoll_textz2 ::= */
	  yytestcase (yyruleno == 149);
	  {
	      yygotominor.yy0 = NULL;
	  }
	  break;
      case 40:			/* extra_pointsxy ::= GEOJSON_COMMA point_coordxy extra_pointsxy */
      case 42:			/* extra_pointsxyz ::= GEOJSON_COMMA point_coordxyz extra_pointsxyz */
	  yytestcase (yyruleno == 42);
	  {
	      ((gaiaPointPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 43:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
      case 44:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 44);
      case 49:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 49);
      case 50:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 50);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromLinestring (p_data,
						   (gaiaLinestringPtr)
						   yymsp[-1].minor.yy0);
	  }
	  break;
      case 45:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
      case 46:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 46);
      case 51:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 51);
      case 52:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 52);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromLinestringSrid (p_data,
						       (gaiaLinestringPtr)
						       yymsp[-1].minor.yy0,
						       (int *) yymsp[-5].minor.
						       yy0);
	  }
	  break;
      case 47:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
      case 48:			/* linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 48);
      case 53:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 53);
      case 54:			/* linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 54);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromLinestringSrid (p_data,
						       (gaiaLinestringPtr)
						       yymsp[-1].minor.yy0,
						       (int *) yymsp[-11].minor.
						       yy0);
	  }
	  break;
      case 55:			/* linestring_text ::= GEOJSON_OPEN_BRACKET point_coordxy GEOJSON_COMMA point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_linestring_xy (p_data,
						  (gaiaPointPtr)
						  yymsp[-4].minor.yy0);
	  }
	  break;
      case 56:			/* linestring_textz ::= GEOJSON_OPEN_BRACKET point_coordxyz GEOJSON_COMMA point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      ((gaiaPointPtr) yymsp[-4].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-2].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_linestring_xyz (p_data,
						   (gaiaPointPtr)
						   yymsp[-4].minor.yy0);
	  }
	  break;
      case 57:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
      case 58:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 58);
      case 63:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 63);
      case 64:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 64);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPolygon (p_data,
						(gaiaPolygonPtr)
						yymsp[-1].minor.yy0);
	  }
	  break;
      case 59:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
      case 60:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 60);
      case 65:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 65);
      case 66:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 66);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPolygonSrid (p_data,
						    (gaiaPolygonPtr)
						    yymsp[-1].minor.yy0,
						    (int *) yymsp[-5].minor.
						    yy0);
	  }
	  break;
      case 61:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
      case 62:			/* polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 62);
      case 67:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 67);
      case 68:			/* polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 68);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPolygonSrid (p_data,
						    (gaiaPolygonPtr)
						    yymsp[-1].minor.yy0,
						    (int *) yymsp[-11].minor.
						    yy0);
	  }
	  break;
      case 69:			/* polygon_text ::= GEOJSON_OPEN_BRACKET ring extra_rings GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_polygon_xy (p_data,
					       (gaiaRingPtr) yymsp[-2].minor.
					       yy0);
	  }
	  break;
      case 70:			/* polygon_textz ::= GEOJSON_OPEN_BRACKET ringz extra_ringsz GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaRingPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_polygon_xyz (p_data,
						(gaiaRingPtr) yymsp[-2].minor.
						yy0);
	  }
	  break;
      case 71:			/* ring ::= GEOJSON_OPEN_BRACKET point_coordxy GEOJSON_COMMA point_coordxy GEOJSON_COMMA point_coordxy GEOJSON_COMMA point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET */
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
		  (void *) geoJSON_ring_xy (p_data,
					    (gaiaPointPtr) yymsp[-8].minor.yy0);
	  }
	  break;
      case 73:			/* extra_rings ::= GEOJSON_COMMA ring extra_rings */
      case 76:			/* extra_ringsz ::= GEOJSON_COMMA ringz extra_ringsz */
	  yytestcase (yyruleno == 76);
	  {
	      ((gaiaRingPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaRingPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 74:			/* ringz ::= GEOJSON_OPEN_BRACKET point_coordxyz GEOJSON_COMMA point_coordxyz GEOJSON_COMMA point_coordxyz GEOJSON_COMMA point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET */
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
		  (void *) geoJSON_ring_xyz (p_data,
					     (gaiaPointPtr) yymsp[-8].minor.
					     yy0);
	  }
	  break;
      case 77:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
      case 78:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 78);
      case 83:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 83);
      case 84:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 84);
      case 91:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 91);
      case 92:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 92);
      case 97:			/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 97);
      case 98:			/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 98);
      case 109:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 109);
      case 110:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 110);
      case 115:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 115);
      case 116:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 116);
      case 127:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 127);
      case 128:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 128);
      case 134:		/* geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 134);
	  {
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 79:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
      case 80:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 80);
      case 85:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 85);
      case 86:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 86);
      case 93:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 93);
      case 94:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 94);
      case 99:			/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 99);
      case 100:		/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 100);
      case 111:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 111);
      case 112:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 112);
      case 117:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 117);
      case 118:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 118);
      case 129:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 129);
      case 130:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 130);
      case 135:		/* geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 135);
      case 136:		/* geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 136);
	  {
	      yygotominor.yy0 =
		  (void *) geoJSON_setSrid ((gaiaGeomCollPtr) yymsp[-1].minor.
					    yy0, (int *) yymsp[-5].minor.yy0);
	  }
	  break;
      case 81:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
      case 82:			/* multipoint ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 82);
      case 87:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 87);
      case 88:			/* multipointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipoint_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 88);
      case 95:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 95);
      case 96:			/* multilinestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 96);
      case 101:		/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 101);
      case 102:		/* multilinestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multilinestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 102);
      case 113:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 113);
      case 114:		/* multipolygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 114);
      case 119:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 119);
      case 120:		/* multipolygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON multipolygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 120);
      case 131:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 131);
      case 132:		/* geocoll ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_text GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 132);
      case 137:		/* geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON short_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 137);
      case 138:		/* geocollz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA GEOJSON_CRS GEOJSON_COLON long_crs GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS GEOJSON_COLON geocoll_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 138);
	  {
	      yygotominor.yy0 =
		  (void *) geoJSON_setSrid ((gaiaGeomCollPtr) yymsp[-1].minor.
					    yy0, (int *) yymsp[-11].minor.yy0);
	  }
	  break;
      case 89:			/* multipoint_text ::= GEOJSON_OPEN_BRACKET point_coordxy extra_pointsxy GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multipoint_xy (p_data,
						  (gaiaPointPtr)
						  yymsp[-2].minor.yy0);
	  }
	  break;
      case 90:			/* multipoint_textz ::= GEOJSON_OPEN_BRACKET point_coordxyz extra_pointsxyz GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPointPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPointPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multipoint_xyz (p_data,
						   (gaiaPointPtr)
						   yymsp[-2].minor.yy0);
	  }
	  break;
      case 103:		/* multilinestring_text ::= GEOJSON_OPEN_BRACKET linestring_text multilinestring_text2 GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multilinestring_xy (p_data,
						       (gaiaLinestringPtr)
						       yymsp[-2].minor.yy0);
	  }
	  break;
      case 105:		/* multilinestring_text2 ::= GEOJSON_COMMA linestring_text multilinestring_text2 */
      case 108:		/* multilinestring_textz2 ::= GEOJSON_COMMA linestring_textz multilinestring_textz2 */
	  yytestcase (yyruleno == 108);
	  {
	      ((gaiaLinestringPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 106:		/* multilinestring_textz ::= GEOJSON_OPEN_BRACKET linestring_textz multilinestring_textz2 GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaLinestringPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaLinestringPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multilinestring_xyz (p_data,
							(gaiaLinestringPtr)
							yymsp[-2].minor.yy0);
	  }
	  break;
      case 121:		/* multipolygon_text ::= GEOJSON_OPEN_BRACKET polygon_text multipolygon_text2 GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multipolygon_xy (p_data,
						    (gaiaPolygonPtr)
						    yymsp[-2].minor.yy0);
	  }
	  break;
      case 123:		/* multipolygon_text2 ::= GEOJSON_COMMA polygon_text multipolygon_text2 */
      case 126:		/* multipolygon_textz2 ::= GEOJSON_COMMA polygon_textz multipolygon_textz2 */
	  yytestcase (yyruleno == 126);
	  {
	      ((gaiaPolygonPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 124:		/* multipolygon_textz ::= GEOJSON_OPEN_BRACKET polygon_textz multipolygon_textz2 GEOJSON_CLOSE_BRACKET */
	  {
	      ((gaiaPolygonPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaPolygonPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_multipolygon_xyz (p_data,
						     (gaiaPolygonPtr)
						     yymsp[-2].minor.yy0);
	  }
	  break;
      case 139:		/* geocoll_text ::= GEOJSON_OPEN_BRACKET coll_point geocoll_text2 GEOJSON_CLOSE_BRACKET */
      case 140:		/* geocoll_text ::= GEOJSON_OPEN_BRACKET coll_linestring geocoll_text2 GEOJSON_CLOSE_BRACKET */
	  yytestcase (yyruleno == 140);
      case 141:		/* geocoll_text ::= GEOJSON_OPEN_BRACKET coll_polygon geocoll_text2 GEOJSON_CLOSE_BRACKET */
	  yytestcase (yyruleno == 141);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_geomColl_xy (p_data,
						(gaiaGeomCollPtr)
						yymsp[-2].minor.yy0);
	  }
	  break;
      case 143:		/* geocoll_text2 ::= GEOJSON_COMMA coll_point geocoll_text2 */
      case 144:		/* geocoll_text2 ::= GEOJSON_COMMA coll_linestring geocoll_text2 */
	  yytestcase (yyruleno == 144);
      case 145:		/* geocoll_text2 ::= GEOJSON_COMMA coll_polygon geocoll_text2 */
	  yytestcase (yyruleno == 145);
      case 150:		/* geocoll_textz2 ::= GEOJSON_COMMA coll_pointz geocoll_textz2 */
	  yytestcase (yyruleno == 150);
      case 151:		/* geocoll_textz2 ::= GEOJSON_COMMA coll_linestringz geocoll_textz2 */
	  yytestcase (yyruleno == 151);
      case 152:		/* geocoll_textz2 ::= GEOJSON_COMMA coll_polygonz geocoll_textz2 */
	  yytestcase (yyruleno == 152);
	  {
	      ((gaiaGeomCollPtr) yymsp[-1].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[0].minor.yy0;
	      yygotominor.yy0 = yymsp[-1].minor.yy0;
	  }
	  break;
      case 146:		/* geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_pointz geocoll_textz2 GEOJSON_CLOSE_BRACKET */
      case 147:		/* geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_linestringz geocoll_textz2 GEOJSON_CLOSE_BRACKET */
	  yytestcase (yyruleno == 147);
      case 148:		/* geocoll_textz ::= GEOJSON_OPEN_BRACKET coll_polygonz geocoll_textz2 GEOJSON_CLOSE_BRACKET */
	  yytestcase (yyruleno == 148);
	  {
	      ((gaiaGeomCollPtr) yymsp[-2].minor.yy0)->Next =
		  (gaiaGeomCollPtr) yymsp[-1].minor.yy0;
	      yygotominor.yy0 =
		  (void *) geoJSON_geomColl_xyz (p_data,
						 (gaiaGeomCollPtr)
						 yymsp[-2].minor.yy0);
	  }
	  break;
      case 153:		/* coll_point ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxy GEOJSON_CLOSE_BRACE */
      case 154:		/* coll_pointz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON point_coordxyz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 154);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPoint (p_data,
					      (gaiaPointPtr) yymsp[-1].minor.
					      yy0);
	  }
	  break;
      case 155:		/* coll_linestring ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_text GEOJSON_CLOSE_BRACE */
      case 156:		/* coll_linestringz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON linestring_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 156);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromLinestring (p_data,
						   (gaiaLinestringPtr)
						   yymsp[-1].minor.yy0);
	  }
	  break;
      case 157:		/* coll_polygon ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_text GEOJSON_CLOSE_BRACE */
      case 158:		/* coll_polygonz ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA GEOJSON_COORDS GEOJSON_COLON polygon_textz GEOJSON_CLOSE_BRACE */
	  yytestcase (yyruleno == 158);
	  {
	      yygotominor.yy0 =
		  geoJSON_buildGeomFromPolygon (p_data,
						(gaiaPolygonPtr)
						yymsp[-1].minor.yy0);
	  }
	  break;
      default:
	  /* (0) main ::= in */ yytestcase (yyruleno == 0);
	  /* (1) in ::= */ yytestcase (yyruleno == 1);
	  /* (2) in ::= in state GEOJSON_NEWLINE */ yytestcase (yyruleno == 2);
	  /* (3) state ::= program */ yytestcase (yyruleno == 3);
	  /* (4) program ::= geo_text */ yytestcase (yyruleno == 4);
	  /* (31) bbox ::= coord GEOJSON_COMMA coord GEOJSON_COMMA coord GEOJSON_COMMA coord */
	  yytestcase (yyruleno == 31);
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
    p_data->geoJson_parse_error = 1;
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
