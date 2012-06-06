(**** This is a build-platform calibration script for a Dual-Head MakerBot Replicator ****)
(**** Do not run this code on any other kind of machine! ****)
G21 (set units to mm)
G90 (set positioning to absolute)
M71 (Tournez les 4 vis   sous la plaque      chauffante et serrezde 4 ou 5 tours.   )
M71 (On va ajuster les   tetes, je vais donc les deplacer dans   differentes posit. )
M71 (Pour chaque posit.  nous allons ajuster 2 vis a la fois.    Tordez pas la plaque)
M71 (Les tetes sont a la bonne hauteur quand vous pouvez glisser une feuille        )
M71 (entre la tete et    la plaque chauffantePrenez une feuille  pour vous aider.   )
M70 ( Attendez...)
(**** begin homing ****)
G162 X Y F2500 (home XY axes maximum)
G161 Z F1100 (home Z axis minimum)
G92 Z-5 (set Z to -5)
G1 Z0.0 (move Z to "0")
G161 Z F100 (home Z axis minimum)
G92 X152 Y75 Z0 
(M132 X Y Z A B (Recall stored home offsets for XYZAB axis)
(**** end homing ****)

M70 ( Attendez...)
G1 Z5 F3300.0
G1 X16.5 Y-74 (Move to front of platform)
G0 Z0
M71 (Ajustez les 2 vis   de devant jusqu'a   pouvoir glisser la  feuille au milieu.  )

M70 ( Attendez...)
G1 Z5 F3300.0
G1 X16.5 Y72 (Move to back of platform)
G0 Z0
M71 (Ajustez les 2 vis   arriere jusqu'a     pouvoir glisser la  feuille au milieu   )

M70 ( Attendez...)
G1 Z5 F3300.0
G1 Y0 X106.5
G0 Z0
M71 (Ajustez les 2 vis   de droite jusqu'a   pouvoir glisser la  feuille au milieu   )

M70 ( Attendez...)
G1 Z5 F3300.0
G1 X-73.5
G0 Z0
M71 (Ajustez les 2 vis   de gauche jusqu'a   pouvoir glisser la  feuille au milieu   )

M70 ( Attendez...)
G1 Z5 F3300.0
G1 Y0 X16.5
G0 Z0
M71 (Verifiez que la     feuille glisse      bien sous les tetes.)

