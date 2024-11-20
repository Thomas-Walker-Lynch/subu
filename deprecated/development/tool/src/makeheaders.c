<!DOCTYPE html>
<html>
<head>
<base href="https://fossil-scm.org/fossil/doc/8cecc544/src/makeheaders.c" />
<meta http-equiv="Content-Security-Policy" content="default-src 'self' data: ; script-src 'self' 'nonce-e2f4709a3deeafd8212401c84e2a082e1fbd3068470984f2' ; style-src 'self' 'unsafe-inline'" />
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Fossil: File Content</title>
<link rel="alternate" type="application/rss+xml" title="RSS Feed"  href="/fossil/timeline.rss" />
<link rel="stylesheet" href="/fossil/style.css?id=9eb7de39" type="text/css"  media="screen" />
</head>
<body>
<div class="header">
  <div class="title"><h1>Fossil</h1>File Content</div>
    <div class="status"><a href='/fossil/login'>Login</a>
</div>
</div>
<div class="mainmenu">
<a id='hbbtn' href='#'>&#9776;</a><a href='/fossil/doc/trunk/www/index.wiki' class=''>Home</a>
<a href='/fossil/timeline' class=''>Timeline</a>
<a href='/fossil/doc/trunk/www/permutedindex.html' class=''>Docs</a>
<a href='https://fossil-scm.org/forum'>Forum</a><a href='/fossil/uv/download.html' class='desktoponly'>Download</a>
</div>
<div id='hbdrop'></div>
<form id='f01' method='GET' action='/fossil/file'>
<input type='hidden' name='udc' value='1'>
<div class="submenu">
<a class="label" href="/fossil/artifact/49c76a69">Artifact</a>
<a class="label" href="/fossil/timeline?n=200&amp;uf=49c76a6973d579ff0b346e5f73182fa72dd797cbb07e8b20612849dc2adef85d">Check-ins Using</a>
<a class="label" href="/fossil/raw/src/makeheaders.c?name=49c76a6973d579ff0b346e5f73182fa72dd797cbb07e8b20612849dc2adef85d">Download</a>
<a class="label" href="/fossil/hexdump?name=49c76a6973d579ff0b346e5f73182fa72dd797cbb07e8b20612849dc2adef85d">Hex</a>
<label class='submenuctrl submenuckbox'><input type='checkbox' name='ln' id='submenuctrl-0' >Line Numbers</label>
</div>
<input type="hidden" name="name" value="src/makeheaders.c">
</form>
<script src='/fossil/builtin/menu.js?id=6f60cb38'></script>
<div class="content">
<h2>Latest version of file 'src/makeheaders.c':</h2>
<ul>
<li>File
<a data-href='/fossil/finfo?name=src/makeheaders.c&m=49c76a6973d579ff' href='/fossil/honeypot'>src/makeheaders.c</a>
&mdash; part of check-in
<span class="timelineHistDsp">[8cecc544]</span>
at
2018-11-02 15:21:54
on branch <a data-href='/fossil/timeline?r=trunk' href='/fossil/honeypot'>trunk</a>
&mdash; Enhance makeheaders so that it is able to deal with static_assert() statements.
(These do not come up in Fossil itself.  This check-in is in response to use
of Makeheaders by external projects.)
 (user:
drh
size: 100011)
<a data-href='/fossil/whatis/49c76a6973d579ff' href='/fossil/honeypot'>[more...]</a>
</ul>
<hr />
<blockquote>
<pre>
/*
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the &quot;2-Clause License&quot; or &quot;FreeBSD License&quot;.)
**
** Copyright 1993 D. Richard Hipp. All rights reserved.
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
**   1. Redistributions of source code must retain the above copyright
**      notice, this list of conditions and the following disclaimer.
**
**   2. Redistributions in binary form must reproduce the above copyright
**      notice, this list of conditions and the following disclaimer in
**      the documentation and/or other materials provided with the
**      distribution.
**
** This software is provided &quot;as is&quot; and any express or implied warranties,
** including, but not limited to, the implied warranties of merchantability
** and fitness for a particular purpose are disclaimed.  In no event shall
** the author or contributors be liable for any direct, indirect, incidental,
** special, exemplary, or consequential damages (including, but not limited
** to, procurement of substitute goods or services; loss of use, data or
** profits; or business interruption) however caused and on any theory of
** liability, whether in contract, strict liability, or tort (including
** negligence or otherwise) arising in any way out of the use of this
** software, even if advised of the possibility of such damage.
**
** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
** appropriate header files.
*/
#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;ctype.h&gt;
#include &lt;memory.h&gt;
#include &lt;sys/stat.h&gt;
#include &lt;assert.h&gt;
#include &lt;string.h&gt;

#if defined( __MINGW32__) ||  defined(__DMC__) || defined(_MSC_VER) || defined(__POCC__)
#  ifndef WIN32
#    define WIN32
#  endif
#else
# include &lt;unistd.h&gt;
#endif

/*
** Macros for debugging.
*/
#ifdef DEBUG
static int debugMask = 0;
# define debug0(F,M)       if( (F)&amp;debugMask ){ fprintf(stderr,M); }
# define debug1(F,M,A)     if( (F)&amp;debugMask ){ fprintf(stderr,M,A); }
# define debug2(F,M,A,B)   if( (F)&amp;debugMask ){ fprintf(stderr,M,A,B); }
# define debug3(F,M,A,B,C) if( (F)&amp;debugMask ){ fprintf(stderr,M,A,B,C); }
# define PARSER      0x00000001
# define DECL_DUMP   0x00000002
# define TOKENIZER   0x00000004
#else
# define debug0(Flags, Format)
# define debug1(Flags, Format, A)
# define debug2(Flags, Format, A, B)
# define debug3(Flags, Format, A, B, C)
#endif

/*
** The following macros are purely for the purpose of testing this
** program on itself.  They don&#39;t really contribute to the code.
*/
#define INTERFACE 1
#define EXPORT_INTERFACE 1
#define EXPORT

/*
** Each token in a source file is represented by an instance of
** the following structure.  Tokens are collected onto a list.
*/
typedef struct Token Token;
struct Token {
  const char *zText;      /* The text of the token */
  int nText;              /* Number of characters in the token&#39;s text */
  int eType;              /* The type of this token */
  int nLine;              /* The line number on which the token starts */
  Token *pComment;        /* Most recent block comment before this token */
  Token *pNext;           /* Next token on the list */
  Token *pPrev;           /* Previous token on the list */
};

/*
** During tokenization, information about the state of the input
** stream is held in an instance of the following structure
*/
typedef struct InStream InStream;
struct InStream {
  const char *z;          /* Complete text of the input */
  int i;                  /* Next character to read from the input */
  int nLine;              /* The line number for character z[i] */
};

/*
** Each declaration in the C or C++ source files is parsed out and stored as
** an instance of the following structure.
**
** A &quot;forward declaration&quot; is a declaration that an object exists that
** doesn&#39;t tell about the objects structure.  A typical forward declaration
** is:
**
**          struct Xyzzy;
**
** Not every object has a forward declaration.  If it does, thought, the
** forward declaration will be contained in the zFwd field for C and
** the zFwdCpp for C++.  The zDecl field contains the complete
** declaration text.
*/
typedef struct Decl Decl;
struct Decl {
  char *zName;       /* Name of the object being declared.  The appearance
                     ** of this name is a source file triggers the declaration
                     ** to be added to the header for that file. */
  const char *zFile; /* File from which extracted.  */
  char *zIf;         /* Surround the declaration with this #if */
  char *zFwd;        /* A forward declaration.  NULL if there is none. */
  char *zFwdCpp;     /* Use this forward declaration for C++. */
  char *zDecl;       /* A full declaration of this object */
  char *zExtra;      /* Extra declaration text inserted into class objects */
  int extraType;     /* Last public:, protected: or private: in zExtraDecl */
  struct Include *pInclude;   /* #includes that come before this declaration */
  int flags;         /* See the &quot;Properties&quot; below */
  Token *pComment;   /* A block comment associated with this declaration */
  Token tokenCode;   /* Implementation of functions and procedures */
  Decl *pSameName;   /* Next declaration with the same &quot;zName&quot; */
  Decl *pSameHash;   /* Next declaration with same hash but different zName */
  Decl *pNext;       /* Next declaration with a different name */
};

/*
** Properties associated with declarations.
**
** DP_Forward and DP_Declared are used during the generation of a single
** header file in order to prevent duplicate declarations and definitions.
** DP_Forward is set after the object has been given a forward declaration
** and DP_Declared is set after the object gets a full declarations.
** (Example:  A forward declaration is &quot;typedef struct Abc Abc;&quot; and the
** full declaration is &quot;struct Abc { int a; float b; };&quot;.)
**
** The DP_Export and DP_Local flags are more permanent.  They mark objects
** that have EXPORT scope and LOCAL scope respectively.  If both of these
** marks are missing, then the object has library scope.  The meanings of
** the scopes are as follows:
**
**    LOCAL scope         The object is only usable within the file in
**                        which it is declared.
**
**    library scope       The object is visible and usable within other
**                        files in the same project.  By if the project is
**                        a library, then the object is not visible to users
**                        of the library.  (i.e. the object does not appear
**                        in the output when using the -H option.)
**
**    EXPORT scope        The object is visible and usable everywhere.
**
** The DP_Flag is a temporary use flag that is used during processing to
** prevent an infinite loop.  It&#39;s use is localized.
**
** The DP_Cplusplus, DP_ExternCReqd and DP_ExternReqd flags are permanent
** and are used to specify what type of declaration the object requires.
*/
#define DP_Forward      0x001   /* Has a forward declaration in this file */
#define DP_Declared     0x002   /* Has a full declaration in this file */
#define DP_Export       0x004   /* Export this declaration */
#define DP_Local        0x008   /* Declare in its home file only */
#define DP_Flag         0x010   /* Use to mark a subset of a Decl list
                                ** for special processing */
#define DP_Cplusplus    0x020   /* Has C++ linkage and cannot appear in a
                                ** C header file */
#define DP_ExternCReqd  0x040   /* Prepend &#39;extern &quot;C&quot;&#39; in a C++ header.
                                ** Prepend nothing in a C header */
#define DP_ExternReqd   0x080   /* Prepend &#39;extern &quot;C&quot;&#39; in a C++ header if
                                ** DP_Cplusplus is not also set. If DP_Cplusplus
                                ** is set or this is a C header then
                                ** prepend &#39;extern&#39; */

/*
** Convenience macros for dealing with declaration properties
*/
#define DeclHasProperty(D,P)    (((D)-&gt;flags&amp;(P))==(P))
#define DeclHasAnyProperty(D,P) (((D)-&gt;flags&amp;(P))!=0)
#define DeclSetProperty(D,P)    (D)-&gt;flags |= (P)
#define DeclClearProperty(D,P)  (D)-&gt;flags &amp;= ~(P)

/*
** These are state properties of the parser.  Each of the values is
** distinct from the DP_ values above so that both can be used in
** the same &quot;flags&quot; field.
**
** Be careful not to confuse PS_Export with DP_Export or
** PS_Local with DP_Local.  Their names are similar, but the meanings
** of these flags are very different.
*/
#define PS_Extern        0x000800    /* &quot;extern&quot; has been seen */
#define PS_Export        0x001000    /* If between &quot;#if EXPORT_INTERFACE&quot;
                                     ** and &quot;#endif&quot; */
#define PS_Export2       0x002000    /* If &quot;EXPORT&quot; seen */
#define PS_Typedef       0x004000    /* If &quot;typedef&quot; has been seen */
#define PS_Static        0x008000    /* If &quot;static&quot; has been seen */
#define PS_Interface     0x010000    /* If within #if INTERFACE..#endif */
#define PS_Method        0x020000    /* If &quot;::&quot; token has been seen */
#define PS_Local         0x040000    /* If within #if LOCAL_INTERFACE..#endif */
#define PS_Local2        0x080000    /* If &quot;LOCAL&quot; seen. */
#define PS_Public        0x100000    /* If &quot;PUBLIC&quot; seen. */
#define PS_Protected     0x200000    /* If &quot;PROTECTED&quot; seen. */
#define PS_Private       0x400000    /* If &quot;PRIVATE&quot; seen. */
#define PS_PPP           0x700000    /* If any of PUBLIC, PRIVATE, PROTECTED */

/*
** The following set of flags are ORed into the &quot;flags&quot; field of
** a Decl in order to identify what type of object is being
** declared.
*/
#define TY_Class         0x00100000
#define TY_Subroutine    0x00200000
#define TY_Macro         0x00400000
#define TY_Typedef       0x00800000
#define TY_Variable      0x01000000
#define TY_Structure     0x02000000
#define TY_Union         0x04000000
#define TY_Enumeration   0x08000000
#define TY_Defunct       0x10000000  /* Used to erase a declaration */

/*
** Each nested #if (or #ifdef or #ifndef) is stored in a stack of
** instances of the following structure.
*/
typedef struct Ifmacro Ifmacro;
struct Ifmacro {
  int nLine;         /* Line number where this macro occurs */
  char *zCondition;  /* Text of the condition for this macro */
  Ifmacro *pNext;    /* Next down in the stack */
  int flags;         /* Can hold PS_Export, PS_Interface or PS_Local flags */
};

/*
** When parsing a file, we need to keep track of what other files have
** be #include-ed.  For each #include found, we create an instance of
** the following structure.
*/
typedef struct Include Include;
struct Include {
  char *zFile;       /* The name of file include.  Includes &quot;&quot; or &lt;&gt; */
  char *zIf;         /* If not NULL, #include should be enclosed in #if */
  char *zLabel;      /* A unique label used to test if this #include has
                      * appeared already in a file or not */
  Include *pNext;    /* Previous include file, or NULL if this is the first */
};

/*
** Identifiers found in a source file that might be used later to provoke
** the copying of a declaration into the corresponding header file are
** stored in a hash table as instances of the following structure.
*/
typedef struct Ident Ident;
struct Ident {
  char *zName;        /* The text of this identifier */
  Ident *pCollide;    /* Next identifier with the same hash */
  Ident *pNext;       /* Next identifier in a list of them all */
};

/*
** A complete table of identifiers is stored in an instance of
** the next structure.
*/
#define IDENT_HASH_SIZE 2237
typedef struct IdentTable IdentTable;
struct IdentTable {
  Ident *pList;                     /* List of all identifiers in this table */
  Ident *apTable[IDENT_HASH_SIZE];  /* The hash table */
};

/*
** The following structure holds all information for a single
** source file named on the command line of this program.
*/
typedef struct InFile InFile;
struct InFile {
  char *zSrc;              /* Name of input file */
  char *zHdr;              /* Name of the generated .h file for this input.
                           ** Will be NULL if input is to be scanned only */
  int flags;               /* One or more DP_, PS_ and/or TY_ flags */
  InFile *pNext;           /* Next input file in the list of them all */
  IdentTable idTable;      /* All identifiers in this input file */
};

/*
** An unbounded string is able to grow without limit.  We use these
** to construct large in-memory strings from lots of smaller components.
*/
typedef struct String String;
struct String {
  int nAlloc;      /* Number of bytes allocated */
  int nUsed;       /* Number of bytes used (not counting null terminator) */
  char *zText;     /* Text of the string */
};

/*
** The following structure contains a lot of state information used
** while generating a .h file.  We put the information in this structure
** and pass around a pointer to this structure, rather than pass around
** all of the information separately.  This helps reduce the number of
** arguments to generator functions.
*/
typedef struct GenState GenState;
struct GenState {
  String *pStr;          /* Write output to this string */
  IdentTable *pTable;    /* A table holding the zLabel of every #include that
                          * has already been generated.  Used to avoid
                          * generating duplicate #includes. */
  const char *zIf;       /* If not NULL, then we are within a #if with
                          * this argument. */
  int nErr;              /* Number of errors */
  const char *zFilename; /* Name of the source file being scanned */
  int flags;             /* Various flags (DP_ and PS_ flags above) */
};

/*
** The following text line appears at the top of every file generated
** by this program.  By recognizing this line, the program can be sure
** never to read a file that it generated itself.
**
** The &quot;#undef INTERFACE&quot; part is a hack to work around a name collision
** in MSVC 2008.
*/
const char zTopLine[] =
  &quot;/* \aThis file was automatically generated.  Do not edit! */\n&quot;
  &quot;#undef INTERFACE\n&quot;;
#define nTopLine (sizeof(zTopLine)-1)

/*
** The name of the file currently being parsed.
*/
static const char *zFilename;

/*
** The stack of #if macros for the file currently being parsed.
*/
static Ifmacro *ifStack = 0;

/*
** A list of all files that have been #included so far in a file being
** parsed.
*/
static Include *includeList = 0;

/*
** The last block comment seen.
*/
static Token *blockComment = 0;

/*
** The following flag is set if the -doc flag appears on the
** command line.
*/
static int doc_flag = 0;

/*
** If the following flag is set, then makeheaders will attempt to
** generate prototypes for static functions and procedures.
*/
static int proto_static = 0;

/*
** A list of all declarations.  The list is held together using the
** pNext field of the Decl structure.
*/
static Decl *pDeclFirst;    /* First on the list */
static Decl *pDeclLast;     /* Last on the list */

/*
** A hash table of all declarations
*/
#define DECL_HASH_SIZE 3371
static Decl *apTable[DECL_HASH_SIZE];

/*
** The TEST macro must be defined to something.  Make sure this is the
** case.
*/
#ifndef TEST
# define TEST 0
#endif

#ifdef NOT_USED
/*
** We do our own assertion macro so that we can have more control
** over debugging.
*/
#define Assert(X)    if(!(X)){ CantHappen(__LINE__); }
#define CANT_HAPPEN  CantHappen(__LINE__)
static void CantHappen(int iLine){
  fprintf(stderr,&quot;Assertion failed on line %d\n&quot;,iLine);
  *(char*)1 = 0;  /* Force a core-dump */
}
#endif

