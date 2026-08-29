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

trailColourMode - "recency", "density" or "lineage" ("perProbe" is still accepted as the old name for lineage). Also on the Display tab, and F6 cycles it live.<BR>
trailPalette - Which of the eight colour ramps to use, 0-7. F7 cycles it live.<BR>
trailFadeTicks - Ticks for a trail to cool from white to dark in recency mode. Short values leave only the expansion front lit.<BR>
evolutionEnabled - Whether children inherit their parent's traits with mutation, or every probe simply uses the founder's genome. Off restores the previous fleet-wide behaviour exactly.<BR>
neutralControl - The control condition. Traits still mutate and are still reported, but every probe behaves as the founder did, so the genome cannot affect survival. Whatever the report shows in this mode is drift. Also the third option on the setup screen's Evolution control.<BR>
mutationStrength - Largest proportional change one trait can take in a generation, as a fraction (0.08 is 8%). Zero means no evolution; too high and inheritance stops meaning anything, because a child resembles its parent no more than a stranger. Also a slider on the Simulation tab, in percent.<BR>
mutationSeed - Fixes the mutation sequence so a run repeats exactly. Change it to get an independent replicate of the same experiment, which is the only way to tell selection from drift.<BR>
probeLabelSize - Point size for probe name labels. The label declutter grid is sized from this, so a larger value shows fewer, bigger names rather than overlapping ones.<BR>
lineageHueSpread - How much of the colour wheel the whole family tree spans, 0 to 1. At 1.0 the top-level branches are as far apart as they can be; lower values tint the entire fleet towards one part of the spectrum.<BR>
showStarStalks / showStarNames / showProbeNames / showProbeTrails - Which overlays start switched on. All four are on the Display tab and on F4 / F1 / F2 / F3.<BR>
trailDensitySaturateAt - Arrivals at which a system is fully white in density mode. The scale is logarithmic, because arrival counts are heavy-tailed and a linear ramp leaves almost everything at the cold end. Default 60: with a typical mean around 12 arrivals per system, 24 washed most of the map out to white.<BR>

# Beginning an expedition

The setup screen is driven entirely by the MOUSE: drag the sliders, click the
profile and sprite buttons, click LAUNCH (or press Enter). The arrow keys still pan
the map behind it, so nothing competes for them - the old screen used Up/Down for
both panning and editing values, which is why they fought.

It has two tabs. SIMULATION holds everything that changes the outcome and therefore
the score; DISPLAY holds everything that only changes how it looks, including the
four overlay toggles that are otherwise F1 to F4. The split is not
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

# Evolution

Each probe carries its own copy of four behavioural traits -- search radius, child
fuel share, replication limit and harvest patience -- and a child inherits its
parent's values nudged by up to mutationStrength either way. There is no fitness
function anywhere. Selection was already in the simulation the moment replication
started costing resources: a probe that over-reaches strands and stops reproducing,
one that hoards gets outbred, one that mines a poor system dry wastes ticks a rival
spends travelling. The work was not adding selection, it was removing the fleet-wide
settings that prevented it.

The setup sliders therefore stop being law and become the FOUNDER'S genome. You are
no longer setting how probes behave, you are choosing what the first one believes.

## Does it actually work? Not proven, and here is how to check

Traits move in every run. Drift moves traits too, so movement on its own means
nothing -- and the numbers involved are much larger than they look. A neutral model
of the same mutation over 65 generations with a population of 25 per generation puts
the 95th percentile of DRIFT ALONE at +62%, and its maximum at +120%. A single run
showing a trait up 110% is inside that envelope.

So the simulation carries its own control. Set Evolution to NEUTRAL and traits still
mutate and are still reported, but every probe behaves as the founder did, so the
genome cannot affect who survives. Anything the report shows in that mode is drift,
measured in a real run's real branching structure rather than approximated outside
it. Run the same settings both ways and compare.

