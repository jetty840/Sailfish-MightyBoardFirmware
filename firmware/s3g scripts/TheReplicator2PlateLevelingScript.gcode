(**** This is a build-platform calibration script for a Single-Head MakerBot Replicator2 ****)
(**** Do not run this code on any other kind of machine! ****)
G21
G90

M71 (Tighten each of the three knobs under   the build platform  about four turns.)
M71 (I'm going to move myextruder around to  different points so you can check...)
M71 (the height. At each point, loosen the   specified knob untilthe nozzle almost...)
M71 (touches the build   plate. The nozzle isat the right height when a thin piece...)
M71 (of paper will slide between the nozzle  and build plate withsome friction.)

M70 ( Please wait)

G162 X Y F2500 (home XY axes maximum)
G161 Z F1100 (home Z axis minimum)
G92 Z-5 (set Z to -5)
G1 Z0.0 (move Z to "0")
G161 Z F100 (home Z axis minimum)
G92 X152 Y75 Z0 

G1 X0 Y70 F3000
G1 Z0 F1000
M71 (Adjust the rear knobuntil paper just    slides between the  nozzle and plate    )
M70 ( Please wait)
G1 Z5

G1 Y-70 X48 F3000
G1 Z0 F1000
M71 (Adjust front right  knob until paper    just slides between nozzle and plate    )
M70 ( Please wait)
G1 Z5

G1 X-48 Y-70 F3000
G1 Z0 F1000
M71 (Adjust front left   knob until paper    just slides between nozzle and plate    )
M70 ( Please wait)
G1 Z5


M71 (Now I'm going to    send my extruder to all three corners   again to recheck.)
M70 ( Please wait)

G1 X0 Y70 F5000
G1 Z0 F1000
M71 (Adjust the rear     knob until paper    just slides between nozzle and plate    )
M70 ( Please wait)
G1 Z5

G1 Y-70 X125 F5000 
G1 Z0 F1000
M71 (Adjust front right  knob until paper    just slides between nozzle and plate    )
M70 ( Please wait)
G1 Z5

G1 Y-70 X-125 F5000
G1 Z0 F1000
M71 (Adjust front left   knob until paper    just slides between nozzle and plate    )
M70 ( Please wait)
G1 Z5

G1 X0 Y0 F5000
G1 Z0 F1000
M71 (Now let's triple    check-- paper shouldjust slide between  nozzle and plate    ) 
