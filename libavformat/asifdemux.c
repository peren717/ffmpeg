#include "avformat.h"
#include "internal.h"
#include "pcm.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"

typedef struct PCMAudioDemuxerContext {
    AVClass *class;
    int sample_rate;
    int channels;
} PCMAudioDemuxerContext;

static int pcm_read_header(AVFormatContext *s)
{
    PCMAudioDemuxerContext *s1 = s->priv_data;
    AVStream *st;
    uint8_t *mime_type = NULL;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);


    st->codecpar->codec_type  = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id    = s->iformat->raw_codec_id;
    st->codecpar->sample_rate = s1->sample_rate;
    st->codecpar->channels    = s1->channels;

    av_opt_get(s->pb, "mime_type", AV_OPT_SEARCH_CHILDREN, &mime_type);
    if (mime_type && s->iformat->mime_type) {
        int rate = 0, channels = 0, little_endian = 0;
        size_t len = strlen(s->iformat->mime_type);
        if (!av_strncasecmp(s->iformat->mime_type, mime_type, len)) { /* audio/L16 */
            uint8_t *options = mime_type + len;
            len = strlen(mime_type);
            while (options < mime_type + len) {
                options = strstr(options, ";");
                if (!options++)
                    break;
                if (!rate)
                    sscanf(options, " rate=%d",     &rate);
                if (!channels)
                    sscanf(options, " channels=%d", &channels);
                if (!little_endian) {
                     char val[14]; /* sizeof("little-endian") == 14 */
                     if (sscanf(options, " endianness=%13s", val) == 1) {
                         little_endian = strcmp(val, "little-endian") == 0;
                     }
                }
            }
            if (rate <= 0) {
                av_log(s, AV_LOG_ERROR,
                       "Invalid sample_rate found in mime_type \"%s\"\n",
                       mime_type);
                av_freep(&mime_type);
                return AVERROR_INVALIDDATA;
            }
            st->codecpar->sample_rate = rate;
            if (channels > 0)
                st->codecpar->channels = channels;
            if (little_endian)
                st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
        }
    }
    av_freep(&mime_type);

    st->codecpar->bits_per_coded_sample =
        av_get_bits_per_sample(st->codecpar->codec_id);

    av_assert0(st->codecpar->bits_per_coded_sample > 0);

    st->codecpar->block_align =
        st->codecpar->bits_per_coded_sample * st->codecpar->channels / 8;

    avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
    return 0;
}

static const AVOption pcm_options[] = {
    { "sample_rate", "", offsetof(PCMAudioDemuxerContext, sample_rate), AV_OPT_TYPE_INT, {.i64 = 44100}, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { "channels",    "", offsetof(PCMAudioDemuxerContext, channels),    AV_OPT_TYPE_INT, {.i64 = 1}, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVOption sln_options[] = {
    { "sample_rate", "", offsetof(PCMAudioDemuxerContext, sample_rate), AV_OPT_TYPE_INT, {.i64 = 8000}, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { "channels",    "", offsetof(PCMAudioDemuxerContext, channels),    AV_OPT_TYPE_INT, {.i64 = 1}, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass sln_demuxer_class = {
    .class_name = "sln demuxer",
    .item_name  = av_default_item_name,
    .option     = sln_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_asif_demuxer = {
    .name           = "asif",
    .long_name      = NULL_IF_CONFIG_SMALL("asif demuxer"),
    .priv_data_size = sizeof(PCMAudioDemuxerContext),
    .read_header    = pcm_read_header,
    .read_packet    = ff_pcm_read_packet,
    .read_seek      = ff_pcm_read_seek,
    .flags          = AVFMT_GENERIC_INDEX,
    .extensions     = "asif",
    .raw_codec_id   = AV_CODEC_ID_ASIF,
    .priv_class     = &sln_demuxer_class,
};
