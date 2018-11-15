from __future__ import print_function

import fnmatch
import hashlib
import os
import shutil
import sys
import subprocess
import traceback
import tempfile
import zipfile
import distutils.sysconfig as dsc

from glob import glob
from setuptools import find_packages
from distutils.core import setup, Extension
from setuptools.command.install import install
from setuptools.command.egg_info import egg_info
from setuptools import setup, Distribution
from multiprocessing import Process


try:
    import pip._internal.pep425tags as pep425tags
    pep425tags.get_supported()
    raise Exception()
except Exception as e:
    import pep425tags

try:
    from urllib.request import urlretrieve
except BaseException:
    from urllib import urlretrieve


PACKAGE_NAME = 'supersqlite'
PACKAGE_SHORT_NAME = 'supersqlite'
DOWNLOAD_REQ_WHEELS = []


def copy_sqlite(src, dest, apsw=False):
    """ Copy the SQLite amalgamation """
    shutil.copy(
        os.path.join(src, 'sqlite3.c'), os.path.join(dest, 'sqlite3.c.pre.c'))
    shutil.copy(
        os.path.join(src, 'sqlite3.h'), os.path.join(dest, 'sqlite3.h'))
    shutil.copy(
        os.path.join(src, 'sqlite3ext.h'), os.path.join(dest, 'sqlite3ext.h'))
    shutil.copy(
        os.path.join(src, 'shell.c'), os.path.join(dest, 'shell.c'))
    shutil.copy(
        os.path.join(src, 'icu.cpp'), os.path.join(dest, 'icu.cpp'))
    if apsw:
        shutil.copy(
            os.path.join(src, 'apsw_shell.c'), os.path.join(dest, 'shell.c'))
    SQLITE_PRE = os.path.join(dest, 'sqlite3.c.pre.c')
    SQLITE_POST = os.path.join(dest, 'sqlite3.c')
    with open(SQLITE_POST, 'w+') as outfile:
        with open(SQLITE_PRE, 'r') as infile:
            for line in infile:
                outfile.write(line)
        outfile.write('''
        # ifndef PLASTICITY_SUPERSQLITE_SQLITE3_C_SHIM
            # define PLASTICITY_SUPERSQLITE_SQLITE3_C_SHIM 1
            # ifdef sqlite3_progress_handler
              # undef sqlite3_progress_handler
            # endif
            # ifdef sqlite3_column_decltype
              # undef sqlite3_column_decltype
            # endif
            # ifdef sqlite3_enable_shared_cache
              # undef sqlite3_enable_shared_cache
            # endif\n
        ''' + '\n')
        outfile.write(
            'void sqlite3_progress_handler(sqlite3* a, int b, int (*c)(void*), void* d){ }' +
            '\n')
        outfile.write('''
        const char *sqlite3_column_decltype(sqlite3_stmt* stmt, int col) {
            int datatype = sqlite3_column_type(stmt, col);
            if (datatype == SQLITE_INTEGER) {
                return "integer";
            } else if (datatype == SQLITE_FLOAT) {
                return "float";
            } else if (datatype == SQLITE_TEXT) {
                return "text";
            } else if (datatype == SQLITE_BLOB) {
                return "blob";
            } else if (datatype == SQLITE_NULL) {
                return "null";
            } else {
                return "other";
            }
        }''' + '\n')
        outfile.write('''
        int sqlite3_enable_shared_cache(int a) {
            return SQLITE_ERROR;
        }
        ''' + '\n')
        outfile.write('#endif\n')


