#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_
#include <stdint.h>
#include <pthread.h>
#include "pathcoll.h"

typedef struct elfheader_s elfheader_t;
typedef struct cleanup_s cleanup_t;
typedef struct x86emu_s x86emu_t;
typedef struct zydis_s zydis_t;
typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct kh_symbolmap_s kh_symbolmap_t;
typedef struct callbacklist_s callbacklist_t;
typedef struct library_s library_t;
typedef struct kh_fts_s kh_fts_t;
typedef struct kh_threadstack_s kh_threadstack_t;
typedef struct kh_cancelthread_s kh_cancelthread_t;
typedef struct zydis_dec_s zydis_dec_t;
typedef struct atfork_fnc_s {
    uintptr_t prepare;
    uintptr_t parent;
    uintptr_t child;
    void*     handle;
} atfork_fnc_t;
#ifdef DYNAREC
typedef struct dynablocklist_s dynablocklist_t;
typedef struct mmaplist_s      mmaplist_t;
typedef struct dynmap_s {
    dynablocklist_t* dynablocks;    // the dynabockist of the block
} dynmap_t;
#define DYNAMAP_SIZE (1<<20)
#define DYNAMAP_SHIFT 12
#endif

typedef void* (*procaddess_t)(const char* name);

#define MAX_SIGNAL 64

typedef struct tlsdatasize_s {
    int32_t     tlssize;
    void*       tlsdata;
} tlsdatasize_t;

void free_tlsdatasize(void* p);

typedef struct needed_libs_s {
    int         cap;
    int         size;
    library_t   **libs;
} needed_libs_t;

void add_neededlib(needed_libs_t* needed, library_t* lib);
void free_neededlib(needed_libs_t* needed);

typedef struct base_segment_s {
    uintptr_t       base;
    uint32_t        limit;
    int             present;
    pthread_key_t   key;
} base_segment_t;

