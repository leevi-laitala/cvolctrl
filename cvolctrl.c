#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pulse/pulseaudio.h>
#include <dbus-1.0/dbus/dbus.h>

/* Notification settings */
enum { LOW, NORMAL, CRITICAL };
static unsigned char level = LOW;
static char* application = "cvolctrl";
static char* urgency = "urgency";
static char* icons[] = { "volume_mute", "volume_down", "volume_up", "volume_off" }; // Last icon is used when muted
static int timeout = 3 * 1000;
static unsigned id = 2593;

/* Bar settings */
#define WIDTH 20
char bar[WIDTH * 4]; // Extra room for special characters
static char* chBar[3]= { "█", "░", "_" }; // Full, Mute, Empty 

/* Pulseaudio globals */
pa_mainloop* mainloop;
struct pa_cvolume volume;
int muted;
int vol;

/* Cmd args */
int volDiff;
int toggleMute;



/* Notification functions */

const char* genbar() {
    sprintf(bar,"%d", vol);
    strcat(bar, "%   ");

    for (unsigned i = 0; i < WIDTH; i++) 
        strcat(bar, (i < (unsigned)(vol * WIDTH / 100)) ? ((muted) ? chBar[1] : chBar[0]) : chBar[2]);

    return bar;
}

const char* icon() {
    int arrlen = sizeof(icons) / sizeof(char*) - 1;
    int clamped = (int)(round(vol - 2) / (100 / arrlen));

    return icons[muted ? arrlen : clamped];
}

void notify(const char* summary, const char* body, const char* icon) {
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, 0);
	DBusMessage* message = dbus_message_new_method_call("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
	DBusMessageIter iter[4];
	dbus_message_iter_init_append(message, iter);

	dbus_message_iter_append_basic(iter, 's', &application);
	dbus_message_iter_append_basic(iter, 'u', &id);
	dbus_message_iter_append_basic(iter, 's', &icon);
	dbus_message_iter_append_basic(iter, 's', &summary);
    
	dbus_message_iter_append_basic(iter, 's', &body);
	dbus_message_iter_open_container(iter, 'a', "s", iter + 1);
	dbus_message_iter_close_container(iter, iter + 1);
	dbus_message_iter_open_container(iter, 'a', "{sv}", iter + 1);
	dbus_message_iter_open_container(iter + 1, 'e', 0, iter + 2);

	dbus_message_iter_append_basic(iter + 2, 's', &urgency);
	dbus_message_iter_open_container(iter + 2, 'v', "y", iter + 3);
	dbus_message_iter_append_basic(iter + 3, 'y', &level);
	dbus_message_iter_close_container(iter + 2, iter + 3);
	dbus_message_iter_close_container(iter + 1, iter + 2);
	dbus_message_iter_close_container(iter, iter + 1);

	dbus_message_iter_append_basic(iter, 'i', &timeout);
	dbus_connection_send(connection, message, 0);
	dbus_connection_flush(connection);
	dbus_message_unref(message);
	dbus_connection_unref(connection);
}



/* Pulseaudio functions */

void successCallback(pa_context* ctxt, int success, void* userdata) {
	pa_mainloop_quit(mainloop, 0);
}

void applyVolume(pa_context* ctxt, const pa_sink_info* i, int eol, void* userdata) {
    if (i) {
        pa_volume_t new_volume = pa_cvolume_avg(&(i->volume));
        float new_volume_level = (float)new_volume / (float)PA_VOLUME_NORM * 100.0f;
        vol = round(new_volume_level) + volDiff;
        vol = (vol > 100) ? 100 : ((vol < 0) ? 0 : vol); // Clamp between 0 and 100

        muted = i->mute ^ toggleMute;

	    volume.channels = 2;
	    volume.values[0] = volume.values[1] = (vol * PA_VOLUME_NORM) / 100;

        pa_context_set_sink_mute_by_index(ctxt, i->index, muted, successCallback, NULL);
    	pa_context_set_sink_volume_by_index(ctxt, i->index, &volume, successCallback, NULL);
    }
}

void stateCallback(pa_context* ctxt, void* userdata) {
    switch (pa_context_get_state(ctxt))
    {
		case PA_CONTEXT_READY:
			pa_context_get_sink_info_list(ctxt, applyVolume, NULL);
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		case PA_CONTEXT_TERMINATED:
		case PA_CONTEXT_FAILED:
		default:
            break;
    }
}



/* Main and IO */

int fail(const char* msg) {
    printf(msg);
    printf("\n\nUsage: cvolctrl <volume> <togglemute>\n\nVolume is signed integer, which is added to current volume level.\nTogglemute is enabled if set to 1 and disabled if set to 0.\n\nExamples:\n cvolctrl 5 0 -> increases volume by 5\n cvolctrl -10 1 -> decreases volume by 10 and toggles mute\n");
}

int main(int argc, const char* argv[]) {
    if (argc != 3)
        return fail("Invalid amount of arguments");

    if (sscanf(argv[1], "%i", &volDiff) != 1)
        return fail("Couldn't parse volume. Has to be a signed integer.");
    
    if (sscanf(argv[2], "%i", &toggleMute) != 1)
        return fail("Invalid mute argument. Has to be 1 or 0 (toggle / ignore)");

    mainloop = pa_mainloop_new();
	pa_context *context = pa_context_new(pa_mainloop_get_api(mainloop), NULL);
	pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL);

    int ret;
	pa_context_set_state_callback(context, stateCallback, NULL);
	pa_mainloop_run(mainloop, &ret);
	pa_context_unref(context);
	pa_mainloop_free(mainloop);
    
    notify(genbar(), "", icon());

	return ret;
}
