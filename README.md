# Delaunay Triangulation Library

A high-performance Delaunay triangulation library implementation using modern C++23. This project implements the **Bowyer-Watson incremental algorithm**, providing both efficient and robust triangulation functionality.

## 📋 Project Overview

Delaunay triangulation is a fundamental algorithm in computational geometry with widespread applications in:
- Terrain modeling and mesh generation
- Finite Element Analysis (FEM)
- Collision detection
- Spatial interpolation and point cloud processing

This implementation uses the classic **Bowyer-Watson algorithm**, guaranteeing that all triangles satisfy the **Delaunay property** (circumcircle emptiness property).

## 🏗️ Project Structure

```
delaunay_triangulation/
├── modules/
│   ├── delaunay/              # Core Delaunay triangulation library
│   │   ├── include/
│   │   │   └── delaunay.h      # API header file
│   │   └── src/
│   │       └── delaunay.cpp    # Algorithm implementation
│   └── csv_parser/             # CSV coordinate file parser
│       ├── include/
│       │   └── csv_parser.h    # CSV parsing API
│       └── src/
├── tests/                       # Unit tests and performance benchmarks
│   ├── test_delaunay.cpp
│   └── test_parser.cpp
├── CMakeLists.txt              # Build configuration
└── README.md
```

## 🚀 Quick Start

### Build the Project

```bash
# 1. Create build directory
mkdir build && cd build

# 2. Configure and build
cmake ..
cmake --build .

# 3. Run tests
ctest
```

**Requirements:**
- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
- CMake 3.28+

### Usage Examples

#### Method 1: Direct Point Array Triangulation

```cpp
#include "delaunay.h"
#include <vector>

int main() {
    // Create a point set
    std::vector<tools_2D::point> points = {
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0}
    };
    
    // Perform triangulation
    auto triangles = delaunay_triangulate(points);
    
    // Process results
    for (const auto& tri : triangles) {
        // Access triangle vertices and properties
        auto circum = tri.circum_circle();
    }
    
    return 0;
}
```

#### Method 2: Read from CSV File

```cpp
#include "delaunay.h"
#include <iostream>

int main() {
    try {
        // Read coordinates from CSV and triangulate
        auto triangles = read_and_triangulate();  // Reads "points.csv"
        
        std::cout << "Generated " << triangles.size() << " triangles\n";
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
```

**CSV File Format** (`points.csv`):

```csv
0.0,0.0
1.0,0.0
1.0,1.0
0.0,1.0
```

Or with space separation:
```
0.0 0.0
1.0 0.0
1.0 1.0
0.0 1.0
```

## 📚 API Reference

### Core Functions

#### `delaunay_triangulate()`

```cpp
auto delaunay_triangulate(const std::vector<tools_2D::point>& points) 
    -> std::vector<tools_2D::triangle>;
```

**Parameters:**
- `points` - Input point set, minimum 3 non-collinear points required

**Returns:**
- Vector of triangles satisfying the Delaunay property

**Complexity:**
- Time: O(n log n) average case, O(n²) worst case
- Space: O(n)

**Notes:**
- Collinear points may result in degenerate triangles
- Floating-point precision may affect numerical stability

#### `read_and_triangulate()`

```cpp
auto read_and_triangulate() -> std::vector<tools_2D::triangle>;
```

**Functionality:**
- Reads coordinates from `points.csv`
- Automatically performs triangulation

**Exceptions:**
- `parse_error` - CSV file format error
- `std::invalid_argument` - Less than 4 points provided

### Auxiliary Classes (from geometry_tool library)

#### `tools_2D::point`
Represents a point in 2D space with:
- `get_x()`, `get_y()` - Coordinate accessors
- Basic geometric operations

#### `tools_2D::triangle`
Represents a triangle providing:
- `circum_circle()` - Get circumcircle
- `containsVertex(point)` - Check vertex containment
- `get_edges()` - Get three edges

## 🔧 Algorithm Details: Bowyer-Watson

This implementation uses the Bowyer-Watson incremental algorithm:

**Phase 1: Initialization**
- Create a super-triangle containing all input points

**Phase 2: Incremental Insertion**
For each point p:
- Identify all "bad" triangles whose circumcircle contains p
- Extract the boundary edges of these triangles
- Construct new triangles using p and boundary edges

**Phase 3: Cleanup**
- Remove all triangles containing any super-triangle vertex

**Key Properties:**
- ✅ Satisfies the empty circumcircle property (Delaunay property)
- ✅ O(n log n) average-case performance
- ✅ Incrementally processes points, suitable for dynamic point sets

## 📊 Test Coverage

Comprehensive unit and performance tests:

```bash
cd build
ctest --verbose
```

**Test Cases:**
- ✅ Basic triangle (3 points)
- ✅ Square (4 points)
- ✅ Random point clouds
- ✅ Collinear point handling
- ✅ Duplicate point handling
- ✅ Delaunay property verification (empty circle check)
- ✅ Performance benchmarks

## 🔗 Dependencies

| Dependency | Purpose | Version |
|-----------|---------|---------|
| geometry_tool | 2D geometric primitives | main |
| Catch2 | Testing framework | v3.x |

Dependencies are automatically downloaded via CMake `FetchContent`.

## 📈 Performance Benchmarks

Typical performance (single-threaded):

| Points | Time | Triangles Generated |
|--------|------|-------------------|
| 100 | ~0.5 ms | ~200 |
| 1,000 | ~5 ms | ~2,000 |
| 10,000 | ~60 ms | ~20,000 |

*Actual values depend on hardware and compiler optimizations*

## 💡 FAQ

**Q: How to handle collinear points?**  
A: When three or more points are collinear, degenerate triangles (area = 0) are generated. Consider removing or perturbing collinear points in preprocessing.

**Q: Does it support 3D triangulation?**  
A: Current implementation is 2D only. Extension to 3D (Tetrahedralization) is possible.

**Q: How to improve performance?**  
A: 
- Randomizing point order can improve worst-case behavior
- Enable compiler optimizations (-O3)
- Consider parallelization (currently single-threaded)

## 🏆 Best Practices

```cpp
// ✅ Recommended usage
std::vector<tools_2D::point> points = /* ... */;
if (points.size() >= 3) {
    auto triangles = delaunay_triangulate(points);
    // Use results
}

// ⚠️ Avoid: Not validating point count
auto bad = delaunay_triangulate(small_points);  // May fail
```

## 🐛 Known Limitations

1. Collinear points produce degenerate results - requires data preprocessing
2. Floating-point precision may affect numerical accuracy
3. No parallelization - no GPU acceleration support

## 📝 License & Author

```
Author: HUANG He
Email: he.huang@utt.fr
Copyright: © 2025
License: MIT
```

## 🤝 Contributing

Contributions and suggestions for improvement are welcome!

## 📖 References

- Bowyer, Adrian. "Computing Dirichlet tessellations." The Computer Journal 24.2 (1981): 162-166.
- Watson, David F. "Computing the n-dimensional Delaunay tessellation with application to Voronoi polytopes." The Computer Journal 24.2 (1981): 167-172.