def get_modules(THIRD_PARTY, INTERNAL, PROJ_PATH,
                source_for_module_with_pyinit):
    """ Get all modules this package needs compiled """
    PYSQLITE2 = INTERNAL + '/pysqlite2'
    APSW = INTERNAL + '/apsw'
    PYSQLITE = THIRD_PARTY + '/_pysqlite'
    APSW_TP = THIRD_PARTY + '/_apsw'
    SQLITE3 = THIRD_PARTY + '/sqlite3'
    ICU = os.path.relpath(SQLITE3 + '/icu', PROJ_PATH)
    includes = [os.path.relpath(SQLITE3, PROJ_PATH)]
    libraries = [os.path.relpath(SQLITE3, PROJ_PATH)]
    compile_args = ["-O4", "-std=c++11"]
    link_args = ["-flto"]
    libraries.append(ICU)
    includes.append(ICU)
    link_args.append('-L' + ICU)
    SO_PREFIX = PACKAGE_NAME + '.third_party'

    SQLITE_PRE = os.path.relpath(
        os.path.join(SQLITE3, 'sqlite3.c.pre.c'), PROJ_PATH)
    SQLITE_POST = os.path.relpath(
        os.path.join(SQLITE3, 'sqlite3.c'), PROJ_PATH)
    ICU_POST = os.path.relpath(
        os.path.join(SQLITE3, 'icu.cpp'), PROJ_PATH)
    SQLITE_EXT = os.path.relpath(
        os.path.join(SQLITE3, 'ext'), PROJ_PATH)

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
    # icu_skip = ['unifiedcache.cpp', 'uresdata.cpp', 'usprep.cpp',
    #             'ucnv_u7.cpp', 'ucnv2022.cpp']
    # for root, dirnames, filenames in list(os.walk(ICU)):
    #     for filename in filenames:
    #         if filename.lower().endswith('.cpp'):
    #             source = os.path.relpath(os.path.join(root, filename), ICU)
    #             if os.path.basename(source) in icu_skip:
    #                 continue
    #             icu_sources.append(source)

    with open(ICU_POST, 'w+') as outfile:
        outfile.write(
            '''
            # ifndef PLASTICITY_SUPERSQLITE_ICU_CPP
            # define PLASTICITY_SUPERSQLITE_ICU_CPP 1

            #define UDATA_DEBUG 1
            #define U_STATIC_IMPLEMENTATION 1
            #define UCONFIG_NO_REGULAR_EXPRESSIONS 0
            #define U_DISABLE_RENAMING 1
            #define U_COMMON_IMPLEMENTATION
            #define U_COMBINED_IMPLEMENTATION

            ''' + '\n'.join(
                ['#include "' + source + '"' for source in icu_sources]
            ) + '''

            int _supersqlite_load_icu_data(void) {
                UErrorCode _PLASTICITY_SUPERSQLITE_SET_COMMON_DATA_STATUS;
                udata_setCommonData((const void*)"", &_PLASTICITY_SUPERSQLITE_SET_COMMON_DATA_STATUS);
                return (int) _PLASTICITY_SUPERSQLITE_SET_COMMON_DATA_STATUS;
            }

            # endif
        ''')
    with open(SQLITE_POST, 'w+') as outfile:
        outfile.write('#define U_DISABLE_RENAMING 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_DBPAGE_VTAB 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_DBSTAT_VTAB 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_FTS3 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_FTS3_PARENTHESIS 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_FTS4 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_FTS5 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_GEOPOLY 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_ICU 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_IOTRACE 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_JSON1 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_RBU 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_RTREE 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_SESSION 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_SNAPSHOT 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_STMTVTAB 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_STAT2 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_STAT3 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_STAT4 1' + '\n')
        outfile.write('#define SQLITE_INTROSPECTION_PRAGMAS 1' + '\n')
        outfile.write('#define SQLITE_SOUNDEX 1' + '\n')
        # outfile.write('#define SQLITE_THREADSAFE 0' + '\n')
        outfile.write('#define SQLITE_DEFAULT_MEMSTATUS 0' + '\n')
        outfile.write('#define SQLITE_DEFAULT_WAL_SYNCHRONOUS 1' + '\n')
        outfile.write('#define SQLITE_LIKE_DOESNT_MATCH_BLOBS 1' + '\n')
        outfile.write('#define SQLITE_MAX_EXPR_DEPTH 0' + '\n')
        outfile.write('#define SQLITE_OMIT_DECLTYPE 1' + '\n')
        outfile.write('#define SQLITE_OMIT_PROGRESS_CALLBACK 1' + '\n')
        outfile.write('#define SQLITE_OMIT_SHARED_CACHE 1' + '\n')
        outfile.write('#define SQLITE_USE_ALLOCA 1' + '\n')
        outfile.write('#define SQLITE_ALLOW_COVERING_INDEX_SCAN 1' + '\n')
        outfile.write('#define SQLITE_DISABLE_DIRSYNC 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_UPDATE_DELETE_LIMIT 1' + '\n')
        outfile.write('#define SQLITE_STMTJRNL_SPILL -1' + '\n')
        outfile.write('#define SQLITE_TEMP_STORE 1' + '\n')
        outfile.write('#define SQLITE_USE_URI 1' + '\n')
        outfile.write('#define SQLITE_ENABLE_EXPLAIN_COMMENTS 1' + '\n')
        outfile.write('#define SQLITE_DEFAULT_FOREIGN_KEYS 1' + '\n')
        outfile.write('#define SQLITE_MAX_LENGTH 2147483647' + '\n')
        outfile.write('#define SQLITE_MAX_COLUMN 32767' + '\n')
        outfile.write('#define SQLITE_MAX_SQL_LENGTH 2147483647' + '\n')
        outfile.write('#define SQLITE_MAX_FUNCTION_ARG 127' + '\n')
        outfile.write('#define SQLITE_MAX_COMPOUND_SELECT 65536' + '\n')
        outfile.write(
            '#define SQLITE_MAX_LIKE_PATTERN_LENGTH 2147483647' +
            '\n')
        outfile.write('#define SQLITE_MAX_VARIABLE_NUMBER 1000000000' + '\n')
        outfile.write('#define SQLITE_MAX_TRIGGER_DEPTH 2147483647' + '\n')
        outfile.write('#define SQLITE_MAX_ATTACHED 125' + '\n')
        outfile.write('#define SQLITE_MAX_PAGE_COUNT 2147483646' + '\n')
        outfile.write('\n\n\n')
        with open(SQLITE_PRE, 'r') as infile:
            for line in infile:
                outfile.write(line)
        outfile.write('''
        # ifndef PLASTICITY_SUPERSQLITE_SQLITE3_C_EXT_SHIM
            # define PLASTICITY_SUPERSQLITE_SQLITE3_C_EXT_SHIM 1
        ''' + '\n')
        outfile.write('''
        # include "ext/async/sqlite3async.c"
        # include "ext/expert/sqlite3expert.c"
        # include "ext/lsm1/lsm_ckpt.c"
        # include "ext/lsm1/lsm_file.c"
        # include "ext/lsm1/lsm_log.c"
        # include "ext/lsm1/lsm_main.c"
        # include "ext/lsm1/lsm_mem.c"
        # include "ext/lsm1/lsm_mutex.c"
        # include "ext/lsm1/lsm_shared.c"
        # include "ext/lsm1/lsm_sorted.c"
        # include "ext/lsm1/lsm_str.c"
        # include "ext/lsm1/lsm_tree.c"
        # include "ext/lsm1/lsm_unix.c"
        # include "ext/lsm1/lsm_win32.c"
        # include "ext/lsm1/lsm_varint.c"
        # include "ext/lsm1/lsm_vtab.c"
        # include "ext/userauth/userauth.c"
        # include "ext/misc/dbdump.c"
        # include "ext/misc/mmapwarm.c"
        # include "ext/misc/normalize.c"
        # include "ext/misc/scrub.c"
        # include "ext/misc/vfslog.c"
        # include "zlib.c"
        # include "miniz_tdef.c"
        # include "miniz_tinfl.c"
        # include "miniz_zip.c"
        ''' + '\n')
        outfile.write('#endif\n')

    module = 'sqlite3'
    pyinit_source = source_for_module_with_pyinit(module)
    icu_source = [os.path.relpath(os.path.join(SQLITE3, 'icu.cpp'), PROJ_PATH)]
    zlib_sources = [
        os.path.relpath(
            os.path.join(
                SQLITE3, "zlib.c"), PROJ_PATH), os.path.relpath(
            os.path.join(
                SQLITE3, "miniz_tdef.c"), PROJ_PATH), os.path.relpath(
            os.path.join(
                SQLITE3, "miniz_tinfl.c"), PROJ_PATH), os.path.relpath(
            os.path.join(
                SQLITE3, "miniz_zip.c"), PROJ_PATH)]
    sqlite3 = Extension(
        SO_PREFIX +
        'sqlite3',
        sources=[SQLITE_POST] +
        icu_source +
        [pyinit_source],
        include_dirs=includes,
        library_dirs=libraries,
        libraries=[
            "user32",
            "Advapi32"] if sys.platform == "win32" else [],
        extra_compile_args=compile_args,
        extra_link_args=link_args)

    def sqlite_extension(ext, skip=[], module=None):
        module = module or ext
        pyinit_source = source_for_module_with_pyinit(module)
        return Extension(
            SO_PREFIX + module,
            sources=([
                g for g in glob(
                    os.path.join(
                        SQLITE_EXT,
                        ext,
                        '*.c')) if os.path.basename(g) not in skip]
                     + zlib_sources + [pyinit_source]),
            include_dirs=includes,
            library_dirs=libraries,
            extra_compile_args=["-O4"],
            extra_link_args=link_args)

    def sqlite_misc_extensions(skip=[]):
        miscs = []
        for source in glob(os.path.join(SQLITE_EXT, 'misc', '*.c')):
            if os.path.basename(source) in skip:
                continue
            module = os.path.basename(source)[:-2]
            pyinit_source = source_for_module_with_pyinit(module)
            miscs.append(
                Extension(SO_PREFIX + module,
                          sources=[source] + zlib_sources + [pyinit_source],
                          include_dirs=includes,
                          library_dirs=libraries,
                          extra_compile_args=["-O4"],
                          extra_link_args=link_args))
        return miscs

    lsm1 = sqlite_extension('lsm1')
    userauth = sqlite_extension('userauth')

    return ([sqlite3, lsm1] +
            sqlite_misc_extensions())


