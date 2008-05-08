#ifndef _SIP_RTP_H_
#define _SIP_RTP_H_
/* Include all headers. */
#include <pjsip.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdlib.h>
// gstreamer includes
#include <gst/gst.h>
//gstbuff_ready_cb.c

void rtp_setup(void *);


void rtp_cb(GstBuffer*);


int sip_main(int argc, char *argv[]);


/* Display error */
void app_perror(const char *sender, const char *title, 
		       pj_status_t status);


void boost_priority(void);


/* A bidirectional media stream created when the call is active. */
struct media_stream
{
    /* Static: */
    unsigned		 call_index;	    /* Call owner.		*/
    unsigned		 media_index;	    /* Media index in call.	*/
    pjmedia_transport   *transport;	    /* To send/recv RTP/RTCP	*/

    /* Active? */
    pj_bool_t		 active;	    /* Non-zero if is in call.	*/

    /* Current stream info: */
    pjmedia_stream_info	 si;		    /* Current stream info.	*/

    /* More info: */
    unsigned		 clock_rate;	    /* clock rate		*/
    unsigned		 samples_per_frame; /* samples per frame	*/
    unsigned		 bytes_per_frame;   /* frame size.		*/

    /* RTP session: */
    pjmedia_rtp_session	 out_sess;	    /* outgoing RTP session	*/
    pjmedia_rtp_session	 in_sess;	    /* incoming RTP session	*/

    /* RTCP stats: */
    pjmedia_rtcp_session rtcp;		    /* incoming RTCP session.	*/

    /* Thread: */
    pj_bool_t		 thread_quit_flag;  /* Stop media thread.	*/
    pj_thread_t		*thread;	    /* Media thread.		*/
};


#endif // _SIP_RTP_H_
