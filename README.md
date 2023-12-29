# Credits
Thanks to
https://github.com/astronexus/HYG-Database for the source of astronomical data.
https://github.com/SFML/cmake-sfml-project for the project boilerplate code/template.

## Useful commands

How to check if binary contains debug symbols
nm -C starmap3 | grep ' [BD] '

Generate a memory profile for the binary. (needs debug symbols)
valgrind --tool=callgrind ./starmap3 --fn-skip="0x*" --fn-skip="llvm*"

View the output of valgrind
kcachegrind callgrind.out.<PID>

    For a single-configuration generator (typically the case on Linux and macOS, the first line pre-configures the folder, the second is for regular building..):
    ```
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ```

    For a multi-configuration generator (typically the case on Windows):
    ```
    cmake -S . -B build
    cmake --build build --config Release
    ```

## License

The source code is dual licensed under Public Domain and MIT -- choose whichever you prefer.