def install_custom_sqlite3(THIRD_PARTY, INTERNAL):
    """ Begin install custom SQLite
    Can be safely ignored even if it fails, however, system SQLite
    imitations may prevent large database files with many columns
    from working."""
    if built_local():
        return

    PYSQLITE2 = INTERNAL + '/pysqlite2'
    APSW = INTERNAL + '/apsw'
    PYSQLITE = THIRD_PARTY + '/_pysqlite'
    APSW_TP = THIRD_PARTY + '/_apsw'
    SQLITE3 = THIRD_PARTY + '/sqlite3'

    print("Installing custom SQLite 3 (pysqlite) ....")
    install_env = os.environ.copy()
    install_env["PYTHONPATH"] = INTERNAL + \
        (':' + install_env["PYTHONPATH"] if "PYTHONPATH" in install_env else "")
    copy_sqlite(SQLITE3, PYSQLITE)
    copy_sqlite(SQLITE3, os.path.join(APSW_TP, 'src'), apsw=True)
    rc = subprocess.Popen([
        sys.executable,
        PYSQLITE + '/setup.py',
        'install',
        '--install-lib=' + INTERNAL,
    ], cwd=PYSQLITE, env=install_env).wait()
    if rc:
        print("")
        print("============================================================")
        print("=========================WARNING============================")
        print("============================================================")
        print("It seems like building a custom version of SQLite on your")
        print("machine has failed. This is fine, SuperSQLite will likely work")
        print("just fine with the sytem version of SQLite for most use cases.")
        print("However, if you are trying to load extremely high dimensional")
        print("models > 999 dimensions, you may run in to SQLite limitations")
        print("that can only be resolved by using the custom version of SQLite.")
        print("To troubleshoot make sure you have appropriate build tools on")
        print("your machine for building C programs like GCC and the standard")
        print("library. Also make sure you have the python-dev development")
        print("libraries and headers for building Python C extensions.")
        print("If you need more help with this, please reach out to ")
        print("opensource@plasticity.ai.")
        print("============================================================")
        print("============================================================")
        print("")
    else:
        print("")
        print("============================================================")
        print("=========================SUCCESS============================")
        print("============================================================")
        print("Building a custom version of SQLite on your machine has")
        print("succeeded.")
        print("Listing internal...")
        print(try_list_dir(INTERNAL))
        print("Listing internal/pysqlite2...")
        print(try_list_dir(PYSQLITE2))
        print("============================================================")
        print("============================================================")
        print("")
    print("Installing custom SQLite 3 (apsw) ....")
    rc = subprocess.Popen([
        sys.executable,
        APSW_TP + '/setup.py',
        'install',
        '--install-lib=' + INTERNAL,
    ], cwd=APSW_TP, env=install_env).wait()
    if rc:
        print("")
        print("============================================================")
        print("=========================WARNING============================")
        print("============================================================")
        print("It seems like building a custom version of SQLite on your")
        print("machine has failed. This is fine, SuperSQLite will likely work")
        print("just fine with the sytem version of SQLite for most use cases.")
        print("However, if you are trying to stream a remote model that")
        print("can only be resolved by using the custom version of SQLite.")
        print("To troubleshoot make sure you have appropriate build tools on")
        print("your machine for building C programs like GCC and the standard")
        print("library. Also make sure you have the python-dev development")
        print("libraries and headers for building Python C extensions.")
        print("If you need more help with this, please reach out to ")
        print("opensource@plasticity.ai.")
        print("============================================================")
        print("============================================================")
        print("")
    else:
        print("")
        print("============================================================")
        print("=========================SUCCESS============================")
        print("============================================================")
        print("Building a custom version of SQLite on your machine has")
        print("succeeded.")
        print("Listing internal...")
        print(try_list_dir(INTERNAL))
        print("Listing internal/apsw...")
        print(try_list_dir(APSW))
        print("============================================================")
        print("============================================================")
        print("")
        if not(os.path.exists(APSW)):
            print("Install-lib did not install APSW, installing from egg...")
            for egg in glob(INTERNAL + "/apsw-*.egg"):
                if (os.path.isfile(egg)):
                    print("Found an egg file, extracting...")
                    try:
                        zip_ref = zipfile.ZipFile(egg, 'r')
                    except BaseException:
                        print("Egg extraction error")
                        continue
                    zip_ref.extractall(APSW)
                else:
                    print("Found an egg folder, renaming...")
                    os.rename(egg, APSW)
                print("Renaming apsw.py to __init__.py")
                os.rename(
                    os.path.join(
                        APSW, 'apsw.py'), os.path.join(
                        APSW, '__init__.py'))


