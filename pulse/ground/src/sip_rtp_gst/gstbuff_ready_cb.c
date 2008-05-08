// gstbuff_ready_cb.c

#include "gst_sip_rtp.h"

// FIXME: shouldn't be global

struct media_stream *strm; 
enum { RTCP_INTERVAL = 5000, RTCP_RAND = 2000 };
static unsigned msec_interval;
static pj_timestamp freq, next_rtp, next_rtcp;
static char packet[1500];



void rtp_setup(void *arg)
{ 
    strm = arg;
    /* Boost thread priority if necessary */
    boost_priority();

    /* Let things settle */
    pj_thread_sleep(100);

    msec_interval = strm->samples_per_frame * 1000 / strm->clock_rate;
    pj_get_timestamp_freq(&freq);

    pj_get_timestamp(&next_rtp);
    next_rtp.u64 += (freq.u64 * msec_interval / 1000);

    next_rtcp = next_rtp;
    next_rtcp.u64 += (freq.u64 * (RTCP_INTERVAL+(pj_rand()%RTCP_RAND)) / 1000);
}



// Callback for fakesink data
void cb_handoff(GstElement *fakesink, GstBuffer *buffer, 
        GstPad *pad, gpointer user_data)
{
    static gboolean white = FALSE;

    /* this makes the image black/white */
    memset (GST_BUFFER_DATA (buffer), white ? 0xff : 0x0,
            GST_BUFFER_SIZE (buffer));
    white = !white;

    pj_timestamp now, lesser;
    pj_time_val timeout;
    pj_bool_t send_rtp, send_rtcp;
    send_rtp = send_rtcp = PJ_FALSE;

    printf("!");
    fflush(stdout);


    /* Determine how long to sleep */
    if (next_rtp.u64 < next_rtcp.u64) 
    {
        lesser = next_rtp;
        send_rtp = PJ_TRUE;
    } 
    else 
    {
        lesser = next_rtcp;
        send_rtcp = PJ_TRUE;
    }

    /*----------------------------------------------*/ 
    //  TIMESTAMP
    /*----------------------------------------------*/ 
    pj_get_timestamp(&now);


    if (lesser.u64 <= now.u64) 
    {
        timeout.sec = timeout.msec = 0;
        //printf("immediate "); fflush(stdout);
    } 
    else 
    {
        pj_uint64_t tick_delay;
        tick_delay = lesser.u64 - now.u64;
        timeout.sec = 0;
        timeout.msec = (pj_uint32_t)(tick_delay * 1000 / freq.u64);
        pj_time_val_normalize(&timeout);

        //printf("%d:%03d ", timeout.sec, timeout.msec); fflush(stdout);
    }

    /* Wait for next interval */
    //if (timeout.sec!=0 && timeout.msec!=0) {

    pj_thread_sleep(PJ_TIME_VAL_MSEC(timeout));
    if (strm->thread_quit_flag)
        return;

    //}

    /*----------------------------------------------*/ 
    //  TIME STAMP
    /*----------------------------------------------*/ 
    pj_get_timestamp(&now);


    if (send_rtp || next_rtp.u64 <= now.u64) 
    {
        /*
         * Time to send RTP packet.
         */
        pj_status_t status;
        const void *p_hdr;
        const pjmedia_rtp_hdr *hdr;
        pj_ssize_t size;
        int hdrlen;

        /* Format RTP header */
        status = pjmedia_rtp_encode_rtp( &strm->out_sess, strm->si.tx_pt,
                0, /* marker bit */
                strm->bytes_per_frame, 
                strm->samples_per_frame,
                &p_hdr, &hdrlen);
        if (status == PJ_SUCCESS) 
        {

            //PJ_LOG(4,(THIS_FILE, "\t\tTx seq=%d", pj_ntohs(hdr->seq)));

            hdr = (const pjmedia_rtp_hdr*) p_hdr;

            /* Copy RTP header to packet */
            pj_memcpy(packet, hdr, hdrlen);

            /* Zero the payload */
            pj_bzero(packet+hdrlen, strm->bytes_per_frame);


            size = hdrlen + strm->bytes_per_frame;

            /* Send RTP packet */
            status = pjmedia_transport_send_rtp(strm->transport, 
                    packet, size);


            if (status != PJ_SUCCESS)
                app_perror("cb", "Error sending RTP packet", status);

        } 
        else 
        {
            pj_assert(!"RTP encode() error");
        }



        /* Update RTCP SR */
        pjmedia_rtcp_tx_rtp( &strm->rtcp, (pj_uint16_t)strm->bytes_per_frame);

        /* Schedule next send */
        next_rtp.u64 += (msec_interval * freq.u64 / 1000);
    }

    /******************** SEND RTCP ***************************************/
    if (send_rtcp || next_rtcp.u64 <= now.u64) 
    {
        /*
         * Time to send RTCP packet.
         */
        void *rtcp_pkt;
        int rtcp_len;
        pj_ssize_t size;
        pj_status_t status;

        /* Build RTCP packet */
        pjmedia_rtcp_build_rtcp(&strm->rtcp, &rtcp_pkt, &rtcp_len);


        /* Send packet */
        size = rtcp_len;
        status = pjmedia_transport_send_rtcp(strm->transport,
                rtcp_pkt, size);
        if (status != PJ_SUCCESS) {
            app_perror("cb", "Error sending RTCP packet", status);
        }

        /* Schedule next send */
        next_rtcp.u64 += (freq.u64 * (RTCP_INTERVAL+(pj_rand()%RTCP_RAND)) /
                1000);
    }
    /**************************************************/
}








