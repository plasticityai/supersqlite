/*

 virtualnetwork.c -- SQLite3 extension [VIRTUAL TABLE Routing - shortest path]

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
Luigi Costalli luigi.costalli@gmail.com

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
#include <float.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

static struct sqlite3_module my_net_module;

#define VNET_DIJKSTRA_ALGORITHM	1
#define VNET_A_STAR_ALGORITHM	2

#define VNET_ROUTING_SOLUTION	0xdd
#define VNET_RANGE_SOLUTION		0xbb

#define VNET_INVALID_SRID	-1234

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

/******************************************************************************
/
/ VirtualNetwork structs
/
******************************************************************************/

typedef struct NetworkArcStruct
{
/* an ARC */
    const struct NetworkNodeStruct *NodeFrom;
    const struct NetworkNodeStruct *NodeTo;
    sqlite3_int64 ArcRowid;
    double Cost;
} NetworkArc;
typedef NetworkArc *NetworkArcPtr;

typedef struct NetworkNodeStruct
{
/* a NODE */
    int InternalIndex;
    sqlite3_int64 Id;
    char *Code;
    double CoordX;
    double CoordY;
    int NumArcs;
    NetworkArcPtr Arcs;
} NetworkNode;
typedef NetworkNode *NetworkNodePtr;

typedef struct NetworkStruct
{
/* the main NETWORK structure */
    int Net64;
    int AStar;
    int EndianArch;
    int MaxCodeLength;
    int CurrentIndex;
    int NodeCode;
    int NumNodes;
    char *TableName;
    char *FromColumn;
    char *ToColumn;
    char *GeometryColumn;
    char *NameColumn;
    double AStarHeuristicCoeff;
    NetworkNodePtr Nodes;
} Network;
typedef Network *NetworkPtr;

typedef struct ArcSolutionStruct
{
/* Geometry corresponding to an Arc used by Dijkstra shortest path solution */
    sqlite3_int64 ArcRowid;
    char *FromCode;
    char *ToCode;
    sqlite3_int64 FromId;
    sqlite3_int64 ToId;
    int Points;
    double *Coords;
    int Srid;
    char *Name;
    struct ArcSolutionStruct *Next;

} ArcSolution;
typedef ArcSolution *ArcSolutionPtr;

typedef struct RowSolutionStruct
{
/* a row into the shortest path solution */
    NetworkArcPtr Arc;
    char *Name;
    struct RowSolutionStruct *Next;

} RowSolution;
typedef RowSolution *RowSolutionPtr;

typedef struct RowNodeSolutionStruct
{
/* a row into the "within Cost range" solution */
    NetworkNodePtr Node;
    double Cost;
    int Srid;
    struct RowNodeSolutionStruct *Next;

} RowNodeSolution;
typedef RowNodeSolution *RowNodeSolutionPtr;

typedef struct SolutionStruct
{
/* the shortest path solution */
    unsigned char Mode;
    ArcSolutionPtr FirstArc;
    ArcSolutionPtr LastArc;
    NetworkNodePtr From;
    NetworkNodePtr To;
    double MaxCost;
    RowSolutionPtr First;
    RowSolutionPtr Last;
    RowNodeSolutionPtr FirstNode;
    RowNodeSolutionPtr LastNode;
    RowSolutionPtr CurrentRow;
    RowNodeSolutionPtr CurrentNodeRow;
    sqlite3_int64 CurrentRowId;
    double TotalCost;
    gaiaGeomCollPtr Geometry;
} Solution;
typedef Solution *SolutionPtr;

/******************************************************************************
/
/ Dijkstra and A* common structs
/
******************************************************************************/

typedef struct RoutingNode
{
    int Id;
    struct RoutingNode **To;
    NetworkArcPtr *Link;
    int DimTo;
    struct RoutingNode *PreviousNode;
    NetworkNodePtr Node;
    NetworkArcPtr Arc;
    double Distance;
    double HeuristicDistance;
    int Inspected;
} RoutingNode;
typedef RoutingNode *RoutingNodePtr;

typedef struct RoutingNodes
{
    RoutingNodePtr Nodes;
    NetworkArcPtr *ArcsBuffer;
    RoutingNodePtr *NodesBuffer;
    int Dim;
    int DimLink;
} RoutingNodes;
typedef RoutingNodes *RoutingNodesPtr;

typedef struct HeapNode
{
    RoutingNodePtr Node;
    double Distance;
} HeapNode;
typedef HeapNode *HeapNodePtr;

typedef struct RoutingHeapStruct
{
    HeapNodePtr Nodes;
    int Count;
} RoutingHeap;
typedef RoutingHeap *RoutingHeapPtr;

/******************************************************************************
/
/ VirtualTable structs
/
******************************************************************************/

typedef struct VirtualNetworkStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    NetworkPtr graph;		/* the NETWORK structure */
    RoutingNodesPtr routing;	/* the ROUTING structure */
    int currentAlgorithm;	/* the currently selected Shortest Path Algorithm */
} VirtualNetwork;
typedef VirtualNetwork *VirtualNetworkPtr;

typedef struct VirtualNetworkCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualNetworkPtr pVtab;	/* Virtual table of this cursor */
    SolutionPtr solution;	/* the current solution */
    int eof;			/* the EOF marker */
} VirtualNetworkCursor;
typedef VirtualNetworkCursor *VirtualNetworkCursorPtr;

/*
/
/  implementation of the Dijkstra Shortest Path algorithm
/
////////////////////////////////////////////////////////////
/
/ Author: Luigi Costalli luigi.costalli@gmail.com
/ version 1.0. 2008 October 21
/
*/

static RoutingNodesPtr
routing_init (NetworkPtr graph)
{
/* allocating and initializing the ROUTING struct */
    int i;
    int j;
    int cnt = 0;
    RoutingNodesPtr nd;
    RoutingNodePtr ndn;
    NetworkNodePtr nn;
/* allocating the main Nodes struct */
    nd = malloc (sizeof (RoutingNodes));
/* allocating and initializing  Nodes array */
    nd->Nodes = malloc (sizeof (RoutingNode) * graph->NumNodes);
    nd->Dim = graph->NumNodes;
    nd->DimLink = 0;

/* pre-alloc buffer strategy - GENSCHER 2010-01-05 */
    for (i = 0; i < graph->NumNodes; cnt += graph->Nodes[i].NumArcs, i++);
    nd->NodesBuffer = malloc (sizeof (RoutingNodePtr) * cnt);
    nd->ArcsBuffer = malloc (sizeof (NetworkArcPtr) * cnt);

    cnt = 0;
    for (i = 0; i < graph->NumNodes; i++)
      {
	  /* initializing the Nodes array */
	  nn = graph->Nodes + i;
	  ndn = nd->Nodes + i;
	  ndn->Id = nn->InternalIndex;
	  ndn->DimTo = nn->NumArcs;
	  ndn->Node = nn;
	  ndn->To = &(nd->NodesBuffer[cnt]);
	  ndn->Link = &(nd->ArcsBuffer[cnt]);
	  cnt += nn->NumArcs;

	  for (j = 0; j < nn->NumArcs; j++)
	    {
		/*  setting the outcoming Arcs for the current Node */
		nd->DimLink++;
		ndn->To[j] = nd->Nodes + nn->Arcs[j].NodeTo->InternalIndex;
		ndn->Link[j] = nn->Arcs + j;
	    }
      }
    return (nd);
}

static void
routing_free (RoutingNodes * e)
{
/* memory cleanup; freeing the ROUTING struct */
    free (e->ArcsBuffer);
    free (e->NodesBuffer);
    free (e->Nodes);
    free (e);
}

static RoutingHeapPtr
routing_heap_init (int n)
{
/* allocating and initializing the Heap (min-priority queue) */
    RoutingHeapPtr heap = malloc (sizeof (RoutingHeap));
    heap->Count = 0;
    heap->Nodes = malloc (sizeof (HeapNode) * (n + 1));
    return heap;
}

static void
routing_heap_free (RoutingHeapPtr heap)
{
/* freeing the Heap (min-priority queue) */
    if (heap->Nodes != NULL)
	free (heap->Nodes);
    free (heap);
}

static void
dijkstra_insert (RoutingNodePtr node, HeapNodePtr heap, int size)
{
/* inserting a new Node and rearranging the heap */
    int i;
    HeapNode tmp;
    i = size + 1;
    heap[i].Node = node;
    heap[i].Distance = node->Distance;
    if (i / 2 < 1)
	return;
    while (heap[i].Distance < heap[i / 2].Distance)
      {
	  tmp = heap[i];
	  heap[i] = heap[i / 2];
	  heap[i / 2] = tmp;
	  i /= 2;
	  if (i / 2 < 1)
	      break;
      }
}

static void
dijkstra_enqueue (RoutingHeapPtr heap, RoutingNodePtr node)
{
/* enqueuing a Node into the heap */
    dijkstra_insert (node, heap->Nodes, heap->Count);
    heap->Count += 1;
}

