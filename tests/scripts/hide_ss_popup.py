#!/usr/bin/env python
import dbus
dbus.SessionBus().get_object('superswitcher.SuperSwitcher',
                            '/superswitcher/SuperSwitcher').HidePopup()
