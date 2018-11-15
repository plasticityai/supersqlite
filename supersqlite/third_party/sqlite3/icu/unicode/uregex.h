// © 2016 and later: Unicode, Inc. and others.
 // License & terms of use: http://www.unicode.org/copyright.html
 /*
 **********************************************************************
 *   Copyright (C) 2004-2016, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 **********************************************************************
 *   file name:  uregex.h
 *   encoding:   UTF-8
 *   indentation:4
 *
 *   created on: 2004mar09
 *   created by: Andy Heninger
 *
 *   ICU Regular Expressions, API for C
 */
 
 #ifndef UREGEX_H
 #define UREGEX_H
 
 #include "unicode/utext.h"
 #include "unicode/utypes.h"
 
 #if !UCONFIG_NO_REGULAR_EXPRESSIONS
 
 #include "unicode/localpointer.h"
 #include "unicode/parseerr.h"
 
 struct URegularExpression;
 typedef struct URegularExpression URegularExpression;
 
 
 typedef enum URegexpFlag{
 
 #ifndef U_HIDE_DRAFT_API 
 
     UREGEX_CANON_EQ         = 128,
 #endif /* U_HIDE_DRAFT_API */
 
     UREGEX_CASE_INSENSITIVE = 2,
 
     UREGEX_COMMENTS         = 4,
 
     UREGEX_DOTALL           = 32,
     
     UREGEX_LITERAL = 16,
 
     UREGEX_MULTILINE        = 8,
     
     UREGEX_UNIX_LINES = 1,
 
     UREGEX_UWORD            = 256,
 
      UREGEX_ERROR_ON_UNKNOWN_ESCAPES = 512
 
 }  URegexpFlag;
 
 U_STABLE URegularExpression * U_EXPORT2
 uregex_open( const  UChar          *pattern,
                     int32_t         patternLength,
                     uint32_t        flags,
                     UParseError    *pe,
                     UErrorCode     *status);
 
 U_STABLE URegularExpression *  U_EXPORT2
 uregex_openUText(UText          *pattern,
                  uint32_t        flags,
                  UParseError    *pe,
                  UErrorCode     *status);
 
 #if !UCONFIG_NO_CONVERSION
 U_STABLE URegularExpression * U_EXPORT2
 uregex_openC( const char           *pattern,
                     uint32_t        flags,
                     UParseError    *pe,
                     UErrorCode     *status);
 #endif
 
 
 
 U_STABLE void U_EXPORT2 
 uregex_close(URegularExpression *regexp);
 
 #if U_SHOW_CPLUSPLUS_API
 
 U_NAMESPACE_BEGIN
 
 U_DEFINE_LOCAL_OPEN_POINTER(LocalURegularExpressionPointer, URegularExpression, uregex_close);
 
 U_NAMESPACE_END
 
 #endif
 
 U_STABLE URegularExpression * U_EXPORT2 
 uregex_clone(const URegularExpression *regexp, UErrorCode *status);
 
 U_STABLE const UChar * U_EXPORT2 
 uregex_pattern(const URegularExpression *regexp,
                      int32_t            *patLength,
                      UErrorCode         *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_patternUText(const URegularExpression *regexp,
                           UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_flags(const  URegularExpression   *regexp,
                     UErrorCode           *status);
 
 
 U_STABLE void U_EXPORT2 
 uregex_setText(URegularExpression *regexp,
                const UChar        *text,
                int32_t             textLength,
                UErrorCode         *status);
 
 
 U_STABLE void U_EXPORT2 
 uregex_setUText(URegularExpression *regexp,
                 UText              *text,
                 UErrorCode         *status);
 
 U_STABLE const UChar * U_EXPORT2 
 uregex_getText(URegularExpression *regexp,
                int32_t            *textLength,
                UErrorCode         *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_getUText(URegularExpression *regexp,
                 UText              *dest,
                 UErrorCode         *status);
 
 U_STABLE void U_EXPORT2 
 uregex_refreshUText(URegularExpression *regexp,
                     UText              *text,
                     UErrorCode         *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_matches(URegularExpression *regexp,
                 int32_t            startIndex,
                 UErrorCode        *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_matches64(URegularExpression *regexp,
                  int64_t            startIndex,
                  UErrorCode        *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_lookingAt(URegularExpression *regexp,
                  int32_t             startIndex,
                  UErrorCode         *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_lookingAt64(URegularExpression *regexp,
                    int64_t             startIndex,
                    UErrorCode         *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_find(URegularExpression *regexp,
             int32_t             startIndex, 
             UErrorCode         *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_find64(URegularExpression *regexp,
               int64_t             startIndex, 
               UErrorCode         *status);
 
 U_STABLE UBool U_EXPORT2 
 uregex_findNext(URegularExpression *regexp,
                 UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_groupCount(URegularExpression *regexp,
                   UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2
 uregex_groupNumberFromName(URegularExpression *regexp,
                            const UChar        *groupName,
                            int32_t             nameLength,
                            UErrorCode          *status);
 
 
 U_STABLE int32_t U_EXPORT2
 uregex_groupNumberFromCName(URegularExpression *regexp,
                             const char         *groupName,
                             int32_t             nameLength,
                             UErrorCode          *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_group(URegularExpression *regexp,
              int32_t             groupNum,
              UChar              *dest,
              int32_t             destCapacity,
              UErrorCode          *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_groupUText(URegularExpression *regexp,
                   int32_t             groupNum,
                   UText              *dest,
                   int64_t            *groupLength,
                   UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_start(URegularExpression *regexp,
              int32_t             groupNum,
              UErrorCode          *status);
 
 U_STABLE int64_t U_EXPORT2 
 uregex_start64(URegularExpression *regexp,
                int32_t             groupNum,
                UErrorCode          *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_end(URegularExpression   *regexp,
            int32_t               groupNum,
            UErrorCode           *status);
 
 U_STABLE int64_t U_EXPORT2 
 uregex_end64(URegularExpression *regexp,
              int32_t               groupNum,
              UErrorCode           *status);
 
 U_STABLE void U_EXPORT2 
 uregex_reset(URegularExpression    *regexp,
              int32_t               index,
              UErrorCode            *status);
 
 U_STABLE void U_EXPORT2 
 uregex_reset64(URegularExpression  *regexp,
                int64_t               index,
                UErrorCode            *status);
 
 U_STABLE void U_EXPORT2
 uregex_setRegion(URegularExpression   *regexp,
                  int32_t               regionStart,
                  int32_t               regionLimit,
                  UErrorCode           *status);
 
 U_STABLE void U_EXPORT2 
 uregex_setRegion64(URegularExpression *regexp,
                  int64_t               regionStart,
                  int64_t               regionLimit,
                  UErrorCode           *status);
 
 U_STABLE void U_EXPORT2 
 uregex_setRegionAndStart(URegularExpression *regexp,
                  int64_t               regionStart,
                  int64_t               regionLimit,
                  int64_t               startIndex,
                  UErrorCode           *status);
 
 U_STABLE int32_t U_EXPORT2
 uregex_regionStart(const  URegularExpression   *regexp,
                           UErrorCode           *status);
 
 U_STABLE int64_t U_EXPORT2 
 uregex_regionStart64(const  URegularExpression   *regexp,
                             UErrorCode           *status);
 
 U_STABLE int32_t U_EXPORT2
 uregex_regionEnd(const  URegularExpression   *regexp,
                         UErrorCode           *status);
 
 U_STABLE int64_t U_EXPORT2 
 uregex_regionEnd64(const  URegularExpression   *regexp,
                           UErrorCode           *status);
 
 U_STABLE UBool U_EXPORT2
 uregex_hasTransparentBounds(const  URegularExpression   *regexp,
                                    UErrorCode           *status);
 
 
 U_STABLE void U_EXPORT2  
 uregex_useTransparentBounds(URegularExpression   *regexp, 
                             UBool                b,
                             UErrorCode           *status);
 
 
 U_STABLE UBool U_EXPORT2
 uregex_hasAnchoringBounds(const  URegularExpression   *regexp,
                                  UErrorCode           *status);
 
 
 U_STABLE void U_EXPORT2
 uregex_useAnchoringBounds(URegularExpression   *regexp,
                           UBool                 b,
                           UErrorCode           *status);
 
 U_STABLE UBool U_EXPORT2
 uregex_hitEnd(const  URegularExpression   *regexp,
                      UErrorCode           *status);
 
 U_STABLE UBool U_EXPORT2   
 uregex_requireEnd(const  URegularExpression   *regexp,
                          UErrorCode           *status);
 
 
 
 
 
 U_STABLE int32_t U_EXPORT2 
 uregex_replaceAll(URegularExpression    *regexp,
                   const UChar           *replacementText,
                   int32_t                replacementLength,
                   UChar                 *destBuf,
                   int32_t                destCapacity,
                   UErrorCode            *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_replaceAllUText(URegularExpression *regexp,
                        UText              *replacement,
                        UText              *dest,
                        UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_replaceFirst(URegularExpression  *regexp,
                     const UChar         *replacementText,
                     int32_t              replacementLength,
                     UChar               *destBuf,
                     int32_t              destCapacity,
                     UErrorCode          *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_replaceFirstUText(URegularExpression *regexp,
                          UText              *replacement,
                          UText              *dest,
                          UErrorCode         *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_appendReplacement(URegularExpression    *regexp,
                          const UChar           *replacementText,
                          int32_t                replacementLength,
                          UChar                **destBuf,
                          int32_t               *destCapacity,
                          UErrorCode            *status);
 
 U_STABLE void U_EXPORT2 
 uregex_appendReplacementUText(URegularExpression    *regexp,
                               UText                 *replacementText,
                               UText                 *dest,
                               UErrorCode            *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_appendTail(URegularExpression    *regexp,
                   UChar                **destBuf,
                   int32_t               *destCapacity,
                   UErrorCode            *status);
 
 U_STABLE UText * U_EXPORT2 
 uregex_appendTailUText(URegularExpression    *regexp,
                        UText                 *dest,
                        UErrorCode            *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_split(   URegularExpression      *regexp,
                   UChar                 *destBuf,
                   int32_t                destCapacity,
                   int32_t               *requiredCapacity,
                   UChar                 *destFields[],
                   int32_t                destFieldsCapacity,
                   UErrorCode            *status);
 
 U_STABLE int32_t U_EXPORT2 
 uregex_splitUText(URegularExpression    *regexp,
                   UText                 *destFields[],
                   int32_t                destFieldsCapacity,
                   UErrorCode            *status);
 
 U_STABLE void U_EXPORT2
 uregex_setTimeLimit(URegularExpression      *regexp,
                     int32_t                  limit,
                     UErrorCode              *status);
 
 U_STABLE int32_t U_EXPORT2
 uregex_getTimeLimit(const URegularExpression      *regexp,
                           UErrorCode              *status);
 
 U_STABLE void U_EXPORT2
 uregex_setStackLimit(URegularExpression      *regexp,
                      int32_t                  limit,
                      UErrorCode              *status);
 
 U_STABLE int32_t U_EXPORT2
 uregex_getStackLimit(const URegularExpression      *regexp,
                            UErrorCode              *status);
 
 
 U_CDECL_BEGIN
 typedef UBool U_CALLCONV URegexMatchCallback (
                    const void *context,
                    int32_t     steps);
 U_CDECL_END
 
 U_STABLE void U_EXPORT2
 uregex_setMatchCallback(URegularExpression      *regexp,
                         URegexMatchCallback     *callback,
                         const void              *context,
                         UErrorCode              *status);
 
 
 U_STABLE void U_EXPORT2
 uregex_getMatchCallback(const URegularExpression    *regexp,
                         URegexMatchCallback        **callback,
                         const void                 **context,
                         UErrorCode                  *status);
 
 U_CDECL_BEGIN
 typedef UBool U_CALLCONV URegexFindProgressCallback (
                    const void *context,
                    int64_t     matchIndex);
 U_CDECL_END
 
 
 U_STABLE void U_EXPORT2
 uregex_setFindProgressCallback(URegularExpression              *regexp,
                                 URegexFindProgressCallback      *callback,
                                 const void                      *context,
                                 UErrorCode                      *status);
 
 U_STABLE void U_EXPORT2
 uregex_getFindProgressCallback(const URegularExpression          *regexp,
                                 URegexFindProgressCallback        **callback,
                                 const void                        **context,
                                 UErrorCode                        *status);
 
 #endif   /*  !UCONFIG_NO_REGULAR_EXPRESSIONS  */
 #endif   /*  UREGEX_H  */