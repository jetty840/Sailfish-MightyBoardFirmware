M103
M73 P0
G21 (set units to mm)
G90 (set positioning to absolute)
G162 X Y F2500 (home XY axes maximum)
G161 Z F1100 (home Z axis minimum)
G92 Z-5 (set Z to -5)
G1 Z0.0 F1100 (move away from the Z endstop)
G161 Z F100 (slowly home Z axis minimum again)
