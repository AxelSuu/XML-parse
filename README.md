### C XML Parser

A C XML parser using libexpat. Nested XML elements are flattened into dotted-path keys (`config.stage`) in a flat array.

#### Commands

```bash
./compile            # Build with CMake (creates build/ if needed)
./test               # Run Unity tests (must compile first)
./check              # Compile + valgrind memory check + run tests
./memcheck           # Run valgrind on the parser binary
./parse <file.xml>   # Parse a file from configs/ (e.g. ./parse test.xml)
```

Requirements: `gcc`, `CMake`, `valgrind`.

#### API

```c
int         parse(const char *filepath, XmlMap *out);
const char *xml_get(const XmlMap *map, const char *key);
```

`parse` returns 1 on success, 0 on failure. `xml_get` does a linear scan and returns `NULL` if the key is not found.

#### Limits

| Constant | Value | Meaning |
|---|---|---|
| `MAX_ENTRIES` | 64 | Max entries per `XmlMap` |
| `MAX_KEY_LEN` | 256 | Max key length (bytes) |
| `MAX_VAL_LEN` | 256 | Max value length (bytes) |
| `MAX_DEPTH` | 16 | Max nesting depth |

#### Key format

| XML structure | Key |
|---|---|
| `<config><stage>test</stage></config>` | `config.stage` |
| Repeated consecutive siblings | `list.item[0]`, `list.item[1]`, … |
| Attributes | `users.user[0].@age` |