typedef struct box86context_s {
    path_collection_t   box86_path;     // PATH env. variable
    path_collection_t   box86_ld_lib;   // LD_LIBRARY_PATH env. variable

    path_collection_t   box86_emulated_libs;    // Collection of libs that should not be wrapped

    int                 x86trace;
    int                 trace_tid;
#ifdef DYNAREC
    int                 trace_dynarec;
#endif
    zydis_t             *zydis;         // dlopen the zydis dissasembler
    void*               box86lib;       // dlopen on box86 itself

    int                 argc;
    char**              argv;

    int                 envc;
    char**              envv;

    char*               fullpath;
    char*               box86path;      // path of current box86 executable

    uint32_t            stacksz;
    int                 stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded

    needed_libs_t       neededlibs;     // needed libs for main elf

    uintptr_t           ep;             // entry point

    lib_t               *maplib;        // lib and symbols handling
    lib_t               *local_maplib;  // libs and symbols openned has local (only collection of libs, no symbols)

    kh_threadstack_t    *stacksizes;    // stack sizes attributes for thread (temporary)
    kh_cancelthread_t   *cancelthread;  // thread cancel mecanism is bit complex, create a map to ease it
    bridge_t            *threads;       // threads
    bridge_t            *system;        // other bridges
    uintptr_t           vsyscall;       // vsyscall bridge value
    dlprivate_t         *dlprivate;     // dlopen library map
    kh_symbolmap_t      *glwrappers;    // the map of wrapper for glProcs (for GLX or SDL1/2)
    kh_symbolmap_t      *glmymap;       // link to the mysymbolmap of libGL
    procaddess_t        glxprocaddress;
    kh_symbolmap_t      *alwrappers;    // the map of wrapper for alGetProcAddress
    kh_symbolmap_t      *almymap;       // link to the mysymbolmap if libOpenAL

    callbacklist_t      *callbacks;     // all callbacks

    pthread_mutex_t     mutex_once;
    pthread_mutex_t     mutex_once2;
    pthread_mutex_t     mutex_trace;
    pthread_mutex_t     mutex_lock;
    pthread_mutex_t     mutex_thread;

    library_t           *libclib;       // shortcut to libc library (if loaded, so probably yes)
    library_t           *sdl1lib;       // shortcut to SDL1 library (if loaded)
    void*               sdl1allocrw;
    void*               sdl1freerw;
    library_t           *sdl1mixerlib;
    library_t           *sdl1imagelib;
    library_t           *sdl1ttflib;
    library_t           *sdl2lib;       // shortcut to SDL2 library (if loaded)
    void*               sdl2allocrw;
    void*               sdl2freerw;
    library_t           *sdl2mixerlib;
    library_t           *sdl2imagelib;
    library_t           *sdl2ttflib;
    library_t           *x11lib;
    library_t           *libxcb;
    library_t           *libxcbxfixes;
    library_t           *libxcbshape;
    library_t           *libxcbshm;
    library_t           *libxcbrandr;
    library_t           *libxcbimage;
    library_t           *libxcbkeysyms;
    library_t           *libxcbxtest;
    library_t           *zlib;
    library_t           *vorbisfile;
    library_t           *vorbis;
    library_t           *asound;
    library_t           *pulse;

    int                 deferedInit;
    elfheader_t         **deferedInitList;
    int                 deferedInitSz;
    int                 deferedInitCap;

    pthread_key_t       tlskey;     // then tls key to have actual tlsdata
    void*               tlsdata;    // the initial global tlsdata
    int32_t             tlssize;    // wanted size of tlsdata
    base_segment_t      segtls[3];  // only handling 0/1/2 descriptors

    uintptr_t           *auxval_start;

    cleanup_t   *cleanups;          // atexit functions
    int         clean_sz;
    int         clean_cap;
#ifdef DYNAREC
    pthread_mutex_t     mutex_blocks;
    pthread_mutex_t     mutex_mmap;
    dynablocklist_t     *dynablocks;
    mmaplist_t          *mmaplist;
    int                 mmapsize;
    dynmap_t*           dynmap[DYNAMAP_SIZE];  // 4G of memory mapped by 4K block
#endif
#ifndef NOALIGN
    kh_fts_t            *ftsmap;
#endif
    zydis_dec_t         *dec;           // trace

    int                 forked;         //  how many forks... cleanup only when < 0

    atfork_fnc_t        *atforks;       // fnc for atfork...
    int                 atfork_sz;
    int                 atfork_cap;

    uint8_t             canary[4];

    uintptr_t           signals[MAX_SIGNAL];
    uintptr_t           restorer[MAX_SIGNAL];
    int                 is_sigaction[MAX_SIGNAL];
    x86emu_t            *emu_sig;       // the emu with stack used for signal handling (must be separated from main ones)
    int                 no_sigsegv;
    int                 no_sigill;
#ifdef BUILD_DYNAMIC
    int                 count;      // number of instances
#endif
} box86context_t;

box86context_t *NewBox86Context(int argc);
void FreeBox86Context(box86context_t** context);

// return the index of the added header
int AddElfHeader(box86context_t* ctx, elfheader_t* head);

// return the tlsbase (negative) for the new TLS partition created (no partition index is stored in the context)
int AddTLSPartition(box86context_t* context, int tlssize);

#ifdef DYNAREC
// the nolinker specified if static map or dynamic (can be deleted) has to be used
uintptr_t AllocDynarecMap(int size, int nolinker);
void FreeDynarecMap(uintptr_t addr, uint32_t size);

dynablocklist_t* getDBFromAddress(uintptr_t addr);
void addDBFromAddressRange(uintptr_t addr, uintptr_t size, int nolinker);
void cleanDBFromAddressRange(uintptr_t addr, uintptr_t size, int destroy);

void protectDB(uintptr_t addr, uintptr_t size);
void unprotectDB(uintptr_t addr, uintptr_t size);
#endif

// defined in fact in threads.c
void thread_set_emu(x86emu_t* emu);
x86emu_t* thread_get_emu();

#endif //__BOX86CONTEXT_H_