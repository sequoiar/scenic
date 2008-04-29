#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define _LINUX_TIME_H 1
#include <linux/videodev.h>

#define M_S 0	/* set */
#define M_Q 1	/* query */

#ifndef VIDEO_PALETTE_JPEG
	#define VIDEO_PALETTE_JPEG 21
#endif

typedef struct {
	char *name;
	int pal;
} pals;

pals palettes[]={
	{ "GREY", VIDEO_PALETTE_GREY }, 
	{ "HI240", VIDEO_PALETTE_HI240 }, 
	{ "RGB565", VIDEO_PALETTE_RGB565 }, 
	{ "RGB24", VIDEO_PALETTE_RGB24 }, 
	{ "RGB32", VIDEO_PALETTE_RGB32 }, 
	{ "RGB555", VIDEO_PALETTE_RGB555 }, 
	{ "YUV422", VIDEO_PALETTE_YUV422 }, 
	{ "YUYV", VIDEO_PALETTE_YUYV }, 
	{ "UYVY", VIDEO_PALETTE_UYVY }, 
	{ "YUV420", VIDEO_PALETTE_YUV420 }, 
	{ "YUV411", VIDEO_PALETTE_YUV411 }, 
	{ "RAW", VIDEO_PALETTE_RAW }, 
	{ "YUV422P", VIDEO_PALETTE_YUV422P }, 
	{ "YUV411P", VIDEO_PALETTE_YUV411P }, 
	{ "YUV420P", VIDEO_PALETTE_YUV420P }, 
	{ "YUV410P", VIDEO_PALETTE_YUV410P }, 
	{ "PLANAR", VIDEO_PALETTE_PLANAR }, 
	{ "COMPONENT", VIDEO_PALETTE_COMPONENT }, 
	{ "JPEG", VIDEO_PALETTE_JPEG },
	{ NULL, -1 }
		};

void usage(void)
{
	printf("d device	device to use (default is /dev/video0)\n");
	printf("q		query capabilities\n");
	printf("i channel	set input-channel\n");
	printf("t tuner		set tuner (most of the time 0)\n");
	printf("m mode		mode (PAL/NTSC/SECAM/AUTO)\n");
	printf("f frequency	set frequency\n");
	printf("b brightness	set brightness\n");
	printf("u hue		set hue\n");
	printf("c color		set color\n");
	printf("n contrast	set contrast\n");
	printf("w whiteness	set whiteness\n");
	printf("p palette	set palette (GREY, HI240, RGB565, RGB24,\n");
	printf("		RGB32, RGB555, YUV422, YUYV, UYVY, YUV420,\n");
	printf("		YUV411, RAW, YUV422P, YUV411P, YUV420P,\n");
	printf("		YUV410P, PLANAR, COMPONENT, JPEG (spca5xx");
	printf("		extension))\n");
	printf("s width,height	set size\n");
	printf("\n");
	printf("h		this help\n");
}

void myperror(char *str)
{
	perror(str);
	exit(1);
}

void set_inputchannel(int fd, int channel)
{
	struct video_channel vchan;

	vchan.channel = channel;

	/* get things like norm */
	if (ioctl(fd, VIDIOCGCHAN, &vchan) == -1)
		myperror("VDIOCGCHAN");

	/* do set */
	if (ioctl(fd, VIDIOCSCHAN, &vchan) == -1)
		myperror("VIDIOCSCHAN");
}

