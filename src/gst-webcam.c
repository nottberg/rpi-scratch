#include <gst/gst.h>
#include <glib.h>


static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}

// gst-launch-0.10 v4l2src device=/dev/video0 ! image/jpeg,width=640,height=480,framerate=30/1,rate=30 ! ffdec_mjpeg !
// ffmpegcolorspace ! fbdevsink

static gboolean
link_elements_with_filter (GstElement *element1, GstElement *element2)
{
  gboolean link_ok;
  GstCaps *caps;

  caps = gst_caps_new_simple ("image/jpeg",
	      "width", G_TYPE_INT, 640,
	      "height", G_TYPE_INT, 480,
	      "framerate", GST_TYPE_FRACTION, 30, 1,
          "rate", G_TYPE_INT, 30,
	      NULL);

  link_ok = gst_element_link_filtered (element1, element2, caps);
  gst_caps_unref (caps);

  if (!link_ok) {
    g_warning ("Failed to link element1 and element2!");
  }

  return link_ok;
}

int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Check input arguments */
  //if (argc != 2) {
  //  g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
  //  return -1;
  //}


  //gst-launch v4l2src ! 'video/x-raw-yuv,width=640,height=480,framerate=30/1' ! \
  //queue ! videorate ! 'video/x-raw-yuv,framerate=30/1' ! theoraenc ! \
  //queue ! oggmux ! filesink location=me_funny_dancing.ogg 

  //gst-launch v4l2src device=/dev/video0 ! \
  //'video/x-raw-yuv,width=640,height=480,framerate=30/1' ! \
  //xvimagesink

  // gst-launch-0.10 v4l2src device=/dev/video0 ! image/jpeg,width=640,height=480,framerate=30/1,rate=30 ! ffdec_mjpeg !
  // ffmpegcolorspace ! fbdevsink

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("webcam");
  source   = gst_element_factory_make ("v4l2src",          "video-source");
  decoder  = gst_element_factory_make ("ffdec_mjpeg",      "decoder");
  conv     = gst_element_factory_make ("ffmpegcolorspace", "color-convert");
  sink     = gst_element_factory_make ("fbdevsink",  "display");

  if (!pipeline || !source || !decoder || !conv || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "device", "/dev/video0", NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* video-source | decoder | converter | fboutput */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, decoder, conv, sink, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  //gst_element_link (source, decoder);
  link_elements_with_filter( source, decoder );
  gst_element_link_many( decoder, conv, sink, NULL );
  //g_signal_connect (decoder, "pad-added", G_CALLBACK (on_pad_added), decoder);

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}

