/*

 gg_sqlaux.c -- SQL ancillary functions

 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

 -----------------------------------------------------------------------------
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2008-2015
the Initial Developer. All Rights Reserved.

Contributor(s):

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.
 
*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/gaiaaux.h>
#include <spatialite_private.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

/* 64 bit integer: portable format for printf() */
#if defined(_WIN32) && !defined(__MINGW32__)
#define FRMT64 "%I64d"
#else
#define FRMT64 "%lld"
#endif

GAIAAUX_DECLARE int
gaiaIllegalSqlName (const char *name)
{
/* checks if column-name is an SQL illegal name */
    int i;
    int len;
    if (!name)
	return 1;
    len = strlen (name);
    if (len == 0)
	return 1;
    for (i = 0; i < len; i++)
      {
	  if (name[i] >= 'a' && name[i] <= 'z')
	      continue;
	  if (name[i] >= 'A' && name[i] <= 'Z')
	      continue;
	  if (name[i] >= '0' && name[i] <= '9')
	      continue;
	  if (name[i] == '_')
	      continue;
	  /* the name contains an illegal char */
	  return 1;
      }
    if (name[0] >= 'a' && name[0] <= 'z')
	return 0;
    if (name[0] >= 'A' && name[0] <= 'Z')
	return 0;
/* the first char in the name isn't a letter */
    return 1;
}

