#define BITSTREAM_WRITER_LE

#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "put_bits.h"
#include "bytestream.h"

typedef struct asif_encoder_data
{
    //TODO
} asif_encoder_data;

static av_cold int asif_encode_init(AVCodecContext *avctx)
{
    asif_encoder_data *my_data;

    my_data = avctx-?priv_data;

    //TODO:initialize my_data
    av_log(avctx, AV_LOG_INFO, "Executing inside of asif encoder, asif_encoder_init\n");
    avctx->frame_size = 1000000;
    return 0;
}

static int asif_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
                                const AVFrame *frame, int *got_packet_ptr)
{
    int ret;
    uint8_t *buf;
    av_log(avctx, AV_LOG_INFO, "Executing inside of asif encoder, asif_encoder_frame\n");
    
    if ((ret = ff_alloc_packet2(avctx, avpkt, 16, 16)) < 0)
        return ret;
    
    buf = avpkt->data;
    buf[0] = 'a';
    buf[1] = 's';
    buf[2] = 'i';
    buf[3] = 'f';


    for (int i = 4;i<16;i++)
        buf[i] = ' ';

    *got_packet_ptr = 1;
    return 0;
}

int asif_send_frame(AVCodecContext *avctx, const AVFrame *frame)
{
//TODO




}

int asif_receive_packet(AVCodecContext *avctx, AVPacket *avpkt)
{
//TODO




}


static av_cold int asif_encode_close(AVCodecContext *avctx)
{
    av_log(avctx, AV_LOG_INFO, "Executing inside of asif encoder, asif_encoder_close\n");

    return 0;
}

AVCodec ff_asif_encoder = {
    .name           = "asif",
    .pri_data_size  = sizeof(asif_encoder_data),
    .long_name      = NULL_IF_CONFIG_SMALL("Asif encoder"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ASIF,
    .init           = asif_encode_init,
    //.encode2        = asif_encode_frame,
    .send_frame     = asif_send_frame,
    .recieve_packet = asif_receive_packet,
    .close          = asif_encode_close,
    .capabilities   = AV_CODEC_CAP_SMALL_LAST_FRAME,
    .sample_fmts    = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_U8P,
                                                     AV_SAMPLE_FMT_NONE },
};
