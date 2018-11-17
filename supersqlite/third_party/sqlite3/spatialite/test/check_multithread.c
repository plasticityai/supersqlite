/*

 check_multithread.c -- SpatiaLite Test Case

 Author: Alessandro Furieri <a.furieri@lqt.t>

 ------------------------------------------------------------------------------
 
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
 
Portions created by the Initial Developer are Copyright (C) 2013
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

#ifdef _WIN32
#define _CRT_RAND_S
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#ifndef _WIN32
#include <dirent.h>
#include <fnmatch.h>
#endif

#include "config.h"

#include <sqlite3.h>
#include <spatialite.h>

#ifdef _WIN32
#include "fnmatch4win.h"
#include "scandir4win.h"
#include "asprintf4win.h"
#include "fnmatch_impl4win.h"
#endif

struct test_data
{
    char *test_case_name;
    char *database_name;
    char *sql_statement;
    int expected_rows;
    int expected_columns;
    char **expected_results;
    int *expected_precision;
    struct test_data *next;
};

struct test_list
{
    struct test_data *first;
    struct test_data *last;
    int count;
    struct test_data **array;
};

struct db_conn
{
    sqlite3 *db_handle;
    void *cache;
};

struct thread_params
{
    int id;
    struct test_list *list;
    struct db_conn *conn;
    int done;
    int count;
    int errors;
#ifndef _WIN32
    int started;
    pthread_t thread_id;
    pthread_attr_t attr;
#endif
} mt_params[64];

static struct db_conn *
alloc_connection (void)
{
    struct db_conn *conn = malloc (sizeof (struct db_conn));
    conn->db_handle = NULL;
    conn->cache = spatialite_alloc_connection ();
    return conn;
}

static void
close_connection (struct db_conn *conn, int final)
{
    if (conn->db_handle != NULL)
	sqlite3_close (conn->db_handle);
    conn->db_handle = NULL;
    if (final)
      {
	  if (conn->cache != NULL)
	      spatialite_cleanup_ex (conn->cache);
	  conn->cache = NULL;
      }
}

static void
open_connection (struct db_conn *conn)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open %s db: %s\n", ":memory:",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return;
      }

    spatialite_init_ex (db_handle, conn->cache, 0);

    ret =
	sqlite3_exec (db_handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return;
      }

    conn->db_handle = db_handle;
}

static void
init_thread_params (int id, struct test_list *list)
{
    struct thread_params *p = &(mt_params[id]);
    p->id = id;
    p->conn = alloc_connection ();
    p->list = list;
    p->done = 0;
    p->count = 0;
    p->errors = 0;
#ifndef _WIN32
    p->started = 0;
#endif
}

static void
free_thread_params (int id, int *total_sql, int *failures)
{
    struct thread_params *p = &(mt_params[id]);
    if (p->conn != NULL)
      {
	  close_connection (p->conn, 1);
	  free (p->conn);
	  p->conn = NULL;
      }
#ifndef _WIN32
    if (p->started)
      {
	  void *ptr;
	  pthread_join (p->thread_id, &ptr);
	  pthread_attr_destroy (&(p->attr));
      }
#endif
    *total_sql += p->count;
    *failures += p->errors;
}

int
get_clean_line (FILE * f, char **line)
{
    size_t len = 0;
    ssize_t num_read = 0;
    ssize_t end = 0;
    char *tmp_line = NULL;

#if !defined(_WIN32) &&!defined(__APPLE__)
/* expecting to be on a sane minded platform [linux-like] */
    num_read = getline (&(tmp_line), &len, f);
#else
/* neither Windows nor MacOsX support getline() */
    len = 1024 * 1024;
    tmp_line = malloc (len);
    if (fgets (tmp_line, len, f) == NULL)
      {
	  free (tmp_line);
	  num_read = -1;
      }
    else
	num_read = strlen (tmp_line);
