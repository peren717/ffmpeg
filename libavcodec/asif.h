#include "avcodec.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include "libavutil/attributes.h"
#include "libavutil/float_dsp.h"
#include "bytestream.h"
#include "internal.h"
#include "mathops.h"


static void asif_tableinit(void);