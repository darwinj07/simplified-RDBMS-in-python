/*----------------------------------------------------------------------
  Copyright (c) 1999-2001, Digital Creations, Fredericksburg, VA, USA
  and Andrew Kuchling. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    o Redistributions of source code must retain the above copyright
      notice, this list of conditions, and the disclaimer that follows.

    o Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions, and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

    o Neither the name of Digital Creations nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY DIGITAL CREATIONS AND CONTRIBUTORS *AS
  IS* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL DIGITAL
  CREATIONS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGE.
------------------------------------------------------------------------*/


/*
 * Handwritten code to wrap version 3.x of the Berkeley DB library,
 * written to replace a SWIG-generated file.  It has since been updated
 * to compile with Berkeley DB versions 3.2 through 4.2.
 *
 * This module was started by Andrew Kuchling to remove the dependency
 * on SWIG in a package by Gregory P. Smith who based his work on a
 * similar package by Robin Dunn <robin@alldunn.com> which wrapped
 * Berkeley DB 2.7.x.
 *
 * Development of this module then returned full circle back to Robin Dunn
 * who worked on behalf of Digital Creations to complete the wrapping of
 * the DB 3.x API and to build a solid unit test suite.  Robin has
 * since gone onto other projects (wxPython).
 *
 * Gregory P. Smith <greg@krypto.org> is once again the maintainer.
 *
 * Since January 2008, new maintainer is Jesus Cea <jcea@jcea.es>.
 * Jesus Cea licenses this code to PSF under a Contributor Agreement.
 *
 * Use the pybsddb@jcea.es mailing list for all questions.
 * Things can change faster than the header of this file is updated.
 *
 * https://www.jcea.es/programacion/pybsddb.htm
 *
 * This module contains 8 types:
 *
 * DB           (Database)
 * DBCursor     (Database Cursor)
 * DBEnv        (database environment)
 * DBTxn        (An explicit database transaction)
 * DBLock       (A lock handle)
 * DBSequence   (Sequence)
 * DBSite       (Site)
 * DBLogCursor  (Log Cursor)
 *
 */

/* --------------------------------------------------------------------- */

/*
 * Portions of this module, associated unit tests and build scripts are the
 * result of a contract with The Written Word (http://thewrittenword.com/)
 * Many thanks go out to them for causing me to raise the bar on quality and
 * functionality, resulting in a better berkeleydb package for all of us to use.
 *
 * --Robin
 */

/* --------------------------------------------------------------------- */

/*
 * Work to split it up into a separate header and to add a C API was
 * contributed by Duncan Grisby <duncan@tideway.com>.   See here:
 *  http://sourceforge.net/tracker/index.php?func=detail&aid=1551895&group_id=13900&atid=313900
 */

/* --------------------------------------------------------------------- */

#ifndef _BERKELEYDB_H_
#define _BERKELEYDB_H_

#include <db.h>


/* 40 = 4.0, 33 = 3.3; this will break if the minor revision is > 9 */
#define DBVER (DB_VERSION_MAJOR * 10 + DB_VERSION_MINOR)
#if DB_VERSION_MINOR > 9
#error "eek! DBVER can't handle minor versions > 9"
#endif

#define PY_BERKELEYDB_VERSION "18.1.8"

/* Python object definitions */

struct behaviourFlags {
    /* What is the default behaviour when DB->get or DBCursor->get returns a
       DB_NOTFOUND || DB_KEYEMPTY error?  Return None or raise an exception? */
    unsigned int getReturnsNone : 1;
    /* What is the default behaviour for DBCursor.set* methods when DBCursor->get
     * returns a DB_NOTFOUND || DB_KEYEMPTY  error?  Return None or raise? */
    unsigned int cursorSetReturnsNone : 1;
};



struct DBObject;          /* Forward declaration */
struct DBCursorObject;    /* Forward declaration */
struct DBLogCursorObject; /* Forward declaration */
struct DBTxnObject;       /* Forward declaration */
struct DBSequenceObject;  /* Forward declaration */
#if (DBVER >= 53)
struct DBSiteObject;      /* Forward declaration */
#endif

typedef struct {
    PyObject_HEAD
    DB_ENV*     db_env;
    u_int32_t   flags;             /* saved flags from open() */
    int         closed;
    struct behaviourFlags moduleFlags;
    PyObject*       event_notifyCallback;
    struct DBObject *children_dbs;
    struct DBTxnObject *children_txns;
    struct DBLogCursorObject *children_logcursors;
#if (DBVER >= 53)
    struct DBSiteObject *children_sites;
#endif
    PyObject        *private_obj;
    PyObject        *rep_transport;
    PyObject        *in_weakreflist; /* List of weak references */
} DBEnvObject;

