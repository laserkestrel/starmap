# Credits

Thanks to; <BR>
https://github.com/astronexus/HYG-Database for the source of astronomical data.
https://github.com/SFML/cmake-sfml-project for the project boilerplate code/template.
Michael from www.dream-ware.co.uk for use of the Elite Frontier TTF font.

# Configuration

scaleFactor - The number of parsecs across the width of the display. (1 Parsec ~ 3.26156 Light Years)
sleepTimeMillis - Debugging, introduce artificial pause between each loop of code. Should be 0 for full performance.
worldSeed - Not used anymore. Was used in previous datasets where no angular or distance information available.
quadtreeSearchSize - used to strike a balance for how small the map is divided up. default 128 for around 150,000 starss.
font - to be implemented
summaryShowPerProbe - show console debug info on each probe at end of simulation.
summaryShowFooter - show console  summary at end of simulation.

## Useful build commands

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