static void
dijkstra_shiftdown (HeapNodePtr heap, int size, int i)
{
/* rearranging the heap after removing */
    int c;
    HeapNode tmp;
    for (;;)
      {
	  c = i * 2;
	  if (c > size)
	      break;
	  if (c < size)
	    {
		if (heap[c].Distance > heap[c + 1].Distance)
		    ++c;
	    }
	  if (heap[c].Distance < heap[i].Distance)
	    {
		/* swapping two Nodes */
		tmp = heap[c];
		heap[c] = heap[i];
		heap[i] = tmp;
		i = c;
	    }
	  else
	      break;
      }
}

static RoutingNodePtr
dijkstra_remove_min (HeapNodePtr heap, int size)
{
/* removing the min-priority Node from the heap */
    RoutingNodePtr node = heap[1].Node;
    heap[1] = heap[size];
    --size;
    dijkstra_shiftdown (heap, size, 1);
    return node;
}

static RoutingNodePtr
routing_dequeue (RoutingHeapPtr heap)
{
/* dequeuing a Node from the heap */
    RoutingNodePtr node = dijkstra_remove_min (heap->Nodes, heap->Count);
    heap->Count -= 1;
    return node;
}

static NetworkArcPtr *
dijkstra_shortest_path (RoutingNodesPtr e, NetworkNodePtr pfrom,
			NetworkNodePtr pto, int *ll)
{
/* identifying the Shortest Path - Dijkstra's algorithm */
    int from;
    int to;
    int i;
    int k;
    RoutingNodePtr n;
    RoutingNodePtr p_to;
    NetworkArcPtr p_link;
    int cnt;
    NetworkArcPtr *result;
    RoutingHeapPtr heap;
/* setting From/To */
    from = pfrom->InternalIndex;
    to = pto->InternalIndex;
/* initializing the heap */
    heap = routing_heap_init (e->DimLink);
/* initializing the graph */
    for (i = 0; i < e->Dim; i++)
      {
	  n = e->Nodes + i;
	  n->PreviousNode = NULL;
	  n->Arc = NULL;
	  n->Inspected = 0;
	  n->Distance = DBL_MAX;
      }
/* queuing the From node into the heap */
    e->Nodes[from].Distance = 0.0;
    dijkstra_enqueue (heap, e->Nodes + from);
    while (heap->Count > 0)
      {
	  /* Dijsktra loop */
	  n = routing_dequeue (heap);
	  if (n->Id == to)
	    {
		/* destination reached */
		break;
	    }
	  n->Inspected = 1;
	  for (i = 0; i < n->DimTo; i++)
	    {
		p_to = *(n->To + i);
		p_link = *(n->Link + i);
		if (p_to->Inspected == 0)
		  {
		      if (p_to->Distance == DBL_MAX)
			{
			    /* queuing a new node into the heap */
			    p_to->Distance = n->Distance + p_link->Cost;
			    p_to->PreviousNode = n;
			    p_to->Arc = p_link;
			    dijkstra_enqueue (heap, p_to);
			}
		      else if (p_to->Distance > n->Distance + p_link->Cost)
			{
			    /* updating an already inserted node */
			    p_to->Distance = n->Distance + p_link->Cost;
			    p_to->PreviousNode = n;
			    p_to->Arc = p_link;
			}
		  }
	    }
      }
    routing_heap_free (heap);
    cnt = 0;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* counting how many Arcs are into the Shortest Path solution */
	  cnt++;
	  n = n->PreviousNode;
      }
/* allocating the solution */
    result = malloc (sizeof (NetworkArcPtr) * cnt);
    k = cnt - 1;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* inserting an Arc  into the solution */
	  result[k] = n->Arc;
	  n = n->PreviousNode;
	  k--;
      }
    *ll = cnt;
    return (result);
}

/* END of Luigi Costalli Dijkstra Shortest Path implementation */

static RoutingNodePtr *
dijkstra_range_analysis (RoutingNodesPtr e, NetworkNodePtr pfrom,
			 double max_cost, int *ll)
{
/* identifying all Nodes within a given Cost range - Dijkstra's algorithm */
    int from;
    int i;
    RoutingNodePtr p_to;
    RoutingNodePtr n;
    NetworkArcPtr p_link;
    int cnt;
    RoutingNodePtr *result;
    RoutingHeapPtr heap;
/* setting From */
    from = pfrom->InternalIndex;
/* initializing the heap */
    heap = routing_heap_init (e->DimLink);
/* initializing the graph */
    for (i = 0; i < e->Dim; i++)
      {
	  n = e->Nodes + i;
	  n->PreviousNode = NULL;
	  n->Arc = NULL;
	  n->Inspected = 0;
	  n->Distance = DBL_MAX;
      }
/* queuing the From node into the heap */
    e->Nodes[from].Distance = 0.0;
    dijkstra_enqueue (heap, e->Nodes + from);
    while (heap->Count > 0)
      {
	  /* Dijsktra loop */
	  n = routing_dequeue (heap);
	  n->Inspected = 1;
	  for (i = 0; i < n->DimTo; i++)
	    {
		p_to = *(n->To + i);
		p_link = *(n->Link + i);
		if (p_to->Inspected == 0)
		  {
		      if (p_to->Distance == DBL_MAX)
			{
			    /* queuing a new node into the heap */
			    if (n->Distance + p_link->Cost <= max_cost)
			      {
				  p_to->Distance = n->Distance + p_link->Cost;
				  p_to->PreviousNode = n;
				  p_to->Arc = p_link;
				  dijkstra_enqueue (heap, p_to);
			      }
			}
		      else if (p_to->Distance > n->Distance + p_link->Cost)
			{
			    /* updating an already inserted node */
			    p_to->Distance = n->Distance + p_link->Cost;
			    p_to->PreviousNode = n;
			    p_to->Arc = p_link;
			}
		  }
	    }
      }
    routing_heap_free (heap);
    cnt = 0;
    for (i = 0; i < e->Dim; i++)
      {
	  /* counting how many traversed Nodes */
	  n = e->Nodes + i;
	  if (n->Inspected)
	      cnt++;
      }
/* allocating the solution */
    result = malloc (sizeof (NetworkNodePtr) * cnt);
    cnt = 0;
    for (i = 0; i < e->Dim; i++)
      {
	  /* populating the resultset */
	  n = e->Nodes + i;
	  if (n->Inspected)
	      result[cnt++] = n;
      }
    *ll = cnt;
    return (result);
}

/*
/
/  implementation of the A* Shortest Path algorithm
/
*/

static void
astar_insert (RoutingNodePtr node, HeapNodePtr heap, int size)
{
/* inserting a new Node and rearranging the heap */
    int i;
    HeapNode tmp;
    i = size + 1;
    heap[i].Node = node;
    heap[i].Distance = node->HeuristicDistance;
    if (i / 2 < 1)
	return;
    while (heap[i].Distance < heap[i / 2].Distance)
      {
	  tmp = heap[i];
	  heap[i] = heap[i / 2];
	  heap[i / 2] = tmp;
	  i /= 2;
	  if (i / 2 < 1)
	      break;
      }
}

static void
astar_enqueue (RoutingHeapPtr heap, RoutingNodePtr node)
{
/* enqueuing a Node into the heap */
    astar_insert (node, heap->Nodes, heap->Count);
    heap->Count += 1;
}

static double
astar_heuristic_distance (NetworkNodePtr n1, NetworkNodePtr n2, double coeff)
{
/* computing the euclidean distance intercurring between two nodes */
    double dx = n1->CoordX - n2->CoordX;
    double dy = n1->CoordY - n2->CoordY;
    double dist = sqrt ((dx * dx) + (dy * dy)) * coeff;
    return dist;
}

static NetworkArcPtr *
astar_shortest_path (RoutingNodesPtr e, NetworkNodePtr nodes,
		     NetworkNodePtr pfrom, NetworkNodePtr pto,
		     double heuristic_coeff, int *ll)
{
/* identifying the Shortest Path - A* algorithm */
    int from;
    int to;
    int i;
    int k;
    RoutingNodePtr pAux;
    RoutingNodePtr n;
    RoutingNodePtr p_to;
    NetworkNodePtr pOrg;
    NetworkNodePtr pDest;
    NetworkArcPtr p_link;
    int cnt;
    NetworkArcPtr *result;
    RoutingHeapPtr heap;
/* setting From/To */
    from = pfrom->InternalIndex;
    to = pto->InternalIndex;
    pAux = e->Nodes + from;
    pOrg = nodes + pAux->Id;
    pAux = e->Nodes + to;
    pDest = nodes + pAux->Id;
/* initializing the heap */
    heap = routing_heap_init (e->DimLink);
/* initializing the graph */
    for (i = 0; i < e->Dim; i++)
      {
	  n = e->Nodes + i;
	  n->PreviousNode = NULL;
	  n->Arc = NULL;
	  n->Inspected = 0;
	  n->Distance = DBL_MAX;
	  n->HeuristicDistance = DBL_MAX;
      }
/* queuing the From node into the heap */
    e->Nodes[from].Distance = 0.0;
    e->Nodes[from].HeuristicDistance =
	astar_heuristic_distance (pOrg, pDest, heuristic_coeff);
    astar_enqueue (heap, e->Nodes + from);
    while (heap->Count > 0)
      {
	  /* A* loop */
	  n = routing_dequeue (heap);
	  if (n->Id == to)
	    {
		/* destination reached */
		break;
	    }
	  n->Inspected = 1;
	  for (i = 0; i < n->DimTo; i++)
	    {
		p_to = *(n->To + i);
		p_link = *(n->Link + i);
		if (p_to->Inspected == 0)
		  {
		      if (p_to->Distance == DBL_MAX)
			{
			    /* queuing a new node into the heap */
			    p_to->Distance = n->Distance + p_link->Cost;
			    pOrg = nodes + p_to->Id;
			    p_to->HeuristicDistance =
				p_to->Distance +
				astar_heuristic_distance (pOrg, pDest,
							  heuristic_coeff);
			    p_to->PreviousNode = n;
			    p_to->Arc = p_link;
			    astar_enqueue (heap, p_to);
			}
		      else if (p_to->Distance > n->Distance + p_link->Cost)
			{
			    /* updating an already inserted node */
			    p_to->Distance = n->Distance + p_link->Cost;
			    pOrg = nodes + p_to->Id;
			    p_to->HeuristicDistance =
				p_to->Distance +
				astar_heuristic_distance (pOrg, pDest,
							  heuristic_coeff);
			    p_to->PreviousNode = n;
			    p_to->Arc = p_link;
			}
		  }
	    }
      }
    routing_heap_free (heap);
    cnt = 0;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* counting how many Arcs are into the Shortest Path solution */
	  cnt++;
	  n = n->PreviousNode;
      }
