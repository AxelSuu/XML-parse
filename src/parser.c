#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <expat.h>
#include "../include/parser.h"

typedef struct {
    XmlMap *out;
    char path_stack[MAX_DEPTH][MAX_KEY_LEN];
    int  path_index[MAX_DEPTH];
    char sibling_name[MAX_DEPTH][MAX_KEY_LEN];
    bool is_sequence[MAX_DEPTH];
    int  sibling_count[MAX_DEPTH];
    int  depth;
    char text_buf[MAX_VAL_LEN];
    int  text_len;
} ParseState;

static void build_dotted_key(const ParseState *s, int depth, char *out_key)
{
    int pos = 0;
    for (int i = 0; i < depth && pos < MAX_KEY_LEN - 1; i++) {
        if (i > 0 && pos < MAX_KEY_LEN - 1)
            out_key[pos++] = '.';
        int w = snprintf(out_key + pos, (size_t)(MAX_KEY_LEN - 1 - pos), "%s", s->path_stack[i]);
        if (w < 0) break;
        pos += w;
        if (pos >= MAX_KEY_LEN - 1) { pos = MAX_KEY_LEN - 1; break; }
        if (s->path_index[i] >= 0) {
            w = snprintf(out_key + pos, (size_t)(MAX_KEY_LEN - 1 - pos), "[%d]", s->path_index[i]);
            if (w < 0) break;
            pos += w;
            if (pos >= MAX_KEY_LEN - 1) { pos = MAX_KEY_LEN - 1; break; }
        }
    }
    out_key[pos] = '\0';
}

static void store_entry(XmlMap *map, const char *key, const char *value)
{
    if (map->count >= MAX_ENTRIES) {
        fprintf(stderr, "Warning: MAX_ENTRIES (%d) reached, entry dropped\n", MAX_ENTRIES);
        return;
    }
    strncpy(map->entries[map->count].key,   key,   MAX_KEY_LEN - 1);
    strncpy(map->entries[map->count].value, value, MAX_VAL_LEN - 1);
    map->entries[map->count].key[MAX_KEY_LEN - 1]   = '\0';
    map->entries[map->count].value[MAX_VAL_LEN - 1] = '\0';
    map->count++;
}

/* On the second occurrence of an element at depth d, rename all stored entries
   that were keyed without the [0] index to include it. */
static void retroactive_reindex(XmlMap *map, ParseState *s, int d)
{
    char old_prefix[MAX_KEY_LEN];
    char new_prefix[MAX_KEY_LEN];

    int saved = s->path_index[d];
    s->path_index[d] = -1;
    build_dotted_key(s, d + 1, old_prefix);
    s->path_index[d] = 0;
    build_dotted_key(s, d + 1, new_prefix);
    s->path_index[d] = saved;

    int old_len = (int)strlen(old_prefix);
    (void)strlen(new_prefix);

    for (int i = 0; i < map->count; i++) {
        char *key = map->entries[i].key;
        int klen  = (int)strlen(key);

        bool exact  = (klen == old_len && strcmp(key, old_prefix) == 0);
        bool prefix = (klen > old_len  && strncmp(key, old_prefix, (size_t)old_len) == 0
                                       && key[old_len] == '.');
        if (!exact && !prefix)
            continue;

        char new_key[MAX_KEY_LEN];
        int suffix_off = old_len;
        int w = snprintf(new_key, sizeof(new_key), "%s%s", new_prefix, key + suffix_off);
        if (w > 0 && w < MAX_KEY_LEN)
            strcpy(key, new_key);
    }
}

