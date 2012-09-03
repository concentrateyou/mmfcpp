/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <math.h>
#include <QString>
#include <video_player.h>

Video_player::Video_player()
    :QWidget(0)
    , trick_mode(0)
    , timer(this)
    , text_mode(Video_player::frame_count)
    , text_helper(this)
    , state(Media::stop)
    , master("master")
    , window(this)
    , source("yuv")
    , sink("opengl", master.create_child("child"), &window)
{
    setupUi(this);
    initialize();
}

Video_player::~Video_player()
{
    ::disconnect(source, sink);
}

void Video_player::initialize()
{
    resize(100, 65);
    window.resize(640, 480);

    connect_signals_slots();
    
    timer.setInterval(400);
    sink.attach(Media::last_pkt_rendered, this);

    sink.register_text_helper(&text_helper);
}

void Video_player::connect_signals_slots()
{
    ::connect(source, sink);
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
    connect(button, SIGNAL(pressed()), this, SLOT(play_pause()));
    connect(slider, SIGNAL(sliderPressed()), this, SLOT(slider_pressed()));
    connect(slider, SIGNAL(sliderReleased()), this, SLOT(slider_released()));
    connect(more_chk_box, SIGNAL(stateChanged(int)), this, SLOT(more_controls(int)));

    connect(luma_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(chru_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(chrv_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(red_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(green_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(blue_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(norm_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(nyuv_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));
    connect(nrgb_radio, SIGNAL(toggled(bool)), this, SLOT(mode_change(bool)));

    connect(no_text_radio, SIGNAL(toggled(bool)), this, SLOT(text_mode_change(bool)));
    connect(time_code_radio, SIGNAL(toggled(bool)), this, SLOT(text_mode_change(bool)));
    connect(frame_count_radio, SIGNAL(toggled(bool)), this, SLOT(text_mode_change(bool)));

    norm_radio->setChecked(true); 
    time_code_radio->setChecked(true); 
    show_extra_controls(false);
}

void Video_player::show()
{
    window.show();
}

int Video_player::start(int start, int end)
{
    int ret = 0;
    timer.start();
    ret = ::start(source, start, end);
    master.start(start);
    if (0 == trick_mode)
    {
        state = Media::play;
        button->setText("Pause");
    }
    MEDIA_LOG("\nStart: %d", start);
    return ret;
}

int Video_player::stop(int& time)
{
    int ret = 0;
    timer.stop();
    ret = ::stop(source, time);
    uint64_t tmp = 0;
    master.stop(tmp);
    state = Media::stop;
    button->setText("Play");
    return ret;
}

float Video_player::fps() const
{
    return source.fps();
}

int Video_player::duration() const
{
    return source.duration();
}

int Video_player::set_parameters(int width, int height, Media::type fmt, float fps, const char* path)
{
    int ret = source.set_parameters(path, fmt, fps, width, height);
    if (ret == 1)
    {
        slider->setRange(0, source.duration());
        window.setWindowTitle(path);
    }
    return ret;
}

void Video_player::time_out()
{
    if (trick_mode)
    {
        int time = slider->value();
        int end = (state == Media::play)?source.duration():time;
        start(time, end);
    }
    else
    {
        slider->setValue(sink.current_position());
    }
}

void Video_player::slider_pressed()
{
    trick_mode = 1;
    if (state == Media::stop)
    {
        timer.start();
    }
}

void Video_player::slider_released()
{
    int time = slider->value();
    int end = (state == Media::play)?source.duration():time;
    start(time, end);
    if (state == Media::stop)
    {
        timer.stop();
    }
    trick_mode = 0;
}

void Video_player::play_pause()
{
    if (state == Media::play)
    {
        int time = 0;
        stop(time);
    }
    else
    {
        start(slider->value(), source.duration());
    }
}

void Video_player::mode_change(bool status)
{
    int mode = -1;

    if (false == status)
    {
        return;
    }

    if (luma_radio->isChecked())
    {
        mode = 0;
    }
    else if (chru_radio->isChecked())
    {
        mode = 1;
    }
    else if (chrv_radio->isChecked())
    {
        mode = 2;
    }
    else if (red_radio->isChecked())
    {
        mode = 3;
    }
    else if (green_radio->isChecked())
    {
        mode = 4;
    }
    else if (blue_radio->isChecked())
    {
        mode = 5;
    }
    else if (norm_radio->isChecked())
    {
        mode = 6;
    }
    else if (nyuv_radio->isChecked())
    {
        mode = 7;
    }
    else if (nrgb_radio->isChecked())
    {
        mode = 8;
    }

    if (mode >= 0)
    {
        window.set_mode(mode);
    }
}

void Video_player::text_mode_change(bool status)
{
    if (false == status)
    {
        return;
    }

    if (no_text_radio->isChecked())
    {
        text_mode = Video_player::no_text;
    }
    else if (time_code_radio->isChecked())
    {
        text_mode = Video_player::time_code;
    }
    else if (frame_count_radio->isChecked())
    {
        text_mode = Video_player::frame_count;
    }
    qDebug() << "text_mode_change " << text_mode;
}

void Video_player::more_controls(int state)
{
    show_extra_controls((state == Qt::Checked));
    if (state == Qt::Checked)
    {
        resize(width(), 160);
        move(x(), window.height()-175);
    }
    else
    {
        resize(width(), 65);
        move(x(), window.height()-80);
    }    
}

void Video_player::show_extra_controls(bool ok)
{
    if (ok)
    {
        text_frame->show();
        video_mode_frame->show();
    }
    else
    {
        text_frame->hide();
        video_mode_frame->hide();
    }
}

void Video_player::resizeEvent(QResizeEvent* event)
{
}

int Video_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    (void)event;
    (void)obj;
    (void)params;
    if (!trick_mode)
    {
        timer.stop();
        time_out();
        state == Media::stop;
    }
    qDebug() << "event_handler: " << event;
    return 0;
}

void Video_player::set_text_mode(Video_player::Text_mode mode)
{
    text_mode = mode;
}

Text_helper::Text_helper(Video_player* p)
    :player(p)
{
}

Text_helper::~Text_helper()
{
}

void Text_helper::read_text(char* text, int length, uint64_t time)
{
    switch (player->text_mode)
    {
        case Video_player::time_code:
            {
                int sec = time/1000000;
                int min = (sec/60);
                int hr = (min/60);
                min %= 60; sec %= 60;
                snprintf(text, length, "%02d:%02d:%02d:%06d", hr, min, sec, time%1000000);
            }
            break;
        case Video_player::frame_count:
            {
                int curr_frame = 1+(int)ceil(((float)time*player->fps())/1000000.0);
                int frame_count = (int)ceil(((float)player->duration()*player->fps())/1000000.0);
                snprintf(text, length, "%d/%d", curr_frame, frame_count);
            }
            break;
        default:
            snprintf(text, length, "");
            break;
    }    
}