/* allocating the solution */
    result = malloc (sizeof (NetworkArcPtr) * cnt);
    k = cnt - 1;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* inserting an Arc  into the solution */
	  result[k] = n->Arc;
	  n = n->PreviousNode;
	  k--;
      }
    *ll = cnt;
    return (result);
}

/* END of A* Shortest Path implementation */

static int
cmp_nodes_code (const void *p1, const void *p2)
{
/* compares two nodes  by CODE [for BSEARCH] */
    NetworkNodePtr pN1 = (NetworkNodePtr) p1;
    NetworkNodePtr pN2 = (NetworkNodePtr) p2;
    return strcmp (pN1->Code, pN2->Code);
}

static int
cmp_nodes_id (const void *p1, const void *p2)
{
/* compares two nodes  by ID [for BSEARCH] */
    NetworkNodePtr pN1 = (NetworkNodePtr) p1;
    NetworkNodePtr pN2 = (NetworkNodePtr) p2;
    if (pN1->Id == pN2->Id)
	return 0;
    if (pN1->Id > pN2->Id)
	return 1;
    return -1;
}

static NetworkNodePtr
find_node_by_code (NetworkPtr graph, const char *code)
{
/* searching a Node (by Code) into the sorted list */
    NetworkNodePtr ret;
    NetworkNode pN;
    pN.Code = (char *) code;
    ret =
	bsearch (&pN, graph->Nodes, graph->NumNodes, sizeof (NetworkNode),
		 cmp_nodes_code);
    return ret;
}

static NetworkNodePtr
find_node_by_id (NetworkPtr graph, const sqlite3_int64 id)
{
/* searching a Node (by Id) into the sorted list */
    NetworkNodePtr ret;
    NetworkNode pN;
    pN.Id = id;
    ret =
	bsearch (&pN, graph->Nodes, graph->NumNodes, sizeof (NetworkNode),
		 cmp_nodes_id);
    return ret;
}

static void
delete_solution (SolutionPtr solution)
{
/* deleting the current solution */
    ArcSolutionPtr pA;
    ArcSolutionPtr pAn;
    RowSolutionPtr pR;
    RowSolutionPtr pRn;
    RowNodeSolutionPtr pN;
    RowNodeSolutionPtr pNn;
    if (!solution)
	return;
    pA = solution->FirstArc;
    while (pA)
      {
	  pAn = pA->Next;
	  if (pA->FromCode)
	      free (pA->FromCode);
	  if (pA->ToCode)
	      free (pA->ToCode);
	  if (pA->Coords)
	      free (pA->Coords);
	  if (pA->Name)
	      free (pA->Name);
	  free (pA);
	  pA = pAn;
      }
    pR = solution->First;
    while (pR)
      {
	  pRn = pR->Next;
	  if (pR->Name)
	      free (pR->Name);
	  free (pR);
	  pR = pRn;
      }
    pN = solution->FirstNode;
    while (pN)
      {
	  pNn = pN->Next;
	  free (pN);
	  pN = pNn;
      }
    if (solution->Geometry)
	gaiaFreeGeomColl (solution->Geometry);
    free (solution);
}

static void
reset_solution (SolutionPtr solution)
{
/* resetting the current solution */
    ArcSolutionPtr pA;
    ArcSolutionPtr pAn;
    RowSolutionPtr pR;
    RowSolutionPtr pRn;
    RowNodeSolutionPtr pN;
    RowNodeSolutionPtr pNn;
    if (!solution)
	return;
    pA = solution->FirstArc;
    while (pA)
      {
	  pAn = pA->Next;
	  if (pA->FromCode)
	      free (pA->FromCode);
	  if (pA->ToCode)
	      free (pA->ToCode);
	  if (pA->Coords)
	      free (pA->Coords);
	  free (pA);
	  pA = pAn;
      }
    pR = solution->First;
    while (pR)
      {
	  pRn = pR->Next;
	  if (pR->Name)
	      free (pR->Name);
	  free (pR);
	  pR = pRn;
      }
    pN = solution->FirstNode;
    while (pN)
      {
	  pNn = pN->Next;
	  free (pN);
	  pN = pNn;
      }
    if (solution->Geometry)
	gaiaFreeGeomColl (solution->Geometry);
    solution->FirstArc = NULL;
    solution->LastArc = NULL;
    solution->From = NULL;
    solution->To = NULL;
    solution->MaxCost = 0.0;
    solution->First = NULL;
    solution->Last = NULL;
    solution->CurrentRow = NULL;
    solution->CurrentNodeRow = NULL;
    solution->CurrentRowId = 0;
    solution->TotalCost = 0.0;
    solution->Geometry = NULL;
}

static SolutionPtr
alloc_solution (void)
{
/* allocates and initializes the current solution */
    SolutionPtr p = malloc (sizeof (Solution));
    p->FirstArc = NULL;
    p->LastArc = NULL;
    p->From = NULL;
    p->To = NULL;
    p->MaxCost = 0.0;
    p->First = NULL;
    p->Last = NULL;
    p->FirstNode = NULL;
    p->LastNode = NULL;
    p->CurrentRow = NULL;
    p->CurrentNodeRow = NULL;
    p->CurrentRowId = 0;
    p->TotalCost = 0.0;
    p->Geometry = NULL;
    return p;
}

static void
add_arc_to_solution (SolutionPtr solution, NetworkArcPtr arc)
{
/* inserts an Arc into the Shortest Path solution */
    RowSolutionPtr p = malloc (sizeof (RowSolution));
    p->Arc = arc;
    p->Name = NULL;
    p->Next = NULL;
    solution->TotalCost += arc->Cost;
    if (!(solution->First))
	solution->First = p;
    if (solution->Last)
	solution->Last->Next = p;
    solution->Last = p;
}

static void
add_node_to_solution (SolutionPtr solution, RoutingNodePtr node, int srid)
{
/* inserts a Node into the "within Cost range" solution */
    RowNodeSolutionPtr p = malloc (sizeof (RowNodeSolution));
    p->Node = node->Node;
    p->Cost = node->Distance;
    p->Srid = srid;
    p->Next = NULL;
    if (!(solution->FirstNode))
	solution->FirstNode = p;
    if (solution->LastNode)
	solution->LastNode->Next = p;
    solution->LastNode = p;
}

static void
set_arc_name_into_solution (SolutionPtr solution, sqlite3_int64 arc_id,
			    const char *name)
{
/* sets the Name identifyin an Arc into the Solution */
    RowSolutionPtr row = solution->First;
    while (row != NULL)
      {
	  if (row->Arc->ArcRowid == arc_id)
	    {
		int len = strlen (name);
		if (row->Name != NULL)
		    free (row->Name);
		row->Name = malloc (len + 1);
		strcpy (row->Name, name);
		return;
	    }
	  row = row->Next;
      }
}

static void
add_arc_geometry_to_solution (SolutionPtr solution, sqlite3_int64 arc_id,
			      const char *from_code, const char *to_code,
			      sqlite3_int64 from_id, sqlite3_int64 to_id,
			      int points, double *coords, int srid,
			      const char *name)
{
/* inserts an Arc Geometry into the Shortest Path solution */
    int len;
    ArcSolutionPtr p = malloc (sizeof (ArcSolution));
    p->ArcRowid = arc_id;
    p->FromCode = NULL;
    len = strlen (from_code);
    if (len > 0)
      {
	  p->FromCode = malloc (len + 1);
	  strcpy (p->FromCode, from_code);
      }
    p->ToCode = NULL;
    len = strlen (to_code);
    if (len > 0)
      {
	  p->ToCode = malloc (len + 1);
	  strcpy (p->ToCode, to_code);
      }
    p->FromId = from_id;
    p->ToId = to_id;
    p->Points = points;
    p->Coords = coords;
    p->Srid = srid;
    if (name == NULL)
	p->Name = NULL;
    else
      {
	  len = strlen (name);
	  p->Name = malloc (len + 1);
	  strcpy (p->Name, name);
      }
    p->Next = NULL;
    if (!(solution->FirstArc))
	solution->FirstArc = p;
    if (solution->LastArc)
	solution->LastArc->Next = p;
    solution->LastArc = p;
}

