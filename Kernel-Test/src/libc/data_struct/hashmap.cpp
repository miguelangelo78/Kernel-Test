#include <libc/hashmap.h>
#include <system.h>

unsigned int hashmap_string_hash(void * _key) {
	unsigned int hash = 0;
	char * key = (char *)_key;
	int c;
	/* This is the so-called "sdbm" hash. It comes from a piece of
	* public domain code from a clone of ndbm. */
	while ((c = *key++)) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

int hashmap_string_comp(void * a, void * b) {
	return !strcmp((const char*)a, (const char*)b);
}

void * hashmap_string_dupe(void * key) {
	return strdup((const char*)key);
}

unsigned int hashmap_int_hash(void * key) {
	return (unsigned int)key;
}

int hashmap_int_comp(void * a, void * b) {
	return (int)a == (int)b;
}

void * hashmap_int_dupe(void * key) {
	return key;
}

static void hashmap_int_free(void * ptr) {
	return;
}

hashmap_t * hashmap_create(int size) {
	hashmap_t * map = (hashmap_t*)malloc(sizeof(hashmap_t));

	map->hash_func = &hashmap_string_hash;
	map->hash_comp = &hashmap_string_comp;
	map->hash_key_dup = &hashmap_string_dupe;
	map->hash_key_free = &free;
	map->hash_val_free = &free;

	map->size = size;
	map->entries = (hashmap_entry_t**)malloc(sizeof(hashmap_entry_t *) * size);
	memset(map->entries, 0x00, sizeof(hashmap_entry_t *) * size);

	return map;
}

hashmap_t * hashmap_create_int(int size) {
	hashmap_t * map = (hashmap_t*)malloc(sizeof(hashmap_t));

	map->hash_func = &hashmap_int_hash;
	map->hash_comp = &hashmap_int_comp;
	map->hash_key_dup = &hashmap_int_dupe;
	map->hash_key_free = &hashmap_int_free;
	map->hash_val_free = &free;

	map->size = size;
	map->entries = (hashmap_entry_t**)malloc(sizeof(hashmap_entry_t *) * size);
	memset(map->entries, 0x00, sizeof(hashmap_entry_t *) * size);

	return map;
}

void * hashmap_set(hashmap_t * map, void * key, void * value) {
	unsigned int hash = map->hash_func(key) % map->size;

	hashmap_entry_t * x = map->entries[hash];
	if (!x) {
		hashmap_entry_t * e = (hashmap_entry_t*)malloc(sizeof(hashmap_entry_t));
		e->key = (char*)map->hash_key_dup(key);
		e->value = value;
		e->next = 0;
		map->entries[hash] = e;
		return 0;
	}
	else {
		hashmap_entry_t * p = 0;
		do {
			if (map->hash_comp(x->key, key)) {
				void * out = x->value;
				x->value = value;
				return out;
			}
			else {
				p = x;
				x = x->next;
			}
		} while (x);
		hashmap_entry_t * e = (hashmap_entry_t*)malloc(sizeof(hashmap_entry_t));
		e->key = (char*)map->hash_key_dup(key);
		e->value = value;
		e->next = 0;

		p->next = e;
		return 0;
	}
}

void * hashmap_get(hashmap_t * map, void * key) {
	unsigned int hash = map->hash_func(key) % map->size;

	hashmap_entry_t * x = map->entries[hash];
	if (!x) {
		return 0;
	}
	else {
		do {
			if (map->hash_comp(x->key, key)) {
				return x->value;
			}
			x = x->next;
		} while (x);
		return 0;
	}
}

void * hashmap_remove(hashmap_t * map, void * key) {
	unsigned int hash = map->hash_func(key) % map->size;

	hashmap_entry_t * x = map->entries[hash];
	if (!x) {
		return 0;
	}
	else {
		if (map->hash_comp(x->key, key)) {
			void * out = x->value;
			map->entries[hash] = x->next;
			map->hash_key_free(x->key);
			map->hash_val_free(x);
			return out;
		}
		else {
			hashmap_entry_t * p = x;
			x = x->next;
			do {
				if (map->hash_comp(x->key, key)) {
					void * out = x->value;
					p->next = x->next;
					map->hash_key_free(x->key);
					map->hash_val_free(x);
					return out;
				}
				p = x;
				x = x->next;
			} while (x);
		}
		return 0;
	}
}

int hashmap_has(hashmap_t * map, void * key) {
	unsigned int hash = map->hash_func(key) % map->size;

	hashmap_entry_t * x = map->entries[hash];
	if (!x) {
		return 0;
	}
	else {
		do {
			if (map->hash_comp(x->key, key)) {
				return 1;
			}
			x = x->next;
		} while (x);
		return 0;
	}

}

list_t * hashmap_keys(hashmap_t * map) {
	list_t * l = list_create();

	for (unsigned int i = 0; i < map->size; ++i) {
		hashmap_entry_t * x = map->entries[i];
		while (x) {
			list_insert(l, x->key);
			x = x->next;
		}
	}

	return l;
}

list_t * hashmap_values(hashmap_t * map) {
	list_t * l = list_create();

	for (unsigned int i = 0; i < map->size; ++i) {
		hashmap_entry_t * x = map->entries[i];
		while (x) {
			list_insert(l, x->value);
			x = x->next;
		}
	}

	return l;
}

void hashmap_free(hashmap_t * map) {
	for (unsigned int i = 0; i < map->size; ++i) {
		hashmap_entry_t * x = map->entries[i], *p;
		while (x) {
			p = x;
			x = x->next;
			map->hash_key_free(p->key);
			map->hash_val_free(p);
		}
	}
	free(map->entries);
}