#endif

    if (num_read < 1)
      {
	  fprintf (stderr, "failed to read at %li: %li\n", ftell (f), num_read);
	  return -1;
      }
    /* trim the trailing new line and any comments */
    for (end = 0; end < num_read; ++end)
      {
	  if (*(tmp_line + end) == '\n')
	      break;
	  if (*(tmp_line + end) == '#')
	      break;
      }
    /* trim any trailing spaces */
    while (end > 0)
      {
	  if (*(tmp_line + end - 1) != ' ')
	    {
		break;
	    }
	  *(tmp_line + end - 1) = '\0';
	  end--;
      }
    *line = malloc (end + 1);
    memcpy (*line, tmp_line, end);
    (*line)[end] = '\0';
    free (tmp_line);
    return 0;
}

void
handle_precision (char *expected_result, int *precision)
{
    int i;
    int prcsn;
    int result_len = strlen (expected_result);
    *precision = 0;
    for (i = result_len - 1; i >= 0; --i)
      {
	  if (expected_result[i] == ':')
	    {
		prcsn = atoi (&(expected_result[i + 1]));
		if (prcsn > 0)
		  {
		      expected_result[i] = '\0';
		      *precision = prcsn;
		  }
		break;
	    }
      }
}

struct test_data *
read_one_case (const char *filepath)
{
    int num_results;
    int i;
    char *tmp;
    FILE *f;
    struct test_data *data;

    f = fopen (filepath, "r");

    data = malloc (sizeof (struct test_data));
    data->next = NULL;
    get_clean_line (f, &(data->test_case_name));
    get_clean_line (f, &(data->database_name));
    get_clean_line (f, &(data->sql_statement));
    get_clean_line (f, &(tmp));
    data->expected_rows = atoi (tmp);
    free (tmp);
    get_clean_line (f, &(tmp));
    data->expected_columns = atoi (tmp);
    free (tmp);
    num_results = (data->expected_rows + 1) * data->expected_columns;
    data->expected_results = malloc (num_results * sizeof (char *));
    data->expected_precision = malloc (num_results * sizeof (int));
    for (i = 0; i < num_results; ++i)
      {
	  get_clean_line (f, &(data->expected_results[i]));
	  handle_precision (data->expected_results[i],
			    &(data->expected_precision[i]));
      }
    fclose (f);
    return data;
}

void
cleanup_test_data (struct test_data *data)
{
    int i;
    int num_results = (data->expected_rows + 1) * (data->expected_columns);

    for (i = 0; i < num_results; ++i)
      {
	  free (data->expected_results[i]);
      }
    free (data->expected_results);
    free (data->expected_precision);
    free (data->test_case_name);
    free (data->database_name);
    free (data->sql_statement);
    free (data);
}

static void
add_test (struct test_list *list, struct test_data *data)
{
    if (list->first == NULL)
	list->first = data;
    if (list->last != NULL)
	list->last->next = data;
    list->last = data;
}

static void
list_cleanup (struct test_list *list)
{
    struct test_data *p;
    struct test_data *pn;
    p = list->first;
    while (p != NULL)
      {
	  pn = p->next;
	  cleanup_test_data (p);
	  p = pn;
      }
    if (list->array != NULL)
	free (list->array);
}

static int
build_test_array (struct test_list *list)
{
    struct test_data *p;
    struct test_data **pn;
    list->count = 0;
    if (list->array != NULL)
	free (list->array);
    list->array = NULL;
    p = list->first;
    while (p != NULL)
      {
	  list->count++;
	  p = p->next;
      }
    if (list->count <= 0)
	return 0;
    list->array = malloc (sizeof (struct test_data *) * list->count);
    pn = list->array;
    p = list->first;
    while (p != NULL)
      {
	  *pn = p;
	  pn++;
	  p = p->next;
      }
    return 1;
}

int
test_case_filter (const struct dirent *entry)
{
    return (fnmatch ("*.testcase", entry->d_name, FNM_PERIOD) == 0);
}