def custom_compile(THIRD_PARTY, INTERNAL):
    """ Compile resources this package needs """
    install_custom_sqlite3(THIRD_PARTY, INTERNAL)


# Redirect output to a file
tee = open(
    os.path.join(
        tempfile.gettempdir(),
        PACKAGE_SHORT_NAME +
        '.install'),
    'a+')


class TeeUnbuffered:
    def __init__(self, stream):
        self.stream = stream
        self.errors = ""

    def write(self, data):
        self.stream.write(data)
        self.stream.flush()
        tee.write(data)
        tee.flush()

    def flush(self):
        self.stream.flush()
        tee.flush()


sys.stdout = TeeUnbuffered(sys.stdout)
sys.stderr = TeeUnbuffered(sys.stderr)

# Setup path constants
PROJ_PATH = os.path.abspath(os.path.dirname(os.path.realpath(__file__)))
THIRD_PARTY = PROJ_PATH + '/' + PACKAGE_NAME + '/third_party'
BUILD_PATH = PROJ_PATH + '/build'
BUILD_THIRD_PARTY = BUILD_PATH + '/lib/' + PACKAGE_NAME + '/third_party'
INTERNAL = THIRD_PARTY + '/internal'
BINARY_EXTENSIONS = ('.so', '.pyd', '.dll', '.o', '.obj', '.lib')

