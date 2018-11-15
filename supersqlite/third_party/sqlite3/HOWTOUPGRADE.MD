There are a few components that need updating when updating the `sqlite3`:

1. Replace `_apsw` with the latest version, some small changes are made to `_apsw/setup.py` all of which 
are commented with `# PLASTICITY`. The source code is available on GitHub.

2. Replace `_pysqlite` with the latest version, some small changes are made to `_pysqlite/setup.py` all of which are commented with `# PLASTICITY`. The source code for `_pysqlite` is taken from `python2`'s internal site-packages and `python3`'s internal site-packages.

3. Update `sqlite3.c.pre.c` (with `sqlite3.c`), `sqlite3.h`, `sqlite3_ext.h`, and `shell.c` from the latest SQLite Amalgamation package.

4. Add `shell.c` from the root of `_apsw` as `apsw_shell.c`.

6. Download the ICU source files (Unix). And add the `source/common` folder to the `icu` folder and then the `i8n` folder. Also add the `source/stubdata/stubdata.cpp` file to the `icu` folder and the `source/data/in/icudt*l.dat` file to the `icu/data/` folder.

7. Make modifications to the source files as instructed by `icu_sources.py`.

8. Update any documentation in the README with the SQLite version compiled.