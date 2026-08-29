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
startingViewRadiusParsecs - The starting zoom, given as the radius around Sol that is guaranteed to be in frame. Defining it as a radius rather than "parsecs across the window" keeps the opening view the same neighbourhood on any resolution or aspect ratio - a wider monitor simply shows more to either side instead of a shorter strip. Home returns to it at any time. (1 Parsec ~ 3.26156 Light Years.)<BR>
starLabelMaxVisible - How many star labels to draw at once. The brightest visible stars are chosen, so names thin out gracefully as you zoom out rather than disappearing all at once. Each label is its own draw call, so this also bounds their cost.<BR>
viewTiltDegrees - The map is a tilted view of the equatorial plane. 90 is straight down, where height above the plane is invisible; 0 is edge on. Around 75 reads well.<BR>
viewDepthParsecs - How far above and below the plane the view reaches. An orthographic view is unbounded in depth, so without this every distant star projects into frame and the stalks become a solid curtain. It is a bigger decision than it looks: at 50,000 stars an 8 pc slab hides 88% of the catalogue, and the probes still fly out there. Startup prints exactly how many stars the current value is hiding, so check that line rather than guessing.<BR>
sleepTimeMillis - Debugging, introduce artificial pause between each loop of code. Should be 0 for full performance.<BR>
loadStarsLimit - Load only the nearest N stars from the catalogue.<BR>
quadtreeSearchSize - Stars per quadtree leaf before it splits. Default 128.<BR>
probeSearchRadiusParsecs - How far a probe will look for its next unvisited star. In parsecs, not pixels: how far a probe can see no longer depends on your window size.<BR>
probeSpeedParsecsPerTick - How far a probe moves each simulation tick.<BR>
probeIndividualReplicationLimit - How many times a single probe may replicate before shutting down.<BR>
replicateOnFirstArrival - Whether a new probe may copy itself at the very first system it reaches, or must establish itself there first and wait for the next one. Roughly doubles the growth exponent: measured over 125 ticks, switching it on took the fleet from 1,266 probes to 24,383 while systems explored went only from 142 to 198. Also on the setup screen.<BR>
starSpriteStyle - How a star is drawn: "softGlow", "coreHalo", "diffractionSpikes" or "bloomRing". Cycle them live with F5.<BR>
zoomMinPixelsPerParsec / zoomMaxPixelsPerParsec - Limits for mouse-wheel zoom.<BR>
summaryShowPerProbe - show console debug info on each probe at end of simulation.<BR>
summaryShowFooter - print the full debrief to the console as well as drawing it on screen. Keep this on: it is where you look when something like a missing content folder has gone wrong.<BR>
frameBudgetMillis - How much of each frame may be spent running simulation ticks. The display gets whatever is left, so it stays responsive instead of being dragged along at whatever rate the simulation manages. Replaces the old sleepTimeMillis.<BR>
coverageTargetPercent - End the run once this share of the catalogue has been reached.<BR>
frontierStallTicks - End the run if no new furthest system has been reached for this many ticks.<BR>
maxProbes - Safety ceiling on fleet size. With the resource economy off this is the only thing that ends a run; with it on it should never be reached, and if it is, replication is too cheap for the galaxy you set.<BR>

## The resource economy

resourcesEnabled - Whether replication has to be paid for. Off restores the old behaviour, which is worth trying once for contrast: measured over the same 500-star catalogue, free replication built 25,738 probes in 127 ticks and reached 36% of the galaxy before hitting the cap, while the economy built 348 over 1,750 ticks and reached 92%. A fleet 74 times smaller explored two and a half times as much, because most of what a huge fleet does is retrace ground another lineage already covered.<BR>
systemResourceScale - Material in an averagely rich system. Raising it does NOT help: doubling it halved efficiency in testing, because abundance buys more probes and more probes waste more journeys.<BR>
resourceFeatureParsecs - How large a rich or poor region is. This is the setting that makes the galaxy have a geography rather than a texture: small values speckle richness so every neighbourhood averages the same and every lineage meets an identical galaxy, large values give broad lodes and deserts, so which way a lineage happens to expand decides whether it thrives.<BR>
resourceSeed - Fixes that geography. The same seed gives the same galaxy every run, which is what makes two runs with different parameters comparable rather than a test of luck.<BR>
replicationCostMetals / Volatiles / Fissiles - The bill of materials for one copy. The setup screen's "Probe build cost" slider scales all three together, so their proportions hold.<BR>
harvestPerTick - Units of each resource a probe extracts per tick while parked at a system. Lower means longer stays and a slower, more deliberate expansion.<BR>
maxHarvestTicks - Give up on a system after this long even if it still holds something.<BR>
fuelPerParsec - Volatiles burnt per parsec flown. Volatiles are both a build material and the propellant, which is what makes distance genuinely dangerous rather than merely slow.<BR>
fuelSafetyMargin - A probe will not depart unless it holds this multiple of the fuel the trip needs. Below 1.0 it will confidently set off on journeys it cannot finish.<BR>
childFuelShare - Share of the parent's remaining volatiles handed to a new probe. It comes out of the parent's tank, so each successive child is fuelled a little worse than the last.<BR>

