#include "asif.h"

static av_cold int asif_encode_init(AVCodecContext *avctx)
{
    avctx->frame_size = 0;
    avctx->bits_per_coded_sample = av_get_bits_per_sample(avctx->codec->id);
    avctx->block_align           = avctx->channels * avctx->bits_per_coded_sample / 8;
    avctx->bit_rate              = avctx->block_align * 8LL * avctx->sample_rate;
    return 0;
}


/**
 * Write ASIF samples macro
 * @param type   Datatype of native machine format
 * @param endian bytestream_put_xxx() suffix
 * @param src    Source pointer (variable name)
 * @param dst    Destination pointer (variable name)
 * @param n      Total number of samples (variable name)
 * @param shift  Bitshift (bits)
 * @param offset Sample value offset
 */
//TODO: modify this macro
#define ENCODE_PLANAR(type, endian, dst, n, shift, offset)              \
    n /= avctx->channels;                                               \
    for (c = 0; c < avctx->channels; c++) {                             \
        int i;                                                          \
        samples_ ## type = (const type *) frame->extended_data[c];      \
        for (i = n; i > 0; i--) {                                       \
            register type v = (*samples_ ## type++ >> shift) + offset;  \
            bytestream_put_ ## endian(&dst, v);                         \
        }                                                               \
    }
//memcpy?
//increment pointer

static int asif_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
                            const AVFrame *frame, int *got_packet_ptr)
{
    int n, c, sample_size, v, ret;
    const short *samples;
    unsigned char *dst;
    const uint8_t *samples_uint8_t;


    sample_size = av_get_bits_per_sample(avctx->codec->id) / 8;
    n           = frame->nb_samples * avctx->channels;
    samples     = (const short *)frame->data[0];

    if ((ret = ff_alloc_packet2(avctx, avpkt, n * sample_size, n * sample_size)) < 0)
        return ret;
    dst = avpkt->data;

    ENCODE_PLANAR(int8_t, be64, samples, dst, n, 0, 0);   //TODO:Pass the value

    *got_packet_ptr = 1;
    return 0;
}

AVCodec ff_asif_encoder = {
    .id = AV_CODEC_ID_ASIF,
    .type = AVMEDIA_TYPE_AUDIO,
    .name = "asif",
    .long_name = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
    .encode2      = pcm_encode_frame,
    .capabilities = AV_CODEC_CAP_VARIABLE_FRAME_SIZE,
    .sample_fmts  = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_U8,             
                                                   AV_SAMPLE_FMT_NONE },
};
