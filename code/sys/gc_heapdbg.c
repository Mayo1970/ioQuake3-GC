#if defined(HW_DOL) && defined(GC_DEBUG)
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include <reent.h>
#include <stdint.h>

extern void *__malloc_sbrk_base;
extern void *__malloc_av_[];
extern void __malloc_lock( struct _reent * );
extern void __malloc_unlock( struct _reent * );

typedef struct {
    void *ptr;
    unsigned int size;
    char tag;
} gc_heap_tag_t;

typedef struct {
    unsigned int address, size, aboveSize;
    char aboveTag;
} gc_heap_free_t;

static gc_heap_tag_t gc_heapTags[4096];
static unsigned int gc_heapOverflow;

void GC_HeapTag( void *ptr, unsigned int size, char tag ) {
    int i;
    if ( !ptr ) return;
    __malloc_lock( _REENT );
    for ( i = 0; i < 4096; i++ ) {
        if ( !gc_heapTags[i].ptr ) {
            gc_heapTags[i].ptr = ptr;
            gc_heapTags[i].size = size;
            gc_heapTags[i].tag = tag;
            __malloc_unlock( _REENT );
            return;
        }
    }
    gc_heapOverflow++;
    __malloc_unlock( _REENT );
}

void GC_HeapUntag( void *ptr ) {
    int i;
    if ( !ptr ) return;
    __malloc_lock( _REENT );
    for ( i = 0; i < 4096; i++ ) {
        if ( gc_heapTags[i].ptr == ptr ) {
            gc_heapTags[i].ptr = NULL;
            break;
        }
    }
    __malloc_unlock( _REENT );
}

void GC_HeapMap( const char *why ) {
    gc_heap_free_t largest[8] = {{0}};
    unsigned int buckets[6] = {0};
    unsigned int start, top, current, chunks = 0, used = 0, freeCount = 0;
    unsigned int freeBytes = 0, overflow, bad = 0;
    int i, j;

    __malloc_lock( _REENT );
    start = ((uintptr_t)__malloc_sbrk_base + 7u) & ~7u;
    top = (uintptr_t)__malloc_av_[2];
    current = start;
    overflow = gc_heapOverflow;
    if ( __malloc_sbrk_base == (void *)-1 || start > top ) bad = 1;
    while ( !bad && current <= top ) {
        unsigned int size = ((unsigned int *)current)[1] & ~3u;
        unsigned int next = current + size;
        int isFree;
        if ( size < 16 || (size & 7) || next <= current ||
             (current != top && next > top) ) {
            bad = 1;
            break;
        }
        isFree = current == top || !(((unsigned int *)next)[1] & 1u);
        chunks++;
        if ( isFree ) {
            gc_heap_free_t entry;
            unsigned int bucket;
            freeCount++;
            freeBytes += size;
            bucket = size < 1024 ? 0 : size < 4096 ? 1 : size < 16384 ? 2 :
                     size < 65536 ? 3 : size < 262144 ? 4 : 5;
            buckets[bucket]++;
            entry.address = current;
            entry.size = size;
            entry.aboveSize = 0;
            entry.aboveTag = '?';
            if ( current != top ) {
                entry.aboveSize = ((unsigned int *)next)[1] & ~3u;
                for ( i = 0; i < 4096; i++ ) {
                    if ( gc_heapTags[i].ptr == (void *)(next + 8) ) {
                        entry.aboveTag = gc_heapTags[i].tag;
                        break;
                    }
                }
            }
            for ( i = 0; i < 8; i++ ) {
                if ( size > largest[i].size ) {
                    for ( j = 7; j > i; j-- ) largest[j] = largest[j - 1];
                    largest[i] = entry;
                    break;
                }
            }
        } else {
            used++;
        }
        if ( current == top ) break;
        current = next;
    }
    __malloc_unlock( _REENT );

    Com_Printf( "GC heap %s: start 0x%x top 0x%x, chunks %i used %i free %i, free bytes %i, largest free %i\n",
                why, start, top, chunks, used, freeCount, freeBytes, largest[0].size );
    Com_Printf( "GC heap free sizes: <1K %i, 1-4K %i, 4-16K %i, 16-64K %i, 64-256K %i, >=256K %i; tag overflow %u\n",
                buckets[0], buckets[1], buckets[2], buckets[3], buckets[4], buckets[5], overflow );
    for ( i = 0; i < 8 && largest[i].size; i++ ) {
        Com_Printf( "GC heap free 0x%x size %i, above size %i tag %c\n",
                    largest[i].address, largest[i].size, largest[i].aboveSize, largest[i].aboveTag );
    }
    if ( bad ) Com_Printf( "GC heap: invalid or uninitialized chunk at 0x%x\n", current );
}
#endif
