# gpio_stimulus.py — Python peripheral for toggling PA0/PA1 from Renode Monitor
#
# PA0 = Direction (EXTI0)
# PA1 = Enable    (EXTI1)

self.direction = False
self.enable    = False

def ToggleDirection(self):
    self.direction = not self.direction
    self.Connections[0].Set(self.direction)  # drive PA0
    self.Log(LogLevel.Info, "Direction -> {}".format(self.direction))

def ToggleEnable(self):
    self.enable = not self.enable
    self.Connections[1].Set(self.enable)     # drive PA1
    self.Log(LogLevel.Info, "Enable -> {}".format(self.enable))
