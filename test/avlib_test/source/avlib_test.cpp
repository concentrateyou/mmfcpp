/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <video_widget.h>
#include <video_renderer.h>

#include <av_file_src.h>
#include <audio_renderer.h>
#include <avcodec_audio_decoder.h>
#include <avcodec_video_decoder.h>

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
    MEDIA_TRACE();

    QApplication app(argc, argv);
    Video_widget window(0);

    int time = 0;
    Av_file_src src("av");
    Video_renderer video_sink("opengl", &window);
    Audio_renderer audio_sink("alsa", "default");
    Avcodec_audio_decoder audio_decoder("audio_decoder");
    Avcodec_video_decoder video_decoder("video_decoder");

    if (argc < 2)
    {
        printf("\n\tInsufficient arguments\n");
        return 0;
    }

    if (1 != src.set_file_path(argv[1]))
    {
        printf("\nInvalid File Path\n");
		return 0;
	}

    window.show();
    connect(src, video_decoder);
    connect(src, audio_decoder);
    connect(audio_decoder, audio_sink);
    connect(video_decoder, video_sink);
    start(src, 0, 0);
    int ret = app.exec();
    stop(src, time);
    disconnect(audio_decoder, audio_sink);
    disconnect(video_decoder, video_sink);
    disconnect(src, audio_decoder);
    disconnect(src, video_decoder);

    return ret;
}

