#!/bin/bash

gst-launch -v videotestsrc ! x264enc ! rtph264pay ! udpsink host=localhost port=5060
