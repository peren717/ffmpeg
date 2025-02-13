#include "avformat.h"
#include "internal.h"
#include "pcm.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"

typedef struct ASIFAudioDemuxerContext {
    int sample_rate;
    int channels;
    int sample_per_channel;
} ASIFAudioDemuxerContext;

//Check file type
static int probe(AVProbeData *p)
{
    if(p->buf[0] == 'a' && p->buf[1] == 's' && p->buf[2] == 'i' && p->buf[3] == 'f')
    {
        return AVPROBE_SCORE_MAX;   //return AVPROBE_SCORE_MAX if the file is coded by asif encoder
    }
    else
    {
        return 0;
    }
}

//Read the header
static int asif_read_header(AVFormatContext *s)
{
    ASIFAudioDemuxerContext *s1 = s->priv_data;
    AVStream *st;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);

    s1->sample_rate = avio_rl32(s1);
    s1->channels = avio_rl16(s1);
    s1->sample_per_channel = avio_rl32(s1);

    st->codecpar->codec_type  = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id    = AV_CODEC_ID_ASIF;
    st->codecpar->format      = AV_SAMPLE_FMT_U8P; 
    st->codecpar->sample_rate = s1->sample_rate;
    st->codecpar->channels    = s1->channels;
    

    return 0;
}

static int asif_read_packet(AVFormatContext *s, AVPacket *p)
{
    ASIFAudioDemuxerContext *s1 = s->priv_data;
    int total_sample = s1->sample_per_channel * s1->channels;
    AVIOContext *avio_ctx = s->pb;
    return av_get_packet(avio_ctx, p, total_sample);
}

AVInputFormat ff_asif_demuxer = {
    .name           = "asif",
    .long_name      = NULL_IF_CONFIG_SMALL("asif demuxer"),
    .priv_data_size = sizeof(ASIFAudioDemuxerContext),
    .read_header    = asif_read_header,
    .read_packet    = asif_read_packet,
    .flags          = AVFMT_GENERIC_INDEX,
    .extensions     = "asif",
    .raw_codec_id   = AV_CODEC_ID_ASIF,
};
