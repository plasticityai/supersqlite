/* 

demo5.c

Author: Sandro Furieri a.furieri@lqt.it

This software is provided 'as-is', without any express or implied
warranty.  In no event will the author be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
these headers are required in order to support
SQLite/SpatiaLite
*/
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>

#define ARG_NONE	0
#define ARG_DB_PATH	1
#define ARG_TABLE	2
#define ARG_GEOMETRY	3

static void
do_print_list (gaiaVectorLayersListPtr list, int n_mode)
{
/* prints the layers list */
    gaiaVectorLayerPtr lyr;
    gaiaLayerAttributeFieldPtr fld;
    const char *mode = "FAST";
    if (n_mode == GAIA_VECTORS_LIST_OPTIMISTIC)
	mode = "OPTIMISTIC";
    if (n_mode == GAIA_VECTORS_LIST_PESSIMISTIC)
	mode = "PESSIMISTIC";

    printf ("\n****** VectorLayersList (mode=%s) *********\n", mode);
    if (list == NULL)
      {
	  printf ("The VectorLayersList is empty !!!\n\n");
	  return;
      }

    lyr = list->First;
    while (lyr)
      {
	  /* printing the Layer Header */
	  const char *lyr_type = "UnknownType";
	  const char *geom_type = "UnknownType";
	  const char *dims = "UnknownDims";
	  switch (lyr->LayerType)
	    {
	    case GAIA_VECTOR_TABLE:
		lyr_type = "BasedOnSqlTable";
		break;
	    case GAIA_VECTOR_VIEW:
		lyr_type = "BasedOnSqlView";
		break;
	    case GAIA_VECTOR_VIRTUAL:
		lyr_type = "BasedOnVirtualShape";
		break;
	    };
	  switch (lyr->GeometryType)
	    {
	    case GAIA_VECTOR_GEOMETRY:
		geom_type = "GEOMETRY";
		break;
	    case GAIA_VECTOR_POINT:
		geom_type = "POINT";
		break;
	    case GAIA_VECTOR_LINESTRING:
		geom_type = "LINESTRING";
		break;
	    case GAIA_VECTOR_POLYGON:
		geom_type = "POLYGON";
		break;
	    case GAIA_VECTOR_MULTIPOINT:
		geom_type = "MULTIPOINT";
		break;
	    case GAIA_VECTOR_MULTILINESTRING:
		geom_type = "MULTILINESTRING";
		break;
	    case GAIA_VECTOR_MULTIPOLYGON:
		geom_type = "MULTIPOLYGON";
		break;
	    case GAIA_VECTOR_GEOMETRYCOLLECTION:
		geom_type = "GEOMETRYCOLLECTION";
		break;
	    };
	  switch (lyr->Dimensions)
	    {
	    case GAIA_XY:
		dims = "XY";
		break;
	    case GAIA_XY_Z:
		dims = "XYZ";
		break;
	    case GAIA_XY_M:
		dims = "XYM";
		break;
	    case GAIA_XY_Z_M:
		dims = "XYXM";
		break;
	    };
	  printf ("VectorLayer: Type=%s TableName=%s\n", lyr_type,
		  lyr->TableName);
	  printf ("\tGeometryName=%s SRID=%d GeometryType=%s Dims=%s\n",
		  lyr->GeometryName, lyr->Srid, geom_type, dims);
	  if (lyr->ExtentInfos)
	    {
		printf ("\tRowCount=%d\n", lyr->ExtentInfos->Count);
		printf ("\tExtentMin %f / %f\n\tExtentMax %f / %f\n",
			lyr->ExtentInfos->MinX,
			lyr->ExtentInfos->MinY, lyr->ExtentInfos->MaxX,
			lyr->ExtentInfos->MaxY);
	    }
	  if (lyr->AuthInfos)
	      printf ("\tReadOnly=%s Hidden=%s\n",
		      (lyr->AuthInfos->IsReadOnly == 0) ? "FALSE" : "TRUE",
		      (lyr->AuthInfos->IsHidden == 0) ? "FALSE" : "TRUE");
	  fld = lyr->First;
	  while (fld)
	    {
		/* printing AttributeFields infos */
		printf ("\t\tField #%d) FieldName=%s\n", fld->Ordinal,
			fld->AttributeFieldName);
		printf ("\t\t\t");
		if (fld->NullValuesCount)
		    printf ("NullValues=%d ", fld->NullValuesCount);
		if (fld->IntegerValuesCount)
		    printf ("IntegerValues=%d ", fld->IntegerValuesCount);
		if (fld->DoubleValuesCount)
		    printf ("DoubleValues=%d ", fld->DoubleValuesCount);
		if (fld->TextValuesCount)
		    printf ("TextValues=%d ", fld->TextValuesCount);
		if (fld->BlobValuesCount)
		    printf ("BlobValues=%d ", fld->BlobValuesCount);
		printf ("\n");
		if (fld->MaxSize)
		    printf ("\t\t\tMaxSize/Length=%d\n", fld->MaxSize->MaxSize);
		if (fld->IntRange)
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT: M$ runtime doesn't supports %lld for 64 bits */
		    printf ("\t\t\tIntRange %I64d / %I64d\n",
#else
		    printf ("\t\t\tIntRange %lld / %lld\n",
#endif
			    fld->IntRange->MinValue, fld->IntRange->MaxValue);
		if (fld->DoubleRange)
		    printf ("\t\t\tDoubleRange %f / %f\n",
			    fld->DoubleRange->MinValue,
			    fld->DoubleRange->MaxValue);
		fld = fld->Next;
	    }
	  lyr = lyr->Next;
      }
    printf ("\n");
}

static void
do_help ()
{
/* printing the argument list */
    fprintf (stderr, "\n\nusage: demo5 ARGLIST\n");
    fprintf (stderr,
	     "==============================================================\n");
    fprintf (stderr, "-d or --db-path     pathname   the SpatiaLite DB path\n");
    fprintf (stderr,
	     "-t or --table      table-name  the table to be checked\n");
    fprintf (stderr,
	     "-g or --geometry  column_name  geometry column [optional]\n\n");
    fprintf (stderr, "you can specify one of the following modes:\n");
    fprintf (stderr, "-o or --optimistic              OPTIMISTIC mode\n");
    fprintf (stderr, "-p or --pessimistic             PESSIMISTIC mode\n");
}


int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    int i;
    int next_arg = ARG_NONE;
    int mode = GAIA_VECTORS_LIST_OPTIMISTIC;
    int error = 0;
    const char *db_path = NULL;
    const char *table = NULL;
    const char *geometry = NULL;
    gaiaVectorLayersListPtr list;
    void *cache;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    for (i = 1; i < argc; i++)
      {
	  /* parsing the invocation arguments */
	  if (next_arg != ARG_NONE)
	    {
		switch (next_arg)
		  {
		  case ARG_DB_PATH:
		      db_path = argv[i];
		      break;
		  case ARG_TABLE:
		      table = argv[i];
		      break;
		  case ARG_GEOMETRY:
		      geometry = argv[i];
		      break;
		  };
		next_arg = ARG_NONE;
		continue;
	    }
	  if (strcasecmp (argv[i], "--help") == 0
	      || strcmp (argv[i], "-h") == 0)
	    {
		do_help ();
		return -1;
	    }
	  if (strcasecmp (argv[i], "-d") == 0
	      || strcasecmp (argv[i], "--db-path") == 0)
	    {
		next_arg = ARG_DB_PATH;
		continue;
	    }
	  if (strcasecmp (argv[i], "-t") == 0
	      || strcmp (argv[i], "--table") == 0)
	    {
		next_arg = ARG_TABLE;
		continue;
	    }
	  if (strcasecmp (argv[i], "-g") == 0
	      || strcmp (argv[i], "--geometry") == 0)
	    {
		next_arg = ARG_GEOMETRY;
		continue;
	    }
	  if (strcasecmp (argv[i], "-p") == 0
	      || strcmp (argv[i], "--pessimistic") == 0)
	    {
		mode = GAIA_VECTORS_LIST_PESSIMISTIC;
		next_arg = ARG_NONE;
		continue;
	    }
	  if (strcasecmp (argv[i], "-o") == 0
	      || strcmp (argv[i], "--optimistic") == 0)
	    {
		mode = GAIA_VECTORS_LIST_OPTIMISTIC;
		next_arg = ARG_NONE;
		continue;
	    }
	  fprintf (stderr, "unknown argument: %s\n", argv[i]);
	  error = 1;
      }
    if (error)
      {
	  do_help ();
	  return -1;
      }

/* checking the arguments */
    if (!db_path)
      {
	  fprintf (stderr, "did you forget setting the --db-path argument ?\n");
	  error = 1;
      }

    if (error)
      {
	  do_help ();
	  return -1;
      }


/*
trying to connect the test DB: 
- this demo is intended to create an existing, already populated database
*/
    ret = sqlite3_open_v2 (db_path, &handle,
			   SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  printf ("cannot open '%s': %s\n", argv[1], sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }
    cache = spatialite_alloc_connection ();
    spatialite_init_ex (handle, cache, 0);


/* showing the SQLite version */
    printf ("SQLite version: %s\n", sqlite3_libversion ());
/* showing the SpatiaLite version */
    printf ("SpatiaLite version: %s\n", spatialite_version ());
    printf ("\n\n");


/* listing the requested layer(s) */
    list = gaiaGetVectorLayersList (handle, table, geometry, mode);
    do_print_list (list, mode);
    gaiaFreeVectorLayersList (list);

/* disconnecting the test DB */
    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  printf ("close() error: %s\n", sqlite3_errmsg (handle));
	  return -1;
      }
    spatialite_cleanup_ex (cache);
    printf ("\n\nsample successfully terminated\n");
    spatialite_shutdown();
    return 0;
}
