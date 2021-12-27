# cvolctrl

Simple volume notifier written in C

![](resources/preview.gif)



## Usage

```
Usage: cvolctrl <volume> <togglemute>

Volume is signed integer, which is added to current volume level.
Togglemute is enabled if set to 1 and disabled if set to 0.

Examples:
 cvolctrl 5 0 -> increases volume by 5
 cvolctrl -10 1 -> decreases volume by 10 and toggles mute

```



Bind to keyboard volume buttons. In this example xbindkeys config file:

```
"python /path/to/cvolctrl/cvolctrl 10 0"
   XF86AudioRaiseVolume

"python /path/to/cvolctrl/cvolctrl -10 0"
   XF86AudioLowerVolume

"python /path/to/cvolctrl/cvolctrl 0 1"
   XF86AudioMute
```



## Dependencies

cvolctrl requires the following libraries:

- dbus
- pulseaudio