static void
build_solution (sqlite3 * handle, NetworkPtr graph, SolutionPtr solution,
		NetworkArcPtr * shortest_path, int cnt)
{
/* formatting the Shortest Path solution */
    int i;
    char *sql;
    int err;
    int error = 0;
    int ret;
    sqlite3_int64 arc_id;
    const unsigned char *blob;
    int size;
    sqlite3_int64 from_id;
    sqlite3_int64 to_id;
    char *from_code;
    char *to_code;
    char *name;
    int tbd;
    int ind;
    int base = 0;
    int block = 128;
    int how_many;
    sqlite3_stmt *stmt;
    char *xfrom;
    char *xto;
    char *xgeom;
    char *xname;
    char *xtable;
    gaiaOutBuffer sql_statement;
    if (cnt > 0)
      {
	  /* building the solution */
	  for (i = 0; i < cnt; i++)
	    {
		add_arc_to_solution (solution, shortest_path[i]);
	    }
      }

    if (graph->GeometryColumn == NULL && graph->NameColumn == NULL)
      {
	  /* completely skipping Geometry */
	  return;
      }

    tbd = cnt;
    while (tbd > 0)
      {
	  /* requesting max 128 arcs at each time */
	  if (tbd < block)
	      how_many = tbd;
	  else
	      how_many = block;
/* preparing the Geometry representing this solution [reading arcs] */
	  gaiaOutBufferInitialize (&sql_statement);
	  if (graph->NameColumn)
	    {
		/* a Name column is defined */
		if (graph->GeometryColumn == NULL)
		  {
		      xfrom = gaiaDoubleQuotedSql (graph->FromColumn);
		      xto = gaiaDoubleQuotedSql (graph->ToColumn);
		      xname = gaiaDoubleQuotedSql (graph->NameColumn);
		      xtable = gaiaDoubleQuotedSql (graph->TableName);
		      sql =
			  sqlite3_mprintf
			  ("SELECT ROWID, \"%s\", \"%s\", NULL, \"%s\" FROM \"%s\" WHERE ROWID IN (",
			   xfrom, xto, xname, xtable);
		      free (xfrom);
		      free (xto);
		      free (xname);
		      free (xtable);
		  }
		else
		  {
		      xfrom = gaiaDoubleQuotedSql (graph->FromColumn);
		      xto = gaiaDoubleQuotedSql (graph->ToColumn);
		      xgeom = gaiaDoubleQuotedSql (graph->GeometryColumn);
		      xname = gaiaDoubleQuotedSql (graph->NameColumn);
		      xtable = gaiaDoubleQuotedSql (graph->TableName);
		      sql =
			  sqlite3_mprintf
			  ("SELECT ROWID, \"%s\", \"%s\", \"%s\", \"%s\" FROM \"%s\" WHERE ROWID IN (",
			   xfrom, xto, xgeom, xname, xtable);
		      free (xfrom);
		      free (xto);
		      free (xgeom);
		      free (xname);
		      free (xtable);
		  }
		gaiaAppendToOutBuffer (&sql_statement, sql);
		sqlite3_free (sql);
	    }
	  else
	    {
		/* no Name column is defined */
		xfrom = gaiaDoubleQuotedSql (graph->FromColumn);
		xto = gaiaDoubleQuotedSql (graph->ToColumn);
		xgeom = gaiaDoubleQuotedSql (graph->GeometryColumn);
		xtable = gaiaDoubleQuotedSql (graph->TableName);
		sql =
		    sqlite3_mprintf
		    ("SELECT ROWID, \"%s\", \"%s\", \"%s\" FROM \"%s\" WHERE ROWID IN (",
		     xfrom, xto, xgeom, xtable);
		free (xfrom);
		free (xto);
		free (xgeom);
		free (xtable);
		gaiaAppendToOutBuffer (&sql_statement, sql);
		sqlite3_free (sql);
	    }
	  for (i = 0; i < how_many; i++)
	    {
		if (i == 0)
		    gaiaAppendToOutBuffer (&sql_statement, "?");
		else
		    gaiaAppendToOutBuffer (&sql_statement, ",?");
	    }
	  gaiaAppendToOutBuffer (&sql_statement, ")");
	  if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
	      ret =
		  sqlite3_prepare_v2 (handle, sql_statement.Buffer,
				      strlen (sql_statement.Buffer), &stmt,
				      NULL);
	  else
	      ret = SQLITE_ERROR;
	  gaiaOutBufferReset (&sql_statement);
	  if (ret != SQLITE_OK)
	    {
		error = 1;
		goto abort;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  ind = 1;
	  for (i = 0; i < cnt; i++)
	    {
		if (i < base)
		    continue;
		if (i >= (base + how_many))
		    break;
		sqlite3_bind_int64 (stmt, ind, shortest_path[i]->ArcRowid);
		ind++;
	    }
	  while (1)
	    {
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;
		if (ret == SQLITE_ROW)
		  {
		      arc_id = -1;
		      from_id = -1;
		      to_id = -1;
		      from_code = NULL;
		      to_code = NULL;
		      blob = NULL;
		      size = 0;
		      name = NULL;
		      err = 0;
		      if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
			  arc_id = sqlite3_column_int64 (stmt, 0);
		      else
			  err = 1;
		      if (graph->NodeCode)
			{
			    /* nodes are identified by TEXT codes */
			    if (sqlite3_column_type (stmt, 1) == SQLITE_TEXT)
				from_code =
				    (char *) sqlite3_column_text (stmt, 1);
			    else
				err = 1;
			    if (sqlite3_column_type (stmt, 2) == SQLITE_TEXT)
				to_code =
				    (char *) sqlite3_column_text (stmt, 2);
			    else
				err = 1;
			}
		      else
			{
			    /* nodes are identified by INTEGER ids */
			    if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
				from_id = sqlite3_column_int64 (stmt, 1);
			    else
				err = 1;
			    if (sqlite3_column_type (stmt, 2) == SQLITE_INTEGER)
				to_id = sqlite3_column_int64 (stmt, 2);
			    else
				err = 1;
			}
		      if (graph->GeometryColumn != NULL)
			{
			    if (sqlite3_column_type (stmt, 3) == SQLITE_BLOB)
			      {
				  blob =
				      (const unsigned char *)
				      sqlite3_column_blob (stmt, 3);
				  size = sqlite3_column_bytes (stmt, 3);
			      }
			    else
				err = 1;
			}
		      if (graph->NameColumn)
			{
			    if (sqlite3_column_type (stmt, 4) == SQLITE_TEXT)
				name = (char *) sqlite3_column_text (stmt, 4);
			}
		      if (err)
			  error = 1;
		      else if (graph->GeometryColumn != NULL)
			{
			    /* saving the Arc geometry into the temporary struct */
			    gaiaGeomCollPtr geom =
				gaiaFromSpatiaLiteBlobWkb (blob, size);
			    if (geom)
			      {
				  /* OK, we have fetched a valid Geometry */
				  if (geom->FirstPoint == NULL
				      && geom->FirstPolygon == NULL
				      && geom->FirstLinestring != NULL
				      && geom->FirstLinestring ==
				      geom->LastLinestring)
				    {
					/* Geometry is LINESTRING as expected */
					int iv;
					int points =
					    geom->FirstLinestring->Points;
					double *coords =
					    malloc (sizeof (double) *
						    (points * 2));
					for (iv = 0; iv < points; iv++)
					  {
					      double x;
					      double y;
					      gaiaGetPoint
						  (geom->
						   FirstLinestring->Coords, iv,
						   &x, &y);
					      *(coords + ((iv * 2) + 0)) = x;
					      *(coords + ((iv * 2) + 1)) = y;
					  }
					if (from_code == NULL)
					    from_code = "";
					if (to_code == NULL)
					    to_code = "";
					add_arc_geometry_to_solution (solution,
								      arc_id,
								      from_code,
								      to_code,
								      from_id,
								      to_id,
								      points,
								      coords,
								      geom->Srid,
								      name);
				    }
				  else
				      error = 1;
				  gaiaFreeGeomColl (geom);
			      }
			    else
				error = 1;
			}
		      else if (name != NULL)
			  set_arc_name_into_solution (solution, arc_id, name);
		  }
	    }
	  sqlite3_finalize (stmt);
	  tbd -= how_many;
	  base += how_many;
      }
  abort:
    if (shortest_path)
	free (shortest_path);
    if (!error && graph->GeometryColumn != NULL)
      {
	  /* building the Geometry representing the Shortest Path Solution */
	  gaiaLinestringPtr ln;
	  int tot_pts = 0;
	  RowSolutionPtr pR;
	  ArcSolutionPtr pA;
	  int srid = -1;
	  if (solution->FirstArc)
	      srid = (solution->FirstArc)->Srid;
	  pR = solution->First;
	  while (pR)
	    {
		pA = solution->FirstArc;
		while (pA)
		  {
		      /* computing how many vertices do we need to build the LINESTRING */
		      if (pR->Arc->ArcRowid == pA->ArcRowid)
			{
			    if (pR == solution->First)
				tot_pts += pA->Points;
			    else
				tot_pts += (pA->Points - 1);
			    if (pA->Srid != srid)
				srid = -1;
			}
		      pA = pA->Next;
		  }
		pR = pR->Next;
	    }
	  /* creating the Shortest Path Geometry - LINESTRING */
	  ln = gaiaAllocLinestring (tot_pts);
	  solution->Geometry = gaiaAllocGeomColl ();
	  solution->Geometry->Srid = srid;
	  gaiaInsertLinestringInGeomColl (solution->Geometry, ln);
	  tot_pts = 0;
	  pR = solution->First;
	  while (pR)
	    {
		/* building the LINESTRING */
		int skip;
		if (pR == solution->First)
		    skip = 0;	/* for first arc we must copy any vertex */
		else
		    skip = 1;	/* for subsequent arcs we must skip first vertex [already inserted from previous arc] */
		pA = solution->FirstArc;
		while (pA)
		  {
		      if (pR->Arc->ArcRowid == pA->ArcRowid)
			{
			    /* copying vertices from correspoinding Arc Geometry */
			    int ini;
			    int iv;
			    int rev;
			    double x;
			    double y;
			    if (graph->NodeCode)
			      {
				  /* nodes are identified by TEXT codes */
				  if (strcmp
				      (pR->Arc->NodeFrom->Code,
				       pA->ToCode) == 0)
				      rev = 1;
				  else
				      rev = 0;
			      }
			    else
			      {
				  /* nodes are identified by INTEGER ids */
				  if (pR->Arc->NodeFrom->Id == pA->ToId)
				      rev = 1;
				  else
				      rev = 0;
			      }
			    if (rev)
			      {
				  /* copying Arc vertices in reverse order */
				  if (skip)
				      ini = pA->Points - 2;
				  else
				      ini = pA->Points - 1;
				  for (iv = ini; iv >= 0; iv--)
				    {
					x = *(pA->Coords + ((iv * 2) + 0));
					y = *(pA->Coords + ((iv * 2) + 1));
					gaiaSetPoint (ln->Coords, tot_pts, x,
						      y);
					tot_pts++;
				    }
			      }
			    else
			      {
				  /* copying Arc vertices in normal order */
				  if (skip)
				      ini = 1;
				  else
				      ini = 0;
				  for (iv = ini; iv < pA->Points; iv++)
				    {
					x = *(pA->Coords + ((iv * 2) + 0));
					y = *(pA->Coords + ((iv * 2) + 1));
					gaiaSetPoint (ln->Coords, tot_pts, x,
						      y);
					tot_pts++;
				    }
			      }
			    if (pA->Name)
			      {
				  int len = strlen (pA->Name);
				  pR->Name = malloc (len + 1);
				  strcpy (pR->Name, pA->Name);
			      }
			    break;
			}
		      pA = pA->Next;
		  }
		pR = pR->Next;
	    }
      }
}

static int
find_srid (sqlite3 * handle, NetworkPtr graph)
{
/* attempting to retrieve the appropriate Srid */
    sqlite3_stmt *stmt;
    int ret;
    int srid = VNET_INVALID_SRID;
    char *sql;

    if (graph->GeometryColumn == NULL)
	return srid;

    sql = sqlite3_mprintf ("SELECT srid FROM geometry_columns WHERE "
			   "Lower(f_table_name) = Lower(%Q) AND Lower(f_geometry_column) = Lower(%Q)",
			   graph->TableName, graph->GeometryColumn);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
	return srid;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	      srid = sqlite3_column_int (stmt, 0);
      }
    sqlite3_finalize (stmt);
    return srid;
}

