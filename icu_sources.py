icu_sources = [
    'utypes.cpp',
    'uloc.cpp',
    'ustring.cpp',
    'ucase.cpp',
    'ubrk.cpp',
    'brkiter.cpp',
    'filteredbrk.cpp',
    'ucharstriebuilder.cpp',
    'uobject.cpp',
    'resbund.cpp',
    'servrbf.cpp',
    'servlkf.cpp',
    'serv.cpp',
    'servnotf.cpp',
    'servls.cpp',
    'servlk.cpp',
    'servslkf.cpp',
    'stringtriebuilder.cpp',
    'uvector.cpp',
    'ustrenum.cpp',
    'uenum.cpp',
    'unistr.cpp',
    'appendable.cpp',
    'rbbi.cpp',
    'rbbi_cache.cpp',
    'cstring.cpp',
    'umath.cpp',
    'charstr.cpp',
    'rbbidata.cpp',
    'ustrfmt.cpp',
    'ucharstrie.cpp',
    'uloc_keytype.cpp',
    'uhash.cpp',
    'locdispnames.cpp',
    'brkeng.cpp',
    'dictionarydata.cpp',
    'udataswp.cpp',
    'uinvchar.cpp',
    'uresbund.cpp',
    'uresdata.cpp',  # modified due to duplicate symbol `gEmptyString2`
    'resource.cpp',
    'locavailable.cpp',
    'utrie2.cpp',
    'ucol_swp.cpp',
    'utrie_swap.cpp',
    'schriter.cpp',
    'uchriter.cpp',
    'locid.cpp',  # modified due to duplicate include `bytesinkutil.h`
    'locbased.cpp',
    'chariter.cpp',
    'uvectr32.cpp',
    'bytestrie.cpp',
    'ustack.cpp',
    'umutex.cpp',
    'uniset.cpp',  # modified due to duplicate symbol `compareUnicodeString2`
    'stringpiece.cpp',
    'locutil.cpp',
    'unifilt.cpp',
    'util.cpp',  # modified due to duplicate symbol `BACKSLASH2`, `UPPER_U2`, and `LOWER_U2`
    'bmpset.cpp',
    'unifunct.cpp',
    'unisetspan.cpp',
    'uniset_props.cpp',  # modified due to duplicate include `_dbgct2`
    'patternprops.cpp',
    'bytesinkutil.cpp',  # modified due to duplicate include `bytesinkutil.h`
    'dictbe.cpp',
    'rbbirb.cpp',
    'utext.cpp',  # modified due to duplicate symbol `gEmptyString3`
    'utf_impl.cpp',
    'propsvec.cpp',
    'locmap.cpp',
    'loclikely.cpp',
    'uloc_tag.cpp',
    'ustrtrns.cpp',
    'udatamem.cpp',
    'putil.cpp',
    'uhash_us.cpp',
    'uprops.cpp',
    'uchar.cpp',  # modified due to duplicate symbol `_enumPropertyStartsRange2`
    'parsepos.cpp',
    'ruleiter.cpp',
    'rbbitblb.cpp',
    'edits.cpp',
    'rbbinode.cpp',
    'bytestream.cpp',
    'rbbiscan.cpp',
    'loadednormalizer2impl.cpp',
    'characterproperties.cpp',
    'locresdata.cpp',
    'normalizer2impl.cpp',  # modified due to duplicate include `bytesinkutil.h`
    'normalizer2.cpp',
    'rbbisetb.cpp',
    'rbbistbl.cpp',
    'unistr_case.cpp',
    'unames.cpp',  # modified due to duplicate symbol `DATA_TYPE2`
    'propname.cpp',
    'ustrcase.cpp',
    'ustrcase_locale.cpp',
    'ubidi.cpp',
    'ucptrie.cpp',
    'umutablecptrie.cpp',  # modified due to duplicate symbol `getRange2` and `OVERFLOW2`
    'cmemory.cpp',
    'utrie2_builder.cpp',  # modified due to duplicate symbol `writeBlock2`
    'uscript.cpp',
    'uscript_props.cpp',
    'utrie.cpp',  # modified due to duplicate symbol `equal_uint322` and `enumSameValue2`
    'ucmndata.cpp',
    'uarrsort.cpp',
    'umapfile.cpp',
    'ucln_cmn.cpp',  # modified due to duplicate include `ucln_imp.h`
    'uregex.cpp',  # modified due to duplicate symbol `BACKSLASH3`
    'ucol.cpp',
    'coll.cpp',  # modified due to duplicate symbol `gService2`, `getService2`, `initService2`, `hasService2`, `availableLocaleList2`
    'collation.cpp',
    'ucoleitr.cpp',
    'rematch.cpp',  # modified due to duplicate symbol `BACKSLASH4`
    'regexcmp.cpp',
    'repattrn.cpp',
    'collationroot.cpp',
    'ucol_res.cpp',
    'collationbuilder.cpp',
    'coleitr.cpp',
    'sharedobject.cpp',
    'collationdata.cpp',
    'uiter.cpp',
    'ucln_in.cpp',  # modified due to duplicate symbol `copyright2` and duplicate include `ucln_imp.h`
    'uniset_closure.cpp',
    'unifiedcache.cpp',  # modified due to duplicate symbol `gCacheInitOnce2`
    'regexst.cpp',
    'collationweights.cpp',
    'caniter.cpp',
    'collationiterator.cpp',
    'collationfastlatin.cpp',
    'collationtailoring.cpp',
    'usetiter.cpp',
    'collationdatareader.cpp',
    'collationruleparser.cpp',
    'collationdatabuilder.cpp',
    'regeximp.cpp',
    'collationsets.cpp',
    'utf16collationiterator.cpp',
    'uvectr64.cpp',
    'rulebasedcollator.cpp',
    'collationrootelements.cpp',
    'ucol_sit.cpp',  # modified due to duplicate symbol `internalBufferSize2`
    'ulist.cpp',
    'uset.cpp',
    'regextxt.cpp',
    'ucharstrieiterator.cpp',
    'collationfcd.cpp',
    'collationkeys.cpp',
    'unistr_case_locale.cpp',
    'collationsettings.cpp',
    'collationcompare.cpp',
    'utf8collationiterator.cpp',
    'uitercollationiterator.cpp',
    'collationfastlatinbuilder.cpp',
    'collationdatawriter.cpp',
    'uset_props.cpp',
    'utrace.cpp',
    'sortkey.cpp',
    'unistr_titlecase_brkiter.cpp',
    'ubidi_props.cpp',  # modified due to duplicate symbol `_enumPropertyStartsRange3`
    'bocsu.cpp',
    'ubidiln.cpp',
    'ubidiwrt.cpp',
    'ustr_titlecase_brkiter.cpp',
    'wintz.cpp',
    'stubdata.cpp',
    'udata.cpp',
    # modified due to to comment out `extern "C" const DataHeader U_DATA_API
    # U_ICUDATA_ENTRY_POINT;` and cast `(const DataHeader*)` due to
    # stubdata.cpp being added
]


# Other modifications:

# Modify: regexcst.h
# Replace the header gaurd with:
# #ifndef REGEXCST_H
# #define REGEXCST_H

# Modify: regexcmp.h
# Replace the header gaurd with:
# #ifndef REGEXCMP_H
# #define REGEXCMP_H

# Modify: regexcst.h
# Append '2' to every enum in Regex_PatternParseAction
# Replace all of the references to those enums in regexcst.h and regexcmp.cpp

# Modify: regexcst.h
# Replace: `gRuleParseStateTable` symbol with `gRuleParseStateTable2`
# Replace with `gRuleParseStateTable` in regexcmp.cpp
