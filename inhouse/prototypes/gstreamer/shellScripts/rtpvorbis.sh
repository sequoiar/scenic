#!/bin/bash

gst-launch-0.10 -v interleave name=i ! jackaudiosink \
audiotestsrc volume=0.5 freq=200  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_REAR_LEFT" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.5 freq=200  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_LFE" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT" ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=300  ! "audio/x-raw-int, channel-position=(GstAudioChannelPosition)GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT" ! audioconvert ! queue ! i. 
