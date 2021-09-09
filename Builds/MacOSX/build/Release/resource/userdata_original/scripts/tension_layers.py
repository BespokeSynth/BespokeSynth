#requires the "schedule_seq.py" script and the "chord_tools.py" script

Cm = tuple(get_chord('C3','m',0))
Fm = tuple(get_chord('F2','m',2))
Bb = tuple(get_chord('Bb2','M',0))
Gm = tuple(get_chord('G2','m',2))

sequence = [( Cm+('G#3','D3' ),  ['C2']*8  ),
            ( Fm+('',   'C#3'),  ['C#2']*8 ),
            ( Bb+('G#3','C3' ),  ['D2']*8  ),
            ( Gm+('G#3','Bb2'),  ['D2']*8  )]

schedule_seq(sequence, bars=8)
 