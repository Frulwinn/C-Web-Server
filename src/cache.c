#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "cache.h"

/**
 * Allocate a cache entry
 */
struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)
{
    struct cache_entry *new_entry = malloc(sizeof(*new_entry));
    //strdup is mallocing it is creating a copy of
    new_entry->path = strdup(path);
    new_entry->content_type = strdup(content_type);
    new_entry->content_length = content_length;
    new_entry->content = malloc(content_length);

    //make your own copy of through memcpy
    memcpy(new_entry->content, content, content_length);

    new_entry->prev = NULL;
    new_entry->next = NULL;

    return new_entry;
}

/**
 * Deallocate a cache entry
 */
void free_entry(struct cache_entry *entry)
{
   free(entry->path);
   free(entry->content_type);
   free(entry->content);
   free(entry);

   printf("entry is free!\n");
}

/**
 * Insert a cache entry at the head of the linked list
 */
void dllist_insert_head(struct cache *cache, struct cache_entry *ce)
{
    // Insert at the head of the list
    if (cache->head == NULL) {
        cache->head = cache->tail = ce;
        ce->prev = ce->next = NULL;
    } else {
        cache->head->prev = ce;
        ce->next = cache->head;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Move a cache entry to the head of the list
 */
void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)
{
    if (ce != cache->head) {
        if (ce == cache->tail) {
            // We're the tail
            cache->tail = ce->prev;
            cache->tail->next = NULL;

        } else {
            // We're neither the head nor the tail
            ce->prev->next = ce->next;
            ce->next->prev = ce->prev;
        }

        ce->next = cache->head;
        cache->head->prev = ce;
        ce->prev = NULL;
        cache->head = ce;
    }
}


/**
 * Removes the tail from the list and returns it
 * 
 * NOTE: does not deallocate the tail
 */
struct cache_entry *dllist_remove_tail(struct cache *cache)
{
    struct cache_entry *oldtail = cache->tail;

    cache->tail = oldtail->prev;
    cache->tail->next = NULL;

    cache->cur_size--;

    return oldtail;
}

/**
 * Create a new cache
 * 
 * max_size: maximum number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache *cache_create(int max_size, int hashsize)
{
    struct cache *c = malloc(sizeof(struct cache));
    c->index = hashtable_create(hashsize, NULL);
    //c->head = c->tail = NULL;
    c->head = NULL;
    c->tail = NULL;
    c->max_size = max_size;
    c->cur_size = 0;

    return c;
}

void cache_free(struct cache *cache)
{
    struct cache_entry *cur_entry = cache->head;

    hashtable_destroy(cache->index);

    while (cur_entry != NULL) {
        struct cache_entry *next_entry = cur_entry->next;

        free_entry(cur_entry);

        cur_entry = next_entry;
    }

    free(cache);
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
//adding to the head of the linked list
//void just putting something on but not returning anything
void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)
{
    struct cache_entry *ce = alloc_entry(path, content_type, content, content_length);
    dllist_insert_head(cache, ce);
    hashtable_put(cache->index, path, ce);
    cache->cur_size++;
    if(cache->cur_size > cache->max_size) {
        struct cache_entry *oldTail = dllist_remove_tail(cache);
        hashtable_delete(cache->index, oldTail->path);
        free_entry(oldTail);
        printf("Current size: %d, max size: %d\n", cache->cur_size, cache->max_size);
        
    }	
}

/**
 * Retrieve an entry from the cache
 */
struct cache_entry *cache_get(struct cache *cache, char *path)
{
    //find the entry
    struct cache_entry *ce = hashtable_get(cache->index, path);

    //try to find the cache entry pointer by path in the hash table
    if (ce == NULL) {
        //if not found, return NULL
        return NULL;
    }
    //move it to the head of the list
    dllist_move_to_head(cache, ce);

    return ce;
}
