/*
 * PCM codecs
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * PCM codecs
 */

#include "libavutil/attributes.h"
#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "mathops.h"

typedef struct ASIFDecode {
    int sample_rate;
    int channels;
    int sample_per_channel;
} ASIFDecode;

static av_cold int pcm_decode_init(AVCodecContext *avctx)
{
    ASIFDecode *s = avctx->priv_data;
    int i;

    if (avctx->channels <= 0) 
    {
        av_log(avctx, AV_LOG_ERROR, "ASIF channels out of bounds\n");
        return AVERROR(EINVAL);
    }
    else
    {
        return 0;
    }
}


static int pcm_decode_frame(AVCodecContext *avctx, void *data,
                            int *got_frame_ptr, AVPacket *avpkt)
{
    const uint8_t *src = avpkt->data;
    int buf_size       = avpkt->size;
    ASIFDecode *s      = avctx->priv_data;
    AVFrame *frame     = data;
    uint8_t *samples;
    
    frame->format = AV_SAMPLE_FMT_U8P;
    frame->nb_samples = avctx->sample_per_channel;
    frame->sample_rate = avctx->sample_rate;
    frame->channels = avctx->channels;

    if (avctx->channels == 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR(EINVAL);
    }

    if (avctx->codec_id != avctx->codec->id) {
        av_log(avctx, AV_LOG_ERROR, "codec ids mismatch\n");
        return AVERROR(EINVAL);
    }

    /* get output buffer */
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    for(int i=0;i<avctx->channels;i++)
    {
        uint8_t *sample = frame->extended_data[i];
    
        int prevValue=0;
        for(int j=0;j<avctx->sample_per_channel;j++)
        {
            if(j==0)
            {
                prevValue = bytestream_get_byte(&src);
                AV_WB8(sample, prevValue);
                sample++;
            }
            else
            {
                int delta = bytestream_get_byte(&src);
                int value = delta + prevValue;
                AV_WB8(sample, value);
                prevValue = value; 
                sample++;

            }
        }
    }

    *got_frame_ptr = 1;
    return buf_size;
}

static av_cold int pcm_decode_close(AVCodecContext *avctx)
{
    ASIFDecode *s = avctx->priv_data;

    av_freep(&s->fdsp);

    return 0;
}


AVCodec ff_asif_decoder = {
    .name           = "asif",
    .pri_data_size  = sizeof(asif_encoder_data),
    .long_name      = NULL_IF_CONFIG_SMALL("Asif decoder"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ASIF,
    .init           = asif_encode_init,
    .decode         = asif_decode_frame,
    .recieve_packet = asif_receive_packet,
    .close          = asif_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_U8P,
                                                     AV_SAMPLE_FMT_NONE },
};
