# cvolctrl

Simple volume notifier written in C

![](resources/preview.gif)

<br>

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
"/path/to/cvolctrl/cvolctrl 10 0"
   XF86AudioRaiseVolume

"/path/to/cvolctrl/cvolctrl -10 0"
   XF86AudioLowerVolume

"/path/to/cvolctrl/cvolctrl 0 1"
   XF86AudioMute
```

<br>

## Configuration

Battnotif only creates text which is shown in notification, the overall look of the notification
is configured from your notification daemon eg. dunst. Notifications consists of icon, summary (title)
and body. Battnotif only chooses icon from a list, generates summary and sends notification.

There are currently no command line arguments to change the look of the notification, all configuration
needs to be done in the source code before compiling. Below are shown default configuration and their
explanations.

<br>

###### Notification icons

```
static char* icons[] = { "volume_mute", "volume_down", "volume_up", "volume_off" }; // Last icon is used when muted
```

Icons are list of strings which holds the names of the icons. Icons are fetched recursively by your
notification daemon from `/usr/share/icons`. Meaning that icons can be inside folders, as long as they are
under mentioned path. Icon names must be listed without file extensions or their paths.

Icons must be in ascending order, where low volume icon is first. Last icon is used when volume is set to mute.

<br>

###### Notification timeout

```
static int timeout = 3 * 1000;
```

Timeout tells how long the notification is shown in milliseconds.

<br>

###### Bar

```
#define WIDTH 20
static char* chBar[3]= { "█", "░", "_" }; // Full, Mute, Empty 
```

Width macro tells how many characters wide the bar is.

Bar is generated using three different characters. Bar is filled by the volume percentage using the first
character, and remaining width is filled with third character. If volume is muted, then the bar is filled
using the second character, remaining is still filled with the third character.


## Dependencies

cvolctrl requires the following libraries:

- dbus
- pulseaudio

