# Monte Carlo Path Tracer

A Monte Carlo path tracer inspired by *Ray Tracing in One Weekend*, refactored to be run multithreaded on CPU with metrics for benchmarking.

![Final scene render](images/final_scene.png)

## Build

Install OpenMP with
```bash
brew install libomp
```

Then build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -BUILD_VIEWER
cmake --build build -j
```

## Run

### Offline render

```bash
./build/inOneWeekend
```

### Offline benchmark mode

```bash
./build/inOneWeekend --benchmark
```

### Real-time viewer

```bash
./build/viewer
```

Controls:
- WASD - move
- Shift / Space - down / up
- Right mouse drag - look around

## Results

Benchmark scene rendered at 400 x 225, 50 samples/pixel, max depth 50, on an Apple M1 MacBook.

| Backend | Threads | Render time | Throughput | Speedup |
|---------|--------:|------------:|-----------:|--------:|
| Single-threaded CPU | 1 | 13.20 s | 0.34 Mrays/s | 1.00x |
| OpenMP CPU | 8 | 3.37 s | 1.34 Mrays/s | 3.92x |

## Credits

Project based on Peter Shirley's *Ray Tracing in One Weekend* series.
