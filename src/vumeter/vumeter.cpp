/* vumeter.c */

#include "vumeter.h"
#include <cmath>
#include <cstdlib> // for abs
#include <string>
#include <cstring>

static const int GRADIENT_SIZE = 20;

static void gtk_vumeter_class_init (GtkVumeterClass * klass);
static void gtk_vumeter_init (GtkVumeter * vumeter);
static void gtk_vumeter_size_request (GtkWidget * widget,
    GtkRequisition * requisition);
static void gtk_vumeter_size_allocate (GtkWidget * widget,
    GtkAllocation * allocation);
static void gtk_vumeter_realize (GtkWidget * widget);
static gboolean gtk_vumeter_expose (GtkWidget * widget, GdkEventExpose * event);
static void gtk_vumeter_paint (GtkWidget * widget);
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
gtk_vumeter_set_state (GtkVumeter * vumeter, gint num)
{
  vumeter->sel = num;
  gtk_vumeter_paint (GTK_WIDGET (vumeter));
}

GtkWidget *
gtk_vumeter_new ()
{
  return GTK_WIDGET (gtk_type_new (gtk_vumeter_get_type ()));
}

typedef struct RGB
{
  gdouble r;
  gdouble b;
  gdouble g;
} RGB;

static RGB GRADIENT[GRADIENT_SIZE];

static RGB
HSVtoRGB (gdouble h, gdouble s, gdouble v)
{
  RGB result;
  gint i;
  gdouble f, p, q, t;

  if (s == 0) {
    /* achromatic (grey) */
    result.r = result.g = result.b = v;
    return result;
  }

  h /= 60;                      /* sector 0 to 5 */
  i = floor (h);
  f = h - i;                    /* factorial part of h */
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));

  switch (i) {
    case 0:
      result.r = v;
      result.g = t;
      result.b = p;
      break;
    case 1:
      result.r = q;
      result.g = v;
      result.b = p;
      break;
    case 2:
      result.r = p;
      result.g = v;
      result.b = t;
      break;
    case 3:
      result.r = p;
      result.g = q;
      result.b = v;
      break;
    case 4:
      result.r = t;
      result.g = p;
      result.b = v;
      break;
    case 5:
    default:
      result.r = v;
      result.g = p;
      result.b = q;
      break;
  }
  return result;
}

static void
init_gradient ()
{
  gint i, j;
  for (i = 0, j = GRADIENT_SIZE - 1; i < GRADIENT_SIZE; ++i, --j) {
    double val = ((i + 2) / (double) GRADIENT_SIZE) * 120.0;
    RGB rgb = HSVtoRGB (val, 1.0, 1.0);
    GRADIENT[j].r = rgb.r;
    GRADIENT[j].g = rgb.g;
    GRADIENT[j].b = rgb.b;
  }
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
  init_gradient ();
}

static void
gtk_vumeter_init (GtkVumeter * vumeter)
{
  vumeter->sel = 0.0;
}

static void
gtk_vumeter_size_request (GtkWidget * widget, GtkRequisition * requisition)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_VUMETER (widget));
  g_return_if_fail (requisition != NULL);

  requisition->width = 5;
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
  attributes.width = 5;
  attributes.height = 100;

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

  gtk_vumeter_paint (widget);

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

    return widget->allocation.height - floor (widget->allocation.height * (def / 115.0f));
}

static void
gtk_vumeter_paint (GtkWidget * widget)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
	static const int db_points[] = { -50, -40, -20, -30, -10, -3, 0, 4 };

    cairo_paint (cr);
    cairo_pattern_t *gradient = cairo_pattern_create_linear(0.0, 0.0, 0.0, widget->allocation.height);
    for (gint i = 0; i < GRADIENT_SIZE; ++i) 
    {
        gdouble offset = 1.0 - (i  / (gdouble)GRADIENT_SIZE);
        cairo_pattern_add_color_stop_rgba(gradient, offset, GRADIENT[i].r, GRADIENT[i].g, GRADIENT[i].b, 1);
    }

    cairo_text_extents_t te;
    cairo_select_font_face (cr, "Sans",
            CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 8);
    cairo_set_source_rgb (cr, 0.80, 0.80, 0.80); // text colour
	char buf[32];
    gdouble max_text_width = 0;
    for (size_t i = 0; i < sizeof(db_points) / sizeof(db_points[0]); ++i)
    {
        snprintf(buf, sizeof(buf), "-%d", std::abs(db_points[i]));
        cairo_text_extents (cr, buf, &te);
        max_text_width = std::max(te.width, max_text_width);

        gdouble vertical_offset = db_to_vertical_offset(widget, db_points[i]);

        cairo_move_to (cr, widget->allocation.width -  te.width - te.x_bearing,
            vertical_offset);
        cairo_show_text (cr, buf);
    }

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, db_to_vertical_offset(widget, GTK_VUMETER(widget)->sel), widget->allocation.width - max_text_width, widget->allocation.height);

    cairo_set_source(cr, gradient);
    cairo_pattern_destroy(gradient);
    cairo_fill(cr);

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
