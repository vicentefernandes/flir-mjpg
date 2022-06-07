//
// Created by vince on 21/04/20.
//

#include <gst/gst.h>
#include <iostream>
#include "SpinnakerHandler.h"

using namespace std;

static GMainLoop *loop;
SpinnakerHandler spin;

static void cb_test_ (GstElement *appsrc, guint unused_size, gpointer user_data)
{
    cout << "CB enough... " << endl;
    ///TODO release spinnaker data structures
}


static gboolean timeout(void *){
    cout << "Timeout writting to device. Please close all apps reading from /dev/video7." << endl << "Quiting . . ." << endl;
    g_main_loop_quit (loop);
}

static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;

    size = 2448*2048*3;


    ImagePtr spinImage = spin.acquireImage();

    void *data = spinImage->GetData();

    buffer = gst_buffer_new_wrapped(data,size);

    ///TODO release image and buffer ??

    GST_BUFFER_PTS (buffer) = timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, /*30*/ 15); ////we are just getting 8 fps from the cam. TODO: try to increase framerate. check if it is possible to reduce the acq. resolution.

    timestamp += GST_BUFFER_DURATION (buffer);

    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);


    ///gst_buffer_unref (buffer);


    if (ret != GST_FLOW_OK) {
        /* something wrong, stop pushing */
        cout << "!GST_FLOW_OK ..." << endl;
        g_main_loop_quit (loop);
    }


    static int first=1;
    static int timeout_id;
    if(!first){
        g_source_remove(timeout_id);
    }
    first=0;
    timeout_id = g_timeout_add(5000, timeout, NULL);

}





gint main(gint argc, gchar *argv[])
{

    GstElement *pipeline, *appsrc, *conv, *videosink;


    /* init GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* setup pipeline */
    pipeline = gst_pipeline_new ("pipeline");
    appsrc = gst_element_factory_make ("appsrc", "source");
    conv = gst_element_factory_make ("videoconvert", "conv");

    videosink = gst_element_factory_make("v4l2sink", "mySync");
    g_object_set (G_OBJECT (videosink), "device", "/dev/video7", NULL);

    cout << "start feedind to device: /dev/video7" << endl;


    /* setup */
    g_object_set (G_OBJECT (appsrc), "caps",
                  gst_caps_new_simple ("video/x-raw",
                                       "format", G_TYPE_STRING, "BGR",
                                       "width", G_TYPE_INT, 2448,
                                       "height", G_TYPE_INT, 2048,
                                       "framerate", GST_TYPE_FRACTION, 15, 1,
                                       NULL),
///                  "stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
///                  "format", GST_FORMAT_TIME,
///                  "is-live", TRUE,
                  NULL);


    GstCaps *scaleCap = gst_caps_new_simple ("video/x-raw",
                         "format", G_TYPE_STRING, "I420",
                         "width", G_TYPE_INT, 1280,
                         "height", G_TYPE_INT, 720,
                         "framerate", GST_TYPE_FRACTION, 15, 1,
                         NULL);
    GstElement *videoscale;
    videoscale = gst_element_factory_make ("videoscale", "scale");

    gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videoscale, videosink, NULL);

    auto link_ok = gst_element_link_filtered(videoscale, videosink, scaleCap);

    gst_element_link_many (appsrc, conv, videoscale, videosink, NULL);

    /* setup appsrc */
    g_object_set (G_OBJECT (appsrc), "stream-type", 0, "is-live", 1, /*"format", GST_FORMAT_TIME,*/ NULL);



    spin.init();
    spin.prepareAcquisition();


    g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
    g_signal_connect (appsrc, "enough-data", G_CALLBACK (cb_test_), NULL);


    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);
    spin.deinit();
    return 0;
}




/// gst-launch-1.0 v4l2src device=/dev/video7 ! "video/x-raw, width=640, height=480, format=(string)BGR" ! videoconvert !  omxh264enc bitrate=3000000 control-rate=2 ! h264parse config-interval=10 ! rtph264pay ! udpsink host=144.64.97.197 port=7777