static void
build_range_solution (SolutionPtr solution,
		      RoutingNodePtr * range_nodes, int cnt, int srid)
{
/* formatting the "within Cost range" solution */
    int i;
    if (cnt > 0)
      {
	  /* building the solution */
	  for (i = 0; i < cnt; i++)
	    {
		add_node_to_solution (solution, range_nodes[i], srid);
	    }
      }
    if (range_nodes)
	free (range_nodes);
}

static void
dijkstra_solve (sqlite3 * handle, NetworkPtr graph, RoutingNodesPtr routing,
		SolutionPtr solution)
{
/* computing a Dijkstra Shortest Path solution */
    int cnt;
    NetworkArcPtr *shortest_path =
	dijkstra_shortest_path (routing, solution->From, solution->To, &cnt);
    build_solution (handle, graph, solution, shortest_path, cnt);
}

static void
astar_solve (sqlite3 * handle, NetworkPtr graph, RoutingNodesPtr routing,
	     SolutionPtr solution)
{
/* computing an A* Shortest Path solution */
    int cnt;
    NetworkArcPtr *shortest_path =
	astar_shortest_path (routing, graph->Nodes, solution->From,
			     solution->To, graph->AStarHeuristicCoeff, &cnt);
    build_solution (handle, graph, solution, shortest_path, cnt);
}

static void
dijkstra_within_cost_range (RoutingNodesPtr routing, SolutionPtr solution,
			    int srid)
{
/* computing a Dijkstra "within cost range" solution */
    int cnt;
    RoutingNodePtr *range_nodes =
	dijkstra_range_analysis (routing, solution->From, solution->MaxCost,
				 &cnt);
    build_range_solution (solution, range_nodes, cnt, srid);
}

static void
network_free (NetworkPtr p)
{
/* memory cleanup; freeing any allocation for the network struct */
    NetworkNodePtr pN;
    int i;
    if (!p)
	return;
    for (i = 0; i < p->NumNodes; i++)
      {
	  pN = p->Nodes + i;
	  if (pN->Code)
	      free (pN->Code);
	  if (pN->Arcs)
	      free (pN->Arcs);
      }
    if (p->Nodes)
	free (p->Nodes);
    if (p->TableName)
	free (p->TableName);
    if (p->FromColumn)
	free (p->FromColumn);
    if (p->ToColumn)
	free (p->ToColumn);
    if (p->GeometryColumn)
	free (p->GeometryColumn);
    if (p->NameColumn)
	free (p->NameColumn);
    free (p);
}

static NetworkPtr
network_init (const unsigned char *blob, int size)
{
/* parsing the HEADER block */
    NetworkPtr graph;
    int net64;
    int aStar = 0;
    int nodes;
    int node_code;
    int max_code_length;
    int endian_arch = gaiaEndianArch ();
    const char *table;
    const char *from;
    const char *to;
    const char *geom;
    const char *name = NULL;
    double a_star_coeff = 1.0;
    int len;
    const unsigned char *ptr;
    if (size < 9)
	return NULL;
    if (*(blob + 0) == GAIA_NET_START)	/* signature - legacy format using 32bit ints */
	net64 = 0;
    else if (*(blob + 0) == GAIA_NET64_START)	/* signature - format using 64bit ints */
	net64 = 1;
    else if (*(blob + 0) == GAIA_NET64_A_STAR_START)	/* signature - format using 64bit ints AND supporting A* */
      {
	  net64 = 1;
	  aStar = 1;
      }
    else
	return NULL;
    if (*(blob + 1) != GAIA_NET_HEADER)	/* signature */
	return NULL;
    nodes = gaiaImport32 (blob + 2, 1, endian_arch);	/* # nodes */
    if (nodes <= 0)
	return NULL;
    if (*(blob + 6) == GAIA_NET_CODE)	/* Nodes identified by a TEXT code */
	node_code = 1;
    else if (*(blob + 6) == GAIA_NET_ID)	/* Nodes indentified by an INTEGER id */
	node_code = 0;
    else
	return NULL;
    max_code_length = *(blob + 7);	/* Max TEXT Code length */
    if (*(blob + 8) != GAIA_NET_TABLE)	/* signature for TABLE NAME */
	return NULL;
    ptr = blob + 9;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* TABLE NAME is varlen */
    ptr += 2;
    table = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_FROM)	/* signature for FromNode COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* FromNode COLUMN is varlen */
    ptr += 2;
    from = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_TO)	/* signature for ToNode COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* ToNode COLUMN is varlen */
    ptr += 2;
    to = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_GEOM)	/* signature for Geometry COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* Geometry COLUMN is varlen */
    ptr += 2;
    geom = (char *) ptr;
    ptr += len;
    if (net64)
      {
	  if (*ptr != GAIA_NET_NAME)	/* signature for Name COLUMN - may be empty */
	      return NULL;
	  ptr++;
	  len = gaiaImport16 (ptr, 1, endian_arch);	/* Name COLUMN is varlen */
	  ptr += 2;
	  name = (char *) ptr;
	  ptr += len;
      }
    if (net64 && aStar)
      {
	  if (*ptr != GAIA_NET_A_STAR_COEFF)	/* signature for A* Heuristic Coeff */
	      return NULL;
	  ptr++;
	  a_star_coeff = gaiaImport64 (ptr, 1, endian_arch);
	  ptr += 8;
      }
    if (*ptr != GAIA_NET_END)	/* signature */
	return NULL;
    graph = malloc (sizeof (Network));
    graph->Net64 = net64;
    graph->AStar = aStar;
    graph->EndianArch = endian_arch;
    graph->CurrentIndex = 0;
    graph->NodeCode = node_code;
    graph->MaxCodeLength = max_code_length;
    graph->NumNodes = nodes;
    graph->Nodes = malloc (sizeof (NetworkNode) * nodes);
    len = strlen (table);
    graph->TableName = malloc (len + 1);
    strcpy (graph->TableName, table);
    len = strlen (from);
    graph->FromColumn = malloc (len + 1);
    strcpy (graph->FromColumn, from);
    len = strlen (to);
    graph->ToColumn = malloc (len + 1);
    strcpy (graph->ToColumn, to);
    len = strlen (geom);
    if (len <= 1)
	graph->GeometryColumn = NULL;
    else
      {
	  graph->GeometryColumn = malloc (len + 1);
	  strcpy (graph->GeometryColumn, geom);
      }
    if (!net64)
      {
	  /* Name column is not supported */
	  graph->NameColumn = NULL;
      }
    else
      {
	  len = strlen (name);
	  if (len <= 1)
	      graph->NameColumn = NULL;
	  else
	    {
		graph->NameColumn = malloc (len + 1);
		strcpy (graph->NameColumn, name);
	    }
      }
    graph->AStarHeuristicCoeff = a_star_coeff;
    return graph;
}

