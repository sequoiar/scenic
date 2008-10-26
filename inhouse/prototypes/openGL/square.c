#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <assert.h>

static gboolean delete_event(GtkWidget *widget,
                             GdkEvent *event,
                             gpointer data)
{
    g_print("Delete event occured\n");

    return FALSE;
}

static gboolean exposeCb(GtkWidget *drawingArea, GdkEventExpose *event,
        gpointer userData)
{
    GdkGLContext *glContext = gtk_widget_get_gl_context(drawingArea);
    GdkGLDrawable *glDrawable = gtk_widget_get_gl_drawable(drawingArea);

    if (!gdk_gl_drawable_gl_begin(glDrawable, glContext))
            g_assert_not_reached();

    // draw in here
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glBegin(GL_POLYGON);
        glVertex3f(0.25, 0.25, 0.0);
        glVertex3f(0.75, 0.25, 0.0);
        glVertex3f(0.75, 0.75, 0.0);
        glVertex3f(0.25, 0.75, 0.0);
    glEnd();
    
    glPopMatrix();

    if (gdk_gl_drawable_is_double_buffered(glDrawable))
        gdk_gl_drawable_swap_buffers(glDrawable);
    else
        glFlush();

    gdk_gl_drawable_gl_end (glDrawable);

    return TRUE;
}


static gboolean
idleCb(gpointer userData)
{
    // update control data/params here
    GtkWidget *drawingArea = GTK_WIDGET(userData);

    gdk_window_invalidate_rect (drawingArea->window, 
            &drawingArea->allocation, FALSE);
    gdk_window_process_updates (drawingArea->window, FALSE);

    return TRUE;
}

static gboolean 
configureCb(GtkWidget *drawingArea, GdkEventConfigure *event, gpointer 
        user_data)
{
    GdkGLContext *glContext = gtk_widget_get_gl_context(drawingArea);
    GdkGLDrawable *glDrawable = gtk_widget_get_gl_drawable(drawingArea);

    if (!gdk_gl_drawable_gl_begin(glDrawable, glContext))
        g_assert_not_reached();

    glLoadIdentity();
    glViewport(0, 0, drawingArea->allocation.width, 
            drawingArea->allocation.height);

    gdk_gl_drawable_gl_end(glDrawable);

    return TRUE;
}


static void startEventLoop()
{
    gtk_main();
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *drawingArea;
    GdkGLConfig *glconfig;

    gtk_init(&argc, &argv);
    gtk_gl_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    drawingArea = gtk_drawing_area_new();

    // put drawing area in our window
    gtk_container_add(GTK_CONTAINER(window), drawingArea);

    // register window callbacks
    g_signal_connect_swapped(window, "destroy", 
            G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "delete_event", 
            G_CALLBACK(delete_event), NULL);
    
    gtk_widget_set_events(drawingArea, GDK_EXPOSURE_MASK);

    //gtk_widget_show(window);

    // prepare GL
    glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA |
                                         GDK_GL_MODE_DEPTH |
                                         GDK_GL_MODE_DOUBLE);
    if (!glconfig)
        g_assert_not_reached();

    if (!gtk_widget_set_gl_capability(drawingArea, glconfig, NULL, TRUE,
                GDK_GL_RGBA_TYPE))
        g_assert_not_reached();

    g_signal_connect(drawingArea, "configure-event", G_CALLBACK(configureCb),
            NULL);
    g_signal_connect(drawingArea, "expose-event", G_CALLBACK(exposeCb), NULL);

    gtk_widget_show_all(window);
    
    g_timeout_add (1000 / 60, idleCb, drawingArea);

    startEventLoop();

    return 0;
}

