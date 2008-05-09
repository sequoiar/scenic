/*
 * Copyright (c) 2006 INdT.
 * @author Talita Menezes <talita.menezes@indt.org.br>
 * @author Cidorvan Leite <cidorvan.leite@indt.org.br>
 *
 */


#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <hildon-widgets/hildon-banner.h>
#include <hildon-widgets/hildon-program.h>
#include <jpeglib.h>
#include <libgnomevfs/gnome-vfs.h>


#define SAVE_FOLDER_DEFAULT  	 ".images/"
#define PHOTO_NAME_DEFAULT	 "Picture"
#define PHOTO_NAME_SUFIX_DEFAULT ".jpg"

static void create_jpeg(unsigned char *buffer);
static void take_photo(GtkWidget * widget, gpointer data);
static gboolean cb_have_data(GstPad * pad, GstBuffer * buffer,
			     gpointer u_data);
static gboolean expose_cb(GtkWidget * widget, GdkEventExpose * event,
			  gpointer data);
static void show_note(char *note);

int picture_requested = 0;
static /*HILDON_WINDOW */ GtkWidget *window;

static gboolean
cb_have_data(GstPad * pad, GstBuffer * buffer, gpointer u_data)
{
	unsigned char *data_photo =
	    (unsigned char *) GST_BUFFER_DATA(buffer);
	if (picture_requested) {
		picture_requested = 0;
		create_jpeg(data_photo);
	}
	return TRUE;
}

static void take_photo(GtkWidget * widget, gpointer buffer)
{
	picture_requested = 1;
	//show_note("Photo Saved");
}


static gboolean
expose_cb(GtkWidget * widget, GdkEventExpose * event, gpointer data)
{

	gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data),
				     GDK_WINDOW_XWINDOW(widget->window));
	return FALSE;
}

int main(int argc, char **argv)
{

	HildonProgram *program;
	GtkWidget *screen, *button, *hbox, *vbox_button, *vbox;

	GstElement *pipeline, *src, *sink, *csp, *csp2, *flt;
	GstPad *pad;
	GstCaps *caps;
	gboolean link_ok;

	gtk_init(&argc, &argv);
	gst_init(&argc, &argv);
	gnome_vfs_init();

	program = HILDON_PROGRAM(hildon_program_get_instance());
	g_set_application_name("Test camera");

	/* Create HildonWindow and set it to HildonProgram */
	window = /*HILDON_WINDOW */ GTK_WIDGET(hildon_window_new());
	hildon_program_add_window(program, HILDON_WINDOW(window));

	/* Connect signal to X in the upper corner */
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(gtk_main_quit), NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	vbox_button = gtk_vbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox_button, FALSE, FALSE, 0);

	screen = gtk_drawing_area_new();
	gtk_widget_set_size_request(screen, 500, 380);
	gtk_box_pack_start(GTK_BOX(vbox), screen, FALSE, FALSE, 0);

	button = gtk_button_new_with_label("Take photo");
	gtk_widget_set_size_request(button, 170, 380);
	gtk_box_pack_start(GTK_BOX(vbox_button), button, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(take_photo), NULL);
	gtk_container_add(GTK_CONTAINER(window), hbox);

	/* Begin the main application */
	gtk_widget_show_all(GTK_WIDGET(window));

	// Pipeline elements
	pipeline = gst_pipeline_new("test-camera");
	// The camera
	src = gst_element_factory_make("v4l2src", "src");
	// Filter
	flt = gst_element_factory_make("capsfilter", "flt");
	// Filter
	csp = gst_element_factory_make("ffmpegcolorspace", "csp");
	// Filter
	csp2 = gst_element_factory_make("ffmpegcolorspace", "csp2");
	// The screen sink
	sink = gst_element_factory_make("xvimagesink", "sink");
	gst_bin_add_many(GST_BIN(pipeline), src, flt, csp, csp2, sink,
			 NULL);

	// Links Camera -> First filter (flt) -> Second filter (csp)
	gst_element_link_many(src, flt, csp, NULL);

	// As soon as screen is exposed, window ID will be advised to the sink
	g_signal_connect(screen, "expose-event", G_CALLBACK(expose_cb),
			 sink);

	// caps between csp and csp2, links Second Filter (csp) -> Third Filter (csp2)
	// with a caps between them

	caps = gst_caps_new_simple("video/x-raw-rgb",
				   "width", G_TYPE_INT, 640,
				   "height", G_TYPE_INT, 480,
				   "framerate", GST_TYPE_FRACTION, 15, 1,
				   "bpp", G_TYPE_INT, 24,
				   "depth", G_TYPE_INT, 24, NULL);

	link_ok = gst_element_link_filtered(csp, csp2, caps);

	if (!link_ok) {
		g_warning("Failed to link elements !");
	}
	gst_caps_unref(caps);

	// caps between csp2 and sink (the screen), links Third Filter (csp2) with screen sink
	// with a caps between them

	// After that, all GStreamer elements have been connected and the pipeline is done

	pad = gst_element_get_pad(GST_ELEMENT(csp), "src");
	caps = gst_caps_new_simple("video/x-raw-yuv", NULL);

	link_ok = gst_element_link_filtered(csp2, sink, caps);

	if (!link_ok) {
		g_warning("Failed to link elements !");
	}
	gst_caps_unref(caps);

	// callback whenever there is data (will read image when photo is requested) 
	gst_pad_add_buffer_probe(pad, G_CALLBACK(cb_have_data), NULL);
	gst_object_unref(pad);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	gtk_main();

	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);

	return 0;
}

static void create_jpeg(unsigned char *data)
{
	/* Boilerplate function that gets a framebuffer and writes a JPEG file */

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, width, height;
	unsigned char *line;
	int byte_pixel;
	FILE *outfile;
	gchar *filename = NULL;
	char aux[256];
	GnomeVFSURI *test_uri = NULL;

	filename =
	    g_build_filename(getenv("MYDOCSDIR"), SAVE_FOLDER_DEFAULT,
			     PHOTO_NAME_DEFAULT, NULL);

	sprintf(aux, "%s%s", filename, PHOTO_NAME_SUFIX_DEFAULT);
	test_uri = gnome_vfs_uri_new(aux);
	i = 0;

	while (gnome_vfs_uri_exists(test_uri)) {
		i++;
		sprintf(aux, "%s%d%s", filename, i,
			PHOTO_NAME_SUFIX_DEFAULT);
		g_free(test_uri);
		test_uri = gnome_vfs_uri_new(aux);
	}

	filename = aux;
	g_free(test_uri);

	width = 640;
	height = 480;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	byte_pixel = 3;

	for (i = 0, line = data; i < height;
	     i++, line += width * byte_pixel) {
		jpeg_write_scanlines(&cinfo, &line, 1);
	}
	jpeg_finish_compress(&(cinfo));
	fclose(outfile);
	jpeg_destroy_compress(&(cinfo));
	//show_note("Photo Saved");
}

static void show_note(char *note)
{

	g_assert(window != NULL);
	hildon_banner_show_information(GTK_WIDGET(window), NULL, note);
}