static int
network_block (NetworkPtr graph, const unsigned char *blob, int size)
{
/* parsing a NETWORK Block */
    const unsigned char *in = blob;
    int nodes;
    int i;
    int ia;
    int index;
    char code[256];
    double x;
    double y;
    sqlite3_int64 nodeId = -1;
    int arcs;
    NetworkNodePtr pN;
    NetworkArcPtr pA;
    int len;
    sqlite3_int64 arcId;
    int nodeToIdx;
    double cost;
    if (size < 3)
	goto error;
    if (*in++ != GAIA_NET_BLOCK)	/* signature */
	goto error;
    nodes = gaiaImport16 (in, 1, graph->EndianArch);	/* # Nodes */
    in += 2;
    for (i = 0; i < nodes; i++)
      {
	  /* parsing each node */
	  if ((size - (in - blob)) < 5)
	      goto error;
	  if (*in++ != GAIA_NET_NODE)	/* signature */
	      goto error;
	  index = gaiaImport32 (in, 1, graph->EndianArch);	/* node internal index */
	  in += 4;
	  if (index < 0 || index >= graph->NumNodes)
	      goto error;
	  if (graph->NodeCode)
	    {
		/* Nodes are identified by a TEXT Code */
		if ((size - (in - blob)) < graph->MaxCodeLength)
		    goto error;
		memcpy (code, in, graph->MaxCodeLength);
		in += graph->MaxCodeLength;
	    }
	  else
	    {
		/* Nodes are identified by an INTEGER Id */
		if (graph->Net64)
		  {
		      if ((size - (in - blob)) < 8)
			  goto error;
		      nodeId = gaiaImportI64 (in, 1, graph->EndianArch);	/* the Node ID: 64bit */
		      in += 8;
		  }
		else
		  {
		      if ((size - (in - blob)) < 4)
			  goto error;
		      nodeId = gaiaImport32 (in, 1, graph->EndianArch);	/* the Node ID: 32bit */
		      in += 4;
		  }
	    }
	  if (graph->AStar)
	    {
		/* fetching node's X,Y coords */
		if ((size - (in - blob)) < 8)
		    goto error;
		x = gaiaImport64 (in, 1, graph->EndianArch);	/* X coord */
		in += 8;
		if ((size - (in - blob)) < 8)
		    goto error;
		y = gaiaImport64 (in, 1, graph->EndianArch);	/* Y coord */
		in += 8;
	    }
	  else
	    {
		x = DBL_MAX;
		y = DBL_MAX;
	    }
	  if ((size - (in - blob)) < 2)
	      goto error;
	  arcs = gaiaImport16 (in, 1, graph->EndianArch);	/* # Arcs */
	  in += 2;
	  if (arcs < 0)
	      goto error;
	  /* initializing the Node */
	  pN = graph->Nodes + index;
	  pN->InternalIndex = index;
	  if (graph->NodeCode)
	    {
		/* Nodes are identified by a TEXT Code */
		pN->Id = -1;
		len = strlen (code);
		pN->Code = malloc (len + 1);
		strcpy (pN->Code, code);
	    }
	  else
	    {
		/* Nodes are identified by an INTEGER Id */
		pN->Id = nodeId;
		pN->Code = NULL;
	    }
	  pN->CoordX = x;
	  pN->CoordY = y;
	  pN->NumArcs = arcs;
	  if (arcs)
	    {
		/* parsing the Arcs */
		pN->Arcs = malloc (sizeof (NetworkArc) * arcs);
		for (ia = 0; ia < arcs; ia++)
		  {
		      /* parsing each Arc */
		      if (graph->Net64)
			{
			    if ((size - (in - blob)) < 22)
				goto error;
			}
		      else
			{
			    if ((size - (in - blob)) < 18)
				goto error;
			}
		      if (*in++ != GAIA_NET_ARC)	/* signature */
			  goto error;
		      if (graph->Net64)
			{
			    arcId = gaiaImportI64 (in, 1, graph->EndianArch);	/* # Arc ROWID: 64bit */
			    in += 8;
			}
		      else
			{
			    arcId = gaiaImport32 (in, 1, graph->EndianArch);	/* # Arc ROWID: 32bit */
			    in += 4;
			}
		      nodeToIdx = gaiaImport32 (in, 1, graph->EndianArch);	/* # NodeTo internal index */
		      in += 4;
		      cost = gaiaImport64 (in, 1, graph->EndianArch);	/* # Cost */
		      in += 8;
		      if (*in++ != GAIA_NET_END)	/* signature */
			  goto error;
		      pA = pN->Arcs + ia;
		      /* initializing the Arc */
		      if (nodeToIdx < 0 || nodeToIdx >= graph->NumNodes)
			  goto error;
		      pA->NodeFrom = pN;
		      pA->NodeTo = graph->Nodes + nodeToIdx;
		      pA->ArcRowid = arcId;
		      pA->Cost = cost;
		  }
	    }
	  else
	      pN->Arcs = NULL;
	  if ((size - (in - blob)) < 1)
	      goto error;
	  if (*in++ != GAIA_NET_END)	/* signature */
	      goto error;
      }
    return 1;
  error:
    return 0;
}

static NetworkPtr
load_network (sqlite3 * handle, const char *table)
{
/* loads the NETWORK struct */
    NetworkPtr graph = NULL;
    sqlite3_stmt *stmt;
    char *sql;
    int ret;
    int header = 1;
    const unsigned char *blob;
    int size;
    char *xname;
    xname = gaiaDoubleQuotedSql (table);
    sql = sqlite3_mprintf ("SELECT NetworkData FROM \"%s\" ORDER BY Id", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto abort;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_BLOB)
		  {
		      blob =
			  (const unsigned char *) sqlite3_column_blob (stmt, 0);
		      size = sqlite3_column_bytes (stmt, 0);
		      if (header)
			{
			    /* parsing the HEADER block */
			    graph = network_init (blob, size);
			    header = 0;
			}
		      else
			{
			    /* parsing ordinary Blocks */
			    if (!graph)
			      {
				  sqlite3_finalize (stmt);
				  goto abort;
			      }
			    if (!network_block (graph, blob, size))
			      {
				  sqlite3_finalize (stmt);
				  goto abort;
			      }
			}
		  }
		else
		  {
		      sqlite3_finalize (stmt);
		      goto abort;
		  }
	    }
	  else
	    {
		sqlite3_finalize (stmt);
		goto abort;
	    }
      }
    sqlite3_finalize (stmt);
    return graph;
  abort:
    network_free (graph);
    return NULL;
}

static int
vnet_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some shapefile */
    VirtualNetworkPtr p_vt;
    int err;
    int ret;
    int i;
    int n_rows;
    int n_columns;
    char *vtable = NULL;
    char *table = NULL;
    const char *col_name = NULL;
    char **results;
    char *err_msg = NULL;
    char *sql;
    int ok_id;
    int ok_data;
    char *xname;
    NetworkPtr graph = NULL;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
/* checking for table_name and geo_column_name */
    if (argc == 4)
      {
	  vtable = gaiaDequotedSql (argv[2]);
	  table = gaiaDequotedSql (argv[3]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] CREATE VIRTUAL: illegal arg list {NETWORK-DATAtable}\n");
	  goto error;
      }
