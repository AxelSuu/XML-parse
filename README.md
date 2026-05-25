### C XML Parser

A C XML parser using libexpat. Nested XML elements are flattened into dotted-path keys (`config.stage`) stored in a caller-provided flat array.

#### Commands

```bash
./compile            # Build with CMake (creates build/ if needed)
./check              # Compile + valgrind memory check + run Unity tests
./test               # Run Unity tests
./memcheck           # Run valgrind on the parser binary
./parse <file.xml>   # Parse a file from configs/ (./parse test.xml)
```

Requirements: `gcc`, `CMake`, `valgrind`.

#### API

```c
XmlStatus   parse(const char *filepath, XmlMap *out);
const char *xml_get(const XmlMap *map, const char *key);
int         xml_count(const XmlMap *map, const char *prefix);
```

`XmlMap` holds a pointer to a caller-provided `XmlEntry` buffer. Use the `XML_MAP` macro for the common stack-allocation case:

```c
XML_MAP(map, MAX_ENTRIES);          // declares XmlEntry buf + XmlMap in one line
XmlStatus s = parse("file.xml", &map);

xml_get(&map, "config.stage");      // single key lookup, NULL if not found
xml_count(&map, "list.item");       // number of indexed sequence entries (0 if none)

// iterate a sequence
int n = xml_count(&map, "list.item");
for (int i = 0; i < n; i++) {
    char key[MAX_KEY_LEN];
    snprintf(key, sizeof(key), "list.item[%d]", i);
    printf("%s\n", xml_get(&map, key));
}
```

#### Return codes

| Code | Value | Meaning |
|---|---|---|
| `XML_OK` | 1 | Success |
| `XML_ERR_ARGS` | -1 | NULL filepath/out, or out->entries NULL / capacity ≤ 0 |
| `XML_ERR_IO` | -2 | File not found or unreadable |
| `XML_ERR_PARSE` | -3 | Malformed XML |
| `XML_ERR_OVERFLOW` | -4 | Entry limit hit; partial results in map |

#### Limits

| Constant | Value | Meaning |
|---|---|---|
| `MAX_ENTRIES` | 256 | Default buffer size (capacity is set per-map) |
| `MAX_KEY_LEN` | 256 | Max key length (bytes) |
| `MAX_VAL_LEN` | 256 | Max value length (bytes) |
| `MAX_DEPTH` | 16 | Max nesting depth |

#### Key format

| XML | Key |
|---|---|
| `<config><stage>test</stage></config>` | `config.stage` |
| Repeated consecutive siblings | `list.item[0]`, `list.item[1]`, … |
| Attributes | `users.user[0].@age` |
| Non-consecutive repeated tags | last value wins, no index |

For static or heap storage, manage the buffer manually:

```c
static XmlEntry buf[MAX_ENTRIES];
XmlMap map = { buf, MAX_ENTRIES, 0 };
```
