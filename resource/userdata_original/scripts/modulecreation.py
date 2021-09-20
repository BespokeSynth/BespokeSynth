import module

if 'osc' in globals():
   osc.delete()
osc = module.create("oscillator", 100, 600)
if 'pulser' in globals():
   pulser.delete()
pulser = module.create("pulser", 200, 300)
pulser.set("interval",5)

pulser.set_target(module.get("script"))
module.get("script").set_target(osc)
osc.set_target(module.get("gain"))  

def on_pulse():
   step = bespoke.get_step(8) % 8
   osc.set_position(150 * (step+1), 600)
   this.play_note(bespoke.get_root() + bespoke.get_scale()[step%7] + 36 + step / 7 * 12, 127, 1.0/8, pan=step / 4.0 - 1)
   