## Trail appearance

trailColourMode - "recency", "density" or "perProbe". Also on the Display tab, and F6 cycles it live.<BR>
trailPalette - Which of the eight colour ramps to use, 0-7. F7 cycles it live.<BR>
trailFadeTicks - Ticks for a trail to cool from white to dark in recency mode. Short values leave only the expansion front lit.<BR>
trailDensitySaturateAt - Arrivals at which a system is fully white in density mode. The scale is logarithmic, because arrival counts are heavy-tailed and a linear ramp leaves almost everything at the cold end. Default 60: with a typical mean around 12 arrivals per system, 24 washed most of the map out to white.<BR>

# Beginning an expedition

The setup screen is driven entirely by the MOUSE: drag the sliders, click the
profile and sprite buttons, click LAUNCH (or press Enter). The arrow keys still pan
the map behind it, so nothing competes for them - the old screen used Up/Down for
both panning and editing values, which is why they fought.

It has two tabs. SIMULATION holds everything that changes the outcome and therefore
the score; DISPLAY holds everything that only changes how it looks. The split is not
just tidiness - with them mixed it was easy to nudge a parameter while meaning to
adjust the view, and then wonder why the score moved.

Each control says what it does and which way it pushes the result, and the line
above LAUNCH tells you how many systems sit within one hop of Sol at the current
search radius. If that reads zero, the first probe has nowhere to go.

Expedition profiles are starting points: Survey (short range, barely replicates -
slow but tidy), Swarm (replicates hard, looks no further than the doorstep), Scout
(long range, few children - far frontier, thin coverage). Move any slider and the
profile becomes Custom. Same galaxy, very different scores.

Changing "Stars loaded" re-reads the catalogue, so it waits until you release the
slider rather than reloading mid-drag. "System richness" does the same, because a
system's stocks are derived at load time from where it sits in the resource field.

# What a probe actually does

A probe with the economy on has something to do besides travel. It arrives at a
system and starts mining, which takes ticks - so a wasted journey now costs real
time as well as fuel. It leaves when it has enough for a copy, when the system is
stripped, or when it has given the place long enough. If it cannot afford a copy it
carries on anyway as a scout and tries to make up the shortfall at the next system.

Volatiles are both a build material and the propellant. A probe will not set off
unless it holds the fuel for the trip plus a margin, so a probe standing in a dry
system with nothing in range simply stops. One that misjudges it - or is handed too
thin a share by a parent that had already fuelled two other children - runs dry
between stars and is lost. In testing that became the ordinary way to die: 248 of
284 probes stranded, against 36 that reached their replication limit.

Systems do not come back. A fleet strips its own neighbourhood - 367 of 378 systems
reached were left with nothing - and then dies out. That is the run ending for a
reason that came from inside the simulation rather than from a number in a config
file, which is the whole point of the exercise.

# Measuring a run

A run now ends on its own and says why: every probe stopped, the coverage target
was reached, the frontier stopped advancing, the tick limit was hit, or the fleet
hit the population cap. The debrief is drawn in the window over the finished map,
and printed to the console as well for headless runs.

Everything is measured per TICK or as a ratio, never per wall-clock second. A
metric that divides by elapsed time is measuring the computer it ran on, which is
why the old "stars per probe-second" figure could not compare two runs.

The number that matters most is EFFICIENCY: unique systems reached divided by total
arrivals. 1.000 means no journey was ever wasted. Because probes only know what
they and their ancestors found, unrelated lineages revisit each other's systems -
and this is the number that measures how much that costs.

