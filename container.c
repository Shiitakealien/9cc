#include "9cc.h"

Vector *new_vector(){
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem){
    if (vec->capacity == vec->len){
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

Map *new_map(){
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, char *key, void *val){
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

void *map_get(Map *map, char *key){
    for (int i = map->keys->len - 1; i >= 0; i--)
        if (strcmp(map->keys->data[i], key) == 0)
            return map->vals->data[i];
    return NULL;
}
    
static int expect(int line, int expected, int actual){
    if (expected == actual)
        return 1;
    fprintf(stderr, "%d: %d expected, but got %d\n",
                    line, expected, actual);
    exit(1);
    return 0;
}

static void test_vector(){
    printf("test vector\n");
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);
    int test[100];
    for (int i = 0; i <100; i++){
        test[i] = i;
        vec_push(vec, (void *)&test[i]);
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0,   *(int *)(vec->data[0]));
    expect(__LINE__, 50,  *(int *)(vec->data[50]));
    expect(__LINE__, 99,  *(int *)(vec->data[99]));

    printf("OK\n");
}

static void test_map() {
    printf("test map\n");
    int test_data[] = {2,4,6};
    Map *map = new_map();

    map_put(map, "foo", (void *)&test_data[0]);
    expect(__LINE__, 2, *(int *)map_get(map, "foo"));

    map_put(map, "bar", (void *)&test_data[1]);
    expect(__LINE__, 4, *(int *)map_get(map, "bar"));
    
    map_put(map, "foo", (void *)&test_data[2]);
    expect(__LINE__, 6, *(int *)map_get(map, "foo"));
    
    printf("OK\n");
}

void runtest(){
    test_vector();
    test_map();
}

