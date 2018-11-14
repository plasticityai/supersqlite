// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
//---------------------------------------------------------------------------------
//
// Generated Header File.  Do not edit by hand.
//    This file contains the state table for the ICU Regular Expression Pattern Parser
//    It is generated by the Perl script "regexcst.pl" from
//    the rule parser state definitions file "regexcst.txt".
//
//   Copyright (C) 2002-2016 International Business Machines Corporation 
//   and others. All rights reserved.  
//
//---------------------------------------------------------------------------------
#ifndef REGEXCST_H
#define REGEXCST_H

#include "unicode/utypes.h"

U_NAMESPACE_BEGIN
//
// Character classes for regex pattern scanning.
//
    static const uint8_t kRuleSet_ascii_letter = 128;
    static const uint8_t kRuleSet_digit_char2 = 129;
    static const uint8_t kRuleSet_rule_char2 = 130;


enum Regex_PatternParseAction {
    doSetBackslash_V2,
    doSetBackslash_h2,
    doBeginNamedBackRef2,
    doSetMatchMode2,
    doEnterQuoteMode2,
    doOpenCaptureParen2,
    doContinueNamedCapture2,
    doSetBackslash_d2,
    doBeginMatchMode2,
    doBackslashX2,
    doSetPosixProp2,
    doIntervalError2,
    doSetLiteralEscaped2,
    doSetBackslash_s2,
    doNOP2,
    doBackslashv2,
    doOpenLookBehind2,
    doPatStart2,
    doPossessiveInterval2,
    doOpenAtomicParen2,
    doOpenLookAheadNeg2,
    doBackslashd2,
    doBackslashZ2,
    doIntervalUpperDigit2,
    doBadNamedCapture2,
    doSetDifference22,
    doSetAddAmp2,
    doSetNamedChar2,
    doNamedChar2,
    doSetBackslash_H2,
    doBackslashb2,
    doBackslashz2,
    doSetBeginDifference12,
    doOpenLookAhead2,
    doMatchModeParen2,
    doBackslashV2,
    doIntevalLowerDigit2,
    doCaret2,
    doSetEnd2,
    doSetNegate2,
    doBackslashS2,
    doOrOperator2,
    doBackslashB2,
    doBackslashw2,
    doBackslashR2,
    doRuleError2,
    doDotAny
    doMatchMode2,
    doSetBackslash_W2,
    doNGPlus2,
    doSetBackslash_D2,
    doPossessiveOpt2,
    doSetNamedRange2,
    doConditionalExpr2,
    doBackslashs2,
    doPossessiveStar2,
    doPlus2,
    doBadOpenParenType2,
    doCloseParen2,
    doNGInterval2,
    doSetProp2,
    doBackRef2,
    doSetBeginUnion2,
    doEscapeError2,
    doOpt2,
    doSetBeginIntersection12,
    doPossessivePlus2,
    doBackslashD2,
    doOpenLookBehindNeg2,
    doSetBegin2,
    doSetIntersection22,
    doCompleteNamedBackRef2,
    doSetRange2,
    doDollar2,
    doBackslashH2,
    doExit2,
    doNGOpt2,
    doOpenNonCaptureParen2,
    doBackslashA2,
    doSetBackslash_v2,
    doBackslashh2,
    doBadModeFlag2,
    doSetNoCloseError2,
    doIntervalSame2,
    doSetAddDash2,
    doBackslashW2,
    doPerlInline2,
    doSetOpError2,
    doSetLiteral2,
    doPatFinish2,
    doBeginNamedCapture2,
    doEscapedLiteralChar2,
    doLiteralChar2,
    doSuppressComments2,
    doMismatchedParenErr2,
    doNGStar2,
    doSetFinish2,
    doInterval2,
    doBackslashG2,
    doStar2,
    doSetBackslash_w2,
    doSetBackslash_S2,
    doProperty2,
    doContinueNamedBackRef2,
    doIntervalInit2,
    rbbiLastAction2};

//-------------------------------------------------------------------------------
//
//  RegexTableEl       represents the structure of a row in the transition table
//                     for the pattern parser state machine.
//-------------------------------------------------------------------------------
struct RegexTableEl {
    Regex_PatternParseAction      fAction;
    uint8_t                       fCharClass;       // 0-127:    an individual ASCII character
                                                    // 128-255:  character class index
    uint8_t                       fNextState;       // 0-250:    normal next-state numbers
                                                    // 255:      pop next-state from stack.
    uint8_t                       fPushState;
    UBool                         fNextChar;
};

