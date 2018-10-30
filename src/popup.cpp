#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "popup.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *box;
	Evas_Object *popup;
	Evas_Object *entry;
	int popupNum;
} appdata_s;



std::string GetStdoutFromCommand(std::string cmd) {
    std::string data;
    FILE *stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream)) {
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        }
        pclose(stream);
    }
    return data;
}



static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}



static void
popup_close(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}


static void
make_popup_text(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	ad->popup = elm_popup_add(ad->win);
	elm_popup_align_set(ad->popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(ad->popup, GetStdoutFromCommand("ls -al").c_str());
	//*get output from tensorflow app

	elm_popup_timeout_set(ad->popup, 3.0);
	evas_object_smart_callback_add(ad->popup, "timeout", popup_close, ad);
	evas_object_smart_callback_add(ad->popup, "block,clicked", popup_close, ad);
	evas_object_show(ad->popup);
	ad->popupNum = 1;
}

static void
make_popup_input_text (void *data, Evas_Object *obj, void *event_info)
{
}



static void
my_box_pack(Evas_Object *box, Evas_Object *child,
 double h_weight, double v_weight, double h_align, double v_align)
{
    /* create a frame we shall use as padding around the child widget */
    Evas_Object *frame = (Evas_Object*)elm_frame_add(box);

    /* use the medium padding style. there is "pad_small", "pad_medium",
    * "pad_large" and "pad_huge" available as styles in addition to the
    * "default" frame style */
    elm_object_style_set (frame, "pad_medium");

    /* set the input weight/aling on the frame insted of the child */
    evas_object_size_hint_weight_set(frame, h_weight, v_weight);
    evas_object_size_hint_align_set(frame, h_align, v_align);
    {
        evas_object_size_hint_weight_set(child, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        /* tell the child that is packed into the frame to be able to expand */
        /* fill the expanded area (above) as opposaed to center in it */

        evas_object_size_hint_align_set(child, EVAS_HINT_FILL, EVAS_HINT_FILL);
        /* actually put the child in the frame and show it */
        evas_object_show(child);
        elm_object_content_set(frame, child);
    }
    /* put the frame into the box instead of the child directly */
    elm_box_pack_end(box, frame);
    /* show the frame */
    evas_object_show(frame);
}



static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);



	{
		/* child object - indent to how relationship */
		/* A box to put things in vertically - default mode for box */
		ad->box = elm_box_add(ad->win);
		evas_object_size_hint_weight_set(ad->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_content_set(ad->conform, ad->box);
		evas_object_show(ad->box);
		{
			/* Label*/
			ad->label = elm_label_add(ad->conform);
			elm_object_text_set(ad->label, "Network Diagnosis");
			my_box_pack(ad->box, ad->label, 1.0, 0.0, 0.5, 0.0);

			/* Button-1 */
			Evas_Object *btn = elm_button_add(ad->conform);
			elm_object_text_set(btn, "Diagnosis Result");
			evas_object_smart_callback_add(btn, "clicked", make_popup_text, ad);
			my_box_pack(ad->box, btn, 1.0, 0.0, -1.0, -1.0);

			/* Button-4 */
			btn = elm_button_add(ad->conform);
			elm_object_text_set(btn, "Other Functionality");
			evas_object_smart_callback_add(btn, "clicked", make_popup_input_text, ad);
			my_box_pack(ad->box, btn, 1.0, 1.0, -1.0, 0.0);

			/* Note: this last button has weight 1 and align 0 so that the whole UI is
			 * nicely and tightly packed at the top of the window.
			 */
		}
	}

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}




static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = (appdata_s*)data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/

	int ret;
	char *language;

	ret = app_event_get_language(event_info, &language);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_event_get_language() failed. Err = %d.", ret);
		return;
	}

	if (language != NULL) {
		elm_language_set(language);
		free(language);
	}
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
