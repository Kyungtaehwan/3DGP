"""
Generate a header-less 16-bit grayscale RAW height map for the terrain.

Fractal value noise (several octaves of a smoothly interpolated random grid)
produces natural rolling hills. Output is SIZE*SIZE little-endian unsigned
shorts (0..65535), row-major = SIZE*SIZE*2 bytes. Matches CHeightMapImage's
loader. 16-bit (vs 8-bit) gives 256x finer height steps, so the surface is
smooth instead of stair-stepped/terraced.

Usage:  python gen_heightmap.py [size] [seed]
"""
import sys, random, math, array

SIZE = int(sys.argv[1]) if len(sys.argv) > 1 else 1024
SEED = int(sys.argv[2]) if len(sys.argv) > 2 else 1234
OUT  = "HeightMap.raw"

random.seed(SEED)

def smootherstep(t):
    # Perlin's quintic fade -> C2-continuous slopes (no visible grid creases).
    return t * t * t * (t * (t * 6 - 15) + 10)

def make_octave(freq):
    """A (freq+1) x (freq+1) grid of random values in [0,1)."""
    n = freq + 1
    return [[random.random() for _ in range(n)] for _ in range(n)]

# Octaves: (grid frequency, amplitude). Low freq = broad hills, high freq = detail.
octaves = [(2, 1.0), (4, 0.5), (8, 0.25), (16, 0.125), (32, 0.0625), (64, 0.03125)]
grids   = [(f, a, make_octave(f)) for (f, a) in octaves]
amp_sum = sum(a for (_, a) in octaves)

height = [0.0] * (SIZE * SIZE)
inv    = 1.0 / (SIZE - 1)

for y in range(SIZE):
    fy = y * inv
    row = y * SIZE
    for (freq, amp, grid) in grids:
        gy = fy * freq
        iy = int(gy)
        if iy >= freq: iy = freq - 1
        ty = smootherstep(gy - iy)
        g0 = grid[iy]
        g1 = grid[iy + 1]
        # Precompute per-column contribution for this row/octave below.
        for x in range(SIZE):
            gx = (x * inv) * freq
            ix = int(gx)
            if ix >= freq: ix = freq - 1
            tx = smootherstep(gx - ix)
            top = g0[ix] * (1 - tx) + g0[ix + 1] * tx
            bot = g1[ix] * (1 - tx) + g1[ix + 1] * tx
            height[row + x] += amp * (top * (1 - ty) + bot * ty)
    if (y & 127) == 0:
        print(f"  row {y}/{SIZE}")

# Normalize to 0..65535 (16-bit) across the whole map.
lo = min(height)
hi = max(height)
span = (hi - lo) or 1.0
vals = array.array('H', (int((h - lo) / span * 65535.0 + 0.5) for h in height))  # 'H' = little-endian u16 on x86

with open(OUT, "wb") as f:
    vals.tofile(f)

print(f"wrote {OUT}: {SIZE}x{SIZE} = {len(vals) * 2} bytes (16-bit)")
