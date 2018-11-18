#!/usr/bin/python
'''idxchk.py - pretty print indexes used in a query

Ported to Python by Tom Lynn (tom-fdyvgr@fish.cx).
Requires pysqlite2, sqlite3 (comes with Python 2.5+) or apsw.

Version 1.01 2008-03-07  Fix to list index method name thanks to Michel Lemay.
                         Added sqlite3 support.
Version 1.0  2006-07-18  Initial version.

Placed in the public domain.  I know no Tcl, corrections welcome.
'''

import sys

try:
    from pysqlite2 import dbapi2
    sqlite_connect, SQLError = dbapi2.connect, dbapi2.OperationalError
except ImportError:
    try:
        from sqlite3 import dbapi2
        sqlite_connect, SQLError = dbapi2.connect, dbapi2.OperationalError
    except ImportError:
        import apsw
        sqlite_connect, SQLError = apsw.Connection, apsw.SQLError

debug = False  # if true, displays SQL.
verbose = False
dbname = ''
sql = ''

if '-debug' in sys.argv:
    debug = True
    sys.argv.remove('-debug')

if '-v' in sys.argv:
    verbose = True
    sys.argv.remove('-v')

if len(sys.argv) <= 1:
    print 'usage: %s [-v] [-debug] dbfile [sqlcmds ...]' % sys.argv[0]
    print
    print '  -v        verbose output: opcodes, databases, tables, cursors'
    print '  -debug    show the internal SQL queries'
    print '  dbfile    a valid sqlite3 database file or ":memory:"'
    print "  sqlcmds   one or more sql statements separated by ';'"
    print
    print 'The last sqlcmd is explained, preceeding ones are executed.'
    print 'If sqlcmds is omitted, then read sqlcmds from stdin.'
    sys.exit(1)

dbname = sys.argv[1]

# if sql parm is missing, read from stdin
if len(sys.argv) > 2:
    sql = ' '.join(sys.argv[2:]) + ' \n;\n'
else:
    sql = sys.stdin.read()


# Connect to database.
session = sqlite_connect(dbname)


def DO(sql, params={}, cur=session.cursor()):
    'Run some SQL.'
    if debug:
        print '>>>', '\n...'.join(sql.split('\n'))
        if params:
            print '    %s' % params
    try:
        cur.execute(sql, params)
        rows = []
        for row in cur:       # apsw doesn't support cur.fetchall()
            rows.append(row)
        return rows
    except SQLError:
        print >>sys.stderr, "BAD SQL:", sql
        raise


# find the last sql statement, others are executed first
# eg, if temp tables are created and indexed, attach other db's, etc.

idxsql = ''
while len(idxsql) == 0:
    sqlcmds = sql.split(';')
    if sqlcmds:
        presql = sqlcmds[:-1]
        idxsql = sqlcmds[-1].strip()
        sql = ';'.join(presql)
    else:
        print 'no sqlcmds to explain'
        session.close()
        sys.exit(2)

# execute any pre sql first
cnt = 1
for s in presql:
    s = s.strip()
    if s:
        if verbose:
            print 'exec sql %d' % cnt
            print '----------------------------------------------------------'
            print s.replace('\n', ' ')[:50], '.....'
            print

        try:
            DO(s)
        except SQLError as e:
            print 'sql error while executing statement %d:' % cnt
            print s + '\n\nerror message:\n' + str(e)
            session.close()
            sys.exit(3)

        cnt += 1

try:
    vcode = DO('EXPLAIN ' + idxsql)
except SQLError as e:
    print 'sql error while explaining statement %d:' % cnt
    print idxsql + '\n\nerror message:\n' + str(e)
    session.close()
    sys.exit(4)


# get database names, in case the presql attached any other dbs or temp tables
if verbose:
    print 'dbnum  dbname'
    print '------ ---------------------------------------------------'

dbarr = {}
for dbnum, dbname, dbfile in DO('pragma database_list'):
    dbarr[dbnum] = dbname
    if verbose:
        print '%6d %s (%s)' % (dbnum, dbname, dbfile)


prevint = -1
idxtbl = {}
nesting = []
cursors = []

# collect cursors on first pass
for addr, opcode, p1, p2, p3 in vcode:
    if opcode == 'Integer':
        prevint = p1
    elif opcode == 'OpenRead':
        if prevint == -1:  # previous opcode was not Integer!
            continue

        dbnum = prevint
        if dbnum not in dbarr:
            # explained statement is probably creating a temp table
            dbarr[dbnum] = 'temp'

        if dbarr[dbnum] == 'temp':
            temp = 'temp_'
        else:
            temp = ''

        if dbarr[dbnum] != 'main' and dbarr[dbnum] != 'temp':
            dbname = dbarr[dbnum] + '.'
        else:
            dbname = ''

        if p2 == 1:  # opening sqlite_master itself, skip
            continue

        schemasql = '''SELECT type, name, tbl_name, rootpage
                       FROM %(dbname)ssqlite_%(temp)smaster
                       WHERE rootpage = %(p2)s''' % locals()

        type, name, tbl_name, rootpage = DO(schemasql)[0]

        cursors.append((p1, type, dbname + name, name, tbl_name))

    else:
        # reset int value, if preceeding opcode not Integer
        prevint = -1


if verbose:
    print
    print 'explain sql'
    print '----------------------------------------------------------'
    print idxsql
    print ''
    print 'opcodes'
    print '----------------------------------------------------------'
    for addr, opcode, p1, p2, p3 in vcode:
        print '%s|%s|%s|%s|%s' % (addr, opcode, p1, p2, p3)
    print