static int
load_testcases (struct test_list *list)
{
    struct dirent **namelist;
    int n;
    int i;
    const char *security_level;
    const char *current_dir;
    char *path;

/* plain SQL testcases */
    current_dir = "sql_stmt_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);

/* security related testcases */
    security_level = getenv ("SPATIALITE_SECURITY");
    if (security_level == NULL)
	;
    else if (strcasecmp (security_level, "relaxed") == 0)
      {
	  current_dir = "sql_stmt_security_tests";
	  path = sqlite3_mprintf ("%s", current_dir);
	  n = scandir (path, &namelist, test_case_filter, alphasort);
	  if (n < 0)
	    {
		perror ("scandir");
		return 0;
	    }
	  sqlite3_free (path);
	  for (i = 0; i < n; ++i)
	    {
		struct test_data *data;
		char *path;
		path = sqlite3_mprintf ("%s/%s", current_dir,
					namelist[i]->d_name);
		data = read_one_case (path);
		if (strcmp (data->database_name, ":memory:") == 0)
		    add_test (list, data);
		else
		    cleanup_test_data (data);
		sqlite3_free (path);
		free (namelist[i]);
	    }
	  free (namelist);
      }

#ifndef OMIT_MATHSQL		/* only if MATHSQL is supported */
/* Math SQL testcases */
    current_dir = "sql_stmt_mathsql_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
#endif /* end MATHSQL conditional */

#ifndef OMIT_PROJ		/* only if PROJ is supported */
/* PROJ.4 SQL testcases */
    current_dir = "sql_stmt_proj_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
#endif /* end PROJ conditional */

#ifndef OMIT_GEOS		/* only if GEOS is supported */
/* GEOS SQL testcases */
    current_dir = "sql_stmt_geos_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
#endif /* end GEOS conditional */

#ifdef GEOS_ADVANCED		/* only if GEOS_ADVANCED is supported - 3.4.0 */
/* GEOS ADVANCED SQL testcases */
    current_dir = "sql_stmt_geosadvanced_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
#endif /* end GEOS_ADVANCED conditional */

#ifdef ENABLE_LWGEOM		/* only if LWGEOM is supported */
/* LWGEOM SQL testcases */
    current_dir = "sql_stmt_lwgeom_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
#endif /* end LWGEOM conditional */

#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */
/* LIBXML2 SQL testcases */
    current_dir = "sql_stmt_libxml2_tests";
    path = sqlite3_mprintf ("%s", current_dir);
    n = scandir (path, &namelist, test_case_filter, alphasort);
    if (n < 0)
      {
	  perror ("scandir");
	  return 0;
      }
    sqlite3_free (path);
    for (i = 0; i < n; ++i)
      {
	  struct test_data *data;
	  char *path;
	  path = sqlite3_mprintf ("%s/%s", current_dir, namelist[i]->d_name);
	  data = read_one_case (path);
	  if (strcmp (data->database_name, ":memory:") == 0)
	      add_test (list, data);
	  else
	      cleanup_test_data (data);
	  sqlite3_free (path);
	  free (namelist[i]);
      }
    free (namelist);
/* security related testcases */
    security_level = getenv ("SPATIALITE_SECURITY");
    if (security_level == NULL)
	;
    else if (strcasecmp (security_level, "relaxed") == 0)
      {
	  current_dir = "sql_stmt_xmlsec_tests";
	  path = sqlite3_mprintf ("%s", current_dir);
	  n = scandir (path, &namelist, test_case_filter, alphasort);
	  if (n < 0)
	    {
		perror ("scandir");
		return 0;
	    }
	  sqlite3_free (path);
	  for (i = 0; i < n; ++i)
	    {
		struct test_data *data;
		char *path;
		path = sqlite3_mprintf ("%s/%s", current_dir,
					namelist[i]->d_name);
		data = read_one_case (path);
		if (strcmp (data->database_name, ":memory:") == 0
		    || strcmp (data->database_name, "NEW:memory:") == 0)
		    add_test (list, data);
		else
		    cleanup_test_data (data);
		sqlite3_free (path);
		free (namelist[i]);
	    }
	  free (namelist);
      }
#endif /* end LIBXML2 conditional */

    return 1;
}

