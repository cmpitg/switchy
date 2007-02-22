#!/usr/bin/env python
import dbus, sys
ss = dbus.SessionBus().get_object('superswitcher.SuperSwitcher',
                                 '/superswitcher/SuperSwitcher')

try:
    n = int(sys.argv[1])
except:
    n = 10

for i in range(n):
    ss.ShowPopup()
    ss.HidePopup()