# Get the package version
__version__ = None
with open(os.path.join(PROJ_PATH, 'version.py')) as f:
    exec(f.read())

# Setup remote wheel configurations
RM_WHEELHOUSE = 'https://s3.amazonaws.com/' + \
    PACKAGE_SHORT_NAME + '.plasticity.ai/wheelhouse/'
TRIED_DOWNLOADING_WHEEL = os.path.join(
    tempfile.gettempdir(),
    PACKAGE_NAME +
    '-' +
    __version__ +
    '-' +
    hashlib.md5(PROJ_PATH.encode('utf-8')).hexdigest() +
    '.whldownload'
)
INSTALLED_FROM_WHEEL = os.path.join(
    tempfile.gettempdir(),
    PACKAGE_NAME +
    '-' +
    __version__ +
    '-' +
    hashlib.md5(PROJ_PATH.encode('utf-8')).hexdigest() +
    '.whlinstall'
)
BUILT_LOCAL = os.path.join(
    tempfile.gettempdir(),
    PACKAGE_NAME +
    '-' +
    __version__ +
    '-' +
    hashlib.md5(PROJ_PATH.encode('utf-8')).hexdigest() +
    '.buildlocal'
)


def try_list_dir(d):
    """ Return a list of files in a directory """
    try:
        return os.listdir(d)
    except BaseException:
        return []


def get_supported_wheels(package_name=PACKAGE_NAME, version=__version__):
    """Get supported wheel strings"""
    def tuple_invalid(t):
        return (
            t[1] == 'none' or
            'fat32' in t[2] or
            'fat64' in t[2] or
            '_universal' in t[2]
        )
    return ['-'.join((package_name, version) + t) + '.whl'
            for t in pep425tags.get_supported() if not(tuple_invalid(t))]


def install_wheel(whl):
    """Installs a wheel file"""
    whl_args = [
        sys.executable,
        '-m',
        'pip',
        'install',
        '--ignore-installed',
    ]
    rc = subprocess.Popen(whl_args + [whl]).wait()
    if rc != 0:
        try:
            import site
            if hasattr(site, 'getusersitepackages'):
                site_packages = site.getusersitepackages()
                print("Installing to user site packages...", site_packages)
                rc = subprocess.Popen(whl_args + ["--user"] + [whl]).wait()
        except ImportError:
            pass
    return rc