SCORE is systems reached, with half the credit for arriving at all and half for not
having wasted a journey doing it. It used to be reach multiplied by efficiency
outright, which sounds fair and is not: it made efficiency the entire game. Over a
2,000 star catalogue a fleet held to one copy each reached 38% of it at a perfect
1.000 - every probe dies young, so lineages never overlap - while a fleet allowed
two copies reached 92% at 0.10. The old formula scored those 767 against 187, which
says the sprawling run was four times worse for reaching two and a half times as
much. Nobody would ever choose to sprawl, so there was no decision left to make.

With the floor at a half, both strategies are live and which one wins depends on
the galaxy: careful expansion wins in a catalogue small enough to finish, and
aggressive expansion wins in one it could never finish. GRADE is that score against
the perfect run for the catalogue you actually loaded - every system, nothing
wasted - so an A means the same thing at 400 stars as at 50,000.

One honest caveat: probes mine in parallel across threads, and when two of them
strip the same system in the same tick the order they win in is down to the
scheduler. Two runs with identical parameters therefore land within about 3% of
each other rather than matching exactly.

# High scores and running again

The debrief has three buttons. NEW EXPEDITION returns to the setup screen with your
last settings still in place, so you can change one slider and try again without
restarting the program. VIEW MAP (also F9) hides the panel so you can pan and zoom
around the finished map with nothing over the top of it; F9 again brings the panel
back. QUIT (also Esc) closes the program.

The top twelve runs are kept in `./content/highscores.json`, alongside the config,
and survive between sessions - delete that file to start the table over. Each entry
records the parameters that produced it, not just the result, so the table is a log
of what actually worked rather than a list of numbers with no explanation. A run that
places is highlighted in the table as NEW ENTRY #n.

# Reading the trails

Trails draw as they happen. A probe's current leg is drawn from the last system it
left to wherever it is right now, so you watch a journey being made rather than
seeing it appear the moment it finished. It costs one line segment per moving probe
- about 7% on top of geometry that was already being rebuilt every frame - which is
why it was never worth not doing.

Colouring used to be one random hue per probe, assigned at birth. It encoded
nothing: siblings got unrelated colours, a parent and its child got unrelated
colours, and a fleet in the thousands turned the map into confetti. That mode is
still there as "Per probe" for comparison, and the comparison is not flattering.

RECENCY fades a leg as it ages, over trailFadeTicks. Whatever is being flown right
now is white, and history recedes to a dark ember, so the expansion front is
obvious and you can follow individual journeys.

DENSITY colours a leg by how many probes have ever arrived at the system it leads
to. This is the wasted-journey figure made visible: in a typical run the interior
burns white because it has been crossed dozens of times, while the frontier stays
cool at one or two visits. If you want to see why efficiency sits around 0.08, this
is the picture of it.

Both use the same eight palettes, chosen on the Display tab or cycled with F7. Each
ramp is a single hue running dark to white, deliberately not a rainbow: brightness
ordering stays readable, where a rainbow looks livelier and is far harder to rank
by eye.

## Probes that appear to fly into empty space

Watch the trails for a while and you may see a probe set off into blackness and
seemingly replicate in the middle of nowhere. Nothing is wrong with the simulation:
instrumenting every replication in a 2,500 star run gave 1,765 of them at a distance
of exactly 0.000 pc from a catalogue star. Probes only ever replicate at a system.

What was wrong was the drawing. Stars outside viewDepthParsecs are culled, and for a
long time probes and trails were not, so a probe travelling to a star deeper than the
slab appeared to head into empty space and arrive nowhere. The space was never empty
- it was full of stars that are not drawn. The scale of it is easy to underestimate:

    nearest    500 stars, 8 pc slab ->  13.6% of the catalogue hidden
    nearest  2,500 stars, 8 pc slab ->  44.5% hidden
    nearest 50,000 stars, 8 pc slab ->  88.5% hidden

So at the default 50,000 stars, seven eighths of the galaxy the probes are working in
is invisible. Probes and trails now fade out as they leave the slab instead of being
drawn at full strength, so a journey heading out of the visible slice reads as
leaving rather than as going nowhere, and the map is one coherent slice of space.
The console reports on startup how much the current depth is hiding.

A newborn's opening leg is also drawn now, from the point where it was built. Its
trail is empty until it reaches its first system, so previously a child detached from
its parent's star and drifted off with no line behind it - which is the other half of
what made replication look like it was happening in interstellar space.

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
F6 - Cycle trail colouring: recency, density, per probe<BR>
F7 - Cycle the trail palette<BR>
F9 - At the debrief, hide or show the results panel over the map<BR>
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
