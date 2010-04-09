/* vumeter.c */

#include "vumeter.h"
#include <cmath>
#include <cstdlib> // for abs
#include <string>
#include <cstring>

static void gtk_vumeter_class_init (GtkVumeterClass * klass);
static void gtk_vumeter_init (GtkVumeter * vumeter);
static void gtk_vumeter_size_request (GtkWidget * widget,
    GtkRequisition * requisition);
static void gtk_vumeter_size_allocate (GtkWidget * widget,
    GtkAllocation * allocation);
static void gtk_vumeter_realize (GtkWidget * widget);
static gboolean gtk_vumeter_expose (GtkWidget * widget, GdkEventExpose * event);
static void gtk_vumeter_paint (GtkWidget * widget, cairo_t *cr); 
static void gtk_vumeter_destroy (GtkObject * object);

GtkType
gtk_vumeter_get_type ()
{
  static GtkType gtk_vumeter_type = 0;
  gchar name[] = "GtkVumeter";

  if (!gtk_vumeter_type) {
    static const GtkTypeInfo gtk_vumeter_info = {
      name,
      sizeof (GtkVumeter),
      sizeof (GtkVumeterClass),
      (GtkClassInitFunc) gtk_vumeter_class_init,
      (GtkObjectInitFunc) gtk_vumeter_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };
    gtk_vumeter_type = gtk_type_unique (GTK_TYPE_WIDGET, &gtk_vumeter_info);
  }

  return gtk_vumeter_type;
}

void
gtk_vumeter_set_peaks (GtkVumeter * vumeter, gdouble peak, gdouble decay_peak)
{
  vumeter->peak = peak;
  vumeter->decay_peak = decay_peak;
}

GtkWidget *
gtk_vumeter_new ()
{
  return GTK_WIDGET (gtk_type_new (gtk_vumeter_get_type ()));
}


static void
gtk_vumeter_class_init (GtkVumeterClass * klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;

  widget_class = (GtkWidgetClass *) klass;
  object_class = (GtkObjectClass *) klass;

  widget_class->realize = gtk_vumeter_realize;
  widget_class->size_request = gtk_vumeter_size_request;
  widget_class->size_allocate = gtk_vumeter_size_allocate;
  widget_class->expose_event = gtk_vumeter_expose;

  object_class->destroy = gtk_vumeter_destroy;
}

static void
gtk_vumeter_init (GtkVumeter * vumeter)
{
  vumeter->peak = -70.0;
  vumeter->decay_peak = -70.0;
}

static void
gtk_vumeter_size_request (GtkWidget * widget, GtkRequisition * requisition)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_VUMETER (widget));
  g_return_if_fail (requisition != NULL);

  requisition->width = 30;
  requisition->height = 100;
}

static void
gtk_vumeter_size_allocate (GtkWidget * widget, GtkAllocation * allocation)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_VUMETER (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window,
        allocation->x, allocation->y, allocation->width, allocation->height);
  }
}

