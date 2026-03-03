# Delaunay Triangulation (2D + 3D)

A C++23 computational geometry project focused on Delaunay triangulation algorithms and Vulkan-based 3D visualization.

## Current Scope

- 2D triangulation
  - `tools_2D::boyer_watson_2D`
  - `tools_2D::guibas_stolfi_2D`
- 3D triangulation
  - `tools_3D::bowyer_watson_3D`
  - `tools_3D::guibas_stolfi_3D`
- 3D boundary extraction
  - `tools_3D::extract_boundary_surface`
- Vulkan viewer app
  - interactive camera
  - algorithm switching
  - point cloud reseeding

## Project Layout

```text
delaunay_triangulation/
├── modules/
│   ├── geometry_tool/          # 2D/3D geometry + triangulation
│   └── csv_parser/             # CSV parser module
├── apps/
│   └── vulkan_viewer/          # Vulkan visualization app
├── samples/
│   └── sample_3d.cpp           # CLI sample for 3D triangulation
├── tests/
├── CMakeLists.txt
├── CMakePresets.json
└── vcpkg.json
```

## Build

```bash
cmake --preset clangcl-vcpkg-debug
cmake --build build
```

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## 3D Sample (new)

A dedicated sample executable is now provided:

- source: `samples/sample_3d.cpp`
- target: `sample_3d`

Run:

```bash
./build/samples/sample_3d.exe
./build/samples/sample_3d.exe 200 bowyer 42
./build/samples/sample_3d.exe 200 guibas 42
```

Arguments:

1. point count (minimum 8)
2. algorithm: `bowyer` or `guibas`
3. random seed

Output includes:

- input point count
- generated tetrahedra count
- extracted boundary vertices/triangles count

## Vulkan Viewer

Run:

```bash
./build/apps/vulkan_viewer/vulkan_viewer.exe
```

Controls:

- `1`: switch to Bowyer-Watson
- `2`: switch to Guibas-Stolfi
- `R`: reseed point cloud
- `↑ / ↓`: increase/decrease point count
- mouse drag: orbit camera
- mouse wheel: zoom
- `Esc`: exit