/* retrieving the base table columns */
    err = 0;
    ok_id = 0;
    ok_data = 0;
    xname = gaiaDoubleQuotedSql (table);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  err = 1;
	  goto illegal;
      }
    if (n_rows > 1)
      {
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		if (strcasecmp (col_name, "id") == 0)
		    ok_id = 1;
		if (strcasecmp (col_name, "networkdata") == 0)
		    ok_data = 1;
	    }
	  sqlite3_free_table (results);
	  if (!ok_id)
	      err = 1;
	  if (!ok_data)
	      err = 1;
      }
    else
	err = 1;
  illegal:
    if (err)
      {
	  /* something is going the wrong way */
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] cannot build a valid NETWORK\n");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualNetworkPtr) sqlite3_malloc (sizeof (VirtualNetwork));
    if (!p_vt)
	return SQLITE_NOMEM;
    graph = load_network (db, table);
    if (!graph)
      {
	  /* something is going the wrong way */
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] cannot build a valid NETWORK\n");
	  goto error;
      }
    p_vt->db = db;
    p_vt->graph = graph;
    p_vt->currentAlgorithm = VNET_DIJKSTRA_ALGORITHM;
    p_vt->routing = NULL;
    p_vt->pModule = &my_net_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    xname = gaiaDoubleQuotedSql (vtable);
    if (p_vt->graph->NodeCode)
      {
	  if (p_vt->graph->NameColumn)
	    {
		sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (Algorithm TEXT, "
				       "ArcRowid INTEGER, NodeFrom TEXT, NodeTo TEXT,"
				       " Cost DOUBLE, Geometry BLOB, Name TEXT)",
				       xname);
	    }
	  else
	    {
		sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (Algorithm TEXT, "
				       "ArcRowid INTEGER, NodeFrom TEXT, NodeTo TEXT,"
				       " Cost DOUBLE, Geometry BLOB)", xname);
	    }
      }
    else
      {
	  if (p_vt->graph->NameColumn)
	    {
		sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (Algorithm TEXT, "
				       "ArcRowid INTEGER, NodeFrom INTEGER, NodeTo INTEGER,"
				       " Cost DOUBLE, Geometry BLOB, Name TEXT)",
				       xname);
	    }
	  else
	    {
		sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (Algorithm TEXT, "
				       "ArcRowid INTEGER, NodeFrom INTEGER, NodeTo INTEGER,"
				       " Cost DOUBLE, Geometry BLOB)", xname);
	    }
      }
    free (xname);
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  sqlite3_free (sql);
	  goto error;
      }
    sqlite3_free (sql);
    *ppVTab = (sqlite3_vtab *) p_vt;
    p_vt->routing = routing_init (p_vt->graph);
    free (table);
    free (vtable);
    return SQLITE_OK;
  error:
    if (table)
	free (table);
    if (vtable)
	free (vtable);
    return SQLITE_ERROR;
}

static int
vnet_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vnet_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vnet_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo)
{
/* best index selection */
    int i;
    int errors = 0;
    int err = 1;
    int from = 0;
    int to = 0;
    int cost = 0;
    int i_from = -1;
    int i_to = -1;
    int i_cost = -1;
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    for (i = 0; i < pIdxInfo->nConstraint; i++)
      {
	  /* verifying the constraints */
	  struct sqlite3_index_constraint *p = &(pIdxInfo->aConstraint[i]);
	  if (p->usable)
	    {
		if (p->iColumn == 2 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		  {
		      from++;
		      i_from = i;
		  }
		else if (p->iColumn == 3 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		  {
		      to++;
		      i_to = i;
		  }
		else if (p->iColumn == 4 && p->op == SQLITE_INDEX_CONSTRAINT_LE)
		  {
		      cost++;
		      i_cost = i;
		  }
		else
		    errors++;
	    }
      }
    if (from == 1 && to == 1 && errors == 0)
      {
	  /* this one is a valid Shortest Path query */
	  if (i_from < i_to)
	      pIdxInfo->idxNum = 1;	/* first arg is FROM */
	  else
	      pIdxInfo->idxNum = 2;	/* first arg is TO */
	  pIdxInfo->estimatedCost = 1.0;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		if (pIdxInfo->aConstraint[i].usable)
		  {
		      pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
		      pIdxInfo->aConstraintUsage[i].omit = 1;
		  }
	    }
	  err = 0;
      }
    if (from == 1 && cost == 1 && errors == 0)
      {
	  /* this one is a valid "within cost" query */
	  if (i_from < i_cost)
	      pIdxInfo->idxNum = 3;	/* first arg is FROM */
	  else
	      pIdxInfo->idxNum = 4;	/* first arg is COST */
	  pIdxInfo->estimatedCost = 1.0;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		if (pIdxInfo->aConstraint[i].usable)
		  {
		      pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
		      pIdxInfo->aConstraintUsage[i].omit = 1;
		  }
	    }
	  err = 0;
      }
    if (err)
      {
	  /* illegal query */
	  pIdxInfo->idxNum = 0;
      }
    return SQLITE_OK;
}

static int
vnet_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualNetworkPtr p_vt = (VirtualNetworkPtr) pVTab;
    if (p_vt->routing)
	routing_free (p_vt->routing);
    if (p_vt->graph)
	network_free (p_vt->graph);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vnet_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vshp_disconnect() */
    return vnet_disconnect (pVTab);
}

static void
vnet_read_row (VirtualNetworkCursorPtr cursor)
{
/* trying to read a "row" from Shortest Path solution */
    if (cursor->solution->Mode == VNET_RANGE_SOLUTION)
      {
	  if (cursor->solution->CurrentNodeRow == NULL)
	      cursor->eof = 1;
	  else
	      cursor->eof = 0;
      }
    else
      {
	  if (cursor->solution->CurrentRow == NULL)
	      cursor->eof = 1;
	  else
	      cursor->eof = 0;
      }
    return;
}

static int
vnet_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualNetworkCursorPtr cursor =
	(VirtualNetworkCursorPtr)
	sqlite3_malloc (sizeof (VirtualNetworkCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualNetworkPtr) pVTab;
    cursor->solution = alloc_solution ();
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    return SQLITE_OK;
}

static int
vnet_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    delete_solution (cursor->solution);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vnet_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int node_code = 0;
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    VirtualNetworkPtr net = (VirtualNetworkPtr) cursor->pVtab;
    if (idxStr)
	idxStr = idxStr;	/* unused arg warning suppression */
    node_code = net->graph->NodeCode;
    reset_solution (cursor->solution);
    cursor->eof = 1;
    if (idxNum == 1 && argc == 2)
      {
	  /* retrieving the Shortest Path From/To params */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
		    cursor->solution->To =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[1]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
		    cursor->solution->To =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[1]));
	    }
      }
    if (idxNum == 2 && argc == 2)
      {
	  /* retrieving the Shortest Path To/From params */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
		    cursor->solution->To =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[1]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
		    cursor->solution->To =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[1]));
	    }
      }
    if (idxNum == 3 && argc == 2)
      {
	  /* retrieving the From and Cost param */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[0]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[0]));
	    }
	  if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	    {
		int cost = sqlite3_value_int (argv[1]);
		cursor->solution->MaxCost = cost;
	    }
	  else if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	      cursor->solution->MaxCost = sqlite3_value_double (argv[1]);
      }
    if (idxNum == 4 && argc == 2)
      {
	  /* retrieving the From and Cost param */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[1]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[1]));
	    }
	  if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
	    {
		int cost = sqlite3_value_int (argv[0]);
		cursor->solution->MaxCost = cost;
	    }
	  else if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	      cursor->solution->MaxCost = sqlite3_value_double (argv[0]);
      }
    if (cursor->solution->From && cursor->solution->To)
      {
	  cursor->eof = 0;
	  cursor->solution->Mode = VNET_ROUTING_SOLUTION;
	  if (net->currentAlgorithm == VNET_A_STAR_ALGORITHM)
	      astar_solve (net->db, net->graph, net->routing, cursor->solution);
	  else
	      dijkstra_solve (net->db, net->graph, net->routing,
			      cursor->solution);
	  return SQLITE_OK;
      }
    if (cursor->solution->From && cursor->solution->MaxCost > 0.0)
      {
	  int srid = find_srid (net->db, net->graph);
	  cursor->eof = 0;
	  cursor->solution->Mode = VNET_RANGE_SOLUTION;
	  if (net->currentAlgorithm == VNET_DIJKSTRA_ALGORITHM)
	    {
		dijkstra_within_cost_range (net->routing, cursor->solution,
					    srid);
		cursor->solution->CurrentRowId = 0;
		cursor->solution->CurrentNodeRow = cursor->solution->FirstNode;
	    }
	  return SQLITE_OK;
      }
    cursor->eof = 0;
    cursor->solution->Mode = VNET_ROUTING_SOLUTION;
    return SQLITE_OK;
}

static int
vnet_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    if (cursor->solution->Mode == VNET_RANGE_SOLUTION)
      {
	  cursor->solution->CurrentNodeRow =
	      cursor->solution->CurrentNodeRow->Next;
	  if (!(cursor->solution->CurrentNodeRow))
	    {
		cursor->eof = 1;
		return SQLITE_OK;
	    }
      }
    else
      {
	  if (cursor->solution->CurrentRowId == 0)
	      cursor->solution->CurrentRow = cursor->solution->First;
	  else
	      cursor->solution->CurrentRow = cursor->solution->CurrentRow->Next;
	  if (!(cursor->solution->CurrentRow))
	    {
		cursor->eof = 1;
		return SQLITE_OK;
	    }
      }
    (cursor->solution->CurrentRowId)++;
    vnet_read_row (cursor);
    return SQLITE_OK;
}

static int
vnet_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    return cursor->eof;
}