static const struct RegexTableEl gRuleParseStateTable2[] = {
    {doNOP2, 0, 0, 0, TRUE}
    , {doPatStart2, 255, 2,0,  FALSE}     //  1      start
    , {doLiteralChar2, 254, 14,0,  TRUE}     //  2      term
    , {doLiteralChar2, 130, 14,0,  TRUE}     //  3 
    , {doSetBegin2, 91 /* [ */, 123, 205, TRUE}     //  4 
    , {doNOP2, 40 /* ( */, 27,0,  TRUE}     //  5 
    , {doDotAny2, 46 /* . */, 14,0,  TRUE}     //  6
    , {doCaret2, 94 /* ^ */, 14,0,  TRUE}     //  7 
    , {doDollar2, 36 /* $ */, 14,0,  TRUE}     //  8 
    , {doNOP2, 92 /* \ */, 89,0,  TRUE}     //  9 
    , {doOrOperator2, 124 /* | */, 2,0,  TRUE}     //  10 
    , {doCloseParen2, 41 /* ) */, 255,0,  TRUE}     //  11 
    , {doPatFinish2, 253, 2,0,  FALSE}     //  12 
    , {doRuleError2, 255, 206,0,  FALSE}     //  13 
    , {doNOP2, 42 /* * */, 68,0,  TRUE}     //  14      expr-quant
    , {doNOP2, 43 /* + */, 71,0,  TRUE}     //  15 
    , {doNOP2, 63 /* ? */, 74,0,  TRUE}     //  16 
    , {doIntervalInit2, 123 /* { */, 77,0,  TRUE}     //  17 
    , {doNOP2, 40 /* ( */, 23,0,  TRUE}     //  18 
    , {doNOP2, 255, 20,0,  FALSE}     //  19 
    , {doOrOperator2, 124 /* | */, 2,0,  TRUE}     //  20      expr-cont
    , {doCloseParen2, 41 /* ) */, 255,0,  TRUE}     //  21 
    , {doNOP2, 255, 2,0,  FALSE}     //  22 
    , {doSuppressComments2, 63 /* ? */, 25,0,  TRUE}     //  23      open-paren-quant
    , {doNOP2, 255, 27,0,  FALSE}     //  24 
    , {doNOP2, 35 /* # */, 50, 14, TRUE}     //  25      open-paren-quant2
    , {doNOP2, 255, 29,0,  FALSE}     //  26 
    , {doSuppressComments2, 63 /* ? */, 29,0,  TRUE}     //  27      open-paren
    , {doOpenCaptureParen2, 255, 2, 14, FALSE}     //  28 
    , {doOpenNonCaptureParen2, 58 /* : */, 2, 14, TRUE}     //  29      open-paren-extended
    , {doOpenAtomicParen2, 62 /* > */, 2, 14, TRUE}     //  30 
    , {doOpenLookAhead2, 61 /* = */, 2, 20, TRUE}     //  31 
    , {doOpenLookAheadNeg2, 33 /* ! */, 2, 20, TRUE}     //  32 
    , {doNOP2, 60 /* < */, 46,0,  TRUE}     //  33 
    , {doNOP2, 35 /* # */, 50, 2, TRUE}     //  34 
    , {doBeginMatchMode2, 105 /* i */, 53,0,  FALSE}     //  35 
    , {doBeginMatchMode2, 100 /* d */, 53,0,  FALSE}     //  36 
    , {doBeginMatchMode2, 109 /* m */, 53,0,  FALSE}     //  37 
    , {doBeginMatchMode2, 115 /* s */, 53,0,  FALSE}     //  38 
    , {doBeginMatchMode2, 117 /* u */, 53,0,  FALSE}     //  39 
    , {doBeginMatchMode2, 119 /* w */, 53,0,  FALSE}     //  40 
    , {doBeginMatchMode2, 120 /* x */, 53,0,  FALSE}     //  41 
    , {doBeginMatchMode2, 45 /* - */, 53,0,  FALSE}     //  42 
    , {doConditionalExpr2, 40 /* ( */, 206,0,  TRUE}     //  43 
    , {doPerlInline2, 123 /* { */, 206,0,  TRUE}     //  44 
    , {doBadOpenParenType2, 255, 206,0,  FALSE}     //  45 
    , {doOpenLookBehind2, 61 /* = */, 2, 20, TRUE}     //  46      open-paren-lookbehind
    , {doOpenLookBehindNeg2, 33 /* ! */, 2, 20, TRUE}     //  47 
    , {doBeginNamedCapture2, 128, 64,0,  FALSE}     //  48 
    , {doBadOpenParenType2, 255, 206,0,  FALSE}     //  49 
    , {doNOP2, 41 /* ) */, 255,0,  TRUE}     //  50      paren-comment
    , {doMismatchedParenErr2, 253, 206,0,  FALSE}     //  51 
    , {doNOP2, 255, 50,0,  TRUE}     //  52 
    , {doMatchMode2, 105 /* i */, 53,0,  TRUE}     //  53      paren-flag
    , {doMatchMode2, 100 /* d */, 53,0,  TRUE}     //  54 
    , {doMatchMode2, 109 /* m */, 53,0,  TRUE}     //  55 
    , {doMatchMode2, 115 /* s */, 53,0,  TRUE}     //  56 
    , {doMatchMode2, 117 /* u */, 53,0,  TRUE}     //  57 
    , {doMatchMode2, 119 /* w */, 53,0,  TRUE}     //  58 
    , {doMatchMode2, 120 /* x */, 53,0,  TRUE}     //  59 
    , {doMatchMode2, 45 /* - */, 53,0,  TRUE}     //  60 
    , {doSetMatchMode2, 41 /* ) */, 2,0,  TRUE}     //  61 
    , {doMatchModeParen2, 58 /* : */, 2, 14, TRUE}     //  62 
    , {doBadModeFlag2, 255, 206,0,  FALSE}     //  63 
    , {doContinueNamedCapture2, 128, 64,0,  TRUE}     //  64      named-capture
    , {doContinueNamedCapture2, 129, 64,0,  TRUE}     //  65 
    , {doOpenCaptureParen2, 62 /* > */, 2, 14, TRUE}     //  66 
    , {doBadNamedCapture2, 255, 206,0,  FALSE}     //  67 
    , {doNGStar2, 63 /* ? */, 20,0,  TRUE}     //  68      quant-star
    , {doPossessiveStar2, 43 /* + */, 20,0,  TRUE}     //  69 
    , {doStar2, 255, 20,0,  FALSE}     //  70 
    , {doNGPlus2, 63 /* ? */, 20,0,  TRUE}     //  71      quant-plus
    , {doPossessivePlus2, 43 /* + */, 20,0,  TRUE}     //  72 
    , {doPlus2, 255, 20,0,  FALSE}     //  73 
    , {doNGOpt2, 63 /* ? */, 20,0,  TRUE}     //  74      quant-opt
    , {doPossessiveOpt2, 43 /* + */, 20,0,  TRUE}     //  75 
    , {doOpt2, 255, 20,0,  FALSE}     //  76 
    , {doNOP2, 129, 79,0,  FALSE}     //  77      interval-open
    , {doIntervalError2, 255, 206,0,  FALSE}     //  78 
    , {doIntevalLowerDigit2, 129, 79,0,  TRUE}     //  79      interval-lower
    , {doNOP2, 44 /* , */, 83,0,  TRUE}     //  80 
    , {doIntervalSame2, 125 /* } */, 86,0,  TRUE}     //  81 
    , {doIntervalError2, 255, 206,0,  FALSE}     //  82 
    , {doIntervalUpperDigit2, 129, 83,0,  TRUE}     //  83      interval-upper
    , {doNOP2, 125 /* } */, 86,0,  TRUE}     //  84 
    , {doIntervalError2, 255, 206,0,  FALSE}     //  85 
    , {doNGInterval2, 63 /* ? */, 20,0,  TRUE}     //  86      interval-type
    , {doPossessiveInterval2, 43 /* + */, 20,0,  TRUE}     //  87 
    , {doInterval2, 255, 20,0,  FALSE}     //  88 
    , {doBackslashA2, 65 /* A */, 2,0,  TRUE}     //  89      backslash
    , {doBackslashB2, 66 /* B */, 2,0,  TRUE}     //  90 
    , {doBackslashb2, 98 /* b */, 2,0,  TRUE}     //  91 
    , {doBackslashd2, 100 /* d */, 14,0,  TRUE}     //  92 
    , {doBackslashD2, 68 /* D */, 14,0,  TRUE}     //  93 
    , {doBackslashG2, 71 /* G */, 2,0,  TRUE}     //  94 
    , {doBackslashh2, 104 /* h */, 14,0,  TRUE}     //  95 
    , {doBackslashH2, 72 /* H */, 14,0,  TRUE}     //  96 
    , {doNOP2, 107 /* k */, 115,0,  TRUE}     //  97 
    , {doNamedChar2, 78 /* N */, 14,0,  FALSE}     //  98 
    , {doProperty2, 112 /* p */, 14,0,  FALSE}     //  99 
    , {doProperty2, 80 /* P */, 14,0,  FALSE}     //  100 
    , {doBackslashR2, 82 /* R */, 14,0,  TRUE}     //  101 
    , {doEnterQuoteMode2, 81 /* Q */, 2,0,  TRUE}     //  102 
    , {doBackslashS2, 83 /* S */, 14,0,  TRUE}     //  103 
    , {doBackslashs2, 115 /* s */, 14,0,  TRUE}     //  104 
    , {doBackslashv2, 118 /* v */, 14,0,  TRUE}     //  105 
    , {doBackslashV2, 86 /* V */, 14,0,  TRUE}     //  106 
    , {doBackslashW2, 87 /* W */, 14,0,  TRUE}     //  107 
    , {doBackslashw2, 119 /* w */, 14,0,  TRUE}     //  108 
    , {doBackslashX2, 88 /* X */, 14,0,  TRUE}     //  109 
    , {doBackslashZ2, 90 /* Z */, 2,0,  TRUE}     //  110 
    , {doBackslashz2, 122 /* z */, 2,0,  TRUE}     //  111 
    , {doBackRef2, 129, 14,0,  TRUE}     //  112 
    , {doEscapeError2, 253, 206,0,  FALSE}     //  113 
    , {doEscapedLiteralChar2, 255, 14,0,  TRUE}     //  114 
    , {doBeginNamedBackRef2, 60 /* < */, 117,0,  TRUE}     //  115      named-backref
    , {doBadNamedCapture2, 255, 206,0,  FALSE}     //  116 
    , {doContinueNamedBackRef2, 128, 119,0,  TRUE}     //  117      named-backref-2
    , {doBadNamedCapture2, 255, 206,0,  FALSE}     //  118 
    , {doContinueNamedBackRef2, 128, 119,0,  TRUE}     //  119      named-backref-3
    , {doContinueNamedBackRef2, 129, 119,0,  TRUE}     //  120 
    , {doCompleteNamedBackRef2, 62 /* > */, 14,0,  TRUE}     //  121 
    , {doBadNamedCapture2, 255, 206,0,  FALSE}     //  122 
    , {doSetNegate2, 94 /* ^ */, 126,0,  TRUE}     //  123      set-open
    , {doSetPosixProp2, 58 /* : */, 128,0,  FALSE}     //  124 
    , {doNOP2, 255, 126,0,  FALSE}     //  125 
    , {doSetLiteral2, 93 /* ] */, 141,0,  TRUE}     //  126      set-open2
    , {doNOP2, 255, 131,0,  FALSE}     //  127 
    , {doSetEnd2, 93 /* ] */, 255,0,  TRUE}     //  128      set-posix
    , {doNOP2, 58 /* : */, 131,0,  FALSE}     //  129 
    , {doRuleError2, 255, 206,0,  FALSE}     //  130 
    , {doSetEnd2, 93 /* ] */, 255,0,  TRUE}     //  131      set-start
    , {doSetBeginUnion2, 91 /* [ */, 123, 148, TRUE}     //  132 
    , {doNOP2, 92 /* \ */, 191,0,  TRUE}     //  133 
    , {doNOP2, 45 /* - */, 137,0,  TRUE}     //  134 
    , {doNOP2, 38 /* & */, 139,0,  TRUE}     //  135 
    , {doSetLiteral2, 255, 141,0,  TRUE}     //  136 
    , {doRuleError2, 45 /* - */, 206,0,  FALSE}     //  137      set-start-dash
    , {doSetAddDash2, 255, 141,0,  FALSE}     //  138 
    , {doRuleError2, 38 /* & */, 206,0,  FALSE}     //  139      set-start-amp
    , {doSetAddAmp2, 255, 141,0,  FALSE}     //  140 
    , {doSetEnd2, 93 /* ] */, 255,0,  TRUE}     //  141      set-after-lit
    , {doSetBeginUnion2, 91 /* [ */, 123, 148, TRUE}     //  142 
    , {doNOP2, 45 /* - */, 178,0,  TRUE}     //  143 
    , {doNOP2, 38 /* & */, 169,0,  TRUE}     //  144 
    , {doNOP2, 92 /* \ */, 191,0,  TRUE}     //  145 
    , {doSetNoCloseError2, 253, 206,0,  FALSE}     //  146 
    , {doSetLiteral2, 255, 141,0,  TRUE}     //  147 
    , {doSetEnd2, 93 /* ] */, 255,0,  TRUE}     //  148      set-after-set
    , {doSetBeginUnion2, 91 /* [ */, 123, 148, TRUE}     //  149 
    , {doNOP2, 45 /* - */, 171,0,  TRUE}     //  150 
    , {doNOP2, 38 /* & */, 166,0,  TRUE}     //  151 
    , {doNOP2, 92 /* \ */, 191,0,  TRUE}     //  152 
    , {doSetNoCloseError2, 253, 206,0,  FALSE}     //  153 
    , {doSetLiteral2, 255, 141,0,  TRUE}     //  154 
    , {doSetEnd2, 93 /* ] */, 255,0,  TRUE}     //  155      set-after-range
    , {doSetBeginUnion2, 91 /* [ */, 123, 148, TRUE}     //  156 
    , {doNOP2, 45 /* - */, 174,0,  TRUE}     //  157 
    , {doNOP2, 38 /* & */, 176,0,  TRUE}     //  158 
    , {doNOP2, 92 /* \ */, 191,0,  TRUE}     //  159 
    , {doSetNoCloseError2, 253, 206,0,  FALSE}     //  160 
    , {doSetLiteral2, 255, 141,0,  TRUE}     //  161 
    , {doSetBeginUnion2, 91 /* [ */, 123, 148, TRUE}     //  162      set-after-op
    , {doSetOpError2, 93 /* ] */, 206,0,  FALSE}     //  163 
    , {doNOP2, 92 /* \ */, 191,0,  TRUE}     //  164 
    , {doSetLiteral2, 255, 141,0,  TRUE}     //  165 
    , {doSetBeginIntersection12, 91 /* [ */, 123, 148, TRUE}     //  166      set-set-amp
    , {doSetIntersection22, 38 /* & */, 162,0,  TRUE}     //  167 
    , {doSetAddAmp2, 255, 141,0,  FALSE}     //  168 
    , {doSetIntersection22, 38 /* & */, 162,0,  TRUE}     //  169      set-lit-amp
    , {doSetAddAmp2, 255, 141,0,  FALSE}     //  170 
    , {doSetBeginDifference12, 91 /* [ */, 123, 148, TRUE}     //  171      set-set-dash
    , {doSetDifference22, 45 /* - */, 162,0,  TRUE}     //  172 
    , {doSetAddDash2, 255, 141,0,  FALSE}     //  173 
    , {doSetDifference22, 45 /* - */, 162,0,  TRUE}     //  174      set-range-dash
    , {doSetAddDash2, 255, 141,0,  FALSE}     //  175 
    , {doSetIntersection22, 38 /* & */, 162,0,  TRUE}     //  176      set-range-amp
    , {doSetAddAmp2, 255, 141,0,  FALSE}     //  177 
    , {doSetDifference22, 45 /* - */, 162,0,  TRUE}     //  178      set-lit-dash
    , {doSetAddDash2, 91 /* [ */, 141,0,  FALSE}     //  179 
    , {doSetAddDash2, 93 /* ] */, 141,0,  FALSE}     //  180 
    , {doNOP2, 92 /* \ */, 183,0,  TRUE}     //  181 
    , {doSetRange2, 255, 155,0,  TRUE}     //  182 
    , {doSetOpError2, 115 /* s */, 206,0,  FALSE}     //  183      set-lit-dash-escape
    , {doSetOpError2, 83 /* S */, 206,0,  FALSE}     //  184 
    , {doSetOpError2, 119 /* w */, 206,0,  FALSE}     //  185 
    , {doSetOpError2, 87 /* W */, 206,0,  FALSE}     //  186 
    , {doSetOpError2, 100 /* d */, 206,0,  FALSE}     //  187 
    , {doSetOpError2, 68 /* D */, 206,0,  FALSE}     //  188 
    , {doSetNamedRange2, 78 /* N */, 155,0,  FALSE}     //  189 
    , {doSetRange2, 255, 155,0,  TRUE}     //  190 
    , {doSetProp2, 112 /* p */, 148,0,  FALSE}     //  191      set-escape
    , {doSetProp2, 80 /* P */, 148,0,  FALSE}     //  192 
    , {doSetNamedChar2, 78 /* N */, 141,0,  FALSE}     //  193 
    , {doSetBackslash_s2, 115 /* s */, 155,0,  TRUE}     //  194 
    , {doSetBackslash_S2, 83 /* S */, 155,0,  TRUE}     //  195 
    , {doSetBackslash_w2, 119 /* w */, 155,0,  TRUE}     //  196 
    , {doSetBackslash_W2, 87 /* W */, 155,0,  TRUE}     //  197 
    , {doSetBackslash_d2, 100 /* d */, 155,0,  TRUE}     //  198 
    , {doSetBackslash_D2, 68 /* D */, 155,0,  TRUE}     //  199 
    , {doSetBackslash_h2, 104 /* h */, 155,0,  TRUE}     //  200 
    , {doSetBackslash_H2, 72 /* H */, 155,0,  TRUE}     //  201 
    , {doSetBackslash_v2, 118 /* v */, 155,0,  TRUE}     //  202 
    , {doSetBackslash_V2, 86 /* V */, 155,0,  TRUE}     //  203 
    , {doSetLiteralEscaped2, 255, 141,0,  TRUE}     //  204 
    , {doSetFinish2, 255, 14,0,  FALSE}     //  205      set-finish
    , {doExit2, 255, 206,0,  TRUE}     //  206      errorDeath
 };