Doing exactly that over a 10,000 star catalogue, three runs each, roughly 35
generations:

    trait               selection runs            neutral runs
    Search radius       -13.4 .. +22.4 %          + 4.1 .. +85.0 %
    Child fuel share    - 6.4 ..  +4.9 %          + 2.6 .. +30.1 %
    Replication limit   +18.8 .. +89.8 %          +11.4 .. +26.7 %
    Harvest patience    -19.2 ..  +4.1 %          + 2.8 .. +26.1 %

Nothing separates. Every trait's selection range overlaps its control range, so at
three runs each NONE of these movements is demonstrably selection. The nearest thing
to a signal is the replication limit, where all three selection runs rose and the
mean was two and a half times the control's -- worth more runs, not worth believing
yet.

Two things worth taking from that. Search radius rising in a run is not evidence
that reaching further works: the control drifted UP more than selection did. And an
earlier claim in this file, that child fuel share was under clear selection, came
from five runs with no control and does not survive one -- under selection it barely
moves while the control drifts upward, which if anything hints at the opposite.

The lesson is the general one: with a founder population of one, mutation is the
only source of variation, generations are few, and the effective population per
generation is small. That combination makes drift enormous. Read the EVOLUTION block
as one sample from a wide distribution, and use the neutral control before believing
any of it.

# Probe names

A probe is named for its path through the family tree and the system it was built
at: `ABCAB@Gl 65A`. One letter per generation says which child it was, so the name
is unique by construction, its LENGTH is the generation, and a shared prefix means
shared ancestry -- ABCAB and ABCAA are siblings, ABCAB and BAA parted at the root.
Unnamed catalogue entries fall back to `#<id>`.

This replaced `<birth star, 3 chars>-<parent's birth star, 3 chars>-<code>`, which
had three separate faults. The code was not an identifier but the generation depth,
reached by incrementing the parent's, so every probe at a given depth carried the
same one and names were never unique. Three characters collided constantly: "Gl 65A"
and "Gl 244B" both truncate to "GL ". And 84 of the nearest 2,500 stars have no name
in the catalogue at all, which left that group empty and produced names starting
with a bare dash.

The family tree turns out to be long and thin rather than bushy. If every probe made
three copies, 1,800 probes would be about seven generations deep; in practice paths
run past twenty, because a resource economy that only just affords one copy per probe
produces a chain rather than a tree. Twenty-letter labels are unreadable, so paths
longer than six letters are drawn as generation plus the last three branches --
`21:AAA@GJ 3100` -- while the full path stays the identity in the console and the
debrief. probeLabelSize sets the point size; the declutter grid sizes itself from it,
so raising it thins labels out rather than letting them overlap.

# Reading the trails

TRAIT colours by a genome value, chosen on the Display tab. The map starts mottled
and converges towards one colour if a strategy is taking over -- selection rendered
directly on the starfield.

LINEAGE gives each family its own band of the colour wheel. The root probe owns the
whole wheel and every child is handed a slice of its parent's share, so a subtree
always occupies one contiguous arc: how far apart two probes look is how far apart
they are in the family tree. Green sweeping one half of the galaxy and magenta the
other is two branches of one family dividing the work between them.

Two earlier attempts are worth recording because both looked right and were not.
Stepping the hue a fixed amount per generation accumulates drift, wraps the wheel
every handful of generations, and leaves distant strangers the same colour as close
relatives -- no better than the random colouring it replaced. Then reserving a slice
of each parent's arc for the parent itself, and splitting only what was left, left
that slice never subdivided: with a replication limit of 3 the entire 0-90 degree
wedge, every red, orange and yellow, became unreachable. Simulated over 3,000 probes
it produced no hue at all between 0.02 and 0.22 of the wheel. Children now divide
their parent's whole arc and a probe's own colour is the middle of its arc, which
uses the full spectrum and keeps a parent away from the boundary it shares with a
sibling subtree.

A colour change where two trails meet is not always a probe changing colour: trails
from different probes join at shared stars, so what looks like one path can be two.

Recency and Density use the same eight palettes, chosen on the Display tab or cycled with F7. Each
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
F6 - Cycle trail colouring: recency, density, lineage, trait<BR>
F7 - Cycle the trail palette<BR>
F8 - Show this list of keys on screen<BR>
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
