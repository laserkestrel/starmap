# About

Starmap is a simulation of a Von Neumann probe set against a galaxy, experimenting with navigation and replication algorithms to effeciently explore the galaxy and visualise as the experiment unfolds. 

# Credits

Thanks to; <BR>
https://github.com/astronexus/HYG-Database for the source of astronomical data.<BR>
https://github.com/SFML/cmake-sfml-project for the project boilerplate code/template.<BR>
Michael from www.dream-ware.co.uk for use of the Elite Frontier TTF font.

# Configuration

displayMode - "borderlessFullscreen" (fills the screen, no border, alt-tabs instantly), "exclusiveFullscreen" (a real mode change) or "windowed" (uses the window width/height below). Toggle windowed/borderless at any time with F11.<BR>
verticalSync - Cap the loop to the monitor's refresh rate. Off by default: the render loop is also the simulation loop, so turning it on slows the simulation to your refresh rate too.<BR>
window.width / window.height - Only used when displayMode is "windowed".<BR>
scaleFactor - How many parsecs span the width of the window. Smaller = closer in. (1 Parsec ~ 3.26156 Light Years.)<BR>
viewTiltDegrees - The map is a tilted view of the equatorial plane. 90 is straight down, where height above the plane is invisible; 0 is edge on. Around 75 reads well.<BR>
viewDepthParsecs - How far above and below the plane the view reaches. An orthographic view is unbounded in depth, so without this every distant star projects into frame and the stalks become a solid curtain.<BR>
sleepTimeMillis - Debugging, introduce artificial pause between each loop of code. Should be 0 for full performance.<BR>
loadStarsLimit - Load only the nearest N stars from the catalogue.<BR>
quadtreeSearchSize - Stars per quadtree leaf before it splits. Default 128.<BR>
probeSearchRadiusParsecs - How far a probe will look for its next unvisited star. In parsecs, not pixels: how far a probe can see no longer depends on your window size.<BR>
probeSpeedParsecsPerTick - How far a probe moves each simulation tick.<BR>
probeIndividualReplicationLimit - How many times a single probe may replicate before shutting down.<BR>
starSpriteStyle - How a star is drawn: "softGlow", "coreHalo", "diffractionSpikes" or "bloomRing". Cycle them live with F5.<BR>
zoomMinPixelsPerParsec / zoomMaxPixelsPerParsec - Limits for mouse-wheel zoom.<BR>
summaryShowPerProbe - show console debug info on each probe at end of simulation.<BR>
summaryShowFooter - show console summary at end of simulation.<BR>

# How positions are drawn

Stars are stored at their true positions in parsecs, taken from the catalogue's own
x/y/z columns, with Sol at the origin. The map is an orthographic view of that space,
tilted back from straight-down so the third dimension is visible: each star is drawn
with a stalk running to the point directly below or above it on the plane, and the
length of that stalk is its height above or below the plane.

The starfield is drawn fresh every frame rather than baked into a texture, because
a baked texture cannot pan or zoom. Each star is one textured quad, so the whole
field is a single draw call, and the quadtree is asked for just the stars that
could be on screen -- the cost tracks what is visible, not the size of the catalogue.
Stars are blended additively, which is what makes a crowded field read as light
rather than paint.

Earlier versions plotted distance against right ascension and discarded declination
entirely, which inflated a star's distance from the centre by 1/cos(declination) and
meant that a probe's "nearest" star was the true nearest only about a quarter of the
time. Probes now search in real three-dimensional space.

# Key Bindings

F1 - Toggle Star Names<BR>
F2 - Toggle Probe Names<BR>
F3 - Toggle Probe Trails<BR>
F4 - Toggle Star Stalks (the lines showing height above/below the plane)<BR>
F5 - Cycle the star sprite style<BR>
F12 - Toggle Debug (Shows Quadtree boundaries, FPS and the number of stars drawn)<BR>
F11 - Toggle between windowed and borderless fullscreen<BR>
Home - Reset the view to Sol at the configured zoom<BR>
Arrow keys - Pan the view (hold Shift to pan faster)<BR>
Mouse wheel - Zoom, centred on the cursor<BR>
ESC - Exit Program<BR>

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