typedef struct DBObject {
    PyObject_HEAD
    DB*             db;
    DBEnvObject*    myenvobj;  /* PyObject containing the DB_ENV */
    u_int32_t       flags;     /* saved flags from open() */
    u_int32_t       setflags;  /* saved flags from set_flags() */
    struct behaviourFlags moduleFlags;
    struct DBTxnObject *txn;
    struct DBCursorObject *children_cursors;
    struct DBSequenceObject *children_sequences;
    struct DBObject **sibling_prev_p;
    struct DBObject *sibling_next;
    struct DBObject **sibling_prev_p_txn;
    struct DBObject *sibling_next_txn;
    PyObject*       associateCallback;
    PyObject*       btCompareCallback;
    PyObject*       dupCompareCallback;
    DBTYPE          primaryDBType;
    DBTYPE          dbtype;
    PyObject        *private_obj;
    PyObject        *in_weakreflist; /* List of weak references */
} DBObject;


typedef struct DBCursorObject {
    PyObject_HEAD
    DBC*            dbc;
    struct DBCursorObject **sibling_prev_p;
    struct DBCursorObject *sibling_next;
    struct DBCursorObject **sibling_prev_p_txn;
    struct DBCursorObject *sibling_next_txn;
    DBObject*       mydb;
    struct DBTxnObject *txn;
    PyObject        *in_weakreflist; /* List of weak references */
} DBCursorObject;


typedef struct DBTxnObject {
    PyObject_HEAD
    DB_TXN*         txn;
    DBEnvObject*    env;
    int             flag_prepare;
    struct DBTxnObject *parent_txn;
    struct DBTxnObject **sibling_prev_p;
    struct DBTxnObject *sibling_next;
    struct DBTxnObject *children_txns;
    struct DBObject *children_dbs;
    struct DBSequenceObject *children_sequences;
    struct DBCursorObject *children_cursors;
    PyObject        *in_weakreflist; /* List of weak references */
} DBTxnObject;


typedef struct DBLogCursorObject {
    PyObject_HEAD
    DB_LOGC*        logc;
    DBEnvObject*    env;
    struct DBLogCursorObject **sibling_prev_p;
    struct DBLogCursorObject *sibling_next;
    PyObject        *in_weakreflist; /* List of weak references */
} DBLogCursorObject;

#if (DBVER >= 53)
typedef struct DBSiteObject {
    PyObject_HEAD
    DB_SITE         *site;
    DBEnvObject     *env;
    struct DBSiteObject **sibling_prev_p;
    struct DBSiteObject *sibling_next;
    PyObject        *in_weakreflist; /* List of weak references */
} DBSiteObject;
#endif

typedef struct {
    PyObject_HEAD
    DB_LOCK         lock;
    int             lock_initialized;  /* Signal if we actually have a lock */
    PyObject        *in_weakreflist; /* List of weak references */
} DBLockObject;


typedef struct DBSequenceObject {
    PyObject_HEAD
    DB_SEQUENCE*     sequence;
    DBObject*        mydb;
    struct DBTxnObject *txn;
    struct DBSequenceObject **sibling_prev_p;
    struct DBSequenceObject *sibling_next;
    struct DBSequenceObject **sibling_prev_p_txn;
    struct DBSequenceObject *sibling_next_txn;
    PyObject        *in_weakreflist; /* List of weak references */
} DBSequenceObject;


/* API structure for use by C code */

/* To access the structure from an external module, use code like the
   following (error checking missed out for clarity):

     BERKELEYDB_api* berkeleydb_api;

     berkeleydb_api = (void **)PyCapsule_Import("berkeleydb._berkeleydb.api",
                                                1);


   Check "api_version" number before trying to use the API.

   The structure's members must not be changed.
*/

#define PY_BERKELEYDB_API_VERSION 4
#define PY_BERKELEYDB_BASE "berkeleydb._berkeleydb."

typedef struct {
    unsigned int api_version;
    /* Type objects */
    PyTypeObject* db_type;
    PyTypeObject* dbcursor_type;
    PyTypeObject* dblogcursor_type;
    PyTypeObject* dbenv_type;
    PyTypeObject* dbtxn_type;
    PyTypeObject* dblock_type;
    PyTypeObject* dbsequence_type;
#if (DBVER >= 53)
    PyTypeObject* dbsite_type;
#endif

    /* Functions */
    int (*makeDBError)(int err);
} BERKELEYDB_api;


#ifndef COMPILING_BERKELEYDB_C

/* If not inside _berkeleydb.c, define type check macros that use the api
   structure.  The calling code must have a value named berkeleydb_api
   pointing to the api structure.
*/

#define DBObject_Check(v)       ((v)->ob_type == berkeleydb_api->db_type)
#define DBCursorObject_Check(v) ((v)->ob_type == berkeleydb_api->dbcursor_type)
#define DBEnvObject_Check(v)    ((v)->ob_type == berkeleydb_api->dbenv_type)
#define DBTxnObject_Check(v)    ((v)->ob_type == berkeleydb_api->dbtxn_type)
#define DBLockObject_Check(v)   ((v)->ob_type == berkeleydb_api->dblock_type)
#define DBSequenceObject_Check(v)  \
    ((berkeleydb_api->dbsequence_type) && \
        ((v)->ob_type == berkeleydb_api->dbsequence_type))
#if (DBVER >= 53)
#define DBSiteObject_Check(v)   ((v)->ob_type == berkeleydb_api->dbsite_type)
#endif

#endif /* COMPILING_BERKELEYDB_C */


#endif /* _BERKELEYDB_H_ */