int
do_one_case (const struct test_data *data, struct db_conn *conn, int thread_id)
{
    int ret;
    char *err_msg = NULL;
    int i;
    char **results;
    int rows;
    int columns;

    if (strcmp (data->database_name, "NEW:memory:") == 0)
	close_connection (conn, 0);
    if (conn->db_handle == NULL)
	open_connection (conn);
    if (conn->db_handle == NULL)
	return -9;

    ret =
	sqlite3_get_table (conn->db_handle, data->sql_statement, &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  printf ("\nFAILED TEST: Thread #%d %s\n", thread_id,
		  data->test_case_name);
	  printf ("Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    if ((rows != data->expected_rows) || (columns != data->expected_columns))
      {
	  printf ("\nFAILED TEST: Thread #%d %s\n", thread_id,
		  data->test_case_name);
	  printf ("Unexpected error: bad result: %i/%i.\n", rows, columns);
	  return -11;
      }
    for (i = 0; i < (data->expected_rows + 1) * data->expected_columns; ++i)
      {
	  if (results[i] != NULL && data->expected_precision[i] == 0)
	    {
		data->expected_precision[i] = strlen (results[i]);
	    }
	  if (results[i] == NULL)
	    {
		if (strcmp ("(NULL)", data->expected_results[i]) == 0)
		  {
		      /* we expected this */
		      continue;
		  }
		else
		  {
		      printf ("\nFAILED TEST: Thread #%d %s\n", thread_id,
			      data->test_case_name);
		      printf ("Null value at %i.\n", i);
		      printf ("Expected value was: %s\n",
			      data->expected_results[i]);
		      return -12;
		  }
	    }
	  else if (strlen (results[i]) == 0)
	    {
		printf ("\nFAILED TEST: Thread #%d %s\n", thread_id,
			data->test_case_name);
		printf ("zero length result at %i\n", i);
		printf ("Expected value was    : %s|\n",
			data->expected_results[i]);
		return -13;
	    }
	  else if (strncmp
		   (results[i], data->expected_results[i],
		    data->expected_precision[i]) != 0)
	    {
		printf ("\nFAILED TEST: Thread #%d %s\n", thread_id,
			data->test_case_name);
		printf ("Unexpected value at %i: %s|\n", i, results[i]);
		printf ("Expected value was   : %s|\n",
			data->expected_results[i]);
		return -14;
	    }
      }
    sqlite3_free_table (results);
    return 0;
}

static void
do_tests (struct thread_params *param)
{
    int i;
    int j;
    struct test_list *list = param->list;
    struct db_conn *conn = param->conn;

    for (i = list->count - 1; i >= 0; i--)
      {
	  /* tests in backward order */
	  struct test_data *px = *(list->array + i);
	  for (j = 0; j < 32; j++)
	    {
		int ret = do_one_case (px, conn, param->id);
		if (ret < 0)
		    param->errors += 1;
		param->count++;
	    }
      }
    for (i = 0; i < list->count; i++)
      {
	  /* tests in forward order */
	  struct test_data *px = *(list->array + i);
	  for (j = 0; j < 32; j++)
	    {
		int ret = do_one_case (px, conn, param->id);
		if (ret < 0)
		    param->errors += 1;
		param->count++;
	    }
      }
    for (i = 0; i < 131072; i++)
      {
	  /* tests in random order */
	  struct test_data *px;
	  int ret;
	  int x;
#ifdef _WIN32
	  x = rand () % list->count;
#else
	  x = random () % list->count;
#endif
	  px = *(list->array + x);
	  ret = do_one_case (px, conn, param->id);
	  if (ret < 0)
	      param->errors += 1;
	  param->count++;
      }
}

#ifdef _WIN32
DWORD WINAPI
exec_thread (void *arg)
#else
void *
exec_thread (void *arg)
#endif
{
/* thread implementation */
    struct thread_params *param = (struct thread_params *) arg;
    do_tests (param);
/* thread termination */
    if (param->errors)
	printf ("\nThread %d reports %d error%c\n", param->id, param->errors,
		(param->errors > 1) ? 's' : ' ');
    param->done = 1;
#ifdef _WIN32
    return 0;
#else
    pthread_exit (NULL);
#endif
}

static void
run_thread (int i)
{
    struct thread_params *param = &(mt_params[i]);
#ifdef _WIN32
    HANDLE thread_handle;
    DWORD dw_thread_id;
    thread_handle =
	CreateThread (NULL, 0, exec_thread, param, 0, &dw_thread_id);
    SetThreadPriority (thread_handle, THREAD_PRIORITY_IDLE);
#else
    int ok_prior = 0;
    int policy;
    int min_prio;
    struct sched_param sp;
    pthread_attr_init (&(param->attr));
    param->started = 1;
    if (pthread_attr_setschedpolicy (&(param->attr), SCHED_RR) == 0)
      {
	  /* attempting to set the lowest priority */
	  if (pthread_attr_getschedpolicy (&(param->attr), &policy) == 0)
	    {
		min_prio = sched_get_priority_min (policy);
		sp.sched_priority = min_prio;
		if (pthread_attr_setschedparam (&(param->attr), &sp) == 0)
		  {
		      /* ok, setting the lowest priority */
		      ok_prior = 1;
		      pthread_create (&(param->thread_id), &(param->attr),
				      exec_thread, param);
		  }
	    }
      }
    if (!ok_prior)
      {
	  /* failure: using standard priority */
	  pthread_create (&(param->thread_id), NULL, exec_thread, param);
      }
#endif
}

int
main (int argc, char *argv[])
{
    int i;
    int failures = 0;
    int total_sql = 0;
    int num_threads = 4;
    int mt_enabled = 0;
    struct test_list list;
    const char *env_var;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    env_var = getenv ("SPATIALITE_MULTITHREAD_TEST");
    if (env_var == NULL)
	;
    else if (strcasecmp (env_var, "yes") == 0)
	mt_enabled = 1;
    if (!mt_enabled)
      {
	  printf
	      ("check_multithread: SPATIALITE_MULTITHREAD_TEST non enabled .... skipping ...\n");
	  return 0;
      }
    env_var = getenv ("SPATIALITE_NUM_THREADS");
    if (env_var == NULL)
	;
    else
      {
	  num_threads = atoi (env_var);
	  if (num_threads < 1)
	      num_threads = 1;
	  if (num_threads > 64)
	      num_threads = 64;
      }
    printf ("Testing %d concurrent threads\n", num_threads);

/* seeding the pseudo-random generator */
#ifdef _WIN32
    srand (getpid ());
#else
    srandom (getpid ());
#endif

    list.first = NULL;
    list.last = NULL;
    list.count = 0;
    list.array = NULL;
    if (!load_testcases (&list))
      {
	  fprintf (stderr, "Unable to load testcases !!!\n");
	  return -1;
      }
    if (!build_test_array (&list))
      {
	  fprintf (stderr, "No valid testcases found !!!\n");
	  return -1;
      }

    spatialite_initialize ();

    for (i = 0; i < num_threads; i++)
	init_thread_params (i, &list);

    for (i = 0; i < num_threads; i++)
	run_thread (i);

    while (1)
      {
	  int cnt;
#ifdef _WIN32
	  Sleep (5 * 1000);
#else
	  sleep (5);
#endif
	  cnt = 0;
	  for (i = 0; i < num_threads; i++)
	    {
		struct thread_params *p = &(mt_params[i]);
		if (p->done != 1)
		    cnt++;
	    }
	  if (cnt == 0)
	      break;
      }


    for (i = 0; i < num_threads; i++)
	free_thread_params (i, &total_sql, &failures);
    printf
	("\n\nALL MULTITHREAD TEST PASSED\n\t[%d SQL tests: %d failures]\n\n",
	 total_sql, failures);

    list_cleanup (&list);

    spatialite_shutdown ();

    return 0;
}