/*
** Memory allocation functions that are guaranteed never to return NULL.
*/
static void *SafeMalloc(int nByte){
  void *p = malloc( nByte );
  if( p==0 ){
    fprintf(stderr,&quot;Out of memory.  Can&#39;t allocate %d bytes.\n&quot;,nByte);
    exit(1);
  }
  return p;
}
static void SafeFree(void *pOld){
  if( pOld ){
    free(pOld);
  }
}
static void *SafeRealloc(void *pOld, int nByte){
  void *p;
  if( pOld==0 ){
    p = SafeMalloc(nByte);
  }else{
    p = realloc(pOld, nByte);
    if( p==0 ){
      fprintf(stderr,
        &quot;Out of memory.  Can&#39;t enlarge an allocation to %d bytes\n&quot;,nByte);
      exit(1);
    }
  }
  return p;
}
static char *StrDup(const char *zSrc, int nByte){
  char *zDest;
  if( nByte&lt;=0 ){
    nByte = strlen(zSrc);
  }
  zDest = SafeMalloc( nByte + 1 );
  strncpy(zDest,zSrc,nByte);
  zDest[nByte] = 0;
  return zDest;
}

/*
** Return TRUE if the character X can be part of an identifier
*/
#define ISALNUM(X)  ((X)==&#39;_&#39; || isalnum(X))

/*
** Routines for dealing with unbounded strings.
*/
static void StringInit(String *pStr){
  pStr-&gt;nAlloc = 0;
  pStr-&gt;nUsed = 0;
  pStr-&gt;zText = 0;
}
static void StringReset(String *pStr){
  SafeFree(pStr-&gt;zText);
  StringInit(pStr);
}
static void StringAppend(String *pStr, const char *zText, int nByte){
  if( nByte&lt;=0 ){
    nByte = strlen(zText);
  }
  if( pStr-&gt;nUsed + nByte &gt;= pStr-&gt;nAlloc ){
    if( pStr-&gt;nAlloc==0 ){
      pStr-&gt;nAlloc = nByte + 100;
      pStr-&gt;zText = SafeMalloc( pStr-&gt;nAlloc );
    }else{
      pStr-&gt;nAlloc = pStr-&gt;nAlloc*2 + nByte;
      pStr-&gt;zText = SafeRealloc(pStr-&gt;zText, pStr-&gt;nAlloc);
    }
  }
  strncpy(&amp;pStr-&gt;zText[pStr-&gt;nUsed],zText,nByte);
  pStr-&gt;nUsed += nByte;
  pStr-&gt;zText[pStr-&gt;nUsed] = 0;
}
#define StringGet(S) ((S)-&gt;zText?(S)-&gt;zText:&quot;&quot;)

/*
** Compute a hash on a string.  The number returned is a non-negative
** value between 0 and 2**31 - 1
*/
static int Hash(const char *z, int n){
  int h = 0;
  if( n&lt;=0 ){
    n = strlen(z);
  }
  while( n-- ){
    h = h ^ (h&lt;&lt;5) ^ *z++;
  }
  return h &amp; 0x7fffffff;
}

/*
** Given an identifier name, try to find a declaration for that
** identifier in the hash table.  If found, return a pointer to
** the Decl structure.  If not found, return 0.
*/
static Decl *FindDecl(const char *zName, int len){
  int h;
  Decl *p;

  if( len&lt;=0 ){
    len = strlen(zName);
  }
  h = Hash(zName,len) % DECL_HASH_SIZE;
  p = apTable[h];
  while( p &amp;&amp; (strncmp(p-&gt;zName,zName,len)!=0 || p-&gt;zName[len]!=0) ){
    p = p-&gt;pSameHash;
  }
  return p;
}

/*
** Install the given declaration both in the hash table and on
** the list of all declarations.
*/
static void InstallDecl(Decl *pDecl){
  int h;
  Decl *pOther;

  h = Hash(pDecl-&gt;zName,0) % DECL_HASH_SIZE;
  pOther = apTable[h];
  while( pOther &amp;&amp; strcmp(pDecl-&gt;zName,pOther-&gt;zName)!=0 ){
    pOther = pOther-&gt;pSameHash;
  }
  if( pOther ){
    pDecl-&gt;pSameName = pOther-&gt;pSameName;
    pOther-&gt;pSameName = pDecl;
  }else{
    pDecl-&gt;pSameName = 0;
    pDecl-&gt;pSameHash = apTable[h];
    apTable[h] = pDecl;
  }
  pDecl-&gt;pNext = 0;
  if( pDeclFirst==0 ){
    pDeclFirst = pDeclLast = pDecl;
  }else{
    pDeclLast-&gt;pNext = pDecl;
    pDeclLast = pDecl;
  }
}

/*
** Look at the current ifStack.  If anything declared at the current
** position must be surrounded with
**
**      #if   STUFF
**      #endif
**
** Then this routine computes STUFF and returns a pointer to it.  Memory
** to hold the value returned is obtained from malloc().
*/
static char *GetIfString(void){
  Ifmacro *pIf;
  char *zResult = 0;
  int hasIf = 0;
  String str;

  for(pIf = ifStack; pIf; pIf=pIf-&gt;pNext){
    if( pIf-&gt;zCondition==0 || *pIf-&gt;zCondition==0 ) continue;
    if( !hasIf ){
      hasIf = 1;
      StringInit(&amp;str);
    }else{
      StringAppend(&amp;str,&quot; &amp;&amp; &quot;,4);
    }
    StringAppend(&amp;str,pIf-&gt;zCondition,0);
  }
  if( hasIf ){
    zResult = StrDup(StringGet(&amp;str),0);
    StringReset(&amp;str);
  }else{
    zResult = 0;
  }
  return zResult;
}

/*
** Create a new declaration and put it in the hash table.  Also
** return a pointer to it so that we can fill in the zFwd and zDecl
** fields, and so forth.
*/
static Decl *CreateDecl(
  const char *zName,       /* Name of the object being declared. */
  int nName                /* Length of the name */
){
  Decl *pDecl;

  pDecl = SafeMalloc( sizeof(Decl) + nName + 1);
  memset(pDecl,0,sizeof(Decl));
  pDecl-&gt;zName = (char*)&amp;pDecl[1];
  sprintf(pDecl-&gt;zName,&quot;%.*s&quot;,nName,zName);
  pDecl-&gt;zFile = zFilename;
  pDecl-&gt;pInclude = includeList;
  pDecl-&gt;zIf = GetIfString();
  InstallDecl(pDecl);
  return pDecl;
}

/*
** Insert a new identifier into an table of identifiers.  Return TRUE if
** a new identifier was inserted and return FALSE if the identifier was
** already in the table.
*/
static int IdentTableInsert(
  IdentTable *pTable,       /* The table into which we will insert */
  const char *zId,          /* Name of the identifiers */
  int nId                   /* Length of the identifier name */
){
  int h;
  Ident *pId;

  if( nId&lt;=0 ){
    nId = strlen(zId);
  }
  h = Hash(zId,nId) % IDENT_HASH_SIZE;
  for(pId = pTable-&gt;apTable[h]; pId; pId=pId-&gt;pCollide){
    if( strncmp(zId,pId-&gt;zName,nId)==0 &amp;&amp; pId-&gt;zName[nId]==0 ){
      /* printf(&quot;Already in table: %.*s\n&quot;,nId,zId); */
      return 0;
    }
  }
  pId = SafeMalloc( sizeof(Ident) + nId + 1 );
  pId-&gt;zName = (char*)&amp;pId[1];
  sprintf(pId-&gt;zName,&quot;%.*s&quot;,nId,zId);
  pId-&gt;pNext = pTable-&gt;pList;
  pTable-&gt;pList = pId;
  pId-&gt;pCollide = pTable-&gt;apTable[h];
  pTable-&gt;apTable[h] = pId;
  /* printf(&quot;Add to table: %.*s\n&quot;,nId,zId); */
  return 1;
}

/*
** Check to see if the given value is in the given IdentTable.  Return
** true if it is and false if it is not.
*/
static int IdentTableTest(
  IdentTable *pTable,       /* The table in which to search */
  const char *zId,          /* Name of the identifiers */
  int nId                   /* Length of the identifier name */
){
  int h;
  Ident *pId;

  if( nId&lt;=0 ){
    nId = strlen(zId);
  }
  h = Hash(zId,nId) % IDENT_HASH_SIZE;
  for(pId = pTable-&gt;apTable[h]; pId; pId=pId-&gt;pCollide){
    if( strncmp(zId,pId-&gt;zName,nId)==0 &amp;&amp; pId-&gt;zName[nId]==0 ){
      return 1;
    }
  }
  return 0;
}

/*
** Remove every identifier from the given table.   Reset the table to
** its initial state.
*/
static void IdentTableReset(IdentTable *pTable){
  Ident *pId, *pNext;

  for(pId = pTable-&gt;pList; pId; pId = pNext){
    pNext = pId-&gt;pNext;
    SafeFree(pId);
  }
  memset(pTable,0,sizeof(IdentTable));
}

#ifdef DEBUG
/*
** Print the name of every identifier in the given table, one per line
*/
static void IdentTablePrint(IdentTable *pTable, FILE *pOut){
  Ident *pId;

  for(pId = pTable-&gt;pList; pId; pId = pId-&gt;pNext){
    fprintf(pOut,&quot;%s\n&quot;,pId-&gt;zName);
  }
}
#endif

/*
** Read an entire file into memory.  Return a pointer to the memory.
**
** The memory is obtained from SafeMalloc and must be freed by the
** calling function.
**
** If the read fails for any reason, 0 is returned.
*/
static char *ReadFile(const char *zFilename){
  struct stat sStat;
  FILE *pIn;
  char *zBuf;
  int n;

  if( stat(zFilename,&amp;sStat)!=0
#ifndef WIN32
    || !S_ISREG(sStat.st_mode)
#endif
  ){
    return 0;
  }
  pIn = fopen(zFilename,&quot;r&quot;);
  if( pIn==0 ){
    return 0;
  }
  zBuf = SafeMalloc( sStat.st_size + 1 );
  n = fread(zBuf,1,sStat.st_size,pIn);
  zBuf[n] = 0;
  fclose(pIn);
  return zBuf;
}

/*
** Write the contents of a string into a file.  Return the number of
** errors
*/
static int WriteFile(const char *zFilename, const char *zOutput){
  FILE *pOut;
  pOut = fopen(zFilename,&quot;w&quot;);
  if( pOut==0 ){
    return 1;
  }
  fwrite(zOutput,1,strlen(zOutput),pOut);
  fclose(pOut);
  return 0;
}

/*
** Major token types
*/
#define TT_Space           1   /* Contiguous white space */
#define TT_Id              2   /* An identifier */
#define TT_Preprocessor    3   /* Any C preprocessor directive */
#define TT_Comment         4   /* Either C or C++ style comment */
#define TT_Number          5   /* Any numeric constant */
#define TT_String          6   /* String or character constants. &quot;..&quot; or &#39;.&#39; */
#define TT_Braces          7   /* All text between { and a matching } */
#define TT_EOF             8   /* End of file */
#define TT_Error           9   /* An error condition */
#define TT_BlockComment    10  /* A C-Style comment at the left margin that
                                * spans multiple lines */
#define TT_Other           0   /* None of the above */

/*
** Get a single low-level token from the input file.  Update the
** file pointer so that it points to the first character beyond the
** token.
**
** A &quot;low-level token&quot; is any token except TT_Braces.  A TT_Braces token
** consists of many smaller tokens and is assembled by a routine that
** calls this one.
**
** The function returns the number of errors.  An error is an
** unterminated string or character literal or an unterminated
** comment.
**
** Profiling shows that this routine consumes about half the
** CPU time on a typical run of makeheaders.
*/
static int GetToken(InStream *pIn, Token *pToken){
  int i;
  const char *z;
  int cStart;
  int c;
  int startLine;   /* Line on which a structure begins */
  int nlisc = 0;   /* True if there is a new-line in a &quot;..&quot; or &#39;..&#39; */
  int nErr = 0;    /* Number of errors seen */

  z = pIn-&gt;z;
  i = pIn-&gt;i;
  pToken-&gt;nLine = pIn-&gt;nLine;
  pToken-&gt;zText = &amp;z[i];
  switch( z[i] ){
    case 0:
      pToken-&gt;eType = TT_EOF;
      pToken-&gt;nText = 0;
      break;

    case &#39;#&#39;:
      if( i==0 || z[i-1]==&#39;\n&#39; || (i&gt;1 &amp;&amp; z[i-1]==&#39;\r&#39; &amp;&amp; z[i-2]==&#39;\n&#39;)){
        /* We found a preprocessor statement */
        pToken-&gt;eType = TT_Preprocessor;
        i++;
        while( z[i]!=0 &amp;&amp; z[i]!=&#39;\n&#39; ){
          if( z[i]==&#39;\\&#39; ){
            i++;
            if( z[i]==&#39;\n&#39; ) pIn-&gt;nLine++;
          }
          i++;
        }
        pToken-&gt;nText = i - pIn-&gt;i;
      }else{
        /* Just an operator */
        pToken-&gt;eType = TT_Other;
        pToken-&gt;nText = 1;
      }
      break;

    case &#39; &#39;:
    case &#39;\t&#39;:
    case &#39;\r&#39;:
    case &#39;\f&#39;:
    case &#39;\n&#39;:
      while( isspace(z[i]) ){
        if( z[i]==&#39;\n&#39; ) pIn-&gt;nLine++;
        i++;
      }
      pToken-&gt;eType = TT_Space;
      pToken-&gt;nText = i - pIn-&gt;i;
      break;

    case &#39;\\&#39;:
      pToken-&gt;nText = 2;
      pToken-&gt;eType = TT_Other;
      if( z[i+1]==&#39;\n&#39; ){
        pIn-&gt;nLine++;
        pToken-&gt;eType = TT_Space;
      }else if( z[i+1]==0 ){
        pToken-&gt;nText = 1;
      }
      break;

    case &#39;\&#39;&#39;:
    case &#39;\&quot;&#39;:
      cStart = z[i];
      startLine = pIn-&gt;nLine;
      do{
        i++;
        c = z[i];
        if( c==&#39;\n&#39; ){
          if( !nlisc ){
            fprintf(stderr,
              &quot;%s:%d: (warning) Newline in string or character literal.\n&quot;,
              zFilename, pIn-&gt;nLine);
            nlisc = 1;
          }
          pIn-&gt;nLine++;
        }
        if( c==&#39;\\&#39; ){
          i++;
          c = z[i];
          if( c==&#39;\n&#39; ){
            pIn-&gt;nLine++;
          }
        }else if( c==cStart ){
          i++;
          c = 0;
        }else if( c==0 ){
          fprintf(stderr, &quot;%s:%d: Unterminated string or character literal.\n&quot;,
             zFilename, startLine);
          nErr++;
        }
      }while( c );
      pToken-&gt;eType = TT_String;
      pToken-&gt;nText = i - pIn-&gt;i;
      break;

    case &#39;/&#39;:
      if( z[i+1]==&#39;/&#39; ){
        /* C++ style comment */
        while( z[i] &amp;&amp; z[i]!=&#39;\n&#39; ){ i++; }
        pToken-&gt;eType = TT_Comment;
        pToken-&gt;nText = i - pIn-&gt;i;
      }else if( z[i+1]==&#39;*&#39; ){
        /* C style comment */
        int isBlockComment = i==0 || z[i-1]==&#39;\n&#39;;
        i += 2;
        startLine = pIn-&gt;nLine;
        while( z[i] &amp;&amp; (z[i]!=&#39;*&#39; || z[i+1]!=&#39;/&#39;) ){
          if( z[i]==&#39;\n&#39; ){
            pIn-&gt;nLine++;
            if( isBlockComment ){
              if( z[i+1]==&#39;*&#39; || z[i+2]==&#39;*&#39; ){
                 isBlockComment = 2;
              }else{
                 isBlockComment = 0;
              }
            }
          }
          i++;
        }
        if( z[i] ){
          i += 2;
        }else{
          isBlockComment = 0;
          fprintf(stderr,&quot;%s:%d: Unterminated comment\n&quot;,
            zFilename, startLine);
          nErr++;
        }
        pToken-&gt;eType = isBlockComment==2 ? TT_BlockComment : TT_Comment;
        pToken-&gt;nText = i - pIn-&gt;i;
      }else{
        /* A divide operator */
        pToken-&gt;eType = TT_Other;
        pToken-&gt;nText = 1 + (z[i+1]==&#39;+&#39;);
      }
      break;

    case &#39;0&#39;:
      if( z[i+1]==&#39;x&#39; || z[i+1]==&#39;X&#39; ){
        /* A hex constant */
        i += 2;
        while( isxdigit(z[i]) ){ i++; }
      }else{
        /* An octal constant */
        while( isdigit(z[i]) ){ i++; }
      }
      pToken-&gt;eType = TT_Number;
      pToken-&gt;nText = i - pIn-&gt;i;
      break;

    case &#39;1&#39;: case &#39;2&#39;: case &#39;3&#39;: case &#39;4&#39;:
    case &#39;5&#39;: case &#39;6&#39;: case &#39;7&#39;: case &#39;8&#39;: case &#39;9&#39;:
      while( isdigit(z[i]) ){ i++; }
      if( (c=z[i])==&#39;.&#39; ){
         i++;
         while( isdigit(z[i]) ){ i++; }
         c = z[i];
         if( c==&#39;e&#39; || c==&#39;E&#39; ){
           i++;
           if( ((c=z[i])==&#39;+&#39; || c==&#39;-&#39;) &amp;&amp; isdigit(z[i+1]) ){ i++; }
           while( isdigit(z[i]) ){ i++; }
           c = z[i];
         }
         if( c==&#39;f&#39; || c==&#39;F&#39; || c==&#39;l&#39; || c==&#39;L&#39; ){ i++; }
      }else if( c==&#39;e&#39; || c==&#39;E&#39; ){
         i++;
         if( ((c=z[i])==&#39;+&#39; || c==&#39;-&#39;) &amp;&amp; isdigit(z[i+1]) ){ i++; }
         while( isdigit(z[i]) ){ i++; }
      }else if( c==&#39;L&#39; || c==&#39;l&#39; ){
         i++;
         c = z[i];
         if( c==&#39;u&#39; || c==&#39;U&#39; ){ i++; }
      }else if( c==&#39;u&#39; || c==&#39;U&#39; ){
         i++;
         c = z[i];
         if( c==&#39;l&#39; || c==&#39;L&#39; ){ i++; }
      }
      pToken-&gt;eType = TT_Number;
      pToken-&gt;nText = i - pIn-&gt;i;
      break;

    case &#39;a&#39;: case &#39;b&#39;: case &#39;c&#39;: case &#39;d&#39;: case &#39;e&#39;: case &#39;f&#39;: case &#39;g&#39;:
    case &#39;h&#39;: case &#39;i&#39;: case &#39;j&#39;: case &#39;k&#39;: case &#39;l&#39;: case &#39;m&#39;: case &#39;n&#39;:
    case &#39;o&#39;: case &#39;p&#39;: case &#39;q&#39;: case &#39;r&#39;: case &#39;s&#39;: case &#39;t&#39;: case &#39;u&#39;:
    case &#39;v&#39;: case &#39;w&#39;: case &#39;x&#39;: case &#39;y&#39;: case &#39;z&#39;: case &#39;A&#39;: case &#39;B&#39;:
    case &#39;C&#39;: case &#39;D&#39;: case &#39;E&#39;: case &#39;F&#39;: case &#39;G&#39;: case &#39;H&#39;: case &#39;I&#39;:
    case &#39;J&#39;: case &#39;K&#39;: case &#39;L&#39;: case &#39;M&#39;: case &#39;N&#39;: case &#39;O&#39;: case &#39;P&#39;:
    case &#39;Q&#39;: case &#39;R&#39;: case &#39;S&#39;: case &#39;T&#39;: case &#39;U&#39;: case &#39;V&#39;: case &#39;W&#39;:
    case &#39;X&#39;: case &#39;Y&#39;: case &#39;Z&#39;: case &#39;_&#39;:
      while( isalnum(z[i]) || z[i]==&#39;_&#39; ){ i++; };
      pToken-&gt;eType = TT_Id;
      pToken-&gt;nText = i - pIn-&gt;i;
      break;

    case &#39;:&#39;:
      pToken-&gt;eType = TT_Other;
      pToken-&gt;nText = 1 + (z[i+1]==&#39;:&#39;);
      break;

    case &#39;=&#39;:
    case &#39;&lt;&#39;:
    case &#39;&gt;&#39;:
    case &#39;+&#39;:
    case &#39;-&#39;:
    case &#39;*&#39;:
    case &#39;%&#39;:
    case &#39;^&#39;:
    case &#39;&amp;&#39;:
    case &#39;|&#39;:
      pToken-&gt;eType = TT_Other;
      pToken-&gt;nText = 1 + (z[i+1]==&#39;=&#39;);
      break;

    default:
      pToken-&gt;eType = TT_Other;
      pToken-&gt;nText = 1;
      break;
  }
  pIn-&gt;i += pToken-&gt;nText;
  return nErr;
}

/*
** This routine recovers the next token from the input file which is
** not a space or a comment or any text between an &quot;#if 0&quot; and &quot;#endif&quot;.
**
** This routine returns the number of errors encountered.  An error
** is an unterminated token or unmatched &quot;#if 0&quot;.
**
** Profiling shows that this routine uses about a quarter of the
** CPU time in a typical run.
*/
static int GetNonspaceToken(InStream *pIn, Token *pToken){
  int nIf = 0;
  int inZero = 0;
  const char *z;
  int value;
  int startLine;
  int nErr = 0;

  startLine = pIn-&gt;nLine;
  while( 1 ){
    nErr += GetToken(pIn,pToken);
    /* printf(&quot;%04d: Type=%d nIf=%d [%.*s]\n&quot;,
       pToken-&gt;nLine,pToken-&gt;eType,nIf,pToken-&gt;nText,
       pToken-&gt;eType!=TT_Space ? pToken-&gt;zText : &quot;&lt;space&gt;&quot;); */
    pToken-&gt;pComment = blockComment;
    switch( pToken-&gt;eType ){
      case TT_Comment:          /*0123456789 12345678 */
       if( strncmp(pToken-&gt;zText, &quot;/*MAKEHEADERS-STOP&quot;, 18)==0 ) return nErr;
       break;

      case TT_Space:
        break;

      case TT_BlockComment:
        if( doc_flag ){
          blockComment = SafeMalloc( sizeof(Token) );
          *blockComment = *pToken;
        }
        break;

      case TT_EOF:
        if( nIf ){
          fprintf(stderr,&quot;%s:%d: Unterminated \&quot;#if\&quot;\n&quot;,
             zFilename, startLine);
          nErr++;
        }
        return nErr;

      case TT_Preprocessor:
        z = &amp;pToken-&gt;zText[1];
        while( *z==&#39; &#39; || *z==&#39;\t&#39; ) z++;
        if( sscanf(z,&quot;if %d&quot;,&amp;value)==1 &amp;&amp; value==0 ){
          nIf++;
          inZero = 1;
        }else if( inZero ){
          if( strncmp(z,&quot;if&quot;,2)==0 ){
            nIf++;
          }else if( strncmp(z,&quot;endif&quot;,5)==0 ){
            nIf--;
            if( nIf==0 ) inZero = 0;
          }
        }else{
          return nErr;
        }
        break;

      default:
        if( !inZero ){
          return nErr;
        }
        break;
    }
  }
  /* NOT REACHED */
}

/*
** This routine looks for identifiers (strings of contiguous alphanumeric
** characters) within a preprocessor directive and adds every such string
** found to the given identifier table
*/
static void FindIdentifiersInMacro(Token *pToken, IdentTable *pTable){
  Token sToken;
  InStream sIn;
  int go = 1;

  sIn.z = pToken-&gt;zText;
  sIn.i = 1;
  sIn.nLine = 1;
  while( go &amp;&amp; sIn.i &lt; pToken-&gt;nText ){
    GetToken(&amp;sIn,&amp;sToken);
    switch( sToken.eType ){
      case TT_Id:
        IdentTableInsert(pTable,sToken.zText,sToken.nText);
        break;

      case TT_EOF:
        go = 0;
        break;

      default:
        break;
    }
  }
}

/*
** This routine gets the next token.  Everything contained within
** {...} is collapsed into a single TT_Braces token.  Whitespace is
** omitted.
**
** If pTable is not NULL, then insert every identifier seen into the
** IdentTable.  This includes any identifiers seen inside of {...}.
**
** The number of errors encountered is returned.  An error is an
** unterminated token.
*/
static int GetBigToken(InStream *pIn, Token *pToken, IdentTable *pTable){
  const char *zStart;
  int iStart;
  int nBrace;
  int c;
  int nLine;
  int nErr;

  nErr = GetNonspaceToken(pIn,pToken);
  switch( pToken-&gt;eType ){
    case TT_Id:
      if( pTable!=0 ){
        IdentTableInsert(pTable,pToken-&gt;zText,pToken-&gt;nText);
      }
      return nErr;

    case TT_Preprocessor:
      if( pTable!=0 ){
        FindIdentifiersInMacro(pToken,pTable);
      }
      return nErr;

    case TT_Other:
      if( pToken-&gt;zText[0]==&#39;{&#39; ) break;
      return nErr;

    default:
      return nErr;
  }

  iStart = pIn-&gt;i;
  zStart = pToken-&gt;zText;
  nLine = pToken-&gt;nLine;
  nBrace = 1;
  while( nBrace ){
    nErr += GetNonspaceToken(pIn,pToken);
    /* printf(&quot;%04d: nBrace=%d [%.*s]\n&quot;,pToken-&gt;nLine,nBrace,
       pToken-&gt;nText,pToken-&gt;zText); */
    switch( pToken-&gt;eType ){
      case TT_EOF:
        fprintf(stderr,&quot;%s:%d: Unterminated \&quot;{\&quot;\n&quot;,
           zFilename, nLine);
        nErr++;
        pToken-&gt;eType = TT_Error;
        return nErr;

      case TT_Id:
        if( pTable ){
          IdentTableInsert(pTable,pToken-&gt;zText,pToken-&gt;nText);
        }
        break;

      case TT_Preprocessor:
        if( pTable!=0 ){
          FindIdentifiersInMacro(pToken,pTable);
        }
        break;

      case TT_Other:
        if( (c = pToken-&gt;zText[0])==&#39;{&#39; ){
          nBrace++;
        }else if( c==&#39;}&#39; ){
          nBrace--;
        }
        break;

      default:
        break;
    }
  }
  pToken-&gt;eType = TT_Braces;
  pToken-&gt;nText = 1 + pIn-&gt;i - iStart;
  pToken-&gt;zText = zStart;
  pToken-&gt;nLine = nLine;
  return nErr;
}

/*
** This routine frees up a list of Tokens.  The pComment tokens are
** not cleared by this.  So we leak a little memory when using the -doc
** option.  So what.
*/
static void FreeTokenList(Token *pList){
  Token *pNext;
  while( pList ){
    pNext = pList-&gt;pNext;
    SafeFree(pList);
    pList = pNext;
  }
}

/*
** Tokenize an entire file.  Return a pointer to the list of tokens.
**
** Space for each token is obtained from a separate malloc() call.  The
** calling function is responsible for freeing this space.
**
** If pTable is not NULL, then fill the table with all identifiers seen in
** the input file.
*/
static Token *TokenizeFile(const char *zFile, IdentTable *pTable){
  InStream sIn;
  Token *pFirst = 0, *pLast = 0, *pNew;
  int nErr = 0;

  sIn.z = zFile;
  sIn.i = 0;
  sIn.nLine = 1;
  blockComment = 0;

  while( sIn.z[sIn.i]!=0 ){
    pNew = SafeMalloc( sizeof(Token) );
    nErr += GetBigToken(&amp;sIn,pNew,pTable);
    debug3(TOKENIZER, &quot;Token on line %d: [%.*s]\n&quot;,
       pNew-&gt;nLine, pNew-&gt;nText&lt;50 ? pNew-&gt;nText : 50, pNew-&gt;zText);
    if( pFirst==0 ){
      pFirst = pLast = pNew;
      pNew-&gt;pPrev = 0;
    }else{
      pLast-&gt;pNext = pNew;
      pNew-&gt;pPrev = pLast;
      pLast = pNew;
    }
    if( pNew-&gt;eType==TT_EOF ) break;
  }
  if( pLast ) pLast-&gt;pNext = 0;
  blockComment = 0;
  if( nErr ){
    FreeTokenList(pFirst);
    pFirst = 0;
  }

  return pFirst;
}

#if TEST==1
/*
** Use the following routine to test or debug the tokenizer.
*/
void main(int argc, char **argv){
  char *zFile;
  Token *pList, *p;
  IdentTable sTable;

  if( argc!=2 ){
    fprintf(stderr,&quot;Usage: %s filename\n&quot;,*argv);
    exit(1);
  }
  memset(&amp;sTable,0,sizeof(sTable));
  zFile = ReadFile(argv[1]);
  if( zFile==0 ){
    fprintf(stderr,&quot;Can&#39;t read file \&quot;%s\&quot;\n&quot;,argv[1]);
    exit(1);
  }
  pList = TokenizeFile(zFile,&amp;sTable);
  for(p=pList; p; p=p-&gt;pNext){
    int j;
    switch( p-&gt;eType ){
      case TT_Space:
        printf(&quot;%4d: Space\n&quot;,p-&gt;nLine);
        break;
      case TT_Id:
        printf(&quot;%4d: Id           %.*s\n&quot;,p-&gt;nLine,p-&gt;nText,p-&gt;zText);
        break;
      case TT_Preprocessor:
        printf(&quot;%4d: Preprocessor %.*s\n&quot;,p-&gt;nLine,p-&gt;nText,p-&gt;zText);
        break;
      case TT_Comment:
        printf(&quot;%4d: Comment\n&quot;,p-&gt;nLine);
        break;
      case TT_BlockComment:
        printf(&quot;%4d: Block Comment\n&quot;,p-&gt;nLine);
        break;
      case TT_Number:
        printf(&quot;%4d: Number       %.*s\n&quot;,p-&gt;nLine,p-&gt;nText,p-&gt;zText);
        break;
      case TT_String:
        printf(&quot;%4d: String       %.*s\n&quot;,p-&gt;nLine,p-&gt;nText,p-&gt;zText);
        break;
      case TT_Other:
        printf(&quot;%4d: Other        %.*s\n&quot;,p-&gt;nLine,p-&gt;nText,p-&gt;zText);
        break;
      case TT_Braces:
        for(j=0; j&lt;p-&gt;nText &amp;&amp; j&lt;30 &amp;&amp; p-&gt;zText[j]!=&#39;\n&#39;; j++){}
        printf(&quot;%4d: Braces       %.*s...}\n&quot;,p-&gt;nLine,j,p-&gt;zText);
        break;
      case TT_EOF:
        printf(&quot;%4d: End of file\n&quot;,p-&gt;nLine);
        break;
      default:
        printf(&quot;%4d: type %d\n&quot;,p-&gt;nLine,p-&gt;eType);
        break;
    }
  }
  FreeTokenList(pList);
  SafeFree(zFile);
  IdentTablePrint(&amp;sTable,stdout);
}
#endif

#ifdef DEBUG
/*
** For debugging purposes, write out a list of tokens.
*/
static void PrintTokens(Token *pFirst, Token *pLast){
  int needSpace = 0;
  int c;

  pLast = pLast-&gt;pNext;
  while( pFirst!=pLast ){
    switch( pFirst-&gt;eType ){
      case TT_Preprocessor:
        printf(&quot;\n%.*s\n&quot;,pFirst-&gt;nText,pFirst-&gt;zText);
        needSpace = 0;
        break;

      case TT_Id:
      case TT_Number:
        printf(&quot;%s%.*s&quot;, needSpace ? &quot; &quot; : &quot;&quot;, pFirst-&gt;nText, pFirst-&gt;zText);
        needSpace = 1;
        break;

      default:
        c = pFirst-&gt;zText[0];
        printf(&quot;%s%.*s&quot;,
          (needSpace &amp;&amp; (c==&#39;*&#39; || c==&#39;{&#39;)) ? &quot; &quot; : &quot;&quot;,
          pFirst-&gt;nText, pFirst-&gt;zText);
        needSpace = pFirst-&gt;zText[0]==&#39;,&#39;;
        break;
    }
    pFirst = pFirst-&gt;pNext;
  }
}
#endif

/*
** Convert a sequence of tokens into a string and return a pointer
** to that string.  Space to hold the string is obtained from malloc()
** and must be freed by the calling function.
**
** Certain keywords (EXPORT, PRIVATE, PUBLIC, PROTECTED) are always
** skipped.
**
** If pSkip!=0 then skip over nSkip tokens beginning with pSkip.
**
** If zTerm!=0 then append the text to the end.
*/
static char *TokensToString(
  Token *pFirst,    /* First token in the string */
  Token *pLast,     /* Last token in the string */
  char *zTerm,      /* Terminate the string with this text if not NULL */
  Token *pSkip,     /* Skip this token if not NULL */
  int nSkip         /* Skip a total of this many tokens */
){
  char *zReturn;
  String str;
  int needSpace = 0;
  int c;
  int iSkip = 0;
  int skipOne = 0;

  StringInit(&amp;str);
  pLast = pLast-&gt;pNext;
  while( pFirst!=pLast ){
    if( pFirst==pSkip ){ iSkip = nSkip; }
    if( iSkip&gt;0 ){
      iSkip--;
      pFirst=pFirst-&gt;pNext;
      continue;
    }
    switch( pFirst-&gt;eType ){
      case TT_Preprocessor:
        StringAppend(&amp;str,&quot;\n&quot;,1);
        StringAppend(&amp;str,pFirst-&gt;zText,pFirst-&gt;nText);
        StringAppend(&amp;str,&quot;\n&quot;,1);
        needSpace = 0;
        break;

      case TT_Id:
        switch( pFirst-&gt;zText[0] ){
          case &#39;E&#39;:
            if( pFirst-&gt;nText==6 &amp;&amp; strncmp(pFirst-&gt;zText,&quot;EXPORT&quot;,6)==0 ){
              skipOne = 1;
            }
            break;
          case &#39;P&#39;:
            switch( pFirst-&gt;nText ){
              case 6:  skipOne = !strncmp(pFirst-&gt;zText,&quot;PUBLIC&quot;, 6);    break;
              case 7:  skipOne = !strncmp(pFirst-&gt;zText,&quot;PRIVATE&quot;,7);    break;
              case 9:  skipOne = !strncmp(pFirst-&gt;zText,&quot;PROTECTED&quot;,9);  break;
              default: break;
            }
            break;
          default:
            break;
        }
        if( skipOne ){
          pFirst = pFirst-&gt;pNext;
          continue;
        }
        /* Fall thru to the next case */
      case TT_Number:
        if( needSpace ){
          StringAppend(&amp;str,&quot; &quot;,1);
        }
        StringAppend(&amp;str,pFirst-&gt;zText,pFirst-&gt;nText);
        needSpace = 1;
        break;

      default:
        c = pFirst-&gt;zText[0];
        if( needSpace &amp;&amp; (c==&#39;*&#39; || c==&#39;{&#39;) ){
          StringAppend(&amp;str,&quot; &quot;,1);
        }
        StringAppend(&amp;str,pFirst-&gt;zText,pFirst-&gt;nText);
        /* needSpace = pFirst-&gt;zText[0]==&#39;,&#39;; */
        needSpace = 0;
        break;
    }
    pFirst = pFirst-&gt;pNext;
  }
  if( zTerm &amp;&amp; *zTerm ){
    StringAppend(&amp;str,zTerm,strlen(zTerm));
  }
  zReturn = StrDup(StringGet(&amp;str),0);
  StringReset(&amp;str);
  return zReturn;
}

/*
** This routine is called when we see one of the keywords &quot;struct&quot;,
** &quot;enum&quot;, &quot;union&quot; or &quot;class&quot;.  This might be the beginning of a
** type declaration.  This routine will process the declaration and
** remove the declaration tokens from the input stream.
**
** If this is a type declaration that is immediately followed by a
** semicolon (in other words it isn&#39;t also a variable definition)
** then set *pReset to &#39;;&#39;.  Otherwise leave *pReset at 0.  The
** *pReset flag causes the parser to skip ahead to the next token
** that begins with the value placed in the *pReset flag, if that
** value is different from 0.
*/
static int ProcessTypeDecl(Token *pList, int flags, int *pReset){
  Token *pName, *pEnd;
  Decl *pDecl;
  String str;
  int need_to_collapse = 1;
  int type = 0;

  *pReset = 0;
  if( pList==0 || pList-&gt;pNext==0 || pList-&gt;pNext-&gt;eType!=TT_Id ){
    return 0;
  }
  pName = pList-&gt;pNext;

  /* Catch the case of &quot;struct Foo;&quot; and skip it. */
  if( pName-&gt;pNext &amp;&amp; pName-&gt;pNext-&gt;zText[0]==&#39;;&#39; ){
    *pReset = &#39;;&#39;;
    return 0;
  }

  for(pEnd=pName-&gt;pNext; pEnd &amp;&amp; pEnd-&gt;eType!=TT_Braces; pEnd=pEnd-&gt;pNext){
    switch( pEnd-&gt;zText[0] ){
      case &#39;(&#39;:
      case &#39;)&#39;:
      case &#39;*&#39;:
      case &#39;[&#39;:
      case &#39;=&#39;:
      case &#39;;&#39;:
        return 0;
    }
  }
  if( pEnd==0 ){
    return 0;
  }

  /*
  ** At this point, we know we have a type declaration that is bounded
  ** by pList and pEnd and has the name pName.
  */

  /*
  ** If the braces are followed immediately by a semicolon, then we are
  ** dealing a type declaration only.  There is not variable definition
  ** following the type declaration.  So reset...
  */
  if( pEnd-&gt;pNext==0 || pEnd-&gt;pNext-&gt;zText[0]==&#39;;&#39; ){
    *pReset = &#39;;&#39;;
    need_to_collapse = 0;
  }else{
    need_to_collapse = 1;
  }

  if( proto_static==0 &amp;&amp; (flags &amp; (PS_Local|PS_Export|PS_Interface))==0 ){
    /* Ignore these objects unless they are explicitly declared as interface,
    ** or unless the &quot;-local&quot; command line option was specified. */
    *pReset = &#39;;&#39;;
    return 0;
  }

#ifdef DEBUG
  if( debugMask &amp; PARSER ){
    printf(&quot;**** Found type: %.*s %.*s...\n&quot;,
      pList-&gt;nText, pList-&gt;zText, pName-&gt;nText, pName-&gt;zText);
    PrintTokens(pList,pEnd);
    printf(&quot;;\n&quot;);
  }
#endif

  /*
  ** Create a new Decl object for this definition.  Actually, if this
  ** is a C++ class definition, then the Decl object might already exist,
  ** so check first for that case before creating a new one.
  */
  switch( *pList-&gt;zText ){
    case &#39;c&#39;:  type = TY_Class;        break;
    case &#39;s&#39;:  type = TY_Structure;    break;
    case &#39;e&#39;:  type = TY_Enumeration;  break;
    case &#39;u&#39;:  type = TY_Union;        break;
    default:   /* Can&#39;t Happen */      break;
  }
  if( type!=TY_Class ){
    pDecl = 0;
  }else{
    pDecl = FindDecl(pName-&gt;zText, pName-&gt;nText);
    if( pDecl &amp;&amp; (pDecl-&gt;flags &amp; type)!=type ) pDecl = 0;
  }
  if( pDecl==0 ){
    pDecl = CreateDecl(pName-&gt;zText,pName-&gt;nText);
  }
  if( (flags &amp; PS_Static) || !(flags &amp; (PS_Interface|PS_Export)) ){
    DeclSetProperty(pDecl,DP_Local);
  }
  DeclSetProperty(pDecl,type);

  /* The object has a full declaration only if it is contained within
  ** &quot;#if INTERFACE...#endif&quot; or &quot;#if EXPORT_INTERFACE...#endif&quot; or
  ** &quot;#if LOCAL_INTERFACE...#endif&quot;.  Otherwise, we only give it a
  ** forward declaration.
  */
  if( flags &amp; (PS_Local | PS_Export | PS_Interface)  ){
    pDecl-&gt;zDecl = TokensToString(pList,pEnd,&quot;;\n&quot;,0,0);
  }else{
    pDecl-&gt;zDecl = 0;
  }
  pDecl-&gt;pComment = pList-&gt;pComment;
  StringInit(&amp;str);
  StringAppend(&amp;str,&quot;typedef &quot;,0);
  StringAppend(&amp;str,pList-&gt;zText,pList-&gt;nText);
  StringAppend(&amp;str,&quot; &quot;,0);
  StringAppend(&amp;str,pName-&gt;zText,pName-&gt;nText);
  StringAppend(&amp;str,&quot; &quot;,0);
  StringAppend(&amp;str,pName-&gt;zText,pName-&gt;nText);
  StringAppend(&amp;str,&quot;;\n&quot;,2);
  pDecl-&gt;zFwd = StrDup(StringGet(&amp;str),0);
  StringReset(&amp;str);
  StringInit(&amp;str);
  StringAppend(&amp;str,pList-&gt;zText,pList-&gt;nText);
  StringAppend(&amp;str,&quot; &quot;,0);
  StringAppend(&amp;str,pName-&gt;zText,pName-&gt;nText);
  StringAppend(&amp;str,&quot;;\n&quot;,2);
  pDecl-&gt;zFwdCpp = StrDup(StringGet(&amp;str),0);
  StringReset(&amp;str);
  if( flags &amp; PS_Export ){
    DeclSetProperty(pDecl,DP_Export);
  }else if( flags &amp; PS_Local ){
    DeclSetProperty(pDecl,DP_Local);
  }

  /* Here&#39;s something weird.  ANSI-C doesn&#39;t allow a forward declaration
  ** of an enumeration.  So we have to build the typedef into the
  ** definition.
  */
  if( pDecl-&gt;zDecl &amp;&amp; DeclHasProperty(pDecl, TY_Enumeration) ){
    StringInit(&amp;str);
    StringAppend(&amp;str,pDecl-&gt;zDecl,0);
    StringAppend(&amp;str,pDecl-&gt;zFwd,0);
    SafeFree(pDecl-&gt;zDecl);
    SafeFree(pDecl-&gt;zFwd);
    pDecl-&gt;zFwd = 0;
    pDecl-&gt;zDecl = StrDup(StringGet(&amp;str),0);
    StringReset(&amp;str);
  }

  if( pName-&gt;pNext-&gt;zText[0]==&#39;:&#39; ){
    DeclSetProperty(pDecl,DP_Cplusplus);
  }
  if( pName-&gt;nText==5 &amp;&amp; strncmp(pName-&gt;zText,&quot;class&quot;,5)==0 ){
    DeclSetProperty(pDecl,DP_Cplusplus);
  }

  /*
  ** Remove all but pList and pName from the input stream.
  */
  if( need_to_collapse ){
    while( pEnd!=pName ){
      Token *pPrev = pEnd-&gt;pPrev;
      pPrev-&gt;pNext = pEnd-&gt;pNext;
      pEnd-&gt;pNext-&gt;pPrev = pPrev;
      SafeFree(pEnd);
      pEnd = pPrev;
    }
  }
  return 0;
}

/*
** Given a list of tokens that declare something (a function, procedure,
** variable or typedef) find the token which contains the name of the
** thing being declared.
**
** Algorithm:
**
**   The name is:
**
**     1.  The first identifier that is followed by a &quot;[&quot;, or
**
**     2.  The first identifier that is followed by a &quot;(&quot; where the
**         &quot;(&quot; is followed by another identifier, or
**
**     3.  The first identifier followed by &quot;::&quot;, or
**
**     4.  If none of the above, then the last identifier.
**
**   In all of the above, certain reserved words (like &quot;char&quot;) are
**   not considered identifiers.
*/
static Token *FindDeclName(Token *pFirst, Token *pLast){
  Token *pName = 0;
  Token *p;
  int c;

  if( pFirst==0 || pLast==0 ){
    return 0;
  }
  pLast = pLast-&gt;pNext;
  for(p=pFirst; p &amp;&amp; p!=pLast; p=p-&gt;pNext){
    if( p-&gt;eType==TT_Id ){
      static IdentTable sReserved;
      static int isInit = 0;
      static const char *aWords[] = { &quot;char&quot;, &quot;class&quot;,
       &quot;const&quot;, &quot;double&quot;, &quot;enum&quot;, &quot;extern&quot;, &quot;EXPORT&quot;, &quot;ET_PROC&quot;,
       &quot;float&quot;, &quot;int&quot;, &quot;long&quot;,
       &quot;PRIVATE&quot;, &quot;PROTECTED&quot;, &quot;PUBLIC&quot;,
       &quot;register&quot;, &quot;static&quot;, &quot;struct&quot;, &quot;sizeof&quot;, &quot;signed&quot;, &quot;typedef&quot;,
       &quot;union&quot;, &quot;volatile&quot;, &quot;virtual&quot;, &quot;void&quot;, };

      if( !isInit ){
        int i;
        for(i=0; i&lt;sizeof(aWords)/sizeof(aWords[0]); i++){
          IdentTableInsert(&amp;sReserved,aWords[i],0);
        }
        isInit = 1;
      }
      if( !IdentTableTest(&amp;sReserved,p-&gt;zText,p-&gt;nText) ){
        pName = p;
      }
    }else if( p==pFirst ){
      continue;
    }else if( (c=p-&gt;zText[0])==&#39;[&#39; &amp;&amp; pName ){
      break;
    }else if( c==&#39;(&#39; &amp;&amp; p-&gt;pNext &amp;&amp; p-&gt;pNext-&gt;eType==TT_Id &amp;&amp; pName ){
      break;
    }else if( c==&#39;:&#39; &amp;&amp; p-&gt;zText[1]==&#39;:&#39; &amp;&amp; pName ){
      break;
    }
  }
  return pName;
}

/*
** This routine is called when we see a method for a class that begins
** with the PUBLIC, PRIVATE, or PROTECTED keywords.  Such methods are
** added to their class definitions.
*/
static int ProcessMethodDef(Token *pFirst, Token *pLast, int flags){
  Token *pClass;
  char *zDecl;
  Decl *pDecl;
  String str;
  int type;

  pLast = pLast-&gt;pPrev;
  while( pFirst-&gt;zText[0]==&#39;P&#39; ){
    int rc = 1;
    switch( pFirst-&gt;nText ){
      case 6:  rc = strncmp(pFirst-&gt;zText,&quot;PUBLIC&quot;,6); break;
      case 7:  rc = strncmp(pFirst-&gt;zText,&quot;PRIVATE&quot;,7); break;
      case 9:  rc = strncmp(pFirst-&gt;zText,&quot;PROTECTED&quot;,9); break;
      default:  break;
    }
    if( rc ) break;
    pFirst = pFirst-&gt;pNext;
  }
  pClass = FindDeclName(pFirst,pLast);
  if( pClass==0 ){
    fprintf(stderr,&quot;%s:%d: Unable to find the class name for this method\n&quot;,
       zFilename, pFirst-&gt;nLine);
    return 1;
  }
  pDecl = FindDecl(pClass-&gt;zText, pClass-&gt;nText);
  if( pDecl==0 || (pDecl-&gt;flags &amp; TY_Class)!=TY_Class ){
    pDecl = CreateDecl(pClass-&gt;zText, pClass-&gt;nText);
    DeclSetProperty(pDecl, TY_Class);
  }
  StringInit(&amp;str);
  if( pDecl-&gt;zExtra ){
    StringAppend(&amp;str, pDecl-&gt;zExtra, 0);
    SafeFree(pDecl-&gt;zExtra);
    pDecl-&gt;zExtra = 0;
  }
  type = flags &amp; PS_PPP;
  if( pDecl-&gt;extraType!=type ){
    if( type &amp; PS_Public ){
      StringAppend(&amp;str, &quot;public:\n&quot;, 0);
      pDecl-&gt;extraType = PS_Public;
    }else if( type &amp; PS_Protected ){
      StringAppend(&amp;str, &quot;protected:\n&quot;, 0);
      pDecl-&gt;extraType = PS_Protected;
    }else if( type &amp; PS_Private ){
      StringAppend(&amp;str, &quot;private:\n&quot;, 0);
      pDecl-&gt;extraType = PS_Private;
    }
  }
  StringAppend(&amp;str, &quot;  &quot;, 0);
  zDecl = TokensToString(pFirst, pLast, &quot;;\n&quot;, pClass, 2);
  StringAppend(&amp;str, zDecl, 0);
  SafeFree(zDecl);
  pDecl-&gt;zExtra = StrDup(StringGet(&amp;str), 0);
  StringReset(&amp;str);
  return 0;
}

/*
** This routine is called when we see a function or procedure definition.
** We make an entry in the declaration table that is a prototype for this
** function or procedure.
*/
static int ProcessProcedureDef(Token *pFirst, Token *pLast, int flags){
  Token *pName;
  Decl *pDecl;
  Token *pCode;

  if( pFirst==0 || pLast==0 ){
    return 0;
  }
  if( flags &amp; PS_Method ){
    if( flags &amp; PS_PPP ){
      return ProcessMethodDef(pFirst, pLast, flags);
    }else{
      return 0;
    }
  }
  if( (flags &amp; PS_Static)!=0 &amp;&amp; !proto_static ){
    return 0;
  }
  pCode = pLast;
  while( pLast &amp;&amp; pLast!=pFirst &amp;&amp; pLast-&gt;zText[0]!=&#39;)&#39; ){
    pLast = pLast-&gt;pPrev;
  }
  if( pLast==0 || pLast==pFirst || pFirst-&gt;pNext==pLast ){
    fprintf(stderr,&quot;%s:%d: Unrecognized syntax.\n&quot;,
      zFilename, pFirst-&gt;nLine);
    return 1;
  }
  if( flags &amp; (PS_Interface|PS_Export|PS_Local) ){
    fprintf(stderr,&quot;%s:%d: Missing \&quot;inline\&quot; on function or procedure.\n&quot;,
      zFilename, pFirst-&gt;nLine);
    return 1;
  }
  pName = FindDeclName(pFirst,pLast);
  if( pName==0 ){
    fprintf(stderr,&quot;%s:%d: Malformed function or procedure definition.\n&quot;,
      zFilename, pFirst-&gt;nLine);
    return 1;
  }

  /*
  ** At this point we&#39;ve isolated a procedure declaration between pFirst
  ** and pLast with the name pName.
  */
#ifdef DEBUG
  if( debugMask &amp; PARSER ){
    printf(&quot;**** Found routine: %.*s on line %d...\n&quot;, pName-&gt;nText,
       pName-&gt;zText, pFirst-&gt;nLine);
    PrintTokens(pFirst,pLast);
    printf(&quot;;\n&quot;);
  }
#endif
  pDecl = CreateDecl(pName-&gt;zText,pName-&gt;nText);
  pDecl-&gt;pComment = pFirst-&gt;pComment;
  if( pCode &amp;&amp; pCode-&gt;eType==TT_Braces ){
    pDecl-&gt;tokenCode = *pCode;
  }
  DeclSetProperty(pDecl,TY_Subroutine);
  pDecl-&gt;zDecl = TokensToString(pFirst,pLast,&quot;;\n&quot;,0,0);
  if( (flags &amp; (PS_Static|PS_Local2))!=0 ){
    DeclSetProperty(pDecl,DP_Local);
  }else if( (flags &amp; (PS_Export2))!=0 ){
    DeclSetProperty(pDecl,DP_Export);
  }

  if( flags &amp; DP_Cplusplus ){
    DeclSetProperty(pDecl,DP_Cplusplus);
  }else{
    DeclSetProperty(pDecl,DP_ExternCReqd);
  }

  return 0;
}

/*
** This routine is called whenever we see the &quot;inline&quot; keyword.  We
** need to seek-out the inline function or procedure and make a
** declaration out of the entire definition.
*/
static int ProcessInlineProc(Token *pFirst, int flags, int *pReset){
  Token *pName;
  Token *pEnd;
  Decl *pDecl;

  for(pEnd=pFirst; pEnd; pEnd = pEnd-&gt;pNext){
    if( pEnd-&gt;zText[0]==&#39;{&#39; || pEnd-&gt;zText[0]==&#39;;&#39; ){
      *pReset = pEnd-&gt;zText[0];
      break;
    }
  }
  if( pEnd==0 ){
    *pReset = &#39;;&#39;;
    fprintf(stderr,&quot;%s:%d: incomplete inline procedure definition\n&quot;,
      zFilename, pFirst-&gt;nLine);
    return 1;
  }
  pName = FindDeclName(pFirst,pEnd);
  if( pName==0 ){
    fprintf(stderr,&quot;%s:%d: malformed inline procedure definition\n&quot;,
      zFilename, pFirst-&gt;nLine);
    return 1;
  }

#ifdef DEBUG
  if( debugMask &amp; PARSER ){
    printf(&quot;**** Found inline routine: %.*s on line %d...\n&quot;,
       pName-&gt;nText, pName-&gt;zText, pFirst-&gt;nLine);
    PrintTokens(pFirst,pEnd);
    printf(&quot;\n&quot;);
  }
#endif
  pDecl = CreateDecl(pName-&gt;zText,pName-&gt;nText);
  pDecl-&gt;pComment = pFirst-&gt;pComment;
  DeclSetProperty(pDecl,TY_Subroutine);
  pDecl-&gt;zDecl = TokensToString(pFirst,pEnd,&quot;;\n&quot;,0,0);
  if( (flags &amp; (PS_Static|PS_Local|PS_Local2)) ){
    DeclSetProperty(pDecl,DP_Local);
  }else if( flags &amp; (PS_Export|PS_Export2) ){
    DeclSetProperty(pDecl,DP_Export);
  }

  if( flags &amp; DP_Cplusplus ){
    DeclSetProperty(pDecl,DP_Cplusplus);
  }else{
    DeclSetProperty(pDecl,DP_ExternCReqd);
  }

  return 0;
}

/*
** Determine if the tokens between pFirst and pEnd form a variable
** definition or a function prototype.  Return TRUE if we are dealing
** with a variable defintion and FALSE for a prototype.
**
** pEnd is the token that ends the object.  It can be either a &#39;;&#39; or
** a &#39;=&#39;.  If it is &#39;=&#39;, then assume we have a variable definition.
**
** If pEnd is &#39;;&#39;, then the determination is more difficult.  We have
** to search for an occurrence of an ID followed immediately by &#39;(&#39;.
** If found, we have a prototype.  Otherwise we are dealing with a
** variable definition.
*/
static int isVariableDef(Token *pFirst, Token *pEnd){
  if( pEnd &amp;&amp; pEnd-&gt;zText[0]==&#39;=&#39; &amp;&amp;
    (pEnd-&gt;pPrev-&gt;nText!=8 || strncmp(pEnd-&gt;pPrev-&gt;zText,&quot;operator&quot;,8)!=0)
  ){
    return 1;
  }
  while( pFirst &amp;&amp; pFirst!=pEnd &amp;&amp; pFirst-&gt;pNext &amp;&amp; pFirst-&gt;pNext!=pEnd ){
    if( pFirst-&gt;eType==TT_Id &amp;&amp; pFirst-&gt;pNext-&gt;zText[0]==&#39;(&#39; ){
      return 0;
    }
    pFirst = pFirst-&gt;pNext;
  }
  return 1;
}

/*
** Return TRUE if pFirst is the first token of a static assert.
*/
static int isStaticAssert(Token *pFirst){
  if( (pFirst-&gt;nText==13 &amp;&amp; strncmp(pFirst-&gt;zText, &quot;static_assert&quot;, 13)==0)
   || (pFirst-&gt;nText==14 &amp;&amp; strncmp(pFirst-&gt;zText, &quot;_Static_assert&quot;, 14)==0)
  ){
    return 1;
  }else{
    return 0;
  }
}

/*
** This routine is called whenever we encounter a &quot;;&quot; or &quot;=&quot;.  The stuff
** between pFirst and pLast constitutes either a typedef or a global
** variable definition.  Do the right thing.
*/
static int ProcessDecl(Token *pFirst, Token *pEnd, int flags){
  Token *pName;
  Decl *pDecl;
  int isLocal = 0;
  int isVar;
  int nErr = 0;

  if( pFirst==0 || pEnd==0 ){
    return 0;
  }
  if( flags &amp; PS_Typedef ){
    if( (flags &amp; (PS_Export2|PS_Local2))!=0 ){
      fprintf(stderr,&quot;%s:%d: \&quot;EXPORT\&quot; or \&quot;LOCAL\&quot; ignored before typedef.\n&quot;,
        zFilename, pFirst-&gt;nLine);
      nErr++;
    }
    if( (flags &amp; (PS_Interface|PS_Export|PS_Local|DP_Cplusplus))==0 ){
      /* It is illegal to duplicate a typedef in C (but OK in C++).
      ** So don&#39;t record typedefs that aren&#39;t within a C++ file or
      ** within #if INTERFACE..#endif */
      return nErr;
    }
    if( (flags &amp; (PS_Interface|PS_Export|PS_Local))==0 &amp;&amp; proto_static==0 ){
      /* Ignore typedefs that are not with &quot;#if INTERFACE..#endif&quot; unless
      ** the &quot;-local&quot; command line option is used. */
      return nErr;
    }
    if( (flags &amp; (PS_Interface|PS_Export))==0 ){
      /* typedefs are always local, unless within #if INTERFACE..#endif */
      isLocal = 1;
    }
  }else if( flags &amp; (PS_Static|PS_Local2) ){
    if( proto_static==0 &amp;&amp; (flags &amp; PS_Local2)==0 ){
      /* Don&#39;t record static variables unless the &quot;-local&quot; command line
      ** option was specified or the &quot;LOCAL&quot; keyword is used. */
      return nErr;
    }
    while( pFirst!=0 &amp;&amp; pFirst-&gt;pNext!=pEnd &amp;&amp;
       ((pFirst-&gt;nText==6 &amp;&amp; strncmp(pFirst-&gt;zText,&quot;static&quot;,6)==0)
        || (pFirst-&gt;nText==5 &amp;&amp; strncmp(pFirst-&gt;zText,&quot;LOCAL&quot;,6)==0))
    ){
      /* Lose the initial &quot;static&quot; or local from local variables.
      ** We&#39;ll prepend &quot;extern&quot; later. */
      pFirst = pFirst-&gt;pNext;
      isLocal = 1;
    }
    if( pFirst==0 || !isLocal ){
      return nErr;
    }
  }else if( flags &amp; PS_Method ){
    /* Methods are declared by their class.  Don&#39;t declare separately. */
    return nErr;
  }else if( isStaticAssert(pFirst) ){
    return 0;
  }
  isVar =  (flags &amp; (PS_Typedef|PS_Method))==0 &amp;&amp; isVariableDef(pFirst,pEnd);
  if( isVar &amp;&amp; (flags &amp; (PS_Interface|PS_Export|PS_Local))!=0
  &amp;&amp; (flags &amp; PS_Extern)==0 ){
    fprintf(stderr,&quot;%s:%d: Can&#39;t define a variable in this context\n&quot;,
      zFilename, pFirst-&gt;nLine);
    nErr++;
  }
  pName = FindDeclName(pFirst,pEnd-&gt;pPrev);
  if( pName==0 ){
    if( pFirst-&gt;nText==4 &amp;&amp; strncmp(pFirst-&gt;zText,&quot;enum&quot;,4)==0 ){
      /* Ignore completely anonymous enums.  See documentation section 3.8.1. */
      return nErr;
    }else{
      fprintf(stderr,&quot;%s:%d: Can&#39;t find a name for the object declared here.\n&quot;,
        zFilename, pFirst-&gt;nLine);
      return nErr+1;
    }
  }

#ifdef DEBUG
  if( debugMask &amp; PARSER ){
    if( flags &amp; PS_Typedef ){
      printf(&quot;**** Found typedef %.*s at line %d...\n&quot;,
        pName-&gt;nText, pName-&gt;zText, pName-&gt;nLine);
    }else if( isVar ){
      printf(&quot;**** Found variable %.*s at line %d...\n&quot;,
        pName-&gt;nText, pName-&gt;zText, pName-&gt;nLine);
    }else{
      printf(&quot;**** Found prototype %.*s at line %d...\n&quot;,
        pName-&gt;nText, pName-&gt;zText, pName-&gt;nLine);
    }
    PrintTokens(pFirst,pEnd-&gt;pPrev);
    printf(&quot;;\n&quot;);
  }
#endif

  pDecl = CreateDecl(pName-&gt;zText,pName-&gt;nText);
  if( (flags &amp; PS_Typedef) ){
    DeclSetProperty(pDecl, TY_Typedef);
  }else if( isVar ){
    DeclSetProperty(pDecl,DP_ExternReqd | TY_Variable);
    if( !(flags &amp; DP_Cplusplus) ){
      DeclSetProperty(pDecl,DP_ExternCReqd);
    }
  }else{
    DeclSetProperty(pDecl, TY_Subroutine);
    if( !(flags &amp; DP_Cplusplus) ){
      DeclSetProperty(pDecl,DP_ExternCReqd);
    }
  }
  pDecl-&gt;pComment = pFirst-&gt;pComment;
  pDecl-&gt;zDecl = TokensToString(pFirst,pEnd-&gt;pPrev,&quot;;\n&quot;,0,0);
  if( isLocal || (flags &amp; (PS_Local|PS_Local2))!=0 ){
    DeclSetProperty(pDecl,DP_Local);
  }else if( flags &amp; (PS_Export|PS_Export2) ){
    DeclSetProperty(pDecl,DP_Export);
  }
  if( flags &amp; DP_Cplusplus ){
    DeclSetProperty(pDecl,DP_Cplusplus);
  }
  return nErr;
}

/*
** Push an if condition onto the if stack
*/
static void PushIfMacro(
  const char *zPrefix,      /* A prefix, like &quot;define&quot; or &quot;!&quot; */
  const char *zText,        /* The condition */
  int nText,                /* Number of characters in zText */
  int nLine,                /* Line number where this macro occurs */
  int flags                 /* Either 0, PS_Interface, PS_Export or PS_Local */
){
  Ifmacro *pIf;
  int nByte;

  nByte = sizeof(Ifmacro);
  if( zText ){
    if( zPrefix ){
      nByte += strlen(zPrefix) + 2;
    }
    nByte += nText + 1;
  }
  pIf = SafeMalloc( nByte );
  if( zText ){
    pIf-&gt;zCondition = (char*)&amp;pIf[1];
    if( zPrefix ){
      sprintf(pIf-&gt;zCondition,&quot;%s(%.*s)&quot;,zPrefix,nText,zText);
    }else{
      sprintf(pIf-&gt;zCondition,&quot;%.*s&quot;,nText,zText);
    }
  }else{
    pIf-&gt;zCondition = 0;
  }
  pIf-&gt;nLine = nLine;
  pIf-&gt;flags = flags;
  pIf-&gt;pNext = ifStack;
  ifStack = pIf;
}

/*
** This routine is called to handle all preprocessor directives.
**
** This routine will recompute the value of *pPresetFlags to be the
** logical or of all flags on all nested #ifs.  The #ifs that set flags
** are as follows:
**
**        conditional                   flag set
**        ------------------------      --------------------
**        #if INTERFACE                 PS_Interface
**        #if EXPORT_INTERFACE          PS_Export
**        #if LOCAL_INTERFACE           PS_Local
**
** For example, if after processing the preprocessor token given
** by pToken there is an &quot;#if INTERFACE&quot; on the preprocessor
** stack, then *pPresetFlags will be set to PS_Interface.
*/
static int ParsePreprocessor(Token *pToken, int flags, int *pPresetFlags){
  const char *zCmd;
  int nCmd;
  const char *zArg;
  int nArg;
  int nErr = 0;
  Ifmacro *pIf;

  zCmd = &amp;pToken-&gt;zText[1];
  while( isspace(*zCmd) &amp;&amp; *zCmd!=&#39;\n&#39; ){
    zCmd++;
  }
  if( !isalpha(*zCmd) ){
    return 0;
  }
  nCmd = 1;
  while( isalpha(zCmd[nCmd]) ){
    nCmd++;
  }

  if( nCmd==5 &amp;&amp; strncmp(zCmd,&quot;endif&quot;,5)==0 ){
    /*
    ** Pop the if stack
    */
    pIf = ifStack;
    if( pIf==0 ){
      fprintf(stderr,&quot;%s:%d: extra &#39;#endif&#39;.\n&quot;,zFilename,pToken-&gt;nLine);
      return 1;
    }
    ifStack = pIf-&gt;pNext;
    SafeFree(pIf);
  }else if( nCmd==6 &amp;&amp; strncmp(zCmd,&quot;define&quot;,6)==0 ){
    /*
    ** Record a #define if we are in PS_Interface or PS_Export
    */
    Decl *pDecl;
    if( !(flags &amp; (PS_Local|PS_Interface|PS_Export)) ){ return 0; }
    zArg = &amp;zCmd[6];
    while( *zArg &amp;&amp; isspace(*zArg) &amp;&amp; *zArg!=&#39;\n&#39; ){
      zArg++;
    }
    if( *zArg==0 || *zArg==&#39;\n&#39; ){ return 0; }
    for(nArg=0; ISALNUM(zArg[nArg]); nArg++){}
    if( nArg==0 ){ return 0; }
    pDecl = CreateDecl(zArg,nArg);
    pDecl-&gt;pComment = pToken-&gt;pComment;
    DeclSetProperty(pDecl,TY_Macro);
    pDecl-&gt;zDecl = SafeMalloc( pToken-&gt;nText + 2 );
    sprintf(pDecl-&gt;zDecl,&quot;%.*s\n&quot;,pToken-&gt;nText,pToken-&gt;zText);
    if( flags &amp; PS_Export ){
      DeclSetProperty(pDecl,DP_Export);
    }else if( flags &amp; PS_Local ){
      DeclSetProperty(pDecl,DP_Local);
    }
  }else if( nCmd==7 &amp;&amp; strncmp(zCmd,&quot;include&quot;,7)==0 ){
    /*
    ** Record an #include if we are in PS_Interface or PS_Export
    */
    Include *pInclude;
    char *zIf;

    if( !(flags &amp; (PS_Interface|PS_Export)) ){ return 0; }
    zArg = &amp;zCmd[7];
    while( *zArg &amp;&amp; isspace(*zArg) ){ zArg++; }
    for(nArg=0; !isspace(zArg[nArg]); nArg++){}
    if( (zArg[0]==&#39;&quot;&#39; &amp;&amp; zArg[nArg-1]!=&#39;&quot;&#39;)
      ||(zArg[0]==&#39;&lt;&#39; &amp;&amp; zArg[nArg-1]!=&#39;&gt;&#39;)
    ){
      fprintf(stderr,&quot;%s:%d: malformed #include statement.\n&quot;,
        zFilename,pToken-&gt;nLine);
      return 1;
    }
    zIf = GetIfString();
    if( zIf ){
      pInclude = SafeMalloc( sizeof(Include) + nArg*2 + strlen(zIf) + 10 );
      pInclude-&gt;zFile = (char*)&amp;pInclude[1];
      pInclude-&gt;zLabel = &amp;pInclude-&gt;zFile[nArg+1];
      sprintf(pInclude-&gt;zFile,&quot;%.*s&quot;,nArg,zArg);
      sprintf(pInclude-&gt;zLabel,&quot;%.*s:%s&quot;,nArg,zArg,zIf);
      pInclude-&gt;zIf = &amp;pInclude-&gt;zLabel[nArg+1];
      SafeFree(zIf);
    }else{
      pInclude = SafeMalloc( sizeof(Include) + nArg + 1 );
      pInclude-&gt;zFile = (char*)&amp;pInclude[1];
      sprintf(pInclude-&gt;zFile,&quot;%.*s&quot;,nArg,zArg);
      pInclude-&gt;zIf = 0;
      pInclude-&gt;zLabel = pInclude-&gt;zFile;
    }
    pInclude-&gt;pNext = includeList;
    includeList = pInclude;
  }else if( nCmd==2 &amp;&amp; strncmp(zCmd,&quot;if&quot;,2)==0 ){
    /*
    ** Push an #if.  Watch for the special cases of INTERFACE
    ** and EXPORT_INTERFACE and LOCAL_INTERFACE
    */
    zArg = &amp;zCmd[2];
    while( *zArg &amp;&amp; isspace(*zArg) &amp;&amp; *zArg!=&#39;\n&#39; ){
      zArg++;
    }
    if( *zArg==0 || *zArg==&#39;\n&#39; ){ return 0; }
    nArg = pToken-&gt;nText + (int)(pToken-&gt;zText - zArg);
    if( nArg==9 &amp;&amp; strncmp(zArg,&quot;INTERFACE&quot;,9)==0 ){
      PushIfMacro(0,0,0,pToken-&gt;nLine,PS_Interface);
    }else if( nArg==16 &amp;&amp; strncmp(zArg,&quot;EXPORT_INTERFACE&quot;,16)==0 ){
      PushIfMacro(0,0,0,pToken-&gt;nLine,PS_Export);
    }else if( nArg==15 &amp;&amp; strncmp(zArg,&quot;LOCAL_INTERFACE&quot;,15)==0 ){
      PushIfMacro(0,0,0,pToken-&gt;nLine,PS_Local);
    }else if( nArg==15 &amp;&amp; strncmp(zArg,&quot;MAKEHEADERS_STOPLOCAL_INTERFACE&quot;,15)==0 ){
      PushIfMacro(0,0,0,pToken-&gt;nLine,PS_Local);
    }else{
      PushIfMacro(0,zArg,nArg,pToken-&gt;nLine,0);
    }
  }else if( nCmd==5 &amp;&amp; strncmp(zCmd,&quot;ifdef&quot;,5)==0 ){
    /*
    ** Push an #ifdef.
    */
    zArg = &amp;zCmd[5];
    while( *zArg &amp;&amp; isspace(*zArg) &amp;&amp; *zArg!=&#39;\n&#39; ){
      zArg++;
    }
    if( *zArg==0 || *zArg==&#39;\n&#39; ){ return 0; }
    nArg = pToken-&gt;nText + (int)(pToken-&gt;zText - zArg);
    PushIfMacro(&quot;defined&quot;,zArg,nArg,pToken-&gt;nLine,0);
  }else if( nCmd==6 &amp;&amp; strncmp(zCmd,&quot;ifndef&quot;,6)==0 ){
    /*
    ** Push an #ifndef.
    */
    zArg = &amp;zCmd[6];
    while( *zArg &amp;&amp; isspace(*zArg) &amp;&amp; *zArg!=&#39;\n&#39; ){
      zArg++;
    }
    if( *zArg==0 || *zArg==&#39;\n&#39; ){ return 0; }
    nArg = pToken-&gt;nText + (int)(pToken-&gt;zText - zArg);
    PushIfMacro(&quot;!defined&quot;,zArg,nArg,pToken-&gt;nLine,0);
  }else if( nCmd==4 &amp;&amp; strncmp(zCmd,&quot;else&quot;,4)==0 ){
    /*
    ** Invert the #if on the top of the stack
    */
    if( ifStack==0 ){
      fprintf(stderr,&quot;%s:%d: &#39;#else&#39; without an &#39;#if&#39;\n&quot;,zFilename,
         pToken-&gt;nLine);
      return 1;
    }
    pIf = ifStack;
    if( pIf-&gt;zCondition ){
      ifStack = ifStack-&gt;pNext;
      PushIfMacro(&quot;!&quot;,pIf-&gt;zCondition,strlen(pIf-&gt;zCondition),pIf-&gt;nLine,0);
      SafeFree(pIf);
    }else{
      pIf-&gt;flags = 0;
    }
  }else{
    /*
    ** This directive can be safely ignored
    */
    return 0;
  }

  /*
  ** Recompute the preset flags
  */
  *pPresetFlags = 0;
  for(pIf = ifStack; pIf; pIf=pIf-&gt;pNext){
    *pPresetFlags |= pIf-&gt;flags;
  }

  return nErr;
}

/*
** Parse an entire file.  Return the number of errors.
**
** pList is a list of tokens in the file.  Whitespace tokens have been
** eliminated, and text with {...} has been collapsed into a
** single TT_Brace token.
**
** initFlags are a set of parse flags that should always be set for this
** file.  For .c files this is normally 0.  For .h files it is PS_Interface.
*/
static int ParseFile(Token *pList, int initFlags){
  int nErr = 0;
  Token *pStart = 0;
  int flags = initFlags;
  int presetFlags = initFlags;
  int resetFlag = 0;

  includeList = 0;
  while( pList ){
    switch( pList-&gt;eType ){
    case TT_EOF:
      goto end_of_loop;

    case TT_Preprocessor:
      nErr += ParsePreprocessor(pList,flags,&amp;presetFlags);
      pStart = 0;
      presetFlags |= initFlags;
      flags = presetFlags;
      break;

    case TT_Other:
      switch( pList-&gt;zText[0] ){
      case &#39;;&#39;:
        nErr += ProcessDecl(pStart,pList,flags);
        pStart = 0;
        flags = presetFlags;
        break;

      case &#39;=&#39;:
        if( pList-&gt;pPrev-&gt;nText==8
            &amp;&amp; strncmp(pList-&gt;pPrev-&gt;zText,&quot;operator&quot;,8)==0 ){
          break;
        }
        nErr += ProcessDecl(pStart,pList,flags);
        pStart = 0;
        while( pList &amp;&amp; pList-&gt;zText[0]!=&#39;;&#39; ){
          pList = pList-&gt;pNext;
        }
        if( pList==0 ) goto end_of_loop;
        flags = presetFlags;
        break;

      case &#39;:&#39;:
        if( pList-&gt;zText[1]==&#39;:&#39; ){
          flags |= PS_Method;
        }
        break;

      default:
        break;
      }
      break;

    case TT_Braces:
      nErr += ProcessProcedureDef(pStart,pList,flags);
      pStart = 0;
      flags = presetFlags;
      break;

    case TT_Id:
       if( pStart==0 ){
          pStart = pList;
          flags = presetFlags;
       }
       resetFlag = 0;
       switch( pList-&gt;zText[0] ){
       case &#39;c&#39;:
         if( pList-&gt;nText==5 &amp;&amp; strncmp(pList-&gt;zText,&quot;class&quot;,5)==0 ){
           nErr += ProcessTypeDecl(pList,flags,&amp;resetFlag);
         }
         break;

       case &#39;E&#39;:
         if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText,&quot;EXPORT&quot;,6)==0 ){
           flags |= PS_Export2;
           /* pStart = 0; */
         }
         break;

       case &#39;e&#39;:
         if( pList-&gt;nText==4 &amp;&amp; strncmp(pList-&gt;zText,&quot;enum&quot;,4)==0 ){
           if( pList-&gt;pNext &amp;&amp; pList-&gt;pNext-&gt;eType==TT_Braces ){
             pList = pList-&gt;pNext;
           }else{
             nErr += ProcessTypeDecl(pList,flags,&amp;resetFlag);
           }
         }else if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText,&quot;extern&quot;,6)==0 ){
           pList = pList-&gt;pNext;
           if( pList &amp;&amp; pList-&gt;nText==3 &amp;&amp; strncmp(pList-&gt;zText,&quot;\&quot;C\&quot;&quot;,3)==0 ){
             pList = pList-&gt;pNext;
             flags &amp;= ~DP_Cplusplus;
           }else{
             flags |= PS_Extern;
           }
           pStart = pList;
         }
         break;

       case &#39;i&#39;:
         if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText,&quot;inline&quot;,6)==0
          &amp;&amp; (flags &amp; PS_Static)==0
         ){
           nErr += ProcessInlineProc(pList,flags,&amp;resetFlag);
         }
         break;

       case &#39;L&#39;:
         if( pList-&gt;nText==5 &amp;&amp; strncmp(pList-&gt;zText,&quot;LOCAL&quot;,5)==0 ){
           flags |= PS_Local2;
           pStart = pList;
         }
         break;

       case &#39;P&#39;:
         if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText, &quot;PUBLIC&quot;,6)==0 ){
           flags |= PS_Public;
           pStart = pList;
         }else if( pList-&gt;nText==7 &amp;&amp; strncmp(pList-&gt;zText, &quot;PRIVATE&quot;,7)==0 ){
           flags |= PS_Private;
           pStart = pList;
         }else if( pList-&gt;nText==9 &amp;&amp; strncmp(pList-&gt;zText,&quot;PROTECTED&quot;,9)==0 ){
           flags |= PS_Protected;
           pStart = pList;
         }
         break;

       case &#39;s&#39;:
         if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText,&quot;struct&quot;,6)==0 ){
           if( pList-&gt;pNext &amp;&amp; pList-&gt;pNext-&gt;eType==TT_Braces ){
             pList = pList-&gt;pNext;
           }else{
             nErr += ProcessTypeDecl(pList,flags,&amp;resetFlag);
           }
         }else if( pList-&gt;nText==6 &amp;&amp; strncmp(pList-&gt;zText,&quot;static&quot;,6)==0 ){
           flags |= PS_Static;
         }
         break;

       case &#39;t&#39;:
         if( pList-&gt;nText==7 &amp;&amp; strncmp(pList-&gt;zText,&quot;typedef&quot;,7)==0 ){
           flags |= PS_Typedef;
         }
         break;

       case &#39;u&#39;:
         if( pList-&gt;nText==5 &amp;&amp; strncmp(pList-&gt;zText,&quot;union&quot;,5)==0 ){
           if( pList-&gt;pNext &amp;&amp; pList-&gt;pNext-&gt;eType==TT_Braces ){
             pList = pList-&gt;pNext;
           }else{
             nErr += ProcessTypeDecl(pList,flags,&amp;resetFlag);
           }
         }
         break;

       default:
         break;
       }
       if( resetFlag!=0 ){
         while( pList &amp;&amp; pList-&gt;zText[0]!=resetFlag ){
           pList = pList-&gt;pNext;
         }
         if( pList==0 ) goto end_of_loop;
         pStart = 0;
         flags = presetFlags;
       }
       break;

    case TT_String:
    case TT_Number:
       break;

    default:
       pStart = pList;
       flags = presetFlags;
       break;
    }
    pList = pList-&gt;pNext;
  }
  end_of_loop:

  /* Verify that all #ifs have a matching &quot;#endif&quot; */
  while( ifStack ){
    Ifmacro *pIf = ifStack;
    ifStack = pIf-&gt;pNext;
    fprintf(stderr,&quot;%s:%d: This &#39;#if&#39; has no &#39;#endif&#39;\n&quot;,zFilename,
      pIf-&gt;nLine);
    SafeFree(pIf);
  }

  return nErr;
}

/*
** If the given Decl object has a non-null zExtra field, then the text
** of that zExtra field needs to be inserted in the middle of the
** zDecl field before the last &quot;}&quot; in the zDecl.  This routine does that.
** If the zExtra is NULL, this routine is a no-op.
**
** zExtra holds extra method declarations for classes.  The declarations
** have to be inserted into the class definition.
*/
static void InsertExtraDecl(Decl *pDecl){
  int i;
  String str;

  if( pDecl==0 || pDecl-&gt;zExtra==0 || pDecl-&gt;zDecl==0 ) return;
  i = strlen(pDecl-&gt;zDecl) - 1;
  while( i&gt;0 &amp;&amp; pDecl-&gt;zDecl[i]!=&#39;}&#39; ){ i--; }
  StringInit(&amp;str);
  StringAppend(&amp;str, pDecl-&gt;zDecl, i);
  StringAppend(&amp;str, pDecl-&gt;zExtra, 0);
  StringAppend(&amp;str, &amp;pDecl-&gt;zDecl[i], 0);
  SafeFree(pDecl-&gt;zDecl);
  SafeFree(pDecl-&gt;zExtra);
  pDecl-&gt;zDecl = StrDup(StringGet(&amp;str), 0);
  StringReset(&amp;str);
  pDecl-&gt;zExtra = 0;
}

/*
** Reset the DP_Forward and DP_Declared flags on all Decl structures.
** Set both flags for anything that is tagged as local and isn&#39;t
** in the file zFilename so that it won&#39;t be printing in other files.
*/
static void ResetDeclFlags(char *zFilename){
  Decl *pDecl;

  for(pDecl = pDeclFirst; pDecl; pDecl = pDecl-&gt;pNext){
    DeclClearProperty(pDecl,DP_Forward|DP_Declared);
    if( DeclHasProperty(pDecl,DP_Local) &amp;&amp; pDecl-&gt;zFile!=zFilename ){
      DeclSetProperty(pDecl,DP_Forward|DP_Declared);
    }
  }
}

/*
** Forward declaration of the ScanText() function.
*/
static void ScanText(const char*, GenState *pState);

/*
** The output in pStr is currently within an #if CONTEXT where context
** is equal to *pzIf.  (*pzIf might be NULL to indicate that we are
** not within any #if at the moment.)  We are getting ready to output
** some text that needs to be within the context of &quot;#if NEW&quot; where
** NEW is zIf.  Make an appropriate change to the context.
*/
static void ChangeIfContext(
  const char *zIf,       /* The desired #if context */
  GenState *pState       /* Current state of the code generator */
){
  if( zIf==0 ){
    if( pState-&gt;zIf==0 ) return;
    StringAppend(pState-&gt;pStr,&quot;#endif\n&quot;,0);
    pState-&gt;zIf = 0;
  }else{
    if( pState-&gt;zIf ){
      if( strcmp(zIf,pState-&gt;zIf)==0 ) return;
      StringAppend(pState-&gt;pStr,&quot;#endif\n&quot;,0);
      pState-&gt;zIf = 0;
    }
    ScanText(zIf, pState);
    if( pState-&gt;zIf!=0 ){
      StringAppend(pState-&gt;pStr,&quot;#endif\n&quot;,0);
    }
    StringAppend(pState-&gt;pStr,&quot;#if &quot;,0);
    StringAppend(pState-&gt;pStr,zIf,0);
    StringAppend(pState-&gt;pStr,&quot;\n&quot;,0);
    pState-&gt;zIf = zIf;
  }
}

/*
** Add to the string pStr a #include of every file on the list of
** include files pInclude.  The table pTable contains all files that
** have already been #included at least once.  Don&#39;t add any
** duplicates.  Update pTable with every new #include that is added.
*/
static void AddIncludes(
  Include *pInclude,       /* Write every #include on this list */
  GenState *pState         /* Current state of the code generator */
){
  if( pInclude ){
    if( pInclude-&gt;pNext ){
      AddIncludes(pInclude-&gt;pNext,pState);
    }
    if( IdentTableInsert(pState-&gt;pTable,pInclude-&gt;zLabel,0) ){
      ChangeIfContext(pInclude-&gt;zIf,pState);
      StringAppend(pState-&gt;pStr,&quot;#include &quot;,0);
      StringAppend(pState-&gt;pStr,pInclude-&gt;zFile,0);
      StringAppend(pState-&gt;pStr,&quot;\n&quot;,1);
    }
  }
}

/*
** Add to the string pStr a declaration for the object described
** in pDecl.
**
** If pDecl has already been declared in this file, detect that
** fact and abort early.  Do not duplicate a declaration.
**
** If the needFullDecl flag is false and this object has a forward
** declaration, then supply the forward declaration only.  A later
** call to CompleteForwardDeclarations() will finish the declaration
** for us.  But if needFullDecl is true, we must supply the full
** declaration now.  Some objects do not have a forward declaration.
** For those objects, we must print the full declaration now.
**
** Because it is illegal to duplicate a typedef in C, care is taken
** to insure that typedefs for the same identifier are only issued once.
*/
static void DeclareObject(
  Decl *pDecl,        /* The thing to be declared */
  GenState *pState,   /* Current state of the code generator */
  int needFullDecl    /* Must have the full declaration.  A forward
                       * declaration isn&#39;t enough */
){
  Decl *p;               /* The object to be declared */
  int flag;
  int isCpp;             /* True if generating C++ */
  int doneTypedef = 0;   /* True if a typedef has been done for this object */

  /* printf(&quot;BEGIN %s of %s\n&quot;,needFullDecl?&quot;FULL&quot;:&quot;PROTOTYPE&quot;,pDecl-&gt;zName);*/
  /*
  ** For any object that has a forward declaration, go ahead and do the
  ** forward declaration first.
  */
  isCpp = (pState-&gt;flags &amp; DP_Cplusplus) != 0;
  for(p=pDecl; p; p=p-&gt;pSameName){
    if( p-&gt;zFwd ){
      if( !DeclHasProperty(p,DP_Forward) ){
        DeclSetProperty(p,DP_Forward);
        if( strncmp(p-&gt;zFwd,&quot;typedef&quot;,7)==0 ){
          if( doneTypedef ) continue;
          doneTypedef = 1;
        }
        ChangeIfContext(p-&gt;zIf,pState);
        StringAppend(pState-&gt;pStr,isCpp ? p-&gt;zFwdCpp : p-&gt;zFwd,0);
      }
    }
  }

  /*
  ** Early out if everything is already suitably declared.
  **
  ** This is a very important step because it prevents us from
  ** executing the code the follows in a recursive call to this
  ** function with the same value for pDecl.
  */
  flag = needFullDecl ? DP_Declared|DP_Forward : DP_Forward;
  for(p=pDecl; p; p=p-&gt;pSameName){
    if( !DeclHasProperty(p,flag) ) break;
  }
  if( p==0 ){
    return;
  }

  /*
  ** Make sure we have all necessary #includes
  */
  for(p=pDecl; p; p=p-&gt;pSameName){
    AddIncludes(p-&gt;pInclude,pState);
  }

  /*
  ** Go ahead an mark everything as being declared, to prevent an
  ** infinite loop thru the ScanText() function.  At the same time,
  ** we decide which objects need a full declaration and mark them
  ** with the DP_Flag bit.  We are only able to use DP_Flag in this
  ** way because we know we&#39;ll never execute this far into this
  ** function on a recursive call with the same pDecl.  Hence, recursive
  ** calls to this function (through ScanText()) can never change the
  ** value of DP_Flag out from under us.
  */
  for(p=pDecl; p; p=p-&gt;pSameName){
    if( !DeclHasProperty(p,DP_Declared)
     &amp;&amp; (p-&gt;zFwd==0 || needFullDecl)
     &amp;&amp; p-&gt;zDecl!=0
    ){
      DeclSetProperty(p,DP_Forward|DP_Declared|DP_Flag);
    }else{
      DeclClearProperty(p,DP_Flag);
    }
  }

  /*
  ** Call ScanText() recursively (this routine is called from ScanText())
  ** to include declarations required to come before these declarations.
  */
  for(p=pDecl; p; p=p-&gt;pSameName){
    if( DeclHasProperty(p,DP_Flag) ){
      if( p-&gt;zDecl[0]==&#39;#&#39; ){
        ScanText(&amp;p-&gt;zDecl[1],pState);
      }else{
        InsertExtraDecl(p);
        ScanText(p-&gt;zDecl,pState);
      }
    }
  }

  /*
  ** Output the declarations.  Do this in two passes.  First
  ** output everything that isn&#39;t a typedef.  Then go back and
  ** get the typedefs by the same name.
  */
  for(p=pDecl; p; p=p-&gt;pSameName){
    if( DeclHasProperty(p,DP_Flag) &amp;&amp; !DeclHasProperty(p,TY_Typedef) ){
      if( DeclHasAnyProperty(p,TY_Enumeration) ){
        if( doneTypedef ) continue;
        doneTypedef = 1;
      }
      ChangeIfContext(p-&gt;zIf,pState);
      if( !isCpp &amp;&amp; DeclHasAnyProperty(p,DP_ExternReqd) ){
        StringAppend(pState-&gt;pStr,&quot;extern &quot;,0);
      }else if( isCpp &amp;&amp; DeclHasProperty(p,DP_Cplusplus|DP_ExternReqd) ){
        StringAppend(pState-&gt;pStr,&quot;extern &quot;,0);
      }else if( isCpp &amp;&amp; DeclHasAnyProperty(p,DP_ExternCReqd|DP_ExternReqd) ){
        StringAppend(pState-&gt;pStr,&quot;extern \&quot;C\&quot; &quot;,0);
      }
      InsertExtraDecl(p);
      StringAppend(pState-&gt;pStr,p-&gt;zDecl,0);
      if( !isCpp &amp;&amp; DeclHasProperty(p,DP_Cplusplus) ){
        fprintf(stderr,
          &quot;%s: C code ought not reference the C++ object \&quot;%s\&quot;\n&quot;,
          pState-&gt;zFilename, p-&gt;zName);
        pState-&gt;nErr++;
      }
      DeclClearProperty(p,DP_Flag);
    }
  }
  for(p=pDecl; p &amp;&amp; !doneTypedef; p=p-&gt;pSameName){
    if( DeclHasProperty(p,DP_Flag) ){
      /* This has to be a typedef */
      doneTypedef = 1;
      ChangeIfContext(p-&gt;zIf,pState);
      InsertExtraDecl(p);
      StringAppend(pState-&gt;pStr,p-&gt;zDecl,0);
    }
  }
}

/*
** This routine scans the input text given, and appends to the
** string in pState-&gt;pStr the text of any declarations that must
** occur before the text in zText.
**
** If an identifier in zText is immediately followed by &#39;*&#39;, then
** only forward declarations are needed for that identifier.  If the
** identifier name is not followed immediately by &#39;*&#39;, we must supply
** a full declaration.
*/
static void ScanText(
  const char *zText,    /* The input text to be scanned */
  GenState *pState      /* Current state of the code generator */
){
  int nextValid = 0;    /* True is sNext contains valid data */
  InStream sIn;         /* The input text */
  Token sToken;         /* The current token being examined */
  Token sNext;          /* The next non-space token */

  /* printf(&quot;BEGIN SCAN TEXT on %s\n&quot;, zText); */

  sIn.z = zText;
  sIn.i = 0;
  sIn.nLine = 1;
  while( sIn.z[sIn.i]!=0 ){
    if( nextValid ){
      sToken = sNext;
      nextValid = 0;
    }else{
      GetNonspaceToken(&amp;sIn,&amp;sToken);
    }
    if( sToken.eType==TT_Id ){
      int needFullDecl;   /* True if we need to provide the full declaration,
                          ** not just the forward declaration */
      Decl *pDecl;        /* The declaration having the name in sToken */

      /*
      ** See if there is a declaration in the database with the name given
      ** by sToken.
      */
      pDecl = FindDecl(sToken.zText,sToken.nText);
      if( pDecl==0 ) continue;

      /*
      ** If we get this far, we&#39;ve found an identifier that has a
      ** declaration in the database.  Now see if we the full declaration
      ** or just a forward declaration.
      */
      GetNonspaceToken(&amp;sIn,&amp;sNext);
      if( sNext.zText[0]==&#39;*&#39; ){
        needFullDecl = 0;
      }else{
        needFullDecl = 1;
        nextValid = sNext.eType==TT_Id;
      }

      /*
      ** Generate the needed declaration.
      */
      DeclareObject(pDecl,pState,needFullDecl);
    }else if( sToken.eType==TT_Preprocessor ){
      sIn.i -= sToken.nText - 1;
    }
  }
  /* printf(&quot;END SCANTEXT\n&quot;); */
}

/*
** Provide a full declaration to any object which so far has had only
** a forward declaration.
*/
static void CompleteForwardDeclarations(GenState *pState){
  Decl *pDecl;
  int progress;

  do{
    progress = 0;
    for(pDecl=pDeclFirst; pDecl; pDecl=pDecl-&gt;pNext){
      if( DeclHasProperty(pDecl,DP_Forward)
       &amp;&amp; !DeclHasProperty(pDecl,DP_Declared)
      ){
        DeclareObject(pDecl,pState,1);
        progress = 1;
        assert( DeclHasProperty(pDecl,DP_Declared) );
      }
    }
  }while( progress );
}

/*
** Generate an include file for the given source file.  Return the number
** of errors encountered.
**
** if nolocal_flag is true, then we do not generate declarations for
** objected marked DP_Local.
*/
static int MakeHeader(InFile *pFile, FILE *report, int nolocal_flag){
  int nErr = 0;
  GenState sState;
  String outStr;
  IdentTable includeTable;
  Ident *pId;
  char *zNewVersion;
  char *zOldVersion;

  if( pFile-&gt;zHdr==0 || *pFile-&gt;zHdr==0 ) return 0;
  sState.pStr = &amp;outStr;
  StringInit(&amp;outStr);
  StringAppend(&amp;outStr,zTopLine,nTopLine);
  sState.pTable = &amp;includeTable;
  memset(&amp;includeTable,0,sizeof(includeTable));
  sState.zIf = 0;
  sState.nErr = 0;
  sState.zFilename = pFile-&gt;zSrc;
  sState.flags = pFile-&gt;flags &amp; DP_Cplusplus;
  ResetDeclFlags(nolocal_flag ? &quot;no&quot; : pFile-&gt;zSrc);
  for(pId = pFile-&gt;idTable.pList; pId; pId=pId-&gt;pNext){
    Decl *pDecl = FindDecl(pId-&gt;zName,0);
    if( pDecl ){
      DeclareObject(pDecl,&amp;sState,1);
    }
  }
  CompleteForwardDeclarations(&amp;sState);
  ChangeIfContext(0,&amp;sState);
  nErr += sState.nErr;
  zOldVersion = ReadFile(pFile-&gt;zHdr);
  zNewVersion = StringGet(&amp;outStr);
  if( report ) fprintf(report,&quot;%s: &quot;,pFile-&gt;zHdr);
  if( zOldVersion==0 ){
    if( report ) fprintf(report,&quot;updated\n&quot;);
    if( WriteFile(pFile-&gt;zHdr,zNewVersion) ){
      fprintf(stderr,&quot;%s: Can&#39;t write to file\n&quot;,pFile-&gt;zHdr);
      nErr++;
    }
  }else if( strncmp(zOldVersion,zTopLine,nTopLine)!=0 ){
    if( report ) fprintf(report,&quot;error!\n&quot;);
    fprintf(stderr,
       &quot;%s: Can&#39;t overwrite this file because it wasn&#39;t previously\n&quot;
       &quot;%*s  generated by &#39;makeheaders&#39;.\n&quot;,
       pFile-&gt;zHdr, (int)strlen(pFile-&gt;zHdr), &quot;&quot;);
    nErr++;
  }else if( strcmp(zOldVersion,zNewVersion)!=0 ){
    if( report ) fprintf(report,&quot;updated\n&quot;);
    if( WriteFile(pFile-&gt;zHdr,zNewVersion) ){
      fprintf(stderr,&quot;%s: Can&#39;t write to file\n&quot;,pFile-&gt;zHdr);
      nErr++;
    }
  }else if( report ){
    fprintf(report,&quot;unchanged\n&quot;);
  }
  SafeFree(zOldVersion);
  IdentTableReset(&amp;includeTable);
  StringReset(&amp;outStr);
  return nErr;
}

/*
** Generate a global header file -- a header file that contains all
** declarations.  If the forExport flag is true, then only those
** objects that are exported are included in the header file.
*/
static int MakeGlobalHeader(int forExport){
  GenState sState;
  String outStr;
  IdentTable includeTable;
  Decl *pDecl;

  sState.pStr = &amp;outStr;
  StringInit(&amp;outStr);
  /* StringAppend(&amp;outStr,zTopLine,nTopLine); */
  sState.pTable = &amp;includeTable;
  memset(&amp;includeTable,0,sizeof(includeTable));
  sState.zIf = 0;
  sState.nErr = 0;
  sState.zFilename = &quot;(all)&quot;;
  sState.flags = 0;
  ResetDeclFlags(0);
  for(pDecl=pDeclFirst; pDecl; pDecl=pDecl-&gt;pNext){
    if( forExport==0 || DeclHasProperty(pDecl,DP_Export) ){
      DeclareObject(pDecl,&amp;sState,1);
    }
  }
  ChangeIfContext(0,&amp;sState);
  printf(&quot;%s&quot;,StringGet(&amp;outStr));
  IdentTableReset(&amp;includeTable);
  StringReset(&amp;outStr);
  return 0;
}

#ifdef DEBUG
/*
** Return the number of characters in the given string prior to the
** first newline.
*/
static int ClipTrailingNewline(char *z){
  int n = strlen(z);
  while( n&gt;0 &amp;&amp; (z[n-1]==&#39;\n&#39; || z[n-1]==&#39;\r&#39;) ){ n--; }
  return n;
}

/*
** Dump the entire declaration list for debugging purposes
*/
static void DumpDeclList(void){
  Decl *pDecl;

  for(pDecl = pDeclFirst; pDecl; pDecl=pDecl-&gt;pNext){
    printf(&quot;**** %s from file %s ****\n&quot;,pDecl-&gt;zName,pDecl-&gt;zFile);
    if( pDecl-&gt;zIf ){
      printf(&quot;If: [%.*s]\n&quot;,ClipTrailingNewline(pDecl-&gt;zIf),pDecl-&gt;zIf);
    }
    if( pDecl-&gt;zFwd ){
      printf(&quot;Decl: [%.*s]\n&quot;,ClipTrailingNewline(pDecl-&gt;zFwd),pDecl-&gt;zFwd);
    }
    if( pDecl-&gt;zDecl ){
      InsertExtraDecl(pDecl);
      printf(&quot;Def: [%.*s]\n&quot;,ClipTrailingNewline(pDecl-&gt;zDecl),pDecl-&gt;zDecl);
    }
    if( pDecl-&gt;flags ){
      static struct {
        int mask;
        char *desc;
      } flagSet[] = {
        { TY_Class,       &quot;class&quot; },
        { TY_Enumeration, &quot;enum&quot; },
        { TY_Structure,   &quot;struct&quot; },
        { TY_Union,       &quot;union&quot; },
        { TY_Variable,    &quot;variable&quot; },
        { TY_Subroutine,  &quot;function&quot; },
        { TY_Typedef,     &quot;typedef&quot; },
        { TY_Macro,       &quot;macro&quot; },
        { DP_Export,      &quot;export&quot; },
        { DP_Local,       &quot;local&quot; },
        { DP_Cplusplus,   &quot;C++&quot; },
      };
      int i;
      printf(&quot;flags:&quot;);
      for(i=0; i&lt;sizeof(flagSet)/sizeof(flagSet[0]); i++){
        if( flagSet[i].mask &amp; pDecl-&gt;flags ){
          printf(&quot; %s&quot;, flagSet[i].desc);
        }
      }
      printf(&quot;\n&quot;);
    }
    if( pDecl-&gt;pInclude ){
      Include *p;
      printf(&quot;includes:&quot;);
      for(p=pDecl-&gt;pInclude; p; p=p-&gt;pNext){
        printf(&quot; %s&quot;,p-&gt;zFile);
      }
      printf(&quot;\n&quot;);
    }
  }
}
#endif

/*
** When the &quot;-doc&quot; command-line option is used, this routine is called
** to print all of the database information to standard output.
*/
static void DocumentationDump(void){
  Decl *pDecl;
  static struct {
    int mask;
    char flag;
  } flagSet[] = {
    { TY_Class,       &#39;c&#39; },
    { TY_Enumeration, &#39;e&#39; },
    { TY_Structure,   &#39;s&#39; },
    { TY_Union,       &#39;u&#39; },
    { TY_Variable,    &#39;v&#39; },
    { TY_Subroutine,  &#39;f&#39; },
    { TY_Typedef,     &#39;t&#39; },
    { TY_Macro,       &#39;m&#39; },
    { DP_Export,      &#39;x&#39; },
    { DP_Local,       &#39;l&#39; },
    { DP_Cplusplus,   &#39;+&#39; },
  };

  for(pDecl = pDeclFirst; pDecl; pDecl=pDecl-&gt;pNext){
    int i;
    int nLabel = 0;
    char *zDecl;
    char zLabel[50];
    for(i=0; i&lt;sizeof(flagSet)/sizeof(flagSet[0]); i++){
      if( DeclHasProperty(pDecl,flagSet[i].mask) ){
        zLabel[nLabel++] = flagSet[i].flag;
      }
    }
    if( nLabel==0 ) continue;
    zLabel[nLabel] = 0;
    InsertExtraDecl(pDecl);
    zDecl = pDecl-&gt;zDecl;
    if( zDecl==0 ) zDecl = pDecl-&gt;zFwd;
    printf(&quot;%s %s %s %p %d %d %d %d %d\n&quot;,
       pDecl-&gt;zName,
       zLabel,
       pDecl-&gt;zFile,
       pDecl-&gt;pComment,
       pDecl-&gt;pComment ? pDecl-&gt;pComment-&gt;nText+1 : 0,
       pDecl-&gt;zIf ? (int)strlen(pDecl-&gt;zIf)+1 : 0,
       zDecl ? (int)strlen(zDecl) : 0,
       pDecl-&gt;pComment ? pDecl-&gt;pComment-&gt;nLine : 0,
       pDecl-&gt;tokenCode.nText ? pDecl-&gt;tokenCode.nText+1 : 0
    );
    if( pDecl-&gt;pComment ){
      printf(&quot;%.*s\n&quot;,pDecl-&gt;pComment-&gt;nText, pDecl-&gt;pComment-&gt;zText);
    }
    if( pDecl-&gt;zIf ){
      printf(&quot;%s\n&quot;,pDecl-&gt;zIf);
    }
    if( zDecl ){
      printf(&quot;%s&quot;,zDecl);
    }
    if( pDecl-&gt;tokenCode.nText ){
      printf(&quot;%.*s\n&quot;,pDecl-&gt;tokenCode.nText, pDecl-&gt;tokenCode.zText);
    }
  }
}

/*
** Given the complete text of an input file, this routine prints a
** documentation record for the header comment at the beginning of the
** file (if the file has a header comment.)
*/
void PrintModuleRecord(const char *zFile, const char *zFilename){
  int i;
  static int addr = 5;
  while( isspace(*zFile) ){ zFile++; }
  if( *zFile!=&#39;/&#39; || zFile[1]!=&#39;*&#39; ) return;
  for(i=2; zFile[i] &amp;&amp; (zFile[i-1]!=&#39;/&#39; || zFile[i-2]!=&#39;*&#39;); i++){}
  if( zFile[i]==0 ) return;
  printf(&quot;%s M %s %d %d 0 0 0 0\n%.*s\n&quot;,
    zFilename, zFilename, addr, i+1, i, zFile);
  addr += 4;
}


/*
** Given an input argument to the program, construct a new InFile
** object.
*/
static InFile *CreateInFile(char *zArg, int *pnErr){
  int nSrc;
  char *zSrc;
  InFile *pFile;
  int i;

  /*
  ** Get the name of the input file to be scanned.  The input file is
  ** everything before the first &#39;:&#39; or the whole file if no &#39;:&#39; is seen.
  **
  ** Except, on windows, ignore any &#39;:&#39; that occurs as the second character
  ** since it might be part of the drive specifier.  So really, the &quot;:&#39; has
  ** to be the 3rd or later character in the name.  This precludes 1-character
  ** file names, which really should not be a problem.
  */
  zSrc = zArg;
  for(nSrc=2; zSrc[nSrc] &amp;&amp; zArg[nSrc]!=&#39;:&#39;; nSrc++){}
  pFile = SafeMalloc( sizeof(InFile) );
  memset(pFile,0,sizeof(InFile));
  pFile-&gt;zSrc = StrDup(zSrc,nSrc);

  /* Figure out if we are dealing with C or C++ code.  Assume any
  ** file with &quot;.c&quot; or &quot;.h&quot; is C code and all else is C++.
  */
  if( nSrc&gt;2 &amp;&amp; zSrc[nSrc-2]==&#39;.&#39; &amp;&amp; (zSrc[nSrc-1]==&#39;c&#39; || zSrc[nSrc-1]==&#39;h&#39;)){
    pFile-&gt;flags &amp;= ~DP_Cplusplus;
  }else{
    pFile-&gt;flags |= DP_Cplusplus;
  }

  /*
  ** If a separate header file is specified, use it
  */
  if( zSrc[nSrc]==&#39;:&#39; ){
    int nHdr;
    char *zHdr;
    zHdr = &amp;zSrc[nSrc+1];
    for(nHdr=0; zHdr[nHdr]; nHdr++){}
    pFile-&gt;zHdr = StrDup(zHdr,nHdr);
  }

  /* Look for any &#39;c&#39; or &#39;C&#39; in the suffix of the file name and change
  ** that character to &#39;h&#39; or &#39;H&#39; respectively.  If no &#39;c&#39; or &#39;C&#39; is found,
  ** then assume we are dealing with a header.
  */
  else{
    int foundC = 0;
    pFile-&gt;zHdr = StrDup(zSrc,nSrc);
    for(i = nSrc-1; i&gt;0 &amp;&amp; pFile-&gt;zHdr[i]!=&#39;.&#39;; i--){
      if( pFile-&gt;zHdr[i]==&#39;c&#39; ){
        foundC = 1;
        pFile-&gt;zHdr[i] = &#39;h&#39;;
      }else if( pFile-&gt;zHdr[i]==&#39;C&#39; ){
        foundC = 1;
        pFile-&gt;zHdr[i] = &#39;H&#39;;
      }
    }
    if( !foundC ){
      SafeFree(pFile-&gt;zHdr);
      pFile-&gt;zHdr = 0;
    }
  }

  /*
  ** If pFile-&gt;zSrc contains no &#39;c&#39; or &#39;C&#39; in its extension, it
  ** must be a header file.   In that case, we need to set the
  ** PS_Interface flag.
  */
  pFile-&gt;flags |= PS_Interface;
  for(i=nSrc-1; i&gt;0 &amp;&amp; zSrc[i]!=&#39;.&#39;; i--){
    if( zSrc[i]==&#39;c&#39; || zSrc[i]==&#39;C&#39; ){
      pFile-&gt;flags &amp;= ~PS_Interface;
      break;
    }
  }

  /* Done!
  */
  return pFile;
}

/* MS-Windows and MS-DOS both have the following serious OS bug:  the
** length of a command line is severely restricted.  But this program
** occasionally requires long command lines.  Hence the following
** work around.
**
** If the parameters &quot;-f FILENAME&quot; appear anywhere on the command line,
** then the named file is scanned for additional command line arguments.
** These arguments are substituted in place of the &quot;FILENAME&quot; argument
** in the original argument list.
**
** This first parameter to this routine is the index of the &quot;-f&quot;
** parameter in the argv[] array.  The argc and argv are passed by
** pointer so that they can be changed.
**
** Parsing of the parameters in the file is very simple.  Parameters
** can be separated by any amount of white-space (including newlines
** and carriage returns.)  There are now quoting characters of any
** kind.  The length of a token is limited to about 1000 characters.
*/
static void AddParameters(int index, int *pArgc, char ***pArgv){
  int argc = *pArgc;      /* The original argc value */
  char **argv = *pArgv;   /* The original argv value */
  int newArgc;            /* Value for argc after inserting new arguments */
  char **zNew = 0;        /* The new argv after this routine is done */
  char *zFile;            /* Name of the input file */
  int nNew = 0;           /* Number of new entries in the argv[] file */
  int nAlloc = 0;         /* Space allocated for zNew[] */
  int i;                  /* Loop counter */
  int n;                  /* Number of characters in a new argument */
  int c;                  /* Next character of input */
  int startOfLine = 1;    /* True if we are where &#39;#&#39; can start a comment */
  FILE *in;               /* The input file */
  char zBuf[1000];        /* A single argument is accumulated here */

  if( index+1==argc ) return;
  zFile = argv[index+1];
  in = fopen(zFile,&quot;r&quot;);
  if( in==0 ){
    fprintf(stderr,&quot;Can&#39;t open input file \&quot;%s\&quot;\n&quot;,zFile);
    exit(1);
  }
  c = &#39; &#39;;
  while( c!=EOF ){
    while( c!=EOF &amp;&amp; isspace(c) ){
      if( c==&#39;\n&#39; ){
        startOfLine = 1;
      }
      c = getc(in);
      if( startOfLine &amp;&amp; c==&#39;#&#39; ){
        while( c!=EOF &amp;&amp; c!=&#39;\n&#39; ){
          c = getc(in);
        }
      }
    }
    n = 0;
    while( c!=EOF &amp;&amp; !isspace(c) ){
      if( n&lt;sizeof(zBuf)-1 ){ zBuf[n++] = c; }
      startOfLine = 0;
      c = getc(in);
    }
    zBuf[n] = 0;
    if( n&gt;0 ){
      nNew++;
      if( nNew + argc &gt; nAlloc ){
        if( nAlloc==0 ){
          nAlloc = 100 + argc;
          zNew = malloc( sizeof(char*) * nAlloc );
        }else{
          nAlloc *= 2;
          zNew = realloc( zNew, sizeof(char*) * nAlloc );
        }
      }
      if( zNew ){
        int j = nNew + index;
        zNew[j] = malloc( n + 1 );
        if( zNew[j] ){
          strcpy( zNew[j], zBuf );
        }
      }
    }
  }
  newArgc = argc + nNew - 1;
  for(i=0; i&lt;=index; i++){
    zNew[i] = argv[i];
  }
  for(i=nNew + index + 1; i&lt;newArgc; i++){
    zNew[i] = argv[i + 1 - nNew];
  }
  zNew[newArgc] = 0;
  *pArgc = newArgc;
  *pArgv = zNew;
}

#ifdef NOT_USED
/*
** Return the time that the given file was last modified.  If we can&#39;t
** locate the file (because, for example, it doesn&#39;t exist), then
** return 0.
*/
static unsigned int ModTime(const char *zFilename){
  unsigned int mTime = 0;
  struct stat sStat;
  if( stat(zFilename,&amp;sStat)==0 ){
    mTime = sStat.st_mtime;
  }
  return mTime;
}
#endif

/*
** Print a usage comment for this program.
*/
static void Usage(const char *argv0, const char *argvN){
  fprintf(stderr,&quot;%s: Illegal argument \&quot;%s\&quot;\n&quot;,argv0,argvN);
  fprintf(stderr,&quot;Usage: %s [options] filename...\n&quot;
    &quot;Options:\n&quot;
    &quot;  -h          Generate a single .h to standard output.\n&quot;
    &quot;  -H          Like -h, but only output EXPORT declarations.\n&quot;
    &quot;  -v          (verbose) Write status information to the screen.\n&quot;
    &quot;  -doc        Generate no header files.  Instead, output information\n&quot;
    &quot;              that can be used by an automatic program documentation\n&quot;
    &quot;              and cross-reference generator.\n&quot;
    &quot;  -local      Generate prototypes for \&quot;static\&quot; functions and\n&quot;
    &quot;              procedures.\n&quot;
    &quot;  -f FILE     Read additional command-line arguments from the file named\n&quot;
    &quot;              \&quot;FILE\&quot;.\n&quot;
#ifdef DEBUG
    &quot;  -! MASK     Set the debugging mask to the number \&quot;MASK\&quot;.\n&quot;
#endif
    &quot;  --          Treat all subsequent comment-line parameters as filenames,\n&quot;
    &quot;              even if they begin with \&quot;-\&quot;.\n&quot;,
    argv0
  );
}

/*
** The following text contains a few simple #defines that we want
** to be available to every file.
*/
static const char zInit[] =
  &quot;#define INTERFACE 0\n&quot;
  &quot;#define EXPORT_INTERFACE 0\n&quot;
  &quot;#define LOCAL_INTERFACE 0\n&quot;
  &quot;#define EXPORT\n&quot;
  &quot;#define LOCAL static\n&quot;
  &quot;#define PUBLIC\n&quot;
  &quot;#define PRIVATE\n&quot;
  &quot;#define PROTECTED\n&quot;
;

#if TEST==0
int main(int argc, char **argv){
  int i;                /* Loop counter */
  int nErr = 0;         /* Number of errors encountered */
  Token *pList;         /* List of input tokens for one file */
  InFile *pFileList = 0;/* List of all input files */
  InFile *pTail = 0;    /* Last file on the list */
  InFile *pFile;        /* for looping over the file list */
  int h_flag = 0;       /* True if -h is present.  Output unified header */
  int H_flag = 0;       /* True if -H is present.  Output EXPORT header */
  int v_flag = 0;       /* Verbose */
  int noMoreFlags;      /* True if -- has been seen. */
  FILE *report;         /* Send progress reports to this, if not NULL */

  noMoreFlags = 0;
  for(i=1; i&lt;argc; i++){
    if( argv[i][0]==&#39;-&#39; &amp;&amp; !noMoreFlags ){
      switch( argv[i][1] ){
        case &#39;h&#39;:   h_flag = 1;   break;
        case &#39;H&#39;:   H_flag = 1;   break;
        case &#39;v&#39;:   v_flag = 1;   break;
        case &#39;d&#39;:   doc_flag = 1; proto_static = 1; break;
        case &#39;l&#39;:   proto_static = 1; break;
        case &#39;f&#39;:   AddParameters(i, &amp;argc, &amp;argv); break;
        case &#39;-&#39;:   noMoreFlags = 1;   break;
#ifdef DEBUG
        case &#39;!&#39;:   i++;  debugMask = strtol(argv[i],0,0); break;
#endif
        default:    Usage(argv[0],argv[i]); return 1;
      }
    }else{
      pFile = CreateInFile(argv[i],&amp;nErr);
      if( pFile ){
        if( pFileList ){
          pTail-&gt;pNext = pFile;
          pTail = pFile;
        }else{
          pFileList = pTail = pFile;
        }
      }
    }
  }
  if( h_flag &amp;&amp; H_flag ){
    h_flag = 0;
  }
  if( v_flag ){
    report = (h_flag || H_flag) ? stderr : stdout;
  }else{
    report = 0;
  }
  if( nErr&gt;0 ){
    return nErr;
  }
  for(pFile=pFileList; pFile; pFile=pFile-&gt;pNext){
    char *zFile;

    zFilename = pFile-&gt;zSrc;
    if( zFilename==0 ) continue;
    zFile = ReadFile(zFilename);
    if( zFile==0 ){
      fprintf(stderr,&quot;Can&#39;t read input file \&quot;%s\&quot;\n&quot;,zFilename);
      nErr++;
      continue;
    }
    if( strncmp(zFile,zTopLine,nTopLine)==0 ){
      pFile-&gt;zSrc = 0;
    }else{
      if( report ) fprintf(report,&quot;Reading %s...\n&quot;,zFilename);
      pList = TokenizeFile(zFile,&amp;pFile-&gt;idTable);
      if( pList ){
        nErr += ParseFile(pList,pFile-&gt;flags);
        FreeTokenList(pList);
      }else if( zFile[0]==0 ){
        fprintf(stderr,&quot;Input file \&quot;%s\&quot; is empty.\n&quot;, zFilename);
        nErr++;
      }else{
        fprintf(stderr,&quot;Errors while processing \&quot;%s\&quot;\n&quot;, zFilename);
        nErr++;
      }
    }
    if( !doc_flag ) SafeFree(zFile);
    if( doc_flag ) PrintModuleRecord(zFile,zFilename);
  }
  if( nErr&gt;0 ){
    return nErr;
  }
#ifdef DEBUG
  if( debugMask &amp; DECL_DUMP ){
    DumpDeclList();
    return nErr;
  }
#endif
  if( doc_flag ){
    DocumentationDump();
    return nErr;
  }
  zFilename = &quot;--internal--&quot;;
  pList = TokenizeFile(zInit,0);
  if( pList==0 ){
    return nErr+1;
  }
  ParseFile(pList,PS_Interface);
  FreeTokenList(pList);
  if( h_flag || H_flag ){
    nErr += MakeGlobalHeader(H_flag);
  }else{
    for(pFile=pFileList; pFile; pFile=pFile-&gt;pNext){
      if( pFile-&gt;zSrc==0 ) continue;
      nErr += MakeHeader(pFile,report,0);
    }
  }
  return nErr;
}
#endif

</pre>
</blockquote>
</div>
<div class="footer">
This page was generated in about
0.007s by
Fossil 2.9 [6f60cb3881] 2019-02-27 14:34:30
</div>
<script nonce="e2f4709a3deeafd8212401c84e2a082e1fbd3068470984f2">
/*
** Copyright © 2018 Warren Young
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)
**
** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
** Contact: wyoung on the Fossil forum, https://fossil-scm.org/forum/
**
*******************************************************************************
**
** This file contains the JS code specific to the Fossil default skin.
** Currently, the only thing this does is handle clicks on its hamburger
** menu button.
*/
(function() {
  var hbButton = document.getElementById("hbbtn");
  if (!hbButton) return;   // no hamburger button
  if (!document.addEventListener) {
    // Turn the button into a link to the sitemap for incompatible browsers.
    hbButton.href = "/fossil/sitemap";
    return;
  }
  var panel = document.getElementById("hbdrop");
  if (!panel) return;   // site admin might've nuked it
  if (!panel.style) return;  // shouldn't happen, but be sure
  var panelBorder = panel.style.border;
  var panelInitialized = false;   // reset if browser window is resized
  var panelResetBorderTimerID = 0;   // used to cancel post-animation tasks

  // Disable animation if this browser doesn't support CSS transitions.
  //
  // We need this ugly calling form for old browsers that don't allow
  // panel.style.hasOwnProperty('transition'); catering to old browsers
  // is the whole point here.
  var animate = panel.style.transition !== null && (typeof(panel.style.transition) == "string");

  // The duration of the animation can be overridden from the default skin
  // header.txt by setting the "data-anim-ms" attribute of the panel.
  var animMS = panel.getAttribute("data-anim-ms");
  if (animMS) {           // not null or empty string, parse it
    animMS = parseInt(animMS);
    if (isNaN(animMS) || animMS == 0)
      animate = false;    // disable animation if non-numeric or zero
    else if (animMS < 0)
      animMS = 400;       // set default animation duration if negative
  }
  else                    // attribute is null or empty string, use default
    animMS = 400;

  // Calculate panel height despite its being hidden at call time.
  // Based on https://stackoverflow.com/a/29047447/142454
  var panelHeight;  // computed on first panel display
  function calculatePanelHeight() {

    // Clear the max-height CSS property in case the panel size is recalculated
    // after the browser window was resized.
    panel.style.maxHeight = '';

    // Get initial panel styles so we can restore them below.
    var es   = window.getComputedStyle(panel),
        edis = es.display,
        epos = es.position,
        evis = es.visibility;

    // Restyle the panel so we can measure its height while invisible.
    panel.style.visibility = 'hidden';
    panel.style.position   = 'absolute';
    panel.style.display    = 'block';
    panelHeight = panel.offsetHeight + 'px';

    // Revert styles now that job is done.
    panel.style.display    = edis;
    panel.style.position   = epos;
    panel.style.visibility = evis;
  }

  // Show the panel by changing the panel height, which kicks off the
  // slide-open/closed transition set up in the XHR onload handler.
  //
  // Schedule the change for a near-future time in case this is the
  // first call, where the div was initially invisible.  If we were
  // to change the panel's visibility and height at the same time
  // instead, that would prevent the browser from seeing the height
  // change as a state transition, so it'd skip the CSS transition:
  //
  // https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Transitions/Using_CSS_transitions#JavaScript_examples
  function showPanel() {
    // Cancel the timer to remove the panel border after the closing animation,
    // otherwise double-clicking the hamburger button with the panel opened will
    // remove the borders from the (closed and immediately reopened) panel.
    if (panelResetBorderTimerID) {
      clearTimeout(panelResetBorderTimerID);
      panelResetBorderTimerID = 0;
    }
    if (animate) {
      if (!panelInitialized) {
        panelInitialized = true;
        // Set up a CSS transition to animate the panel open and
        // closed.  Only needs to be done once per page load.
        // Based on https://stackoverflow.com/a/29047447/142454
        calculatePanelHeight();
        panel.style.transition = 'max-height ' + animMS +
            'ms ease-in-out';
        panel.style.overflowY  = 'hidden';
        panel.style.maxHeight  = '0';
      }
      setTimeout(function() {
        panel.style.maxHeight = panelHeight;
        panel.style.border    = panelBorder;
      }, 40);   // 25ms is insufficient with Firefox 62
    }
    panel.style.display = 'block';
    document.addEventListener('keydown',panelKeydown,/* useCapture == */true);
    document.addEventListener('click',panelClick,false);
  }

  var panelKeydown = function(event) {
    var key = event.which || event.keyCode;
    if (key == 27) {
      event.stopPropagation();   // ignore other keydown handlers
      panelToggle(true);
    }
  };

  var panelClick = function(event) {
    if (!panel.contains(event.target)) {
      // Call event.preventDefault() to have clicks outside the opened panel
      // just close the panel, and swallow clicks on links or form elements.
      //event.preventDefault();
      panelToggle(true);
    }
  };

  // Return true if the panel is showing.
  function panelShowing() {
    if (animate) {
      return panel.style.maxHeight == panelHeight;
    }
    else {
      return panel.style.display == 'block';
    }
  }

  // Check if the specified HTML element has any child elements. Note that plain
  // text nodes, comments, and any spaces (presentational or not) are ignored.
  function hasChildren(element) {
    var childElement = element.firstChild;
    while (childElement) {
      if (childElement.nodeType == 1) // Node.ELEMENT_NODE == 1
        return true;
      childElement = childElement.nextSibling;
    }
    return false;
  }

  // Reset the state of the panel to uninitialized if the browser window is
  // resized, so the dimensions are recalculated the next time it's opened.
  window.addEventListener('resize',function(event) {
    panelInitialized = false;
  },false);

  // Click handler for the hamburger button.
  hbButton.addEventListener('click',function(event) {
    // Break the event handler chain, or the handler for document → click
    // (about to be installed) may already be triggered by the current event.
    event.stopPropagation();
    event.preventDefault();  // prevent browser from acting on <a> click
    panelToggle(false);
  },false);

  function panelToggle(suppressAnimation) {
    if (panelShowing()) {
      document.removeEventListener('keydown',panelKeydown,/* useCapture == */true);
      document.removeEventListener('click',panelClick,false);
      // Transition back to hidden state.
      if (animate) {
        if (suppressAnimation) {
          var transition = panel.style.transition;
          panel.style.transition = '';
          panel.style.maxHeight = '0';
          panel.style.border = 'none';
          setTimeout(function() {
            // Make sure CSS transition won't take effect now, so restore it
            // asynchronously. Outer variable 'transition' still valid here.
            panel.style.transition = transition;
          }, 40);   // 25ms is insufficient with Firefox 62
        }
        else {
          panel.style.maxHeight = '0';
          panelResetBorderTimerID = setTimeout(function() {
            // Browsers show a 1px high border line when maxHeight == 0,
            // our "hidden" state, so hide the borders in that state, too.
            panel.style.border = 'none';
            panelResetBorderTimerID = 0;   // clear ID of completed timer
          }, animMS);
        }
      }
      else {
        panel.style.display = 'none';
      }
    }
    else {
      if (!hasChildren(panel)) {
        // Only get the sitemap once per page load: it isn't likely to
        // change on us.
        var xhr = new XMLHttpRequest();
        xhr.onload = function() {
          var doc = xhr.responseXML;
          if (doc) {
            var sm = doc.querySelector("ul#sitemap");
            if (sm && xhr.status == 200) {
              // Got sitemap.  Insert it into the drop-down panel.
              panel.innerHTML = sm.outerHTML;
              // Display the panel
              showPanel();
            }
          }
          // else, can't parse response as HTML or XML
        }
        xhr.open("GET", "/fossil/sitemap?popup");   // note the TH1 substitution!
        xhr.responseType = "document";
        xhr.send();
      }
      else {
        showPanel();   // just show what we built above
      }
    }
  }
})();

</script>
<script id='href-data' type='application/json'>{"delay":10,"mouseover":0}</script>
<script nonce="e2f4709a3deeafd8212401c84e2a082e1fbd3068470984f2">
function setAllHrefs(){
var anchors = document.getElementsByTagName("a");
for(var i=0; i<anchors.length; i++){
var j = anchors[i];
if(j.hasAttribute("data-href")) j.href=j.getAttribute("data-href");
}
var forms = document.getElementsByTagName("form");
for(var i=0; i<forms.length; i++){
var j = forms[i];
if(j.hasAttribute("data-action")) j.action=j.getAttribute("data-action");
}
}
function antiRobotDefense(){
var x = document.getElementById("href-data");
var jx = x.textContent || x.innerText;
var g = JSON.parse(jx);
var isOperaMini =
Object.prototype.toString.call(window.operamini)==="[object OperaMini]";
if(g.mouseover && !isOperaMini){
document.getElementByTagName("body")[0].onmousemove=function(){
setTimeout(setAllHrefs, g.delay);
}
}else{
setTimeout(setAllHrefs, g.delay);
}
}
antiRobotDefense()
</script>
</body>
</html>