def skip_wheel():
    """ Checks if a wheel install should be skipped """
    return "SKIP_" + PACKAGE_SHORT_NAME.upper() + "_WHEEL" in os.environ


def installed_wheel():
    """Checks if a pre-compiled remote wheel was installed"""
    return os.path.exists(INSTALLED_FROM_WHEEL)


def tried_downloading_wheel():
    """Checks if already tried downloading a wheel"""
    return os.path.exists(TRIED_DOWNLOADING_WHEEL)


def built_local():
    """Checks if built out the project locally"""
    return os.path.exists(BUILT_LOCAL)


def download_and_install_wheel():
    """Downloads and installs pre-compiled remote wheels"""
    if skip_wheel():
        return False
    if installed_wheel():
        return True
    if tried_downloading_wheel():
        return False
    print("Downloading and installing wheel (if it exists)...")
    tmpwhl_dir = tempfile.gettempdir()
    for whl in get_supported_wheels():
        exitcodes = []
        whl_url = RM_WHEELHOUSE + whl
        dl_path = os.path.join(tmpwhl_dir, whl)
        try:
            print("Trying...", whl_url)
            urlretrieve(whl_url, dl_path)
        except BaseException:
            print("FAILED")
            continue
        extract_dir = os.path.join(
            tempfile.gettempdir(), whl.replace(
                '.whl', ''))
        extract_dir = os.path.join(
            tempfile.gettempdir(), whl.replace(
                '.whl', ''))
        try:
            zip_ref = zipfile.ZipFile(dl_path, 'r')
        except BaseException:
            print("FAILED")
            continue
        zip_ref.extractall(extract_dir)
        for ewhl in glob(extract_dir + "/*/req_wheels/*.whl"):
            print("Installing requirement wheel: ", ewhl)
            package_name = os.path.basename(ewhl).split('-')[0]
            version = os.path.basename(ewhl).split('-')[1]
            requirement = package_name + ">=" + version
            print("Checking if requirement is met: ", requirement)
            req_rc = subprocess.Popen([
                sys.executable,
                '-c',
                "import importlib;"
                "import pkg_resources;"
                "pkg_resources.require('" + requirement + "');"
                "importlib.import_module('" + package_name + "');"
            ]).wait()
            if req_rc == 0:
                print("Requirement met...skipping install of: ", package_name)
            else:
                print("Requirement not met...installing: ", package_name)
                exitcodes.append(install_wheel(ewhl))
        print("Installing wheel: ", dl_path)
        exitcodes.append(install_wheel(dl_path))
        zip_ref.extractall(PROJ_PATH)
        zip_ref.close()
        if len(exitcodes) > 0 and max(exitcodes) == 0 and min(exitcodes) == 0:
            open(TRIED_DOWNLOADING_WHEEL, 'w+').close()
            print("Done downloading and installing wheel")
            return True
    open(TRIED_DOWNLOADING_WHEEL, 'w+').close()
    print("Done trying to download and install wheel (it didn't exist)")
    return False


def parse_requirements(filename):
    """ load requirements from a pip requirements file """
    lineiter = (line.strip() for line in open(filename))
    return [line for line in lineiter if line and not line.startswith("#")]


def build_req_wheels():
    """Builds requirement wheels"""
    if built_local():
        return
    print("Building requirements wheels...")

    # Get wheels from PyPI
    rc = subprocess.Popen([
        sys.executable,
        '-m',
        'pip',
        'wheel',
        '-r',
        'requirements.txt',
        '--wheel-dir=' + PACKAGE_NAME + '/req_wheels'
    ], cwd=PROJ_PATH).wait()

    # Download wheels
    for wheelhouse, package, versions in DOWNLOAD_REQ_WHEELS:
        req_dl_success = False
        for version in versions:
            for whl in get_supported_wheels(package, version):
                exitcodes = []
                whl_url = wheelhouse + whl
                sys.stdout.write("Trying to download... '" + whl_url + "'")
                dl_path = os.path.join(PACKAGE_NAME + '/req_wheels', whl)
                try:
                    urlretrieve(whl_url, dl_path)
                    zip_ref = zipfile.ZipFile(dl_path, 'r')
                    req_dl_success = True
                    sys.stdout.write(" ...SUCCESS\n")
                except BaseException:
                    if os.path.exists(dl_path):
                        os.remove(dl_path)
                    sys.stdout.write(" ...FAIL\n")
                    continue
                sys.stdout.flush()
        # Try to get it from PyPI as last resort
        if not req_dl_success:
            rc2 = subprocess.Popen([
                sys.executable,
                '-m',
                'pip',
                'wheel',
                package,
                '--wheel-dir=' + PACKAGE_NAME + '/req_wheels'
            ], cwd=PROJ_PATH).wait()

    if rc:
        print("Failed to build requirements wheels!")
        pass


