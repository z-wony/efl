#include "evas_common_private.h"
#include "evas_private.h"

static const Eo_Event_Description *_evas_clickable_interface_event_desc[];
static const Eo_Event_Description *_evas_draggable_interface_event_desc[];
static const Eo_Event_Description *_evas_scrollable_interface_event_desc[];
static const Eo_Event_Description *_evas_selectable_interface_event_desc[];
static const Eo_Event_Description *_evas_zoomable_interface_event_desc[];

#include "canvas/evas_signal_interface.eo.c"
#include "canvas/evas_draggable_interface.eo.c"
#include "canvas/evas_clickable_interface.eo.c"
#include "canvas/evas_scrollable_interface.eo.c"
#include "canvas/evas_selectable_interface.eo.c"
#include "canvas/evas_zoomable_interface.eo.c"