static int
vnet_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    RowSolutionPtr row;
    RowNodeSolutionPtr row_node;
    int node_code = 0;
    const char *algorithm;
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    VirtualNetworkPtr net = (VirtualNetworkPtr) cursor->pVtab;
    node_code = net->graph->NodeCode;
    if (cursor->solution->Mode == VNET_RANGE_SOLUTION)
      {
	  /* processing "within Cost range" solution */
	  row_node = cursor->solution->CurrentNodeRow;
	  if (column == 0)
	    {
		/* the currently used Algorithm */
		algorithm = "Dijkstra";
		sqlite3_result_text (pContext, algorithm, strlen (algorithm),
				     SQLITE_STATIC);
	    }
	  if (column == 1)
	    {
		/* the ArcRowId column */
		sqlite3_result_null (pContext);
	    }
	  if (column == 2)
	    {
		/* the NodeFrom column */
		if (node_code)
		    sqlite3_result_text (pContext, cursor->solution->From->Code,
					 strlen (cursor->solution->From->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int64 (pContext, cursor->solution->From->Id);
	    }
	  if (column == 3)
	    {
		/* the NodeTo column */
		if (node_code)
		    sqlite3_result_text (pContext, row_node->Node->Code,
					 strlen (row_node->Node->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int64 (pContext, row_node->Node->Id);
	    }
	  if (column == 4)
	    {
		/* the Cost column */
		sqlite3_result_double (pContext, row_node->Cost);
	    }
	  if (column == 5)
	    {
		/* the Geometry column */
		if (row_node->Srid == VNET_INVALID_SRID)
		    sqlite3_result_null (pContext);
		else
		  {
		      int len;
		      unsigned char *p_result = NULL;
		      gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
		      geom->Srid = row_node->Srid;
		      gaiaAddPointToGeomColl (geom, row_node->Node->CoordX,
					      row_node->Node->CoordY);
		      gaiaToSpatiaLiteBlobWkb (geom, &p_result, &len);
		      sqlite3_result_blob (pContext, p_result, len, free);
		      gaiaFreeGeomColl (geom);
		  }
	    }
	  if (column == 6)
	    {
		/* the [optional] Name column */
		sqlite3_result_null (pContext);
	    }
      }
    else
      {
	  /* processing an ordinary Routing (Shortest Path) solution */
	  if (cursor->solution->CurrentRow == 0)
	    {
		/* special case: this one is the solution summary */
		if (column == 0)
		  {
		      /* the currently used Algorithm */
		      if (net->currentAlgorithm == VNET_A_STAR_ALGORITHM)
			  algorithm = "A*";
		      else
			  algorithm = "Dijkstra";
		      sqlite3_result_text (pContext, algorithm,
					   strlen (algorithm), SQLITE_STATIC);
		  }
		if (cursor->solution->From == NULL
		    || cursor->solution->To == NULL)
		  {
		      /* empty [uninitialized] solution */
		      if (column > 0)
			  sqlite3_result_null (pContext);
		      return SQLITE_OK;
		  }
		if (column == 1)
		  {
		      /* the ArcRowId column */
		      sqlite3_result_null (pContext);
		  }
		if (column == 2)
		  {
		      /* the NodeFrom column */
		      if (node_code)
			  sqlite3_result_text (pContext,
					       cursor->solution->From->Code,
					       strlen (cursor->solution->From->
						       Code), SQLITE_STATIC);
		      else
			  sqlite3_result_int64 (pContext,
						cursor->solution->From->Id);
		  }
		if (column == 3)
		  {
		      /* the NodeTo column */
		      if (node_code)
			  sqlite3_result_text (pContext,
					       cursor->solution->To->Code,
					       strlen (cursor->solution->To->
						       Code), SQLITE_STATIC);
		      else
			  sqlite3_result_int64 (pContext,
						cursor->solution->To->Id);
		  }
		if (column == 4)
		  {
		      /* the Cost column */
		      sqlite3_result_double (pContext,
					     cursor->solution->TotalCost);
		  }
		if (column == 5)
		  {
		      /* the Geometry column */
		      if (!(cursor->solution->Geometry))
			  sqlite3_result_null (pContext);
		      else
			{
			    /* builds the BLOB geometry to be returned */
			    int len;
			    unsigned char *p_result = NULL;
			    gaiaToSpatiaLiteBlobWkb (cursor->solution->Geometry,
						     &p_result, &len);
			    sqlite3_result_blob (pContext, p_result, len, free);
			}
		  }
		if (column == 6)
		  {
		      /* the [optional] Name column */
		      sqlite3_result_null (pContext);
		  }
	    }
	  else
	    {
		/* ordinary case: this one is an Arc used by the solution */
		row = cursor->solution->CurrentRow;
		if (column == 0)
		  {
		      /* the currently used Algorithm */
		      if (net->currentAlgorithm == VNET_A_STAR_ALGORITHM)
			  algorithm = "A*";
		      else
			  algorithm = "Dijkstra";
		      sqlite3_result_text (pContext, algorithm,
					   strlen (algorithm), SQLITE_STATIC);
		  }
		if (column == 1)
		  {
		      /* the ArcRowId column */
		      sqlite3_result_int64 (pContext, row->Arc->ArcRowid);
		  }
		if (column == 2)
		  {
		      /* the NodeFrom column */
		      if (node_code)
			  sqlite3_result_text (pContext,
					       row->Arc->NodeFrom->Code,
					       strlen (row->Arc->NodeFrom->
						       Code), SQLITE_STATIC);
		      else
			  sqlite3_result_int64 (pContext,
						row->Arc->NodeFrom->Id);
		  }
		if (column == 3)
		  {
		      /* the NodeTo column */
		      if (node_code)
			  sqlite3_result_text (pContext, row->Arc->NodeTo->Code,
					       strlen (row->Arc->NodeTo->Code),
					       SQLITE_STATIC);
		      else
			  sqlite3_result_int64 (pContext, row->Arc->NodeTo->Id);
		  }
		if (column == 4)
		  {
		      /* the Cost column */
		      sqlite3_result_double (pContext, row->Arc->Cost);
		  }
		if (column == 5)
		  {
		      /* the Geometry column */
		      sqlite3_result_null (pContext);
		  }
		if (column == 6)
		  {
		      /* the [optional] Name column */
		      if (row->Name)
			  sqlite3_result_text (pContext, row->Name,
					       strlen (row->Name),
					       SQLITE_STATIC);
		      else
			  sqlite3_result_null (pContext);
		  }
	    }
      }
    return SQLITE_OK;
}

static int
vnet_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    *pRowid = cursor->solution->CurrentRowId;
    return SQLITE_OK;
}

static int
vnet_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    VirtualNetworkPtr p_vtab = (VirtualNetworkPtr) pVTab;
    if (pRowid)
	pRowid = pRowid;	/* unused arg warning suppression */
    if (argc == 1)
      {
	  /* performing a DELETE is forbidden */
	  return SQLITE_READONLY;
      }
    else
      {
	  if (sqlite3_value_type (argv[0]) == SQLITE_NULL)
	    {
		/* performing an INSERT is forbidden */
		return SQLITE_READONLY;
	    }
	  else
	    {
		/* performing an UPDATE */
		if (argc == 9)
		  {
		      p_vtab->currentAlgorithm = VNET_DIJKSTRA_ALGORITHM;
		      if (sqlite3_value_type (argv[2]) == SQLITE_TEXT)
			{
			    const unsigned char *algorithm =
				sqlite3_value_text (argv[2]);
			    if (strcmp ((char *) algorithm, "A*") == 0)
				p_vtab->currentAlgorithm =
				    VNET_A_STAR_ALGORITHM;
			    if (strcmp ((char *) algorithm, "a*") == 0)
				p_vtab->currentAlgorithm =
				    VNET_A_STAR_ALGORITHM;
			}
		      if (p_vtab->graph->AStar == 0)
			  p_vtab->currentAlgorithm = VNET_DIJKSTRA_ALGORITHM;
		  }
		return SQLITE_OK;
	    }
      }
    return SQLITE_READONLY;
}

static int
vnet_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vnet_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vnet_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vnet_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vnet_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualNetworkInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_net_module.iVersion = 1;
    my_net_module.xCreate = &vnet_create;
    my_net_module.xConnect = &vnet_connect;
    my_net_module.xBestIndex = &vnet_best_index;
    my_net_module.xDisconnect = &vnet_disconnect;
    my_net_module.xDestroy = &vnet_destroy;
    my_net_module.xOpen = &vnet_open;
    my_net_module.xClose = &vnet_close;
    my_net_module.xFilter = &vnet_filter;
    my_net_module.xNext = &vnet_next;
    my_net_module.xEof = &vnet_eof;
    my_net_module.xColumn = &vnet_column;
    my_net_module.xRowid = &vnet_rowid;
    my_net_module.xUpdate = &vnet_update;
    my_net_module.xBegin = &vnet_begin;
    my_net_module.xSync = &vnet_sync;
    my_net_module.xCommit = &vnet_commit;
    my_net_module.xRollback = &vnet_rollback;
    my_net_module.xFindFunction = NULL;
    my_net_module.xRename = &vnet_rename;
    sqlite3_create_module_v2 (db, "VirtualNetwork", &my_net_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualnetwork_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualNetworkInit (db);
}
