<div align="center"><img src="https://gitlab.com/Plasticity/supersqlite/raw/master/images/SuperSQLite.png" alt="supersqlite" height="70"></div>

## <div align="center">SuperSQLite: a supercharged SQLite library for Python<br /><br />[![pipeline status](https://gitlab.com/Plasticity/supersqlite/badges/master/pipeline.svg)](https://gitlab.com/Plasticity/supersqlite/commits/master)&nbsp;&nbsp;&nbsp;[![Build Status](https://travis-ci.org/plasticityai/supersqlite.svg?branch=master)](https://travis-ci.org/plasticityai/supersqlite)&nbsp;&nbsp;&nbsp;[![Build status](https://ci.appveyor.com/api/projects/status/72lwh2g7a9ddbnt2/branch/master?svg=true)](https://ci.appveyor.com/project/plasticity-admin/supersqlite/branch/master)<br/>[![PyPI version](https://badge.fury.io/py/supersqlite.svg)](https://pypi.python.org/pypi/supersqlite/)&nbsp;&nbsp;&nbsp;[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000)](https://gitlab.com/Plasticity/supersqlite/blob/master/LICENSE.txt)&nbsp;&nbsp;&nbsp;[![Python version](https://img.shields.io/pypi/pyversions/supersqlite.svg)](https://pypi.python.org/pypi/supersqlite/)</div>
A feature-packed Python package and for utilizing SQLite in Python by [Plasticity](https://www.plasticity.ai/). It is intended to be a drop-in replacement to Python's built-in [SQLite API](https://docs.python.org/3/library/sqlite3.html), but without any limitations. It offers unique features like [remote streaming over HTTP](#remote-streaming-over-http) and [bundling of extensions like JSON, R-Trees (geospatial indexing), and Full Text Search](#extensions). SuperSQLite is also packaged with pre-compiled native binaries for SQLite and all of its extensions for nearly every platform as to avoid any C/C++ compiler errors during install.

## Table of Contents
- [Installation](#installation)
- [Motivation](#motivation)
- [Using the Library](#using-the-library)
    * [Connecting](#connecting)
    * [Querying](#querying)
- [Remote Streaming over HTTP](#remote-streaming-over-http)
- [Other Documentation](#other-documentation)
- [Other Programming Languages](#other-programming-languages)
- [Contributing](#contributing)
- [Roadmap](#roadmap)
- [Other Notable Projects](#other-notable-projects)
- [LICENSE and Attribution](#license-and-attribution)

## Installation
You can install this package with `pip`:
```python
pip install supersqlite # Python 2.7
pip3 install supersqlite # Python 3
```

## Motivation
[SQLite](http://www.sqlite.org), is a fast, popular embedded database, used by [large enterprises](https://www.sqlite.org/famous.html). It is the [most widely-deployed database](https://www.sqlite.org/mostdeployed.html) and has billions of deployments. It has a [built-in](https://docs.python.org/3/library/sqlite3.html) binding in Python.

The Python bindings, however, often are compiled against an out-of-date copy of SQLite or may be compiled with [limitations](https://www.sqlite.org/limits.html) set to low levels. Moreover, it is difficult to load extremely useful extensions like [JSON1](https://www.sqlite.org/json1.html) that adds JSON functionality to SQLite or [FTS5](https://www.sqlite.org/fts5.html) that adds full-text search functionality to SQLite since they must be compiled with a C/C++ compiler on each platform before being loaded.

SuperSQLite aims to solve these problems by packaging a newer version of SQLite natively pre-compiled for every platform along with natively pre-compiled SQLite extensions. SuperSQLite also adds useful unique new features like [remote streaming over HTTP](#remote-streaming-over-http) to read from a centralized SQLite database.

Moreover, by default, SQLite does not enable some optimizations that can result in speedups. SuperSQLite compiles SQLite with various [optimizations](#optimizations) and allows you to select your [workload at runtime](#workload-optimizations) to further automatically configure the connection to be optimized for your workload.

## When to use SuperSQLite?

SQLite is [extremely reliable and durable](https://www.sqlite.org/hirely.html) for large amounts of data ([up to 140TB](https://www.sqlite.org/limits.html)). It is considered one of the most [well-engineered and well-tested](https://www.sqlite.org/testing.html) software solutions today, with 711x more test code than implementation code. 

SQLite is [faster than nearly every other database](https://www.sqlite.org/speed.html) at read-heavy use cases (especially compared to databases that may use a client-server model with network latency like MySQL, PostgreSQL, MongoDB, DynamoDB, etc.). You can also instantiate SQLite completely in-memory to remove disk latency, if your data will fit within RAM. For key/value use cases, you can get comparable or better [read/write performance to key/value databases like LevelDB](https://sqlite.org/src4/doc/trunk/www/lsmperf.wiki) with the [LSM1 extension](#extensions).

When you have a write-heavy workload with *multiple* servers that need to write concurrently to a shared database (backend to a website), you would probably want to choose something that has a client-server model instead like PostgreSQL, although SQLite can handle processing write requests fast enough that it is sufficient for most concurrent write loads. In fact, Expensify uses [SQLite for their entire backend](https://blog.expensify.com/2018/01/08/scaling-sqlite-to-4m-qps-on-a-single-server/). If you need the database to be automatically replicated or automatically sharded across machines or other distributed features, you probably want to use something else.

See [Appropriate Uses For SQLite](https://www.sqlite.org/whentouse.html) for more information and [Well-Known Users of SQLite](https://www.sqlite.org/famous.html) for example use cases.

## Using the Library

Instead of 'import sqlite3', use:

    from supersqlite import sqlite3

This retains compatibility with the sqlite3 package, while adding the various
enhancements.

### Connecting

Given the above import, connect to a sqlite database file using:

    conn = sqlite3.connect('foo.db')

### Querying

### Remote Streaming over HTTP

### Workload Optimizations

### Extensions
#### JSON1
#### FTS3, FTS4, FTS5
#### LSM1
#### R\*Tree
#### Other
#### Custom


### Export SQLite Resources

### Optimizations

## Other Documentation
SuperSQLite extends the [apsw](https://github.com/rogerbinns/apsw) Python SQLite wrapper and adds on to its functionality. You can find the full documentation for that library [here](https://rogerbinns.github.io/apsw/), which in turn attempts to implement [PEP 249 (DB API)](https://www.python.org/dev/peps/pep-0249/). The connection object, cursor object, etc. are all [`apsw.Connection`](https://rogerbinns.github.io/apsw/connection.html), [`apsw.Cursor`](https://rogerbinns.github.io/apsw/cursor.html). Note, however, that some monkey-patching has been done to make the library more in-line and compatible as a drop-in replacement for Python's built-in `sqlite3` module.

Other documentation is not available at this time. See the source file directly (it is well commented) if you need more information about a method's arguments or want to see all supported features.

## Other Programming Languages
Currently, this library only supports Python. There are no plans to port it to any other languages, but since SQLite has a native C implementation and has bindings in most languages, you can use the [export functions](#export-sqlite-resources) to load SuperSQLite's SQLite extensions in the SQLite bindings of other programming languages or link SuperSQLite's version of SQLite to a native binary.

## Contributing
The main repository for this project can be found on [GitLab](https://gitlab.com/Plasticity/supersqlite). The [GitHub repository](https://github.com/plasticityai/supersqlite) is only a mirror. Pull requests for more tests, better error-checking, bug fixes, performance improvements, or documentation or adding additional utilties / functionalities are welcome on [GitLab](https://gitlab.com/Plasticity/supersqlite).

You can contact us at [opensource@plasticity.ai](mailto:opensource@plasticity.ai).

## Roadmap

* Out of the box, "fast-write" configuration option that makes the connection optimized for fast-writing.
* Out of the box, "fast-read" configuration option that makes the conenction optimized for
fast-reading.
* Optimize streaming cache behavior

## Other Notable Projects
* [pysqlite](https://github.com/ghaering/pysqlite) - The built-in `sqlite3` module in Python.
* [apsw](https://github.com/rogerbinns/apsw) - Powers the main API of SuperSQLite, aims to port all of SQLite's API functionality (like VFSes) to Python, not just the query APIs.
* [Magnitude](https://github.com/plasticityai/magnitude/) - Another project by Plasticity that uses SuperSQLite's unique features for machine learning embedding models.


## LICENSE and Attribution

This repository is licensed under the license found [here](LICENSE.txt).

The SQLite "feather" icon is taken from the [SQLite project](https://www.sqlite.org) which is released as [public domain](https://www.sqlite.org/copyright.html).

This project is *not* affiliated with the official SQLite project.
