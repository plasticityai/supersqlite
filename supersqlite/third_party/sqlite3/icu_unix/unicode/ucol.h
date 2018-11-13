 // © 2016 and later: Unicode, Inc. and others.
 // License & terms of use: http://www.unicode.org/copyright.html
 /*
 *******************************************************************************
 * Copyright (c) 1996-2015, International Business Machines Corporation and others.
 * All Rights Reserved.
 *******************************************************************************
 */
 
 #ifndef UCOL_H
 #define UCOL_H
 
 #include "unicode/utypes.h"
 
 #if !UCONFIG_NO_COLLATION
 
 #include "unicode/unorm.h"
 #include "unicode/localpointer.h"
 #include "unicode/parseerr.h"
 #include "unicode/uloc.h"
 #include "unicode/uset.h"
 #include "unicode/uscript.h"
 
 struct UCollator;
 typedef struct UCollator UCollator;
 
 
 typedef enum {
   UCOL_EQUAL    = 0,
   UCOL_GREATER    = 1,
   UCOL_LESS    = -1
 } UCollationResult ;
 
 
 typedef enum {
   UCOL_DEFAULT = -1,
 
   UCOL_PRIMARY = 0,
   UCOL_SECONDARY = 1,
   UCOL_TERTIARY = 2,
   UCOL_DEFAULT_STRENGTH = UCOL_TERTIARY,
   UCOL_CE_STRENGTH_LIMIT,
   UCOL_QUATERNARY=3,
   UCOL_IDENTICAL=15,
   UCOL_STRENGTH_LIMIT,
 
   UCOL_OFF = 16,
   UCOL_ON = 17,
   
   UCOL_SHIFTED = 20,
   UCOL_NON_IGNORABLE = 21,
 
   UCOL_LOWER_FIRST = 24,
   UCOL_UPPER_FIRST = 25,
 
 #ifndef U_HIDE_DEPRECATED_API
 
   UCOL_ATTRIBUTE_VALUE_COUNT
 #endif  /* U_HIDE_DEPRECATED_API */
 } UColAttributeValue;
 
  typedef enum {
     UCOL_REORDER_CODE_DEFAULT       = -1,
     UCOL_REORDER_CODE_NONE          = USCRIPT_UNKNOWN,
     UCOL_REORDER_CODE_OTHERS        = USCRIPT_UNKNOWN,
     UCOL_REORDER_CODE_SPACE         = 0x1000,
     UCOL_REORDER_CODE_FIRST         = UCOL_REORDER_CODE_SPACE,
     UCOL_REORDER_CODE_PUNCTUATION   = 0x1001,
     UCOL_REORDER_CODE_SYMBOL        = 0x1002,
     UCOL_REORDER_CODE_CURRENCY      = 0x1003,
     UCOL_REORDER_CODE_DIGIT         = 0x1004,
 #ifndef U_HIDE_DEPRECATED_API
 
     UCOL_REORDER_CODE_LIMIT         = 0x1005
 #endif  /* U_HIDE_DEPRECATED_API */
 } UColReorderCode;
 
 typedef UColAttributeValue UCollationStrength;
 
 typedef enum {
      UCOL_FRENCH_COLLATION, 
      UCOL_ALTERNATE_HANDLING, 
      UCOL_CASE_FIRST, 
      UCOL_CASE_LEVEL,
      UCOL_NORMALIZATION_MODE, 
      UCOL_DECOMPOSITION_MODE = UCOL_NORMALIZATION_MODE,
      UCOL_STRENGTH,  
 #ifndef U_HIDE_DEPRECATED_API
 
      UCOL_HIRAGANA_QUATERNARY_MODE = UCOL_STRENGTH + 1,
 #endif  /* U_HIDE_DEPRECATED_API */
 
      UCOL_NUMERIC_COLLATION = UCOL_STRENGTH + 2, 
 
     /* Do not conditionalize the following with #ifndef U_HIDE_DEPRECATED_API,
      * it is needed for layout of RuleBasedCollator object. */
      UCOL_ATTRIBUTE_COUNT
 } UColAttribute;
 
 typedef enum {
   UCOL_TAILORING_ONLY, 
   UCOL_FULL_RULES 
 } UColRuleOption ;
 
 U_STABLE UCollator* U_EXPORT2 
 ucol_open(const char *loc, UErrorCode *status);
 
 U_STABLE UCollator* U_EXPORT2 
 ucol_openRules( const UChar        *rules,
                 int32_t            rulesLength,
                 UColAttributeValue normalizationMode,
                 UCollationStrength strength,
                 UParseError        *parseError,
                 UErrorCode         *status);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED UCollator* U_EXPORT2
 ucol_openFromShortString( const char *definition,
                           UBool forceDefaults,
                           UParseError *parseError,
                           UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED int32_t U_EXPORT2
 ucol_getContractions( const UCollator *coll,
                   USet *conts,
                   UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 U_STABLE void U_EXPORT2
 ucol_getContractionsAndExpansions( const UCollator *coll,
                   USet *contractions, USet *expansions,
                   UBool addPrefixes, UErrorCode *status);
 
 U_STABLE void U_EXPORT2 
 ucol_close(UCollator *coll);
 
 #if U_SHOW_CPLUSPLUS_API
 
 U_NAMESPACE_BEGIN
 
 U_DEFINE_LOCAL_OPEN_POINTER(LocalUCollatorPointer, UCollator, ucol_close);
 
 U_NAMESPACE_END
 
 #endif
 
 U_STABLE UCollationResult U_EXPORT2 
 ucol_strcoll(    const    UCollator    *coll,
         const    UChar        *source,
         int32_t            sourceLength,
         const    UChar        *target,
         int32_t            targetLength);
 
 U_STABLE UCollationResult U_EXPORT2
 ucol_strcollUTF8(
         const UCollator *coll,
         const char      *source,
         int32_t         sourceLength,
         const char      *target,
         int32_t         targetLength,
         UErrorCode      *status);
 
 U_STABLE UBool U_EXPORT2 
 ucol_greater(const UCollator *coll,
              const UChar     *source, int32_t sourceLength,
              const UChar     *target, int32_t targetLength);
 
 U_STABLE UBool U_EXPORT2 
 ucol_greaterOrEqual(const UCollator *coll,
                     const UChar     *source, int32_t sourceLength,
                     const UChar     *target, int32_t targetLength);
 
 U_STABLE UBool U_EXPORT2 
 ucol_equal(const UCollator *coll,
            const UChar     *source, int32_t sourceLength,
            const UChar     *target, int32_t targetLength);
 
 U_STABLE UCollationResult U_EXPORT2 
 ucol_strcollIter(  const    UCollator    *coll,
                   UCharIterator *sIter,
                   UCharIterator *tIter,
                   UErrorCode *status);
 
 U_STABLE UCollationStrength U_EXPORT2 
 ucol_getStrength(const UCollator *coll);
 
 U_STABLE void U_EXPORT2 
 ucol_setStrength(UCollator *coll,
                  UCollationStrength strength);
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getReorderCodes(const UCollator* coll,
                     int32_t* dest,
                     int32_t destCapacity,
                     UErrorCode *pErrorCode);
 U_STABLE void U_EXPORT2 
 ucol_setReorderCodes(UCollator* coll,
                     const int32_t* reorderCodes,
                     int32_t reorderCodesLength,
                     UErrorCode *pErrorCode);
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getEquivalentReorderCodes(int32_t reorderCode,
                     int32_t* dest,
                     int32_t destCapacity,
                     UErrorCode *pErrorCode);
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getDisplayName(    const    char        *objLoc,
             const    char        *dispLoc,
             UChar             *result,
             int32_t         resultLength,
             UErrorCode        *status);
 
 U_STABLE const char* U_EXPORT2 
 ucol_getAvailable(int32_t localeIndex);
 
 U_STABLE int32_t U_EXPORT2 
 ucol_countAvailable(void);
 
 #if !UCONFIG_NO_SERVICE
 
 U_STABLE UEnumeration* U_EXPORT2
 ucol_openAvailableLocales(UErrorCode *status);
 #endif
 
 U_STABLE UEnumeration* U_EXPORT2
 ucol_getKeywords(UErrorCode *status);
 
 U_STABLE UEnumeration* U_EXPORT2
 ucol_getKeywordValues(const char *keyword, UErrorCode *status);
 
 U_STABLE UEnumeration* U_EXPORT2
 ucol_getKeywordValuesForLocale(const char* key,
                                const char* locale,
                                UBool commonlyUsed,
                                UErrorCode* status);
 
 U_STABLE int32_t U_EXPORT2
 ucol_getFunctionalEquivalent(char* result, int32_t resultCapacity,
                              const char* keyword, const char* locale,
                              UBool* isAvailable, UErrorCode* status);
 
 U_STABLE const UChar* U_EXPORT2 
 ucol_getRules(    const    UCollator    *coll, 
         int32_t            *length);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED int32_t U_EXPORT2
 ucol_getShortDefinitionString(const UCollator *coll,
                               const char *locale,
                               char *buffer,
                               int32_t capacity,
                               UErrorCode *status);
 
 U_DEPRECATED int32_t U_EXPORT2
 ucol_normalizeShortDefinitionString(const char *source,
                                     char *destination,
                                     int32_t capacity,
                                     UParseError *parseError,
                                     UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getSortKey(const    UCollator    *coll,
         const    UChar        *source,
         int32_t        sourceLength,
         uint8_t        *result,
         int32_t        resultLength);
 
 
 U_STABLE int32_t U_EXPORT2 
 ucol_nextSortKeyPart(const UCollator *coll,
                      UCharIterator *iter,
                      uint32_t state[2],
                      uint8_t *dest, int32_t count,
                      UErrorCode *status);
 
 typedef enum {
   UCOL_BOUND_LOWER = 0,
   UCOL_BOUND_UPPER = 1,
   UCOL_BOUND_UPPER_LONG = 2,
 #ifndef U_HIDE_DEPRECATED_API
 
     UCOL_BOUND_VALUE_COUNT
 #endif  /* U_HIDE_DEPRECATED_API */
 } UColBoundMode;
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getBound(const uint8_t       *source,
         int32_t             sourceLength,
         UColBoundMode       boundType,
         uint32_t            noOfLevels,
         uint8_t             *result,
         int32_t             resultLength,
         UErrorCode          *status);
         
 U_STABLE void U_EXPORT2
 ucol_getVersion(const UCollator* coll, UVersionInfo info);
 
 U_STABLE void U_EXPORT2
 ucol_getUCAVersion(const UCollator* coll, UVersionInfo info);
 
 U_STABLE int32_t U_EXPORT2 
 ucol_mergeSortkeys(const uint8_t *src1, int32_t src1Length,
                    const uint8_t *src2, int32_t src2Length,
                    uint8_t *dest, int32_t destCapacity);
 
 U_STABLE void U_EXPORT2 
 ucol_setAttribute(UCollator *coll, UColAttribute attr, UColAttributeValue value, UErrorCode *status);
 
 U_STABLE UColAttributeValue  U_EXPORT2 
 ucol_getAttribute(const UCollator *coll, UColAttribute attr, UErrorCode *status);
 
 U_STABLE void U_EXPORT2
 ucol_setMaxVariable(UCollator *coll, UColReorderCode group, UErrorCode *pErrorCode);
 
 U_STABLE UColReorderCode U_EXPORT2
 ucol_getMaxVariable(const UCollator *coll);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED uint32_t U_EXPORT2 
 ucol_setVariableTop(UCollator *coll, 
                     const UChar *varTop, int32_t len, 
                     UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 U_STABLE uint32_t U_EXPORT2 ucol_getVariableTop(const UCollator *coll, UErrorCode *status);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED void U_EXPORT2 
 ucol_restoreVariableTop(UCollator *coll, const uint32_t varTop, UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 U_STABLE UCollator* U_EXPORT2 
 ucol_safeClone(const UCollator *coll,
                void            *stackBuffer,
                int32_t         *pBufferSize,
                UErrorCode      *status);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 #define U_COL_SAFECLONE_BUFFERSIZE 1
 
 #endif /* U_HIDE_DEPRECATED_API */
 
 U_STABLE int32_t U_EXPORT2 
 ucol_getRulesEx(const UCollator *coll, UColRuleOption delta, UChar *buffer, int32_t bufferLen);
 
 #ifndef U_HIDE_DEPRECATED_API
 
 U_DEPRECATED const char * U_EXPORT2
 ucol_getLocale(const UCollator *coll, ULocDataLocaleType type, UErrorCode *status);
 #endif  /* U_HIDE_DEPRECATED_API */
 
 U_STABLE const char * U_EXPORT2
 ucol_getLocaleByType(const UCollator *coll, ULocDataLocaleType type, UErrorCode *status);
 
 U_STABLE USet * U_EXPORT2
 ucol_getTailoredSet(const UCollator *coll, UErrorCode *status);
 
 #ifndef U_HIDE_INTERNAL_API
 
 U_INTERNAL int32_t U_EXPORT2
 ucol_getUnsafeSet( const UCollator *coll,
                   USet *unsafe,
                   UErrorCode *status);
 
 U_INTERNAL void U_EXPORT2
 ucol_prepareShortStringOpen( const char *definition,
                           UBool forceDefaults,
                           UParseError *parseError,
                           UErrorCode *status);
 #endif  /* U_HIDE_INTERNAL_API */
 
 U_STABLE int32_t U_EXPORT2
 ucol_cloneBinary(const UCollator *coll,
                  uint8_t *buffer, int32_t capacity,
                  UErrorCode *status);
 
 U_STABLE UCollator* U_EXPORT2
 ucol_openBinary(const uint8_t *bin, int32_t length, 
                 const UCollator *base, 
                 UErrorCode *status);
 
 
 #endif /* #if !UCONFIG_NO_COLLATION */
 
 #endif