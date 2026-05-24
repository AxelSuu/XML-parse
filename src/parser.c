#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <expat.h>
#include "../include/parser.h"

typedef struct {
    XmlMap *out;
    char path_stack[MAX_DEPTH][MAX_KEY_LEN];
    int  depth;
    char text_buf[MAX_VAL_LEN];
    int  text_len;
} ParseState;

static void build_dotted_key(
        const char path_stack[][MAX_KEY_LEN], int depth,
        const char *pending_key, char *out_key)
{
    int pos = 0;
    for (int i = 0; i < depth && pos < MAX_KEY_LEN - 1; i++) {
        int w = snprintf(out_key + pos, (size_t)(MAX_KEY_LEN - 1 - pos), "%s.", path_stack[i]);
        if (w < 0 || (pos += w) >= MAX_KEY_LEN - 1) { pos = MAX_KEY_LEN - 1; break; }
    }
    if (pos < MAX_KEY_LEN - 1) {
        int w = snprintf(out_key + pos, (size_t)(MAX_KEY_LEN - 1 - pos), "%s", pending_key);
        if (w > 0) pos += w;
    }
    out_key[pos < MAX_KEY_LEN ? pos : MAX_KEY_LEN - 1] = '\0';
}

static void start_handler(void *data, const XML_Char *name, const XML_Char **attrs)
{
    (void)attrs;
    ParseState *s = (ParseState *)data;
    if (s->depth < MAX_DEPTH) {
        strncpy(s->path_stack[s->depth], name, MAX_KEY_LEN - 1);
        s->path_stack[s->depth][MAX_KEY_LEN - 1] = '\0';
        s->depth++;
    } else {
        fprintf(stderr, "Warning: max nesting depth (%d) exceeded under \"%s\"\n",
                MAX_DEPTH, name);
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

    /* Trim leading and trailing whitespace to detect leaf nodes */
    const char *start = s->text_buf;
    while (*start && isspace((unsigned char)*start))
        start++;
    const char *end = s->text_buf + s->text_len;
    while (end > start && isspace((unsigned char)*(end - 1)))
        end--;

    if (end > start) {
        /* Leaf node: store key-value entry */
        if (s->out->count < MAX_ENTRIES) {
            char full_key[MAX_KEY_LEN] = {0};
            /* path_stack[depth-1] is the current element; build key from depth-1 ancestors + it */
            build_dotted_key(s->path_stack, s->depth - 1, s->path_stack[s->depth - 1], full_key);

            strncpy(s->out->entries[s->out->count].key, full_key, MAX_KEY_LEN - 1);
            s->out->entries[s->out->count].key[MAX_KEY_LEN - 1] = '\0';

            size_t val_len = (size_t)(end - start);
            if (val_len > MAX_VAL_LEN - 1) val_len = MAX_VAL_LEN - 1;
            memcpy(s->out->entries[s->out->count].value, start, val_len);
            s->out->entries[s->out->count].value[val_len] = '\0';

            s->out->count++;
        } else {
            fprintf(stderr, "Warning: MAX_ENTRIES (%d) reached, entry dropped\n", MAX_ENTRIES);
        }
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
