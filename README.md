### C XML Parser

A C XML parser using libexpat. Nested elements are flattened into dotted-path keys ( `config.stage`). Sequences are not supported.

It includes testing with unity. Dependency management and build with CMake. Memory management with valgrind.

``` bash
./compile            # Compiles with CMake
./test               # Runs tests with Unity
./check              # Compiles, checks memory leaks and runs tests
./memcheck           # Runs valgrind on the parser
./parse <file.xml>   # Parses <file.xml> file in /configs
```

Requirements:
- gcc
- CMake
- valgrind