def install_req_wheels():
    """Installs requirement wheels"""
    print("Installing requirements wheels...")
    for whl in glob(PACKAGE_NAME + '/req_wheels/*.whl'):
        rc = install_wheel(whl)
    print("Done installing requirements wheels")


def install_requirements():
    """Installs requirements.txt"""
    print("Installing requirements...")
    rc = subprocess.Popen([
        sys.executable,
        '-m',
        'pip',
        'install',
        '-r',
        'requirements.txt'
    ], cwd=PROJ_PATH).wait()
    if rc:
        print("Failed to install some requirements!")
    print("Done installing requirements")


def get_site_packages():
    """ Gets all site_packages paths """
    try:
        import site
        if hasattr(site, 'getsitepackages'):
            site_packages = site.getsitepackages()
        else:
            from distutils.sysconfig import get_python_lib
            site_packages = [get_python_lib()]
        if hasattr(site, 'getusersitepackages'):
            site_packages = site_packages + [site.getusersitepackages()]
        return site_packages
    except BaseException:
        return []


def source_for_module_with_pyinit(module):
    """ Create PyInit symbols for shared objects compiled with Python's
        Extension()"""
    source_path = os.path.join(BUILD_PATH, 'entrypoints')
    try:
        os.makedirs(source_path)
    except BaseException:
        pass
    source_file = os.path.join(source_path, module + '.c')
    with open(source_file, 'w+') as outfile:
        outfile.write('''
            void init''' + (module) + '''(void) {} //Python 2.7
            void PyInit_''' + (module) + '''(void) {} //Python 3.5
        ''')
    return os.path.relpath(source_file, PROJ_PATH)


def copy_custom_compile():
    """Copy the third party folders into site-packages under
    PACKAGE_NAME/third_party/ and
    ./build/lib/PACKAGE_NAME/third_party/
    for good measure"""

    # Copy locally installed libraries
    from distutils.dir_util import copy_tree
    try:
        site_packages = get_site_packages()
        cp_from = THIRD_PARTY + '/'
        for site_pack in site_packages:
            for globbed in glob(site_pack + '/' + PACKAGE_NAME + '*/'):
                try:
                    cp_to = (globbed + '/' + PACKAGE_NAME +
                             '/third_party/')
                except IndexError as e:
                    print(
                        "Site Package: '" +
                        site_pack +
                        "' did not have " + PACKAGE_NAME)
                    continue
                print("Copying from: ", cp_from, " --> to: ", cp_to)
                copy_tree(cp_from, cp_to)
    except Exception as e:
        print("Error copying internal pysqlite folder to site packages:")
        traceback.print_exc(e)
    try:
        cp_from = THIRD_PARTY + '/'
        cp_to = BUILD_THIRD_PARTY + '/'
        print("Copying from: ", cp_from, " --> to: ", cp_to)
        copy_tree(cp_from, cp_to)
    except Exception as e:
        print("Error copying internal pysqlite folder to build folder:")
        traceback.print_exc(e)


def delete_pip_files():
    """Delete random pip files"""
    try:
        from pip.utils.appdirs import user_cache_dir
    except BaseException:
        try:
            from pip._internal.utils.appdirs import user_cache_dir
        except BaseException:
            return
    for root, dirnames, filenames in os.walk(user_cache_dir('pip/wheels')):
        for filename in fnmatch.filter(filenames, PACKAGE_NAME + '-*.whl'):
            try:
                whl = os.path.join(root, filename)
                print("Deleting...", whl)
                os.remove(whl)
            except BaseException:
                pass
    try:
        site_packages = get_site_packages()
        for site_pack in site_packages:
            for globbed in glob(site_pack + '/' + PACKAGE_NAME + '*/'):
                try:
                    if globbed.endswith('.dist-info/'):
                        shutil.rmtree(globbed)
                except BaseException:
                    pass
    except BaseException:
        pass


