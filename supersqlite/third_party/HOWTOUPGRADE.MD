There are a few components that need updating when updating the version of `sqlite3` since they are all dependent on each other:

1. **Update `apsw`:** If the SQLite3 API has changed, `apsw` may be able to provide Python bindings for those new API functions. Replace the `third_party/_apsw/` folder with the latest version. Some small changes are made to `third_party/_apsw/setup.py` all of which are commented with `# PLASTICITY`. The source code is available on [GitHub](https://github.com/rogerbinns/apsw).

2. **Update `pysqlite`:** If the SQLite3 API has changed, `pysqlite` may be able to provide Python bindings for those new API functions. Replace the `third_party/_pysqlite` folder with the latest version. Some small changes are made to `third_party/_pysqlite/setup.py` all of which are commented with `# PLASTICITY`. The packaging source code (`setup.py`, `MANIFEST.in`) can be found on [GitHub](https://github.com/ghaering/pysqlite). The core source code for `_pysqlite` is taken from `python2`'s [internal modules](https://github.com/python/cpython/tree/ca079a3ea30098aff3197c559a0e32d42dda6d84/Modules/_sqlite) and `python3`'s [internal modules](https://github.com/python/cpython/tree/master/Modules/_sqlite) and placed under `third_party/_pysqlite/src2/` and `third_party/_pysqlite/src3/` respectively.

3. **Update SQLite3:** Download the latest SQLite Amalgamation package from the [SQLite website](https://www.sqlite.org/download.html). Update `third_party/sqlite3/sqlite3.c.pre.c` (with `sqlite3.c`) and update `third_party/sqlite3/sqlite3.h`, `third_party/sqlite3/sqlite3_ext.h`, and `third_party/sqlite3/shell.c` from the SQLite Amalgamation package.

3. **Update SQLite3 Extensions:** Download the latest SQLite source code package from the [SQLite website](https://www.sqlite.org/download.html). Update the `third_party/sqlite3/ext` with the `ext` filder from the SQLite Amalgamation package.

4. **Copy `shell.c`:** Copy `shell.c` from the root of `third_party/_apsw` (which was updated in step #1) to `third_party/sqlite3/apsw_shell.c`.

6. **Update ICU:** Download the ICU source files (Unix version) from the [ICU website](http://site.icu-project.org/download). And add the `source/common` folder to the `third_party/sqlite3/icu` folder and then the `source/i8n` folder to the `third_party/sqlite3/icu` folder (merge the two directories). Also add the `source/stubdata/stubdata.cpp` file to the `third_party/sqlite3/icu` folder and the `source/data/in/icudt*l.dat` file to the `third_party/sqlite3/icu/data/` folder.

7. **Patch ICU:** Make modifications to the source files in `third_party/sqlite3/icu` as instructed by `icu_sources.py`.

8. **Update `miniz`:** The source code is available on [GitHub](https://github.com/richgel999/miniz). Rename `miniz.h` to `zlib.h` and `miniz.c` to `zlib.c` and place all of the `.c` and `.h` files in the project root under `third_party/sqlite3/`. Modify the `zlib.c`, `miniz_zip.h`, and `miniz_tdef.c` file's `#include "miniz.h"` with `#include "zlib.h"`. Also add the follwing:
```c
#if __linux
#define _LARGEFILE64_SOURCE 1
#endif
```
to the top of `zlib.h`.

9. **Update Documentation:** Update any documentation in the `README.md` with the SQLite version compiled.