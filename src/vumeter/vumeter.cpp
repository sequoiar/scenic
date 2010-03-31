/* vumeter.c */

#include "vumeter.h"
#include <cmath>
#include <string>

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
#if 0
static void
init_gradient ()
{
  gint i, j;
  for (i = 0, j = GRADIENT_SIZE - 1; i < GRADIENT_SIZE; ++i, --j) {
    double val = pow (j / (double) GRADIENT_SIZE, M_E) * 120;
    RGB rgb = HSVtoRGB (val, 1.0, 1.0);
    GRADIENT[j].r = rgb.r;
    GRADIENT[j].g = rgb.g;
    GRADIENT[j].b = rgb.b;
  }
}
#endif

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

  requisition->width = 46;
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
  attributes.width = 46;
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

static void
gtk_vumeter_paint (GtkWidget * widget)
{
  cairo_t *cr;
  //const gint HIGHLIGHTED_BARS = GTK_VUMETER (widget)->sel * GRADIENT_SIZE;
  gint i;

  cr = gdk_cairo_create (widget->window);

  cairo_translate (cr, 0, 7);

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_paint (cr);

  cairo_set_source_rgb (cr, 0.2, 0.4, 0);

  static const gdouble HEIGHT = 256.0;
  cairo_pattern_t *gradient = cairo_pattern_create_linear(0.0, 0.0, 0.0, HEIGHT);
  for (i = 0; i < GRADIENT_SIZE; ++i)
  {
      // 0.95 factor makes it more green overall
      gdouble offset = 1.0 - (i  / (gdouble)GRADIENT_SIZE);
      cairo_pattern_add_color_stop_rgba(gradient, offset, GRADIENT[i].r, GRADIENT[i].g, GRADIENT[i].b, 1);
  }
  cairo_rectangle(cr, 0, HEIGHT * (1.0 - GTK_VUMETER(widget)->sel), HEIGHT, HEIGHT);
  cairo_set_source(cr, gradient);
  cairo_fill(cr);
  cairo_pattern_destroy(gradient);

#if 0
  for (i = 0; i < GRADIENT_SIZE; i++) {
      if (i < HIGHLIGHTED_BARS)       /* light up */
          cairo_set_source_rgb (cr, GRADIENT[i].r, GRADIENT[i].g, GRADIENT[i].b);
      else                        /* don't light up */
          cairo_set_source_rgb (cr, 0.2, 0.4, 0);
      cairo_rectangle (cr, 8, (i + 1) * 4, 30, 3);
      cairo_fill (cr);
  }
#endif

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
