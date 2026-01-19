#ifndef TOML_H
#define TOML_H

#include <stdbool.h>
#include <stddef.h>

// toml value types
typedef enum
{
    TOML_STRING,
    TOML_INTEGER,
    TOML_FLOAT,
    TOML_BOOLEAN,
    TOML_ARRAY,
    TOML_TABLE
} toml_type_t;

// toml value structure
typedef struct toml_value toml_value_t;
typedef struct toml_table toml_table_t;
typedef struct toml_array toml_array_t;

struct toml_value
{
    toml_type_t type;
    union
    {
        char         *string;
        long long     integer;
        double        floating;
        bool          boolean;
        toml_array_t *array;
        toml_table_t *table;
    } as;
};

struct toml_array
{
    toml_value_t *values;
    size_t        count;
    size_t        capacity;
};

typedef struct toml_entry
{
    char        *key;
    toml_value_t value;
} toml_entry_t;

struct toml_table
{
    toml_entry_t *entries;
    size_t        count;
    size_t        capacity;
};

// parse toml from string
toml_table_t *toml_parse(const char *input, char **error);

// get value from table by key
toml_value_t *toml_table_get(toml_table_t *table, const char *key);

// get nested value using dot notation (e.g., "section.subsection.key")
toml_value_t *toml_table_get_path(toml_table_t *table, const char *path);

// type check helpers
bool toml_value_is_string(toml_value_t *value);
bool toml_value_is_integer(toml_value_t *value);
bool toml_value_is_float(toml_value_t *value);
bool toml_value_is_boolean(toml_value_t *value);
bool toml_value_is_array(toml_value_t *value);
bool toml_value_is_table(toml_value_t *value);

// array accessors
size_t        toml_array_length(toml_array_t *array);
toml_value_t *toml_array_get(toml_array_t *array, size_t index);

// cleanup
void toml_table_free(toml_table_t *table);

#endif // TOML_H
