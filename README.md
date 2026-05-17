#  OpenGL Graphics Projects

Two OpenGL/FreeGLUT rendering demos: an **Acer Logo** renderer and an animated **Brazil Flag**.
 
## Projects

#  1. Acer Logo (`Acer .cpp`)

Renders the Acer wordmark using GLU tessellation with correct counter (hole) handling for the letters **'a'** and **'e'**.

**Key features:**
- SVG path parser supporting M, L, H, V, C, S, Q, T, Z commands (absolute & relative)
- Cubic and quadratic Bézier curve sampling
- GLU tessellator with `GLU_TESS_WINDING_ODD` for automatic hole subtraction
- Interactive zoom, rotation, and color switching

**Controls:**

#  Key              Action  
 -----            -------- 
  `+` / `=`         Zoom in  
  `-`               Zoom out  
  `r`               Toggle rotation animation  
  `0`               Reset view  
  `1`               Color: Red  
  `2`               Color: Charcoal  
  `3`               Color: Blue  
  `4`               Color: Acer Green (default)  
  `q` / `ESC`        Quit  

---

### 2. Brazil Flag (`Brazil.cpp`)

Animated waving Brazilian flag with all official elements: green background, yellow rhombus, blue circle, white band, curved "ORDEM E PROGRESSO" text, and 27 stars.

**Key features:**
- Left-to-right cloth wave animation using a sine function
- OpenGL Stencil buffer used to clip stars and text inside the circle
- Custom vector font for the motto text rendered along a curve
- Interactive scale, rotation, and translation

**Controls:**

 # Key             Action 
  ----            -------- 
  `1`             Scale up 
  `2`             Scale down 
  `3`             Rotate +10°  
  `4`             Translate right 
  `0`             Reset all transforms 

---

## Build Instructions

### Linux / macOS

```bash
# Acer Logo
g++ -o Acer Acer.cpp -lGL -lGLU -lglut -lm

 # Brazil Flag
g++ -o Brazil Brazil.cpp -lGL -lGLU -lglut -lm

# With FreeGLUT instead of GLUT
g++ -o Acer Acer.cpp -lGL -lGLU -lfreeglut -lm
g++ -o Brazil Brazil.cpp -lGL -lGLU -lfreeglut -lm
```

### Windows (MinGW)

```bash
g++ -o Acer Acer.cpp -lfreeglut -lopengl32 -lglu32
g++ -o Brazil Brazil.cpp -lfreeglut -lopengl32 -lglu32
```

### Dependencies

- OpenGL
- GLU (OpenGL Utility Library)
- FreeGLUT or GLUT

**Install on Ubuntu/Debian:**
```bash
sudo apt install freeglut3-dev
```

**Install on macOS (Homebrew):**
```bash
brew install freeglut
```

---

## Run

```bash
./Acer
./Brazil
```
