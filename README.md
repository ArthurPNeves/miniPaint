# Paint - Computer Graphics Application

A modern paint application built for a Computer Graphics course, featuring a web-based frontend with a C++ backend for graphics algorithms implementation.

![Paint Application](https://img.shields.io/badge/Language-C%2B%2B-blue.svg) ![Frontend](https://img.shields.io/badge/Frontend-HTML%2FJS%2FCSS-orange.svg) ![License](https://img.shields.io/badge/License-Educational-green.svg)

## Features

### Drawing Tools
- **Line Drawing**: DDA and Bresenham algorithms
- **Circle Drawing**: Bresenham circle algorithm
- **Selection Tool**: Select and manipulate drawn objects
- **Clipping Tool**: Crop/clip drawings to regions

### Transformations
- Translation
- Rotation
- Scaling
- Reflection

### Clipping Algorithms
- Cohen-Sutherland line clipping
- Liang-Barsky line clipping

### Interface Features
- Modern, responsive design
- Dark/Light theme toggle
- Fullscreen canvas mode
- Real-time drawing preview
- Color picker and brush size controls

## Project Structure

```
CompGrafica/
├── backend/                 # C++ server with graphics algorithms
│   ├── server.cpp          # HTTP server (httplib)
│   ├── algorithms.cpp      # Drawing algorithms implementation
│   ├── algorithms.h        # Algorithm headers
│   ├── transformations.cpp # Geometric transformations
│   ├── transformations.h   # Transformation headers
│   ├── libs/               # External libraries
│   │   ├── httplib.h       # HTTP server library
│   │   └── json.hpp        # JSON parsing library
│   ├── Makefile           # Build configuration
│   ├── build.sh           # Linux build script
│   ├── build.bat          # Windows build script
│   └── build.ps1          # PowerShell build script
└── frontend/               # Web interface
    ├── index.html         # Main HTML page
    ├── script.js          # JavaScript logic
    └── style.css          # Styling and themes
```

## Prerequisites

### For Linux/macOS:
- **g++** with C++17 support
- **make**

### For Windows:
- **MinGW-w64** (for cross-compilation)
- **Visual Studio** (alternative)

### For Frontend:
- Modern web browser with JavaScript support
- Local web server (optional, for serving static files)

## Building and Running

### Backend (C++ Server)

#### Linux/macOS:
```bash
cd backend
make
./bin/backend
```

#### Windows (using MinGW):
```bash
cd backend
make windows
./bin/backend.exe
```

#### Alternative build scripts:
```bash
# Linux
chmod +x build.sh
./build.sh

# Windows (Command Prompt)
build.bat

# Windows (PowerShell)
.\build.ps1
```

### Frontend

Simply open `frontend/index.html` in your web browser, or serve it with a local web server:

```bash
cd frontend
python -m http.server 8080
# Then visit http://localhost:8080
```

## API Endpoints

The C++ backend provides the following REST API endpoints:

### POST /draw
Rasterizes drawing primitives using computer graphics algorithms.

**Request Body:**
```json
{
  "tipo": "linha",  // "linha" or "circulo"
  "x1": 10, "y1": 10, "x2": 50, "y2": 50,  // For lines
  "xc": 25, "yc": 25, "r": 15               // For circles
}
```

**Response:**
```json
{
  "tipo": "linha",
  "dados": {...},
  "pixels": [[x1, y1], [x2, y2], ...]
}
```

### POST /transform
Applies geometric transformations to objects.

**Request Body:**
```json
{
  "dados": {...},           // Object data
  "tipoObj": "linha",       // Object type
  "transf": "translacao",   // Transformation type
  "params": {...}           // Transformation parameters
}
```

### POST /clip
Clips lines using various clipping algorithms.

**Request Body:**
```json
{
  "x1": 10, "y1": 10, "x2": 50, "y2": 50,  // Line coordinates
  "rx": 0, "ry": 0, "rw": 100, "rh": 100,  // Clipping rectangle
  "algoritmo": 1                            // 1=Cohen-Sutherland, 2=Liang-Barsky
}
```

## Algorithms Implemented

### Rasterization
- **DDA Algorithm**: Digital Differential Analyzer for line drawing
- **Bresenham Line**: Efficient integer-only line drawing
- **Bresenham Circle**: Efficient circle rasterization

### Clipping
- **Cohen-Sutherland**: Region-based line clipping algorithm
- **Liang-Barsky**: Parametric line clipping algorithm

### Transformations
- **Translation**: Moving objects in 2D space
- **Rotation**: Rotating objects around a point
- **Scaling**: Resizing objects
- **Reflection**: Mirroring objects across axes

## Usage

1. **Start the backend server**:
   ```bash
   cd backend && make && ./bin/backend
   ```

2. **Open the frontend** in your browser (`frontend/index.html`)

3. **Select a tool** from the toolbar:
   - **Desenhar**: Draw lines and circles
   - **Selecionar**: Select existing objects
   - **Recortar**: Apply clipping operations

4. **Choose drawing mode**:
   - **Linha**: Draw lines between two points
   - **Círculo**: Draw circles with center and radius

5. **Apply transformations** to selected objects

6. **Use clipping** to crop drawings to specific regions

## Development

### Adding New Algorithms

1. **Add algorithm implementation** to `backend/algorithms.cpp`
2. **Add function declaration** to `backend/algorithms.h`
3. **Update the rasterize function** to handle the new algorithm
4. **Add frontend controls** in `frontend/index.html` and `frontend/script.js`

### Customizing the Interface

- **Themes**: Modify CSS variables in `frontend/style.css`
- **Colors**: Update the color palette in the CSS
- **Layout**: Adjust grid layouts and responsive breakpoints

## Educational Purpose

This project demonstrates fundamental computer graphics concepts:

- **Rasterization algorithms** for primitive drawing
- **Geometric transformations** in 2D space
- **Clipping algorithms** for viewport management
- **Client-server architecture** for graphics applications
- **Web technologies integration** with low-level graphics programming

## License

This project is developed for educational purposes as part of a Computer Graphics course.

## Contributing

This is an educational project. For improvements or bug fixes:

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Test thoroughly
5. Submit a pull request

## Authors

- **Arthur Neves** - Computer Graphics Course Project

---

*Built with ❤️ for learning Computer Graphics fundamentals*
