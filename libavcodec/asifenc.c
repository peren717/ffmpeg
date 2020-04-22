#define BITSTREAM_WRITER_LE

#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "put_bits.h"
#include "bytestream.h"
#include <stdio.h>
#include <stdbool.h>


struct sample_node{
    int counter;
    uint8_t data[100];
    struct Node* next;
};

struct channel_node{
    struct sample_node* head_data;
    struct sample_node* tail_data;
};

typedef struct asif_encoder_data
{
    char asif;
    int32_t freq;
    int16_t channels;
    int32_t spc;    //samples per channel
    struct channel_node* channel_r[20];  //array of channels
    int32_t total_samples;
    bool is_first_frame;  
    
} asif_encoder_data;



static av_cold int asif_encode_init(AVCodecContext *avctx)
{

    asif_encoder_data *my_data;
    my_data = avctx->priv_data;
    my_data->asif="asif";
    my_data->freq=avctx->sample_rate;
    my_data->channels=0;
    my_data->total_samples=0;
    my_data->spc=0;
    my_data->is_first_frame=true;

    //TODO:initialize my_data
    av_log(avctx, AV_LOG_INFO, "Executing inside of asif encoder, asif_encoder_init\n");
    avctx->frame_size = 1000000;
    avctx->bits_per_coded_sample = av_get_bits_per_sample(avctx->codec->id);
    avctx->block_align = avctx ->channels * avctx->bits_per_coded_sample/8;
    avctx->bit_rate = avctx->block_align * 8LL * avctx -> sample_rate;
    return 0;
}

//receiving information from ffmpeg. extracting information from the audio file and we are storing it in sample and channel nodes 
static int asif_send_frame(AVCodecContext *avctx, const AVFrame *frame)
{
    asif_encoder_data *my_data;
    my_data = avctx->priv_data;
    int n_channels = my_data->channels;

    //Allocating space for channels.
    if(my_data->is_first_frame)
    {
        for(int i=0;i<my_data->channels;i++)
        {
            my_data->is_first_frame=false;
            struct channel_node *temp;
            temp->head_data = NULL;
            temp->tail_data = NULL;
            calloc(1, sizeof(temp));  //allocates the space for channel node, sets all bytes to 0.
            my_data->channel_r[i]= temp;
        }
    }
    else
    {
        for(int i=my_data->channels;i<20;i++)
        {
            my_data->channel_r[i] = NULL;
        }
    }
    //Allocating space for sample nodes.
    for(int i=0;i<my_data->channels;i++)
    {
        for(int j=0;j<frame->nb_samples;j++)
        {
            
            uint8_t *sample = frame->extended_data[i];   //TODO:Pointer?
            if(my_data->channel_r[i] = NULL)
            {
                struct sample_node *node;
                calloc(1, sizeof(node));
                for(int k=0;k<100;i++) // for every space inside sample node, set the data value to 0
                {
                    node->data[k]=0;
                }
                node->next=NULL;
                my_data->channel_r[i]->head_data = node;
                my_data->channel_r[i]->tail_data = node;
            }
            //Put values into sample nodes.
            //When the sample node fills up, create another sample node.
            //Update the tail.
            struct sample_node* node =  my_data->channel_r[i]->tail_data;
            if(node->counter == 99)
            {
                
                struct sample_node *newNode;
                calloc(1, sizeof(newNode));
                for(int k=0;k<100;i++) // for every space inside sample node, set the data value to 0
                {
                    newNode->data[k]=0;
                }
                newNode->next=NULL;
                my_data->channel_r[i]->head_data = newNode;
                my_data->channel_r[i]->tail_data = newNode;
                node-> next = newNode;
                int delta = sample - node->data[99];//computing the new deltas
                if(delta < -128)
                    delta = -128;
                else if(delta >127)
                    delta = 127;

                newNode-> data[node->counter] = delta; 
                newNode->counter++;
            }
            else
            {
                int delta = sample - node->data[99];//computing the new deltas
                if(delta < -128)
                    delta = -128;
                else if(delta >127)
                    delta = 127;
                node-> data[node->counter] = delta; 
                node->counter++;
            }
        }
    }

    return 0;
}

static int asif_receive_packet(AVCodecContext *avctx, AVPacket *avpkt)
{
    int ret;
    asif_encoder_data *my_data;
    my_data = avctx->priv_data;
    int packetSize = 14 + avctx->frame_size;
    if ((ret = ff_alloc_packet2(avctx, avpkt, packetSize, packetSize)) < 0)
        return ret;
    uint8_t packet;
    bytestream_put_buffer( &packet, my_data->asif, 4 ); //the stuff we will be putting in to the file
    bytestream_put_le32( &packet, my_data->freq); //the stuff we will be putting in to the file
    bytestream_put_le16( &packet, my_data->channels);
    bytestream_put_le32( &packet, my_data->total_samples);

    for(int i =0; i < my_data->channels; i++)
    {
        struct sample_node* currentNode = my_data->channel_r[i]->head_data;
        while(currentNode != NULL)
        {
            for(int j=0; j < 100 ; j++)
            {
                bytestream_put_byte( &packet, currentNode->data[j]);
            }
            currentNode = currentNode->next;
        }
    }
    
    return 0;
}


static av_cold int asif_encode_close(AVCodecContext *avctx)
{
    av_log(avctx, AV_LOG_INFO, "Executing inside of asif encoder, asif_encoder_close\n");
    asif_encoder_data *my_data;
    my_data = avctx->priv_data;
    for(int i =0; i < my_data->channels; i++)
    {
        struct sample_node* currentNode = my_data->channel_r[i]->head_data;

        while(currentNode != NULL)
        {
            struct sample_node* delNode = currentNode;
            currentNode = currentNode-> next;
            free(delNode);
        }
        free(my_data->channel_r[i]);
    }
    return 0;
}

AVCodec ff_asif_encoder = {
    .name           = "asif",
    .priv_data_size  = sizeof(asif_encoder_data),
    .long_name      = NULL_IF_CONFIG_SMALL("Asif encoder"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ASIF,
    .init           = asif_encode_init,
    .send_frame     = asif_send_frame,
    .receive_packet = asif_receive_packet,
    .close          = asif_encode_close,
    .capabilities   = AV_CODEC_CAP_SMALL_LAST_FRAME,
    .sample_fmts    = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_U8P,
                                                     AV_SAMPLE_FMT_NONE },
};