void query(int fd)
{
	struct video_window vwin;
	struct video_picture vpic;
	int loop;
	struct video_capability vcap;

	if (ioctl(fd, VIDIOCGCAP, &vcap, sizeof(vcap)) == -1)
	{
		myperror("VIDIOCGCAP");
		return;
	}

	printf("Canonical name for this interface: %s\n", vcap.name);
	printf("Type of interface:\n");
	if (vcap.type & VID_TYPE_CAPTURE)
		printf(" Can capture to memory\n");
	if (vcap.type & VID_TYPE_TUNER)
		printf(" Has a tuner of some form\n");
	if (vcap.type & VID_TYPE_TELETEXT)
		printf(" Has teletext capability\n");
	if (vcap.type & VID_TYPE_OVERLAY)
		printf(" Can overlay its image onto the frame buffer\n");
	if (vcap.type & VID_TYPE_CHROMAKEY)
		printf(" Overlay is Chromakeyed\n");
	if (vcap.type & VID_TYPE_CLIPPING)
		printf(" Overlay clipping is supported\n");
	if (vcap.type & VID_TYPE_FRAMERAM)
		printf(" Overlay overwrites frame buffer memory\n");
	if (vcap.type & VID_TYPE_SCALES)
		printf(" The hardware supports image scaling\n");
	if (vcap.type & VID_TYPE_MONOCHROME)
		printf(" Image capture is grey scale only\n");
	if (vcap.type & VID_TYPE_SUBCAPTURE)
		printf(" Capture can be of only part of the image\n");
	printf("\n");
	printf("Number of radio/tv channels if appropriate: %d\n", vcap.channels);
	printf("Number of audio devices if appropriate: %d\n", vcap.audios);
	printf("Maximum capture width in pixels: %d\n", vcap.maxwidth);
	printf("Maximum capture height in pixels: %d\n", vcap.maxheight);
	printf("Minimum capture width in pixels: %d\n", vcap.minwidth);
	printf("Minimum capture height in pixels: %d\n", vcap.minheight);
	printf("\n");

	if (ioctl(fd, VIDIOCGWIN, &vwin, sizeof(vwin)) == -1)
	{
		myperror("VIDIOCGWIN");
		return;
	}
	printf("Image size (x,y): %d, %d\n", vwin.width,vwin.height);
	printf("\n");

	for(loop=0; loop<vcap.channels; loop++)
	{
		struct video_channel vchan;

		vchan.channel = loop;

		if (ioctl(fd, VIDIOCGCHAN, &vchan, sizeof(vchan)) == -1)
			myperror("VDIOCGCHAN");
		else
		{
			printf("The channel number: %d\n", vchan.channel);
			printf(" The input name: %s\n", vchan.name);
			printf(" Number of tuners for this input: %d\n", vchan.tuners);
			if (vchan.flags & VIDEO_VC_TUNER)
				printf(" Channel has tuners\n");
			if (vchan.flags & VIDEO_VC_AUDIO)
				printf(" Channel has audios\n");
/*			if (vchan.flags & VIDEO_VC_NORM)
				printf(" Channel has norm setting\n");	*/
			if (vchan.type & VIDEO_TYPE_TV)
				printf(" The input is a TV input\n");
			if (vchan.type & VIDEO_TYPE_CAMERA)
				printf(" The input is a camera\n");
			printf(" The norm for this channel: %d\n", vchan.norm);

			if (vchan.tuners)
			{
				int loop2;

				set_inputchannel(fd, loop);

				for(loop2=0; loop2<vchan.tuners; loop2++)
				{
					struct video_tuner vtun;

					vtun.tuner = loop2;

					if (ioctl(fd, VIDIOCGTUNER, &vtun) == -1)
						myperror("VIDIOCGTUNER");
					else
					{
						unsigned long cur_freq;

						printf("  Number of the tuner: %d\n", vtun.tuner);
						printf("  Canonical name for this tuner: %s\n", vtun.name);
						printf("  Lowest tunable frequency: %ld\n", vtun.rangelow);
						printf("  Highest tunable frequency: %ld\n", vtun.rangehigh);

						if (ioctl(fd, VIDIOCGFREQ, &cur_freq) == -1)
							myperror("VIDIOCGFREQ");
						else
							printf("  Current frequency: %ld%s\n", cur_freq / 16, vtun.flags & VIDEO_TUNER_LOW?"KHz":"MHz");

						printf("  Flags describing the tuner:\n");
						if (vtun.flags & VIDEO_TUNER_PAL)
							printf("   PAL tuning is supported\n");
						if (vtun.flags & VIDEO_TUNER_NTSC)
							printf("   NTSC tuning is supported\n");
						if (vtun.flags & VIDEO_TUNER_SECAM)
							printf("   SECAM tuning is supported\n");
						if (vtun.flags & VIDEO_TUNER_LOW)
							printf("   Frequency is in a lower range (tuning frequencies are in 1/16th KHz)\n");
						else
							printf("   Frequency is in a higher range (tuning frequencies are in 1/16th MHz)\n");
						if (vtun.flags & VIDEO_TUNER_NORM)
							printf("   The norm for this tuner is settable\n");
						if (vtun.flags & VIDEO_TUNER_STEREO_ON)
							printf("   The tuner is seeing stereo audio\n");
						if (vtun.flags & VIDEO_TUNER_RDS_ON)
							printf("   The tuner is seeing a RDS datastream\n");
						if (vtun.flags & VIDEO_TUNER_MBS_ON)
							printf("   The tuner is seeing a MBS datastream\n");
						printf("  The video signal mode if relevant:\n");
						if (vtun.mode & VIDEO_MODE_PAL)
							printf("   The tuner is in PAL mode\n");
						if (vtun.mode & VIDEO_MODE_NTSC)
							printf("   The tuner is in NTSC mode\n");
						if (vtun.mode & VIDEO_MODE_SECAM)
							printf("   The tuner is in SECAM mode\n");
						if (vtun.mode & VIDEO_MODE_AUTO)
							printf("   The tuner auto switches, or mode does not apply\n");
						printf("  Signal strength if known: %d\n", vtun.signal);
					}
				}
			}
		}
	}

	if (ioctl(fd, VIDIOCGPICT, &vpic) == -1)
		myperror("VIDIOCGPICT");
	printf("Brightness: %d\n", vpic.brightness);
	printf("Hue: %d\n", vpic.hue);
	printf("Colour: %d\n", vpic.colour);
	printf("Contrast: %d\n", vpic.contrast);
	printf("Whiteness: %d\n", vpic.whiteness);
	printf("Depth: %d\n", vpic.depth);
	printf("Palette: ");
	switch(vpic.palette) {
	case VIDEO_PALETTE_GREY:
		printf("Linear intensity grey scale (255 is brightest).\n");
		break;
	case VIDEO_PALETTE_HI240:
		printf("The BT848 8bit colour cube.\n");
		break;
	case VIDEO_PALETTE_RGB565:
		printf("RGB565 packed into 16 bit words.\n");
		break;
	case VIDEO_PALETTE_RGB555:
		printf("RGV555 packed into 16 bit words, top bit undefined.\n");
		break;
	case VIDEO_PALETTE_RGB24:
		printf("RGB888 packed into 24bit words.\n");
		break;
	case VIDEO_PALETTE_RGB32:
		printf("RGB888 packed into the low 3 bytes of 32bit words. The top 8bits are undefined.\n");
		break;
	case VIDEO_PALETTE_YUV422:
		printf("Video style YUV422 - 8bits packed 4bits Y 2bits U 2bits V\n");
		break;
	case VIDEO_PALETTE_YUYV:
		printf("Describe me\n");
		break;
	case VIDEO_PALETTE_UYVY:
		printf("Describe me\n");
		break;
	case VIDEO_PALETTE_YUV420:
		printf("YUV420 capture\n");
		break;
	case VIDEO_PALETTE_YUV411:
		printf("YUV411 capture\n");
		break;
	case VIDEO_PALETTE_RAW:
		printf("RAW capture (BT848)\n");
		break;
	case VIDEO_PALETTE_YUV422P:
		printf("YUV 4:2:2 Planar\n");
		break;
	case VIDEO_PALETTE_YUV411P:
		printf("YUV 4:1:1 Planar\n");
		break;
	case VIDEO_PALETTE_JPEG:
		printf("spca5xx extension\n");
		break;
	default:
		printf("Unknown! (%d)\n", vpic.palette);
	}
	printf("\n");

	for(loop=0; loop<vcap.audios; loop++)
	{
		struct video_audio vaud;

		vaud.audio = loop;

		if (ioctl(fd, VIDIOCGAUDIO, &vaud) == -1)
			myperror("VIDIOCGAUDIO");

		printf("Audio channel: %d\n", vaud.audio);
		printf(" Volume: %d\n", vaud.volume);
		printf(" Bass: %d\n", vaud.bass);
		printf(" Treble: %d\n", vaud.treble);
		printf(" Flags:\n");
		if (vaud.flags & VIDEO_AUDIO_MUTE)
			printf("  The audio is muted\n");
		if (vaud.flags & VIDEO_AUDIO_MUTABLE)
			printf("  Audio muting is supported\n");
		if (vaud.flags & VIDEO_AUDIO_VOLUME)
			printf("  The volume is controllable\n");
		if (vaud.flags & VIDEO_AUDIO_BASS)
			printf("  The bass is controllable\n");
		if (vaud.flags & VIDEO_AUDIO_TREBLE)
			printf("  The treble is controllable\n");
#ifdef VIDEO_AUDIO_BALANCE
		if (vaud.flags & VIDEO_AUDIO_BALANCE)
			printf("  The balance is controllable\n");
#endif
		printf(" Name: %s\n", vaud.name);
		printf(" Mode: ");
		if (vaud.mode == VIDEO_SOUND_MONO)
			printf("  Mono signal\n");
		if (vaud.mode == VIDEO_SOUND_STEREO)
			printf("  Stereo signal (NICAM for TV)\n");
		if (vaud.mode == VIDEO_SOUND_LANG1)
			printf("  European TV alternate language 1\n");
		if (vaud.mode == VIDEO_SOUND_LANG2)
			printf("  European TV alternate language 2\n");
		printf(" Balance: %d\n", vaud.balance);
		printf(" Stepsize: %d\n", vaud.step);
	}
}