prevint = -1  # not present in the original Tcl - bug?

for addr, opcode, p1, p2, p3 in vcode:
    if opcode == 'Integer':
        prevint = p1
    elif opcode == 'OpenRead':
        if prevint == -1:  # previous opcode was not Integer!
            continue

        dbnum = prevint
        if dbnum not in dbarr:
            # explained statement is probably creating a temp table
            dbarr[dbnum] = 'temp'

        if dbarr[dbnum] == 'temp':
            temp = 'temp_'
        else:
            temp = ''

        if dbarr[dbnum] != 'main' and dbarr[dbnum] != 'temp':
            dbname = dbarr[dbnum] + '.'
        else:
            dbname = ''

        schemasql = '''SELECT type, name, tbl_name, rootpage
                       FROM %(dbname)ssqlite_%(temp)smaster
                       WHERE rootpage = %(p2)s''' % locals()

        type, name, tbl_name, rootpage = DO(schemasql)[0]

        idxtab = dbname + tbl_name
        #cursors.append((p1, type, dbnamename))

        if type == 'index':
            # get info for table, all indexes, and this index
            pr_tbl_info = DO('pragma table_info(%s)' % tbl_name)
            pr_idx_list = DO('pragma index_list(%s)' % tbl_name)
            pr_idx_info = DO('pragma index_info(%s)' % name)

            cols = []
            pkcollist = []

            # sort index column names and assemble index columns
            ielems = []
            for seq, cid, iname in pr_idx_info:
                ielems.append((seq, cid, iname))

            for seq, cid, iname in sorted(ielems):
                cols.append(iname)
                pkcollist.append(iname)

            cols = '(%s)' % ','.join(cols)

            # if index itself is unique
            unique = ''
            for iseq, iname, isuniq in pr_idx_list:
                if name == iname and isuniq:
                    unique = ' UNIQUE'
                    break
            cols += unique

            # index is primary key if all pkcollist names are in table pk cols
            i = -1
            # for cid, cname, ctype, ispk in pr_tbl_info:  # outdated.
            for cid, cname, ctype, notnull, dflt_value, ispk in pr_tbl_info:
                try:
                    ispk = int(ispk)
                except ValueError:
                    continue
                if ispk:
                    try:
                        i = pkcollist.index(cname)
                    except ValueError:
                        # didn't find a pk column in the list of index columns
                        break
                    # remove this column name from pkcollist
                    del pkcollist[i]

            if i >= 0 and not pkcollist:
                # found all of the table pk columns in the pkcollist
                cols += ' PRIMARY KEY'

            idxtbl[idxtab] = idxtbl.get(idxtab, [])
            idxtbl[idxtab].append((name, cols))

        elif type == 'table':

            # if not in idxtbl array, add it with empty index info
            if idxtab not in idxtbl:
                idxtbl[idxtab] = []

            if idxtab not in nesting:
                nesting.append(idxtab)

    elif opcode == 'NotExists' or opcode == 'MoveGe' or opcode == 'MoveLt':

        # check for possible primary key usage
        for cp1, ctype, ctab, cname, ctbl in cursors:
            if p1 == cp1 and ctype == 'table':
                idxtbl[ctab].append(('<pk>', '<integer primary key or rowid>'))
                break

    else:
        # reset int value, if preceeding opcode not Integer
        prevint = -1


if verbose:
    print 'table open order (probable join table nesting)'
    print '-----------------------------------------------------------'
    lev = 0
    for tab in nesting:
        print '|   ' * lev + tab
        lev += 1

    if lev > 1:
        print '|   ' * lev

    print
    print 'cursor type   name'
    print '------ ------ ----------------------------------------------'
    for cur in cursors:
        num, type, fullname, name, tbl = cur
        print '%6d %-6.6s %s' % (num, type, fullname)
    print


# remove any duplicate indexes per each table
for tbl, idxlist in idxtbl.items():
    idxtbl[tbl] = sorted(list(set(idxlist)))

# pretty print in column format
# first, figure out column widths
len1 = 6
len2 = 10
len3 = 10
for tbl, idxlist in idxtbl.items():
    if len(tbl) > len1:
        len1 = len(tbl)

    for idx, idxdef in idxlist:
        if len(idx) > len2:
            len2 = len(idx)

        if len(idxdef) > len3:
            len3 = len(idxdef)

fmt = '%-{len1}.{len1}s %-{len2}.{len2}s %-{len3}.{len3}s'

# Substitute in for each "{lenX}" in fmt:
fmt = fmt.replace('%', '%%').replace('{', '%(').replace('}', ')s') % locals()

print fmt % ('table', 'index(es)', 'column(s)')
print fmt % ('-' * len1, '-' * len2, '-' * len3)

# now print in order of table open nesting
for tbl in nesting:
    t = tbl
    idxlist = idxtbl[tbl]
    if not idxlist:
        print fmt % (tbl, '(none)', '')
    else:
        for ientry in idxlist:
            idx, idxdef = ientry
            print fmt % (tbl, idx, idxdef)
            #tbl = ''

    try:
        del idxtbl[t]
    except KeyError:
        pass

# print any other indexes where index was opened, but not table
for tbl in idxtbl:
    idxlist = idxtbl[tbl]
    if not idxlist:
        print fmt % (tbl, '(none)', '')
    else:
        for ientry in idxlist:
            idx, idxdef = ientry
            print fmt % (tbl, idx, idxdef)
            #tbl = ''

print '\nSQLite version:', DO('SELECT sqlite_version()')[0][0]

session.close()