static void
gtk_vumeter_realize (GtkWidget * widget)
{
  GdkWindowAttr attributes;
  guint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_VUMETER (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
      &attributes, attributes_mask);

  gdk_window_set_user_data (widget->window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static gboolean
gtk_vumeter_expose (GtkWidget * widget, GdkEventExpose * event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_VUMETER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  /* set a clip region for the expose event */
  cairo_t *cr = gdk_cairo_create(widget->window);
  cairo_rectangle (cr, event->area.x, event->area.y, event->area.width, event->area.height);
  cairo_clip(cr);

  gtk_vumeter_paint (widget, cr);

  return FALSE;
}

static gdouble db_to_vertical_offset(GtkWidget *widget, gdouble db)
{
    gfloat def = 0.0f; /* Meter deflection %age */
 
    if (db < -70.0f) {
        def = 0.0f;
    } else if (db < -60.0f) {
        def = (db + 70.0f) * 0.25f;
    } else if (db < -50.0f) {
        def = (db + 60.0f) * 0.5f + 2.5f;
    } else if (db < -40.0f) {
        def = (db + 50.0f) * 0.75f + 7.5f;
    } else if (db < -30.0f) {
        def = (db + 40.0f) * 1.5f + 15.0f;
    } else if (db < -20.0f) {
        def = (db + 30.0f) * 2.0f + 30.0f;
    } else if (db < 6.0f) {
        def = (db + 20.0f) * 2.5f + 50.0f;
    } else {
        def = 115.0f;
    }

    /* 115 is the deflection %age that would be 
       when db=6.0. this is an arbitrary
	    endpoint for our scaling.
     */

    return widget->allocation.height - floor(widget->allocation.height * (def / 115.0f));
}

static void
gtk_vumeter_paint (GtkWidget * widget, cairo_t *cr)
{
	static const int db_points[] = { -50, -40, -30, -24, -18, -12, -6, -3, 
        0, 4 };

    cairo_paint (cr);
    static const gdouble RECT_WIDTH = 10;

    // Write numbers
    cairo_text_extents_t te;
    cairo_select_font_face (cr, "Sans",
            CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 7);
    cairo_set_source_rgb (cr, 0.80, 0.80, 0.80); // text colour
	char buf[32];
    gdouble max_text_width = 0;
    for (size_t i = 0; i < sizeof(db_points) / sizeof(db_points[0]); ++i)
    {
        snprintf(buf, sizeof(buf), "%d", db_points[i]);
        cairo_text_extents (cr, buf, &te);
        max_text_width = std::max(te.width, max_text_width);
        gdouble vertical_offset = db_to_vertical_offset(widget, db_points[i]);

        cairo_move_to (cr, widget->allocation.width -  te.width - te.x_bearing,
            vertical_offset - (te.height / 2) - te.y_bearing);
        cairo_show_text (cr, buf);
    }

    // draw dashes
    const int DASH_SIZE = 4;
    cairo_set_line_width(cr, 1);
    for (int i = 4; i >= -50;)
    {
        gdouble x = RECT_WIDTH;
        gdouble y = db_to_vertical_offset(widget, i) + 0.5;
        cairo_move_to (cr, x + DASH_SIZE, y);
        cairo_line_to (cr, x, y);
        cairo_stroke (cr);
        i = i > -10 ? i - 2 : i - 4;
    }

    // draw rectangle
    // green
    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    const gdouble green_rect_height = db_to_vertical_offset(widget, -18.0);
    cairo_rectangle(cr, 0, green_rect_height /* top */,
            RECT_WIDTH, 
            widget->allocation.height - green_rect_height/* bottom */);
    cairo_fill(cr);

    // yellow 
    cairo_set_source_rgb (cr, 0.8, 1.0, 0.0);
    const gdouble yellow_rect_height = db_to_vertical_offset(widget, 0.0);
    cairo_rectangle(cr, 0, yellow_rect_height /* top */,
            RECT_WIDTH,
            green_rect_height - yellow_rect_height/* bottom */);
    cairo_fill(cr);
    
    // red 
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    const gdouble red_rect_height = db_to_vertical_offset(widget, 8.0);
    cairo_rectangle(cr, 0, red_rect_height /* top */,
            RECT_WIDTH,
            yellow_rect_height - red_rect_height/* bottom */);
    cairo_fill(cr);
    
    // Draw a black rectangle that covers everything but the part of the
    // vumeter corresponding to the current amplitude
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, /* top */
            RECT_WIDTH,
            db_to_vertical_offset(widget, GTK_VUMETER(widget)->peak));
    cairo_fill(cr);

    // Draw decay peak
    cairo_set_line_width(cr, 2.0);
    gdouble decay_peak_height =  db_to_vertical_offset(widget, GTK_VUMETER(widget)->decay_peak);
    if (GTK_VUMETER(widget)->decay_peak > 0.0)
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    else
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_move_to(cr, 0, decay_peak_height);
    cairo_line_to(cr, RECT_WIDTH, decay_peak_height);
    cairo_stroke(cr);

    cairo_destroy (cr);
}

    static void
gtk_vumeter_destroy (GtkObject * object)
{
    GtkVumeter *vumeter;
    GtkVumeterClass *klass;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_VUMETER (object));

    vumeter = GTK_VUMETER (object);

    klass = static_cast<GtkVumeterClass*>(gtk_type_class (gtk_widget_get_type ()));

    if (GTK_OBJECT_CLASS (klass)->destroy) {
        (*GTK_OBJECT_CLASS (klass)->destroy) (object);
    }
}
