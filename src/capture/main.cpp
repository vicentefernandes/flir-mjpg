#include <iostream>

#include <gst/gst.h>
#include <iostream>
using namespace std;
/*
int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *sink;
    GstBus *bus;
    GstMessage *msg;
    GMainLoop *loop;
    GstStateChangeReturn ret;    //initialize all elements
    gst_init(&argc, &argv);
    pipeline = gst_pipeline_new ("pipeline");
    source = gst_element_factory_make ("autovideosrc", "source"); // videotestsrc
    sink = gst_element_factory_make ("autovideosink", "sink");

    //check for null objects
    if (!pipeline || !source || !sink) {
        cout << "not all elements created: pipeline["<< !pipeline<< "]" << "source["<< !source<< "]" << "sink["<< !sink << "]" << endl;
        return -1;
    }

    //set video source
    g_object_set(G_OBJECT (source), "location", argv[1], NULL);
    cout << "==>Set video source." << endl;
    g_object_set(G_OBJECT (sink), "sync", FALSE, NULL);
    cout << "==>Set video sink." << endl;

    //add all elements together
    gst_bin_add_many (GST_BIN (pipeline), source, sink, NULL);
    if (gst_element_link (source, sink) != TRUE) {
        cout << "Elements could not be linked." << endl;
        gst_object_unref (pipeline);
        return -1;
    }
    cout << "==>Link elements." << endl;

    //set the pipeline state to playing
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        cout << "Unable to set the pipeline to the playing state." << endl;
        gst_object_unref (pipeline);
        return -1;
    }
    cout << "==>Set video to play." << endl;

    //get pipeline's bus
    bus = gst_element_get_bus (pipeline);
    cout << "==>Setup bus." << endl;

    loop = g_main_loop_new(NULL, FALSE);
    cout << "==>Begin stream." << endl;
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}*/




gint ___main(gint   argc, gchar *argv[])

{
    GstElement *pipeline, *videosrc, *conv,*enc, *pay, *udp;

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

    conv = gst_element_factory_make ("tee", NULL);
    udp = gst_element_factory_make("v4l2sink", NULL);
    gst_bin_add_many (GST_BIN (pipeline), videosrc, conv, udp, NULL);

    if (gst_element_link_many (videosrc, conv,  udp, NULL) != TRUE)
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