static void start_handler(void *data, const XML_Char *name, const XML_Char **attrs)
{
    ParseState *s = (ParseState *)data;
    if (s->depth >= MAX_DEPTH) {
        fprintf(stderr, "Warning: max nesting depth (%d) exceeded under \"%s\"\n",
                MAX_DEPTH, name);
        return;
    }

    int d = s->depth;

    if (strcmp(s->sibling_name[d], name) == 0) {
        if (!s->is_sequence[d]) {
            s->is_sequence[d] = true;
            retroactive_reindex(s->out, s, d);
        }
        s->path_index[d] = ++s->sibling_count[d];
    } else {
        strncpy(s->sibling_name[d], name, MAX_KEY_LEN - 1);
        s->sibling_name[d][MAX_KEY_LEN - 1] = '\0';
        s->sibling_count[d] = 0;
        s->is_sequence[d]   = false;
        s->path_index[d]    = -1;
    }

    strncpy(s->path_stack[d], name, MAX_KEY_LEN - 1);
    s->path_stack[d][MAX_KEY_LEN - 1] = '\0';
    s->depth++;

    /* Store attributes as <current.path>.@attrname */
    for (int i = 0; attrs[i]; i += 2) {
        char attr_key[MAX_KEY_LEN];
        build_dotted_key(s, s->depth, attr_key);
        int pos = (int)strlen(attr_key);
        snprintf(attr_key + pos, (size_t)(MAX_KEY_LEN - 1 - pos), ".@%s", attrs[i]);
        store_entry(s->out, attr_key, attrs[i + 1]);
    }

    s->text_len = 0;
    s->text_buf[0] = '\0';
}

static void char_data_handler(void *data, const XML_Char *chunk, int len)
{
    ParseState *s = (ParseState *)data;
    int space = MAX_VAL_LEN - 1 - s->text_len;
    if (space > 0) {
        int copy = len < space ? len : space;
        memcpy(s->text_buf + s->text_len, chunk, (size_t)copy);
        s->text_len += copy;
        s->text_buf[s->text_len] = '\0';
    }
}

static void end_handler(void *data, const XML_Char *name)
{
    (void)name;
    ParseState *s = (ParseState *)data;
    if (s->depth == 0)
        return;

    const char *start = s->text_buf;
    while (*start && isspace((unsigned char)*start))
        start++;
    const char *end = s->text_buf + s->text_len;
    while (end > start && isspace((unsigned char)*(end - 1)))
        end--;

    if (end > start) {
        char full_key[MAX_KEY_LEN] = {0};
        build_dotted_key(s, s->depth, full_key);

        char value[MAX_VAL_LEN];
        size_t val_len = (size_t)(end - start);
        if (val_len > MAX_VAL_LEN - 1) val_len = MAX_VAL_LEN - 1;
        memcpy(value, start, val_len);
        value[val_len] = '\0';

        store_entry(s->out, full_key, value);
    }

    s->depth--;
    s->text_len = 0;
    s->text_buf[0] = '\0';
}

int parse(const char *filepath, XmlMap *out) {
    if (filepath == NULL || out == NULL) {
        fprintf(stderr, "Invalid arguments to parse()\n");
        return 0;
    }

    FILE *input = fopen(filepath, "rb");
    if (input == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filepath);
        return 0;
    }

    XML_Parser p = XML_ParserCreate(NULL);
    if (p == NULL) {
        fprintf(stderr, "Failed to create XML parser\n");
        fclose(input);
        return 0;
    }

    ParseState state = {0};
    state.out = out;
    for (int i = 0; i < MAX_DEPTH; i++)
        state.path_index[i] = -1;

    XML_SetUserData(p, &state);
    XML_SetStartElementHandler(p, start_handler);
    XML_SetEndElementHandler(p, end_handler);
    XML_SetCharacterDataHandler(p, char_data_handler);

    char buf[4096];
    size_t bytes_read;
    int ok = 1;

    while ((bytes_read = fread(buf, 1, sizeof(buf), input)) > 0) {
        int is_final = feof(input);
        if (XML_Parse(p, buf, (int)bytes_read, is_final) == XML_STATUS_ERROR) {
            fprintf(stderr, "XML parse error: %s at line %lu\n",
                    XML_ErrorString(XML_GetErrorCode(p)),
                    (unsigned long)XML_GetCurrentLineNumber(p));
            ok = 0;
            break;
        }
    }

    XML_ParserFree(p);
    fclose(input);
    return ok;
}

const char *xml_get(const XmlMap *map, const char *key) {
    for (int i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0)
            return map->entries[i].value;
    }
    return NULL;
}
