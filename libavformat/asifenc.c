typedef struct StreamInfo {
    int nb_packets;
    int packet_total_size;
    int packet_max_size;
    /* codec related output */
    int bit_rate;
    AVRational frame_rate;
    int nb_frames;    /* current frame number */
    int total_frames; /* total number of frames */
    int num;
    AVCodecParameters *par;
} StreamInfo;



//TODO:write header function
AVOutputFormat ff_rm_muxer = {
    .name              = "rm",
    .long_name         = NULL_IF_CONFIG_SMALL("RealMedia"),
    .mime_type         = "application/vnd.rn-realmedia",
    .extensions        = "rm,ra",
    .priv_data_size    = sizeof(RMMuxContext),
    .audio_codec       = AV_CODEC_ID_AC3,
    .video_codec       = AV_CODEC_ID_RV10,
    .write_header      = rm_write_header,
    .write_packet      = rm_write_packet,
    .write_trailer     = rm_write_trailer,
    .codec_tag         = (const AVCodecTag* const []){ ff_rm_codec_tags, 0 },
};

rm_write_header