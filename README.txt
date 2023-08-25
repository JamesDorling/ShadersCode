IMAT3907       P2413609
This code generates a terrain with distant dependant tesselation, shadow mapping, fog and
post processing using OpenGL.
This therefore uses a vertex, tesselation control, tesselation evaluation, geometry and fragment 
shaders.

CONTROLS:
W, A, S, D - Move camera
Shift - Speed up camera
Left ctrl - Slow down camera
Scroll wheel - Zoom in / out
0 - reset all values
1 - Draw wireframes
2 - Draw individual vertices
3 - Disable fog
4 - Disable shadow

Post processing:
G - Greyscale
H - Inverted Colours
J - Sharpen colours
K - Blur
L - Earthquake, Nausea, Drunkview

Numpad:
0 - Set the terrain mode to use the heightmap
1 - Set the terrain mode to use perlin noise
2 - Set the colour mode to use just height
3 - Set the colour mode to use both height and slope
4 - Set fog to be calculated exponentially
5 - Set fog to be calculated linearly

