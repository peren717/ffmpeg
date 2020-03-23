#include "asif.h"

typedef struct ASIFDecode {
    short   table[256];
    AVFloatDSPContext *fdsp;
    float   scale;
} ASIFDecode;

static av_cold int asif_decode_init(AVCodecContext *avctx)
{
    ASIFDecode *s = avctx->priv_data;
    int i;


    avctx->sample_fmt = avctx->codec->sample_fmts[0];

    if (avctx->sample_fmt == AV_SAMPLE_FMT_S32)
        avctx->bits_per_raw_sample = av_get_bits_per_sample(avctx->codec_id);

    return 0;
}

static av_cold int asif_decode_close(AVCodecContext *avctx)
{
    ASIFDecode *s = avctx->priv_data;

    av_freep(&s->fdsp);

    return 0;
}

/**
 * Read ASIF samples macro
 * @param size   Data size of native machine format
 * @param endian bytestream_get_xxx() endian suffix
 * @param src    Source pointer (variable name)
 * @param dst    Destination pointer (variable name)
 * @param n      Total number of samples (variable name)
 * @param shift  Bitshift (bits)
 * @param offset Sample value offset
 */
#define DECODE(size, endian, src, dst, n, shift, offset)                       \
    for (; n > 0; n--) {                                                       \
        uint ## size ## _t v = bytestream_get_ ## endian(&src);                \
        AV_WN ## size ## A(dst, (uint ## size ## _t)(v - offset) << shift);    \
        dst += size / 8;                                                       \
    }

#define DECODE_PLANAR(size, endian, src, dst, n, shift, offset)                \
    n /= avctx->channels;                                                      \
    for (c = 0; c < avctx->channels; c++) {                                    \
        int i;                                                                 \
        dst = frame->extended_data[c];                                         \
        for (i = n; i > 0; i--) {                                              \
            uint ## size ## _t v = bytestream_get_ ## endian(&src);            \
            AV_WN ## size ## A(dst, (uint ## size ##_t)(v - offset) << shift); \
            dst += size / 8;                                                   \
        }                                                                      \
    }

static int asif_decode_frame(AVCodecContext *avctx, void *data,
                            int *got_frame_ptr, AVPacket *avpkt)
{
    const uint8_t *src = avpkt->data;
    int buf_size       = avpkt->size;
    ASIFDecode *s       = avctx->priv_data;
    AVFrame *frame     = data;
    int sample_size, c, n, ret, samples_per_block;
    uint8_t *samples;
    int32_t *dst_int32_t;

    samples_per_block = 1; //TODO:figure this out
    sample_size       = 5;  //TODO:figure this out

    if (sample_size == 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid sample_size\n");
        return AVERROR(EINVAL);
    }

    if (avctx->channels == 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR(EINVAL);
    }

    if (avctx->codec_id != avctx->codec->id) {
        av_log(avctx, AV_LOG_ERROR, "codec ids mismatch\n");
        return AVERROR(EINVAL);
    }

    n = avctx->channels * sample_size;

    /* get output buffer */
    frame->nb_samples = n * samples_per_block / avctx->channels;
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;
    samples = frame->data[0];
    
    DECODE();    //TODO:pass the value

    *got_frame_ptr = 1;

    return buf_size;

}

AVCodec ff_asif_decoder={
    .id = AV_CODEC_ID_ASIF,
    .type = AVMEDIA_TYPE_AUDIO,
    .name = "asif",
    .long_name = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
};