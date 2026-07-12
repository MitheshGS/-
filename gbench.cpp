#include <benchmark/benchmark.h>
#include <SFML/System.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <sstream>

#include "Triangulation.h" 

// 1. Peak Memory Tracker
long getPeakMemoryUsageKB() {
    std::ifstream file("/proc/self/status");
    std::string line;
    long ram_usage = 0;
    
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmHWM:") {
            std::stringstream ss(line.substr(6));
            ss >> ram_usage;
            break;
        }
    }
    return ram_usage;
}

// 2. High-Precision Polygon Generator
std::vector<sf::Vector2f> generateStarPolygon(int numVertices) {
    std::vector<sf::Vector2f> points;
    
    // Crucial for 10^8: Pre-allocate 1.6 GB of RAM in one continuous block
    points.reserve(numVertices); 
    
    std::random_device rd;  
    std::mt19937 gen(rd()); // Randomized shapes for every run
    
    // Massively increased radius to prevent 32-bit float collision
    std::uniform_real_distribution<double> radiusDist(10000000.0, 50000000.0);
    
    double centerX = 0.0;
    double centerY = 0.0;
    
    for (int i = 0; i < numVertices; ++i) {
        // Use 64-bit doubles for perfect mathematical slicing
        double angle = (i * 2.0 * M_PI) / numVertices;
        double radius = radiusDist(gen); 
        
        float x = static_cast<float>(centerX + radius * cos(angle));
        float y = static_cast<float>(centerY + radius * sin(angle));
        points.push_back(sf::Vector2f(x, y));
    }
    
    return points;
}

// 3. The Benchmark Harness
static void BM_EarClipping(benchmark::State& state) {
    int n = state.range(0);
    std::vector<sf::Vector2f> points = generateStarPolygon(n);

    for (auto _ : state) {
        std::vector<sf::Vector2f> triangles = triangulate(points);
        benchmark::DoNotOptimize(triangles); 
    }

    state.counters["Peak_Memory_KB"] = getPeakMemoryUsageKB();
    state.SetComplexityN(state.range(0));
}

// Register the benchmark to scale from 100 vertices all the way to 100,000,000 (10^8)
BENCHMARK(BM_EarClipping)->RangeMultiplier(10)->Range(100, 1000000)->Complexity();

BENCHMARK_MAIN();