static const char * const RegexStateNames[] = {    0,
     "start",
     "term",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "expr-quant",
    0,
    0,
    0,
    0,
    0,
     "expr-cont",
    0,
    0,
     "open-paren-quant",
    0,
     "open-paren-quant2",
    0,
     "open-paren",
    0,
     "open-paren-extended",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "open-paren-lookbehind",
    0,
    0,
    0,
     "paren-comment",
    0,
    0,
     "paren-flag",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "named-capture",
    0,
    0,
    0,
     "quant-star",
    0,
    0,
     "quant-plus",
    0,
    0,
     "quant-opt",
    0,
    0,
     "interval-open",
    0,
     "interval-lower",
    0,
    0,
    0,
     "interval-upper",
    0,
    0,
     "interval-type",
    0,
    0,
     "backslash",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "named-backref",
    0,
     "named-backref-2",
    0,
     "named-backref-3",
    0,
    0,
    0,
     "set-open",
    0,
    0,
     "set-open2",
    0,
     "set-posix",
    0,
    0,
     "set-start",
    0,
    0,
    0,
    0,
    0,
     "set-start-dash",
    0,
     "set-start-amp",
    0,
     "set-after-lit",
    0,
    0,
    0,
    0,
    0,
    0,
     "set-after-set",
    0,
    0,
    0,
    0,
    0,
    0,
     "set-after-range",
    0,
    0,
    0,
    0,
    0,
    0,
     "set-after-op",
    0,
    0,
    0,
     "set-set-amp",
    0,
    0,
     "set-lit-amp",
    0,
     "set-set-dash",
    0,
    0,
     "set-range-dash",
    0,
     "set-range-amp",
    0,
     "set-lit-dash",
    0,
    0,
    0,
    0,
     "set-lit-dash-escape",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "set-escape",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
     "set-finish",
     "errorDeath",
    0};

U_NAMESPACE_END
#endif
