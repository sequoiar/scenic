/* vumeter.h */

#ifndef _VUMETER_H_
#define _VUMETER_H_

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS
#define GTK_VUMETER(obj) GTK_CHECK_CAST(obj, gtk_vumeter_get_type (), GtkVumeter)
#define GTK_VUMETER_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_vumeter_get_type(), GtkVumeterClass)
#define GTK_IS_VUMETER(obj) GTK_CHECK_TYPE(obj, gtk_vumeter_get_type())
typedef struct _GtkVumeter GtkVumeter;
typedef struct _GtkVumeterClass GtkVumeterClass;

struct _GtkVumeter
{
  GtkWidget widget;

  gdouble peak;
  gdouble decay_peak;
};

struct _GtkVumeterClass
{
  GtkWidgetClass parent_class;
};

GtkType gtk_vumeter_get_type (void);
void gtk_vumeter_set_peaks (GtkVumeter * vumeter, gdouble peak, gdouble decay_peak);
GtkWidget *gtk_vumeter_new ();

G_END_DECLS
#endif /* _VUMETER_H */