GAIAAUX_DECLARE int
gaiaIsReservedSqliteName (const char *name)
{
/* checks if column-name is an SQLite reserved keyword */
    char *reserved[] = {
	"ALL",
	"ALTER",
	"AND",
	"AS",
	"AUTOINCREMENT",
	"BETWEEN",
	"BLOB",
	"BY",
	"CASE",
	"CHECK",
	"COLLATE",
	"COMMIT",
	"CONSTRAINT",
	"CREATE",
	"CROSS",
	"DATE",
	"DATETIME",
	"DEFAULT",
	"DEFERRABLE",
	"DELETE",
	"DISTINCT",
	"DOUBLE",
	"DROP",
	"ELSE",
	"ESCAPE",
	"EXCEPT",
	"FOREIGN",
	"FROM",
	"FULL",
	"GLOB",
	"GROUP",
	"HAVING",
	"IN",
	"INDEX",
	"INNER",
	"INSERT",
	"INTEGER",
	"INTERSECT",
	"INTO",
	"IS",
	"ISNULL",
	"JOIN",
	"KEY",
	"LEFT",
	"LIKE",
	"LIMIT",
	"MATCH",
	"NATURAL",
	"NOT",
	"NOTNULL",
	"NULL",
	"ON",
	"OR",
	"ORDER",
	"OUTER",
	"PRAGMA",
	"PRIMARY",
	"REFERENCES",
	"REPLACE",
	"RIGHT",
	"ROLLBACK",
	"SELECT",
	"SET",
	"TABLE",
	"TEMP",
	"TEMPORARY",
	"THEN",
	"TEXT",
	"TIMESTAMP",
	"TO",
	"TRANSACTION",
	"UNION",
	"UNIQUE",
	"UPDATE",
	"USING",
	"VALUES",
	"VIEW",
	"WHEN",
	"WHERE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

GAIAAUX_DECLARE int
gaiaIsReservedSqlName (const char *name)
{
/* checks if column-name is an SQL reserved keyword */
    char *reserved[] = {
	"ABSOLUTE",
	"ACTION",
	"ADD",
	"AFTER",
	"ALL",
	"ALLOCATE",
	"ALTER",
	"AND",
	"ANY",
	"ARE",
	"ARRAY",
	"AS",
	"ASC",
	"ASENSITIVE",
	"ASSERTION",
	"ASYMMETRIC",
	"AT",
	"ATOMIC",
	"AUTHORIZATION",
	"AVG",
	"BEFORE",
	"BEGIN",
	"BETWEEN",
	"BIGINT",
	"BINARY",
	"BIT",
	"BIT_LENGTH",
	"BLOB",
	"BOOLEAN",
	"BOTH",
	"BREADTH",
	"BY",
	"CALL",
	"CALLED",
	"CASCADE",
	"CASCADED",
	"CASE",
	"CAST",
	"CATALOG",
	"CHAR",
	"CHARACTER",
	"CHARACTER_LENGTH",
	"CHAR_LENGTH",
	"CHECK",
	"CLOB",
	"CLOSE",
	"COALESCE",
	"COLLATE",
	"COLLATION",
	"COLUMN",
	"COMMIT",
	"CONDITION",
	"CONNECT",
	"CONNECTION",
	"CONSTRAINT",
	"CONSTRAINTS",
	"CONSTRUCTOR",
	"CONTAINS",
	"CONTINUE",
	"CONVERT",
	"CORRESPONDING",
	"COUNT",
	"CREATE",
	"CROSS",
	"CUBE",
	"CURRENT",
	"CURRENT_DATE",
	"CURRENT_DEFAULT_TRANSFORM_GROUP",
	"CURRENT_PATH",
	"CURRENT_ROLE",
	"CURRENT_TIME",
	"CURRENT_TIMESTAMP",
	"CURRENT_TRANSFORM_GROUP_FOR_TYPE",
	"CURRENT_USER",
	"CURSOR",
	"CYCLE",
	"DATA",
	"DATE",
	"DAY",
	"DEALLOCATE",
	"DEC",
	"DECIMAL",
	"DECLARE",
	"DEFAULT",
	"DEFERRABLE",
	"DEFERRED",
	"DELETE",
	"DEPTH",
	"DEREF",
	"DESC",
	"DESCRIBE",
	"DESCRIPTOR",
	"DETERMINISTIC",
	"DIAGNOSTICS",
	"DISCONNECT",
	"DISTINCT",
	"DO",
	"DOMAIN",
	"DOUBLE",
	"DROP",
	"DYNAMIC",
	"EACH",
	"ELEMENT",
	"ELSE",
	"ELSEIF",
	"END",
	"EQUALS",
	"ESCAPE",
	"EXCEPT",
	"EXCEPTION",
	"EXEC",
	"EXECUTE",
	"EXISTS",
	"EXIT",
	"external",
	"EXTRACT",
	"FALSE",
	"FETCH",
	"FILTER",
	"FIRST",
	"FLOAT",
	"FOR",
	"FOREIGN",
	"FOUND",
	"FREE",
	"FROM",
	"FULL",
	"FUNCTION",
	"GENERAL",
	"GET",
	"GLOBAL",
	"GO",
	"GOTO",
	"GRANT",
	"GROUP",
	"GROUPING",
	"HANDLER",
	"HAVING",
	"HOLD",
	"HOUR",
	"IDENTITY",
	"IF",
	"IMMEDIATE",
	"IN",
	"INDICATOR",
	"INITIALLY",
	"INNER",
	"INOUT",
	"INPUT",
	"INSENSITIVE",
	"INSERT",
	"INT",
	"INTEGER",
	"INTERSECT",
	"INTERVAL",
	"INTO",
	"IS",
	"ISOLATION",
	"ITERATE",
	"JOIN",
	"KEY",
	"LANGUAGE",
	"LARGE",
	"LAST",
	"LATERAL",
	"LEADING",
	"LEAVE",
	"LEFT",
	"LEVEL",
	"LIKE",
	"LOCAL",
	"LOCALTIME",
	"LOCALTIMESTAMP",
	"LOCATOR",
	"LOOP",
	"LOWER",
	"MAP",
	"MATCH",
	"MAX",
	"MEMBER",
	"MERGE",
	"METHOD",
	"MIN",
	"MINUTE",
	"MODIFIES",
	"MODULE",
	"MONTH",
	"MULTISET",
	"NAMES",
	"NATIONAL",
	"NATURAL",
	"NCHAR",
	"NCLOB",
	"NEW",
	"NEXT",
	"NO",
	"NONE",
	"NOT",
	"NULL",
	"NULLIF",
	"NUMERIC",
	"OBJECT",
	"OCTET_LENGTH",
	"OF",
	"OLD",
	"ON",
	"ONLY",
	"OPEN",
	"OPTION",
	"OR",
	"ORDER",
	"ORDINALITY",
	"OUT",
	"OUTER",
	"OUTPUT",
	"OVER",
	"OVERLAPS",
	"PAD",
	"PARAMETER",
	"PARTIAL",
	"PARTITION",
	"PATH",
	"POSITION",
	"PRECISION",
	"PREPARE",
	"PRESERVE",
	"PRIMARY",
	"PRIOR",
	"PRIVILEGES",
	"PROCEDURE",
	"PUBLIC",
	"RANGE",
	"READ",
	"READS",
	"REAL",
	"RECURSIVE",
	"REF",
	"REFERENCES",
	"REFERENCING",
	"RELATIVE",
	"RELEASE",
	"REPEAT",
	"RESIGNAL",
	"RESTRICT",
	"RESULT",
	"RETURN",
	"RETURNS",
	"REVOKE",
	"RIGHT",
	"ROLE",
	"ROLLBACK",
	"ROLLUP",
	"ROUTINE",
	"ROW",
	"ROWS",
	"SAVEPOINT",
	"SCHEMA",
	"SCOPE",
	"SCROLL",
	"SEARCH",
	"SECOND",
	"SECTION",
	"SELECT",
	"SENSITIVE",
	"SESSION",
	"SESSION_USER",
	"SET",
	"SETS",
	"SIGNAL",
	"SIMILAR",
	"SIZE",
	"SMALLINT",
	"SOME",
	"SPACE",
	"SPECIFIC",
	"SPECIFICTYPE",
	"SQL",
	"SQLCODE",
	"SQLERROR",
	"SQLEXCEPTION",
	"SQLSTATE",
	"SQLWARNING",
	"START",
	"STATE",
	"STATIC",
	"SUBMULTISET",
	"SUBSTRING",
	"SUM",
	"SYMMETRIC",
	"SYSTEM",
	"SYSTEM_USER",
	"TABLE",
	"TABLESAMPLE",
	"TEMPORARY",
	"THEN",
	"TIME",
	"TIMESTAMP",
	"TIMEZONE_HOUR",
	"TIMEZONE_MINUTE",
	"TO",
	"TRAILING",
	"TRANSACTION",
	"TRANSLATE",
	"TRANSLATION",
	"TREAT",
	"TRIGGER",
	"TRIM",
	"TRUE",
	"UNDER",
	"UNDO",
	"UNION",
	"UNIQUE",
	"UNKNOWN",
	"UNNEST",
	"UNTIL",
	"UPDATE",
	"UPPER",
	"USAGE",
	"USER",
	"USING",
	"VALUE",
	"VALUES",
	"VARCHAR",
	"VARYING",
	"VIEW",
	"WHEN",
	"WHENEVER",
	"WHERE",
	"WHILE",
	"WINDOW",
	"WITH",
	"WITHIN",
	"WITHOUT",
	"WORK",
	"WRITE",
	"YEAR",
	"ZONE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

GAIAAUX_DECLARE char *
gaiaDequotedSql (const char *value)
{
/*
/ returns a well formatted TEXT value from SQL
/ 1] if the input string begins and ends with ' sigle quote will be the target
/ 2] if the input string begins and ends with " double quote will be the target
/ 3] in any othet case the string will simply be copied
*/
    const char *pi = value;
    const char *start;
    const char *end;
    char *clean;
    char *po;
    int len;
    char target;
    int mark = 0;
    if (value == NULL)
	return NULL;

    len = strlen (value);
    clean = malloc (len + 1);
    if (*(value + 0) == '"' && *(value + len - 1) == '"')
	target = '"';
    else if (*(value + 0) == '\'' && *(value + len - 1) == '\'')
	target = '\'';
    else
      {
	  /* no dequoting; simply copying */
	  strcpy (clean, value);
	  return clean;
      }
    start = value;
    end = value + len - 1;
    po = clean;
    while (*pi != '\0')
      {
	  if (mark)
	    {
		if (*pi == target)
		  {
		      *po++ = *pi++;
		      mark = 0;
		      continue;
		  }
		else
		  {
		      /* error: mismatching quote */
		      free (clean);
		      return NULL;
		  }
	    }
	  if (*pi == target)
	    {
		if (pi == start || pi == end)
		  {
		      /* first or last char */
		      pi++;
		      continue;
		  }
		/* found a quote marker */
		mark = 1;
		pi++;
		continue;
	    }
	  *po++ = *pi++;
      }
    *po = '\0';
    return clean;
}

GAIAAUX_DECLARE char *
gaiaQuotedSql (const char *value, int quote)
{
/*
/ returns a well formatted TEXT value for SQL
/ 1] strips trailing spaces
/ 2] masks any QUOTE inside the string, appending another QUOTE
/ 3] works for both SINGLE- and DOUBLE-QUOTE
*/
    const char *p_in;
    const char *p_end;
    char qt;
    char *out;
    char *p_out;
    int len = 0;
    int i;

    if (!value)
	return NULL;
    if (quote == GAIA_SQL_SINGLE_QUOTE)
	qt = '\'';
    else if (quote == GAIA_SQL_DOUBLE_QUOTE)
	qt = '"';
    else
	return NULL;

    p_end = value;
    for (i = (strlen (value) - 1); i >= 0; i--)
      {
	  /* stripping trailing spaces */
	  p_end = value + i;
	  if (value[i] != ' ')
	      break;
      }

    p_in = value;
    while (p_in <= p_end)
      {
	  /* computing the output length */
	  len++;
	  if (*p_in == qt)
	      len++;
	  p_in++;
      }
    if (len == 1 && *value == ' ')
      {
	  /* empty string */
	  len = 0;
      }

    out = malloc (len + 1);
    if (!out)
	return NULL;

    if (len == 0)
      {
	  /* empty string */
	  *out = '\0';
	  return out;
      }

    p_out = out;
    p_in = value;
    while (p_in <= p_end)
      {
	  /* creating the output string */
	  if (*p_in == qt)
	      *p_out++ = qt;
	  *p_out++ = *p_in++;
      }
    *p_out = '\0';
    return out;
}

GAIAAUX_DECLARE char *
gaiaSingleQuotedSql (const char *value)
{
/* convenience method supporting SINGLE-QUOTES */
    return gaiaQuotedSql (value, GAIA_SQL_SINGLE_QUOTE);
}

GAIAAUX_DECLARE char *
gaiaDoubleQuotedSql (const char *value)
{
/* convenience method supporting DOUBLE-QUOTES */
    return gaiaQuotedSql (value, GAIA_SQL_DOUBLE_QUOTE);
}

GAIAAUX_DECLARE void
gaiaCleanSqlString (char *value)
{
/*
/ returns a well formatted TEXT value for SQL
/ 1] strips trailing spaces
/ 2] masks any ' inside the string, appending another '
*/
    char new_value[1024];
    char *p;
    int len;
    int i;
    len = strlen (value);
    for (i = (len - 1); i >= 0; i--)
      {
	  /* stripping trailing spaces */
	  if (value[i] == ' ')
	      value[i] = '\0';
	  else
	      break;
      }
    p = new_value;
    for (i = 0; i < len; i++)
      {
	  if (value[i] == '\'')
	      *(p++) = '\'';
	  *(p++) = value[i];
      }
    *p = '\0';
    strcpy (value, new_value);
}

GAIAAUX_DECLARE void
gaiaInsertIntoSqlLog (sqlite3 * sqlite, const char *user_agent,
		      const char *utf8Sql, sqlite3_int64 * sqllog_pk)
{
/* inserting an event into the SQL Log */
    char *sql_statement;
    int ret;

    *sqllog_pk = -1;
    if (checkSpatialMetaData (sqlite) != 3)
      {
/* CURRENT db-schema (>= 4.0.0) required */
	  return;
      }

    sql_statement = sqlite3_mprintf ("INSERT INTO sql_statements_log "
				     "(id, time_start, user_agent, sql_statement) VALUES ("
				     "NULL, strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), %Q, %Q)",
				     user_agent, utf8Sql);
    ret = sqlite3_exec (sqlite, sql_statement, NULL, 0, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return;
    *sqllog_pk = sqlite3_last_insert_rowid (sqlite);
}

GAIAAUX_DECLARE void
gaiaUpdateSqlLog (sqlite3 * sqlite, sqlite3_int64 sqllog_pk, int success,
		  const char *errMsg)
{
/* completing an event already inserted into the SQL Log */
    char *sql_statement;
    char dummy[64];

    if (checkSpatialMetaData (sqlite) != 3)
      {
/* CURRENT db-schema (>= 4.0.0) required */
	  return;
      }
    sprintf (dummy, FRMT64, sqllog_pk);
    if (success)
      {
	  sql_statement = sqlite3_mprintf ("UPDATE sql_statements_log SET "
					   "time_end = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
					   "success = 1, error_cause = 'success' WHERE id = %s",
					   dummy);
      }
    else
      {
	  sql_statement = sqlite3_mprintf ("UPDATE sql_statements_log SET "
					   "time_end = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
					   "success = 0, error_cause = %Q WHERE id = %s",
					   (errMsg == NULL)
					   ? "UNKNOWN" : errMsg, dummy);
      }
    sqlite3_exec (sqlite, sql_statement, NULL, 0, NULL);
    sqlite3_free (sql_statement);
}

static void
consume_blank (const char *p_start, const char **p_end)
{
/* consuming blanks */
    const char *p = p_start;
    while (1)
      {
	  if (*p == ' ' || *p == '\t')
	    {
		p++;
		continue;
	    }
	  else
	    {
		*p_end = p;
		return;
	    }
      }
}

static int
check_deg_delimiter (const char *p_start, const char **p_end)
{
/* testing a "degrees" delimiter/qualifier */
    unsigned char ctrl1;
    unsigned char ctrl2;
    if (*p_start == 'd')
      {
	  *p_end = p_start + 1;
	  return 1;
      }
    ctrl1 = *(p_start + 0);
    ctrl2 = *(p_start + 1);
    if (ctrl1 == 0xc2 && ctrl2 == 0xb0)
      {
	  *p_end = p_start + 2;
	  return 1;
      }
    return 0;
}

static int
check_min_delimiter (const char *p_start, const char **p_end)
{
/* testing a "minutes" delimiter/qualifier */
    unsigned char ctrl1;
    unsigned char ctrl2;
    unsigned char ctrl3;
    if (*p_start == '\'')
      {
	  *p_end = p_start + 1;
	  return 1;
      }
    ctrl1 = *(p_start + 0);
    ctrl2 = *(p_start + 1);
    ctrl3 = *(p_start + 2);
    if (ctrl1 == 0xe2 && ctrl2 == 0x80 && ctrl3 == 0xb2)
      {
	  *p_end = p_start + 3;
	  return 1;
      }
    return 0;
}

static int
check_sec_delimiter (const char *p_start, const char **p_end)
{
/* testing a "seconds" delimiter/qualifier */
    unsigned char ctrl1;
    unsigned char ctrl2;
    unsigned char ctrl3;
    if (*p_start == '"')
      {
	  *p_end = p_start + 1;
	  return 1;
      }
    ctrl1 = *(p_start + 0);
    ctrl2 = *(p_start + 1);
    ctrl3 = *(p_start + 2);
    if (ctrl1 == 0xe2 && ctrl2 == 0x80 && ctrl3 == 0xb3)
      {
	  *p_end = p_start + 3;
	  return 1;
      }
    return 0;
}

static void
consume_int (const char *p_start, const char **p_end, int *value)
{
/* consuming an integer value */
    char *buf;
    int len = 0;
    const char *p = p_start;
    while (1)
      {
	  if (*p >= '0' && *p <= '9')
	    {
		len++;
		p++;
		continue;
	    }
	  else
	    {
		*p_end = p;
		break;
	    }
      }
    if (len == 0)
      {
	  *value = 181;
	  return;
      }
    buf = malloc (len + 1);
    memcpy (buf, p_start, len);
    *(buf + len) = '\0';
    *value = atoi (buf);
    free (buf);
}

static void
consume_float (const char *p_start, const char **p_end, double *value)
{
/* consuming a double value */
    char *buf;
    int pt = 0;
    int len = 0;
    const char *p = p_start;
    while (1)
      {
	  if (*p >= '0' && *p <= '9')
	    {
		len++;
		p++;
		continue;
	    }
	  else if (*p == '.' || *p == ',')
	    {
		len++;
		pt++;
		p++;
		continue;
	    }
	  else
	    {
		*p_end = p;
		break;
	    }
      }
    if (len == 0 || pt > 1)
      {
	  *value = 61.0;
	  return;
      }
    buf = malloc (len + 1);
    memcpy (buf, p_start, len);
    *(buf + len) = '\0';
    *value = atof (buf);
    free (buf);
}

GAIAAUX_DECLARE int
gaiaParseDMS (const char *dms, double *longitude, double *latitude)
{
/* attempting to parse a DMS string */
    double lg;
    double lt;
    int lat_d;
    int lat_m;
    double lat_s;
    char lat_prefix = '\0';
    int long_d;
    int long_m;
    double long_s;
    char long_prefix = '\0';
    const char *p = dms;
    const char *p_end;
    if (dms == NULL)
	return 0;

/* attempting to parse the latitude */
    consume_blank (p, &p_end);
    p = p_end;
    if (*p == 'S' || *p == 'N')
      {
	  lat_prefix = *p;
	  p++;
	  consume_blank (p, &p_end);
	  p = p_end;
      }
    if (*p >= '0' && *p <= '9')
      {
	  consume_int (p, &p_end, &lat_d);
	  if (lat_d < 0 && lat_d > 90)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_deg_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (*p >= '0' && *p <= '9')
      {
	  consume_int (p, &p_end, &lat_m);
	  if (lat_m < 0 && lat_m >= 60)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_min_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (*p >= '0' && *p <= '9')
      {
	  consume_float (p, &p_end, &lat_s);
	  if (lat_s < 0.0 && lat_s >= 60.0)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_sec_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (lat_prefix == '\0')
      {
	  /* attempting to retrieve the prefix */
	  if (*p == 'S' || *p == 'N')
	    {
		lat_prefix = *p;
		p++;
	    }
	  else
	      return 0;
      }
    lt = (double) lat_d + ((double) lat_m / 60.0) + (lat_s / 3600.0);
    if (lat_prefix == 'S')
	lt *= -1.0;
    if (lt < -90.0 || lt > 90.0)
	return 0;

/* attempting to parse the longitude */
    consume_blank (p, &p_end);
    p = p_end;
    if (*p == 'E' || *p == 'W')
      {
	  long_prefix = *p;
	  p++;
	  consume_blank (p, &p_end);
	  p = p_end;
      }
    if (*p >= '0' && *p <= '9')
      {
	  consume_int (p, &p_end, &long_d);
	  if (long_d < 0 && long_d > 90)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_deg_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (*p >= '0' && *p <= '9')
      {
	  consume_int (p, &p_end, &long_m);
	  if (long_m < 0 && long_m >= 60)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_min_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (*p >= '0' && *p <= '9')
      {
	  consume_float (p, &p_end, &long_s);
	  if (long_s < 0.0 && long_s >= 60.0)
	      return 0;
	  p = p_end;
      }
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (check_sec_delimiter (p, &p_end))
	p = p_end;
    else
	return 0;
    consume_blank (p, &p_end);
    p = p_end;
    if (long_prefix == '\0')
      {
	  /* attempting to retrieve the prefix */
	  if (*p == 'E' || *p == 'W')
	    {
		long_prefix = *p;
		p++;
	    }
	  else
	      return 0;
      }
    lg = (double) long_d + ((double) long_m / 60.0) + (long_s / 3600.0);
    if (long_prefix == 'W')
	lg *= -1.0;
    if (lg < -180.0 || lg > 180.0)
	return 0;

    *longitude = lg;
    *latitude = lt;
    return 1;
}

GAIAAUX_DECLARE char *
gaiaConvertToDMS (double longitude, double latitude)
{
/* formatting a DMS string */
    char *dms0;
    char *dms;
    char long_prefix = 'E';
    char lat_prefix = 'N';
    int long_d;
    int long_m;
    int long_s;
    int lat_d;
    int lat_m;
    int lat_s;
    double val;
    int len;
    if (longitude < -180.0 || longitude > 180.0)
	return NULL;
    if (latitude < -90.0 || latitude > 90.0)
	return NULL;
    if (longitude < 0.0)
      {
	  long_prefix = 'W';
	  longitude *= -1.0;
      }
    if (latitude < 0.0)
      {
	  lat_prefix = 'S';
	  latitude *= -1.0;
      }
    long_d = (int) floor (longitude);
    val = 60.0 * (longitude - (double) long_d);
    long_m = (int) floor (val);
    val = 60.0 * (val - (double) long_m);
    long_s = (int) floor (val);
    if ((val - (double) long_s) > 0.5)
	long_s++;
    lat_d = (int) floor (latitude);
    val = 60.0 * (latitude - (double) lat_d);
    lat_m = (int) floor (val);
    val = 60.0 * (val - (double) lat_m);
    lat_s = (int) floor (val);
    if ((val - (double) lat_s) > 0.5)
	lat_s++;
    dms0 =
	sqlite3_mprintf ("%02d°%02d′%02d″%c %03d°%02d′%02d″%c", lat_d,
			 lat_m, lat_s, lat_prefix, long_d, long_m, long_s,
			 long_prefix);
    len = strlen (dms0);
    dms = malloc (len + 1);
    strcpy (dms, dms0);
    sqlite3_free (dms0);
    return dms;
}

/*********************************************************************
/
/ DISCLAIMER
/
/ the following code implementation (URL percent-encoding/-decoding)
/ simply is a rearranged adaption of this original code released
/ into the Public Domain:
/
/ http://www.geekhideout.com/urlcode.shtml
/
*********************************************************************/

static char
url_to_hex (char code)
{
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

GAIAAUX_DECLARE char *
gaiaEncodeURL (const char *url)
{
/* encoding some URL */
    char *encoded = NULL;
    const char *in = url;
    char *out;
    int len;
    if (url == NULL)
	return NULL;
    len = strlen (url);
    if (len == 0)
	return NULL;

    encoded = malloc ((len * 3) + 1);
    out = encoded;
    while (*in != '\0')
      {
	  if (isalnum (*in) || *in == '-' || *in == '_' || *in == '.'
	      || *in == '~')
	      *out++ = *in;
	  else if (*in == ' ')
	      *out++ = '+';
	  else
	    {
		*out++ = '%';
		*out++ = url_to_hex (*in >> 4);
		*out++ = url_to_hex (*in & 15);
	    }
	  in++;
      }
    *out = '\0';
    return encoded;
}

static char
url_from_hex (char ch)
{
    return isdigit (ch) ? ch - '0' : tolower (ch) - 'a' + 10;
}

GAIAAUX_DECLARE char *
gaiaDecodeURL (const char *encoded)
{
/* decoding some URL */
    char *url = NULL;
    const char *in = encoded;
    char *out;
    int len;
    if (encoded == NULL)
	return NULL;
    len = strlen (encoded);
    if (len == 0)
	return NULL;

    url = malloc (len + 1);
    out = url;
    while (*in != '\0')
      {
	  if (*in == '%')
	    {
		if (*(in + 1) && *(in + 2))
		  {
		      *out++ =
			  url_from_hex (*(in + 1)) << 4 |
			  url_from_hex (*(in + 2));
		      in += 2;
		  }
	    }
	  else if (*in == '+')
	      *out++ = ' ';
	  else
	      *out++ = *in;
	  in++;
      }
    *out = '\0';
    return url;
}

GAIAAUX_DECLARE char *
gaiaDirNameFromPath (const char *path)
{
/* extracting the DirName (if any) from a Path */
    const char *in = path;
    const char *last = NULL;
    int len = 0;
    int dirlen;
    char *name;

    if (path == NULL)
	return NULL;

    while (*in != '\0')
      {
	  /* parsing the Path */
	  len++;
	  if (*in == '/' || *in == '\\')
	    {
		last = in;
		dirlen = len;
	    }
	  in++;
      }
    if (last == NULL)
	return NULL;		/* there is no Dir component */

/* allocating the DirName  to be returned */
    name = malloc (dirlen + 1);
    memcpy (name, path, dirlen);
    *(name + dirlen) = '\0';
    return name;
}

GAIAAUX_DECLARE char *
gaiaFullFileNameFromPath (const char *path)
{
/* extracting the FullFileName (including Extension) from a Path */
    const char *in = path;
    const char *last = path - 1;
    int len;
    char *name;

    if (path == NULL)
	return NULL;

    while (*in != '\0')
      {
	  /* parsing the Path */
	  if (*in == '/' || *in == '\\')
	      last = in;
	  in++;
      }
    len = strlen (last + 1);
    if (len == 0)
	return NULL;

/* allocating the FullFileName to be returned */
    name = malloc (len + 1);
    strcpy (name, last + 1);
    return name;
}

GAIAAUX_DECLARE char *
gaiaFileNameFromPath (const char *path)
{
/* extracting the FileName (excluding Extension) from a Path */
    const char *in = path;
    const char *last = path - 1;
    int len;
    char *name;
    int i;

    if (path == NULL)
	return NULL;

    while (*in != '\0')
      {
	  /* parsing the Path */
	  if (*in == '/' || *in == '\\')
	      last = in;
	  in++;
      }
    len = strlen (last + 1);
    if (len == 0)
	return NULL;

/* allocating the FullFileName to be returned */
    name = malloc (len + 1);
    strcpy (name, last + 1);
    for (i = len - 1; i > 0; i--)
      {
	  if (*(name + i) == '.')
	    {
		/* stripping out the extension */
		*(name + i) = '\0';
		break;
	    }
      }
    return name;
}

GAIAAUX_DECLARE char *
gaiaFileExtFromPath (const char *path)
{
/* extracting the FileExtension (if any) from a Path */
    int len;
    char *name;
    int i;
    int pos = -1;

    if (path == NULL)
	return NULL;

    len = strlen (path);
    for (i = len - 1; i > 0; i--)
      {
	  if (*(path + i) == '/' || *(path + i) == '\\')
	      break;
	  if (*(path + i) == '.')
	    {
		/* found an extension */
		pos = i;
		break;
	    }
      }
    if (pos <= 0)
	return NULL;

/* allocating the FileExtension to be returned */
    len = strlen (path + pos + 1);
    if (len == 0)
	return NULL;
    name = malloc (len + 1);
    strcpy (name, path + pos + 1);
    return name;
}