cmdclass = {}

try:
    from wheel.bdist_wheel import bdist_wheel as bdist_wheel_

    class CustomBdistWheelCommand(bdist_wheel_):
        def run(self):
            if not(download_and_install_wheel()):
                custom_compile(THIRD_PARTY, INTERNAL)
                build_req_wheels()
                open(BUILT_LOCAL, 'w+').close()
            print("Running wheel...")
            bdist_wheel_.run(self)
            print("Done running wheel")
            copy_custom_compile()

    cmdclass['bdist_wheel'] = CustomBdistWheelCommand

except ImportError as e:
    pass


class CustomInstallCommand(install):
    def run(self):
        if not(download_and_install_wheel()):
            custom_compile(THIRD_PARTY, INTERNAL)
            install_req_wheels()
            open(BUILT_LOCAL, 'w+').close()
        print("Running install...")
        p = Process(target=install.run, args=(self,))
        p.start()
        p.join()
        print("Done running install")
        if not(download_and_install_wheel()):
            print("Running egg_install...")
            p = Process(target=install.do_egg_install, args=(self,))
            p.start()
            p.join()
            install_requirements()
            print("Done running egg_install")
        else:
            print("Skipping egg_install")
        copy_custom_compile()

    def finalize_options(self):
        install.finalize_options(self)
        if self.distribution.has_ext_modules():
            self.install_lib = self.install_platlib


cmdclass['install'] = CustomInstallCommand


class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True


MODULES = get_modules(THIRD_PARTY, INTERNAL, PROJ_PATH,
                      source_for_module_with_pyinit)

if __name__ == '__main__':

    # Attempt to install from a remote pre-compiled wheel
    if any([a in sys.argv for a in ['egg_info', 'install']]):
        if download_and_install_wheel():
            open(INSTALLED_FROM_WHEEL, 'w+').close()

    # Only create requirements if not installing from a wheel
    if any([a in sys.argv for a in ['bdist_wheel', 'sdist', 'egg_info']]):
        # The wheel shouldn't have any reqs
        # since it gets packaged with all of its req wheels
        reqs = []
    elif not any([a in sys.argv for a in ['-V']]):
        reqs = parse_requirements('requirements.txt')
        for wheelhouse, package, versions in DOWNLOAD_REQ_WHEELS:
            if package not in reqs:
                reqs.append(package)
        print("Adding requirements: ", reqs)
    else:
        reqs = []

    # Delete pip files
    delete_pip_files()

    setup(
        name=PACKAGE_NAME,
        packages=find_packages(
            exclude=[
                'tests',
                'tests.*']),
        version=__version__,
        description='A supercharged SQLite library for Python.',
        long_description="""
    About
    -----
    A feature-packed Python package and for utilizing SQLite in Python by `Plasticity <https://www.plasticity.ai/>`_. It is intended to be a drop-in replacement to Python's built-in `SQLite API <https://docs.python.org/3/library/sqlite3.html>`_, but without any limitations. It offers unique features like remote streaming over HTTP and bundling of extensions like JSON, R-Trees (geospatial indexing), and Full Text Search.

    Documentation
    -------------
    You can see the full documentation and README at the `GitLab repository <https://gitlab.com/Plasticity/supersqlite>`_ or the `GitHub repository <https://github.com/plasticityai/supersqlite>`_.
        """,
        author='Plasticity',
        author_email='opensource@plasticity.ai',
        url='https://gitlab.com/Plasticity/supersqlite',
        keywords=[
                    'supersqlite',
                    'sqlite',
                    'sqlite3',
                    'apsw',
                    'pysqlite',
                    'sql',
                    'embedded',
                    'database',
                    'db',
                    'http',
                    'remote',
                    'stream',
                    'full',
                    'text',
                    'fulltext',
                    'full-text',
                    'json',
                    'lsm',
                    'blob',
                    'vfs',
                    'fts4',
                    'fts5'],
        license='MIT',
        include_package_data=True,
        install_requires=reqs,
        classifiers=[
            "Development Status :: 5 - Production/Stable",
            'Intended Audience :: Developers',
            "Topic :: Software Development :: Libraries :: Python Modules",
            "Operating System :: OS Independent",
            'License :: OSI Approved :: MIT License',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.0',
            'Programming Language :: Python :: 3.7'],
        cmdclass=cmdclass,
        distclass=BinaryDistribution,
        ext_modules=MODULES
    )

    # Delete pip files
    delete_pip_files()