int main(int argc, char *argv[])
{
	char *device = "/dev/video0";
	int c, fd;
	char runmode = M_S;
	int ichannel = -1, tuner = 0, mode = VIDEO_MODE_PAL;
	unsigned long frequency = 0;
	int bri = -1, hue = -1, col = -1, con = -1, whi =-1, pal = -1;
	int width = -1, height = -1;

	printf("dov4l v" VERSION ", (C) 2003-2006 by folkert@vanheusden.com\n\n");

        while((c = getopt(argc, argv, "p:d:qi:t:m:f:b:u:c:n:w:hs:")) != -1)
        {
                switch(c)
                {
		case 'b':
			bri = atoi(optarg);
			break;
		case 'u':
			hue = atoi(optarg);
			break;
		case 'c':
			col = atoi(optarg);
			break;
		case 'n':
			con = atoi(optarg);
			break;
		case 'w':
			whi = atoi(optarg);
			break;
                case 'd':
                        device = optarg;
			break;
		case 'q':
			runmode = M_Q;
			break;
		case 'h':
			usage();
			break;
		case 'i':
			ichannel = atoi(optarg);
			break;
		case 'm':
			if (strcasecmp(optarg, "PAL") == 0)
				mode = VIDEO_MODE_PAL;
			else if (strcasecmp(optarg, "NTSC") == 0)
				mode = VIDEO_MODE_NTSC;
			else if (strcasecmp(optarg, "SECAM") == 0)
				mode = VIDEO_MODE_SECAM;
			else if (strcasecmp(optarg, "AUTO") == 0)
				mode = VIDEO_MODE_AUTO;
			else
			{
				fprintf(stderr, "don't know mode %s: use PAL/NTSC/SECAM or AUTO\n", optarg);
				return 1;
			}
			break;
		case 'p':
			{
				int index = 0;
				while(palettes[index].name != NULL && strcasecmp(palettes[index].name, optarg) != 0)
					index++;
				if (palettes[index].name == NULL)
				{
					fprintf(stderr, "Palette %s is unknown\n", optarg);
					return 1;
				}
				pal = palettes[index].pal;
				break;
			}
		case 't':
			tuner = atoi(optarg);
			break;
		case 'f':
			frequency = atoi(optarg);
			break;
		case 's':
			{
				char *dummy = strchr(optarg, ',');
				if (!dummy)
				{
					fprintf(stderr, "The -s settings needs a parameter in the form of 'width,height' (no spaces around the ','!).\n");
					return 1;
				}

				*dummy = 0x00;
				width = atoi(optarg);
				height = atoi(dummy + 1);
			}
		}
	}

	fd = open(device, O_RDWR);
	if (fd == -1)
	{
		fprintf(stderr, "cannot open device %s! (%s)\n", device, strerror(errno));
		return 2;
	}

	if (runmode == M_Q)
	{
		query(fd);
	}
	else
	{
		struct video_picture vpic;
		struct video_tuner vtun;
	        struct video_capability vcap;

		/* set input(!)-channel */
		if (ichannel != -1)
		{
			set_inputchannel(fd, ichannel);
		}

		/* see wether this device has a tuner */
	        if (ioctl(fd, VIDIOCGCAP, &vcap, sizeof(vcap)) == -1)
	                myperror("VIDIOCGCAP");
goto ssize;
		if (vcap.type & VID_TYPE_TUNER)
		{
			/* get tuner parameters */
			vtun.tuner = tuner;
			if (ioctl(fd, VIDIOCGTUNER, &vtun) == -1)
				myperror("VIDIOCGTUNER");

			/* set tuner (& mode!) */
			vtun.tuner = tuner;
			vtun.mode = mode;
			if (ioctl(fd, VIDIOCSTUNER, &vtun) == -1)
				myperror("VIDIOCSTUNER");

			/* set frequency */
			if (frequency != 0)
			{
				unsigned long dummy, cur_freq = 0;

				if (vtun.flags & VIDEO_TUNER_LOW)
					dummy = ((double)frequency * 16.0) / 1000;	/* KHz */
				else
					dummy = ((double)frequency * 16.0) / 1000000;	/* MHz */

				/* only set when changed */
				if (ioctl(fd, VIDIOCGFREQ, &cur_freq) == -1)
					myperror("VIDIOCGFREQ");
				else
	 			{
					if (cur_freq != dummy)
					{
						if (ioctl(fd, VIDIOCSFREQ, &dummy) == -1)
							myperror("VIDIOCSFREQ");
					}
				}
			}
		}

		/* set picture properties */
		/* first get original values */
		if (ioctl(fd, VIDIOCGPICT, &vpic) == -1)
			myperror("VIDIOCGPICT");
		if (bri != -1)
			vpic.brightness = bri;
		if (hue != -1)
			vpic.hue = hue;
		if (col != -1)
			vpic.colour = col;
		if (con != -1)
			vpic.contrast = con;
		if (whi != -1)
			vpic.whiteness = whi;
		if (pal != -1)
			vpic.palette = pal;
		/* set */
		if (ioctl(fd, VIDIOCSPICT, &vpic) == -1)
			myperror("VIDIOCSPICT");
ssize:
		/* set size */
		if (width != -1)
		{
			struct video_window vwin;
	        struct v4l2_window v2win;
/*			if (ioctl(fd,VIDIOC_G_FMT , &v2win ) == -1)
			{
				myperror("VIDIOCGWIN");
			}
*/			if (ioctl(fd, VIDIOCGWIN, &vwin ) == -1)
			{
				myperror("VIDIOCGWIN");
			}

			vwin.width = 768; //width;
			vwin.height = 480; //height;

			if (ioctl(fd, VIDIOCSWIN, &vwin) == -1)
			{
                fprintf(stderr,"errno %d",errno);
				myperror("VIDIOCSWIN");
			}
		}
	}

	return 0;
}
