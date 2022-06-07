//
// Created by vince on 02/06/20.
//


#include <gst/gst.h>
#include <iostream>
using namespace std;



gint m___ain(gint   argc, gchar *argv[])
{
    GstElement *pipeline, *videosrc, *conv,*enc, *pay, *udp, *que, *mux;

    // init GStreamer
    gst_init (&argc, &argv);
    GMainLoop *loop;

    loop = g_main_loop_new (NULL, FALSE);

    // setup pipeline
    pipeline = gst_pipeline_new ("pipeline");
/*
    videosrc = gst_element_factory_make ("videotestsrc", "source"); // autovideosrc
    conv = gst_element_factory_make ("videoconvert", "conv");
    enc = gst_element_factory_make("x264enc", "enc");
    pay = gst_element_factory_make("rtph264pay", "pay");
    g_object_set(G_OBJECT(pay), "config-interval", 1, NULL);

    udp = gst_element_factory_make("udpsink", "udp");
    g_object_set(G_OBJECT(udp), "host", "127.0.0.1", NULL);
    g_object_set(G_OBJECT(udp), "port", 5000, NULL);

    gst_bin_add_many (GST_BIN (pipeline), videosrc, conv, enc, pay, udp, NULL);

    if (gst_element_link_many (videosrc, conv, enc, pay, udp, NULL) != TRUE)
    {
        return -1;
    }
*/


    videosrc = gst_element_factory_make ("videotestsrc", "source"); // autovideosrc


    enc = gst_element_factory_make("jpegenc", NULL);
    que = gst_element_factory_make("queue", NULL);
    mux = gst_element_factory_make("multipartmux", NULL);

    udp = gst_element_factory_make("udpsink", "udp");
    g_object_set(G_OBJECT(udp), "host", "127.0.0.1", NULL);
    g_object_set(G_OBJECT(udp), "port", 9098, NULL);

    gst_bin_add_many (GST_BIN (pipeline), videosrc, enc, que, mux, udp, NULL);

    if (gst_element_link_many (videosrc, enc, que, mux, udp, NULL) != TRUE)
    {
        return -1;
    }


    // play
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_main_loop_run (loop);

    // clean up
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);

    return 0;
}



///gst-launch-1.0 -vvv udpsrc port=5000 caps="application/x-rtp" ! rtph264depay ! avdec_h264 ! videoconvert ! xvimagesink
/**

 v=0
m=video 5000 RTP/AVP 96
c=IN IP4 127.0.0.1
a=rtpmap:96 H264/90000


v4l2sink requires modprobe v4l2loopback
 **/