//
// Created by hjOS on 2022/6/24.
//

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define NULL 0
#define PRINTF_DEBUG
#define BOX_TYPE_FTYPE "ftyp"
#define BOX_TYPE_MOOV "moov"
#define BOX_TYPE_MVHD "mvhd"
#define BOX_TYPE_TRAK "trak"
#define BOX_TYPE_TKHD "tkhd"
#define BOX_TYPE_EDTS "edts"
#define BOX_TYPE_MDIA "mdia"
#define BOX_TYPE_MDHD "mdhd"
#define BOX_TYPE_HDLR "hdlr"
#define BOX_TYPE_MINF "minf"
#define BOX_TYPE_VMHD "vmhd"
#define BOX_TYPE_DINF "dinf"
#define BOX_TYPE_DREF "dref"
#define BOX_TYPE_STBL "stbl"
#define BOX_TYPE_STSD "stsd"
#define BOX_TYPE_STTS "stts"
#define BOX_TYPE_STSS "stss"
#define BOX_TYPE_STSC "stsc"
#define BOX_TYPE_STSZ "stsz"
#define BOX_TYPE_STCO "stco"
#define BOX_TYPE_UDTA "udta"
#define MAX_BOX_SIZE_LEN 4
#define MAX_BOX_TYPE_LEN 4
#define MAX_HANDLER_TYPE_LEN 4
#define MAX_FTYP_BRABDS_LEN 4
#define MAX_FTYP_BRABDS_NUM 4
#define MAX_STTS_ENTRY_NUM 8
#define MAX_STSS_ENTRY_NUM 8
#define MAX_STSC_ENTRY_NUM 100
#define MAX_STSZ_ENTRY_NUM 100 /* now parse 100 frame */
#define MAX_STCO_ENTRY_NUM 100
#define MAX_MVHD_RESERVED_LEN 10
#define MAX_PRE_DEFINE_LEN 24
#define MAX_MATRIX_LEN 36
#define MAX_HDLR_NAME_LEN 100
typedef struct t_box_header
{
    int boxSize;
    unsigned char boxType[MAX_BOX_TYPE_LEN+1];
    long largeBoxSize; /* if boxSize=1 use, if boxSize=0, end of file */
} T_BOX_HEADER;
/********************************************************************************************
 55 **                            File Type Box (ftyp): file type, 表明文件类型
 56 **
 57 --------------------------------------------------------------------------------------------
 58 **        字段名称            　　|    长度(bytes)   |        有关描述
 59 --------------------------------------------------------------------------------------------
 60 **        boxsize               |    4            |        box的长度
 61 **        boxtype               |    4            |        box的类型
 62 **        major_brand           |    4            |
 63 **        minor_version         |    4            |        版本号
 64 **        compatible_brands     |    4 * N        |        本文件遵从的多种协议(ismo, iso2, mp41)
 65 ********************************************************************************************/
typedef struct t_box4ftyp_brand
{
    unsigned char brands[MAX_FTYP_BRABDS_LEN+1];
} T_BOX4FTYP_BRAN;
typedef struct t_box4ftyp
{
    unsigned char major_brand[MAX_FTYP_BRABDS_LEN+1];
    int minor_version;
    T_BOX4FTYP_BRAN compatible_brands[MAX_FTYP_BRABDS_NUM];
} T_BOX4FTYP;
/************************************************************************************************************
 81 **                                            mvhd: movie header, 文件的总体信息: 时长, 创建时间等
 82 **
 83 --------------------------------------------------------------------------------------------
 84 **        字段名称            　　|    长度(bytes)   |        有关描述
 85 --------------------------------------------------------------------------------------------
 86 **        boxsize               |    4            |        box的长度
 87 **        boxtype               |    4            |        box的类型
 88 **        version               |    1            |        box版本，0或1，一般为0（以下字节数均按version = 0）
 89 **        flags                 |    3            |
 90 **        creation time         |    4            |        创建时间（相对于UTC时间1904 - 01 - 01零点的秒数）
 91 **        modification time     |    4            |        修改时间
 92 **        time scale            |    4            |        文件媒体在1秒时间内的刻度值，可以理解为1秒长度的时间单元数
 93 **        duration              |    4            |        该track的时间长度，用duration和time scale值可以计算track时长
 94 **        rate                  |    4            |        推荐播放速率，高16位和低16位分别为小数点整数部分和小数部分，即[16.16] 格式.该值为1.0 (0x00010000)
 95 **        volume                |    2            |        与rate类似，[8.8] 格式，1.0（0x0100）表示最大音量
 96 **        reserved              |    10           |        保留位
 97 **        matrix                |    36           |        视频变换矩阵
 98 **        pre-defined           |    24           |
 99 **        next track id         |    4            |        下一个track使用的id号
100 **
101 if (version==1)
102 {
103     unsigned int(64) creation_time;
104     unsigned int(64) modification_time;
105     unsigned int(32) timescale;
106     unsigned int(64) duration;
107 }
108 else
109 {
110     unsigned int(32) creation_time;
111     unsigned int(32) modification_time;
112     unsigned int(32) timescale;
113     unsigned int(32) duration;
114 }
115 ************************************************************************************************************/
typedef struct t_box4mvhd
{
    int creation_time;
    int modification_time;
    int timescale;
    int duration;
    float rate;
    float volume;
    int next_track_id;
} T_BOX4MVHD;
/************************************************************************************************************
128 **                                        tkhd: track header, track的总体信息, 如时长, 宽高等
129 **
130 -------------------------------------------------------------------------------------------------------------
131 **        字段名称            　　 |    长度(bytes)   |        有关描述
132 -------------------------------------------------------------------------------------------------------------
133 **        boxsize                |    4            |        box的长度
134 **        boxtype                |    4            |        box的类型
135 **        version                |    1            |        box版本，0或1，一般为0。（以下字节数均按version = 0）
136 **        flags                  |    3            |        按位或操作结果值，预定义如下;
137                                                     　　　　 0x000001 track_enabled，否则该track不被播放；
138                                                     　　　　 0x000002 track_in_movie，表示该track在播放中被引用；
139                                                     　　　　 0x000004 track_in_preview，表示该track在预览时被引用。
140                                                     　　　　 一般该值为7，如果一个媒体所有track均未设置track_in_movie和track_in_preview,将被理解为所有track均设置了这两项;
141                                                     　　　　 对于hint track，该值为0;
142 **        creation_time          |    4            |        创建时间（相对于UTC时间1904 - 01 - 01零点的秒数）
143 **        modification_time      |    4            |        修改时间
144 **        track_id               |    4            |        id号 不能重复且不能为0
145 **        reserved               |    4            |        保留位
146 **        duration               |    4            |        track的时间长度
147 **        reserved               |    8            |        保留位
148 **        layer                  |    2            |        视频层，默认为0，值小的在上层
149 **        alternate_group        |    2            |        track分组信息，默认为0表示该track未与其他track有群组关系
150 **        volume                 |    2            |        [8.8] 格式，如果为音频track，1.0（0x0100）表示最大音量；否则为0
151 **        reserved               |    2            |        保留位
152 **        matrix                 |    36           |        视频变换矩阵
153 **        width                  |    4            |        宽
154 **        height                 |    4            |        高，均为[16.16] 格式值 与sample描述中的实际画面大小比值，用于播放时的展示宽高
155 if (version==1)
156 {
157     unsigned int(64) creation_time;
158     unsigned int(64) modification_time;
159     unsigned int(32) track_ID;
160     const unsigned int(32) reserved = 0;
161     unsigned int(64) duration;
162 }
163 else
164 {
165     unsigned int(32) creation_time;
166     unsigned int(32) modification_time;
167     unsigned int(32) track_ID;
168     const unsigned int(32) reserved = 0;
169     unsigned int(32) duration;
170 }
171 ************************************************************************************************************/
typedef struct t_box4tkhd
{
    int flags;
    int creation_time;
    int modification_time;
    int track_id;
    int duration;
    int layer;
    int alternate_group;
    float volume;
    float width;
    float height;
} T_BOX4TKHD;
/************************************************************************************************************
187 **                                        mdhd: 包含了了该track的总体信息, mdhd和tkhd 内容大致都是一样的.
188 **
189 -------------------------------------------------------------------------------------------------------------
190 **        字段名称            　　|　　    长度(bytes)　　　|        有关描述
191 -------------------------------------------------------------------------------------------------------------
192 **        boxsize               |    4            　　  |        box的长度
193 **        boxtype               |    4            　　  |        box的类型
194 **        version               |    1　　　　　　　　　|        box版本0或1 一般为0 (以下字节数均按version=0)
195 **        flags                 |    3            　　  |
196 **        creation_time         |    4            　　  |        创建时间（相对于UTC时间1904 - 01 - 01零点的秒数）
197 **        modification_time     |    4            　　  |        修改时间
198 **        time_scale            |    4            　　  |
199 **        duration              |    4            　　　|        track的时间长度
200 **        language              |    2            　　　|        媒体语言码,最高位为0 后面15位为3个字符[见ISO 639-2/T标准中定义]
201 **        pre-defined           |    2            　　  |        保留位
202
203 ** tkhd通常是对指定的track设定相关属性和内容, 而mdhd是针对于独立的media来设置的, 一般情况下二者相同.
204 ************************************************************************************************************/
typedef struct t_box4mdhd
{
    int creation_time;
    int modification_time;
    int timescale;
    int duration;
    short language;
} T_BOX4MDHD;
/************************************************************************************************************
215 **                                        hdlr: Handler Reference Box, 媒体的播放过程信息, 该box也可以被包含在meta box(meta)中
216 **
217 -------------------------------------------------------------------------------------------------------------
218 **        字段名称            　　 |    长度(bytes)    |        有关描述
219 -------------------------------------------------------------------------------------------------------------
220 **        boxsize                |    4             |        box的长度
221 **        boxtype                |    4             |        box的类型
222 **        version                |    1             |        box版本0或1 一般为0 (以下字节数均按version=0)
223 **        flags                  |    3             |
224 **        pre-defined            |    4             |
225 **        handler type           |    4             |        在media box中，该值为4个字符
226                                                     　　　　　　"vide"— video track
227                                                     　　　　　　"soun"— audio track
228                                                     　　　　　　"hint"— hint track
229 **        reserved               |    12            |
230 **        name                   |    不定           |        track type name，以‘’结尾的字符串
231 ************************************************************************************************************/
typedef struct t_box4hdlr
{
    unsigned char handler_type[MAX_HANDLER_TYPE_LEN+1];
    unsigned char name[MAX_HDLR_NAME_LEN+1];
} T_BOX4HDLR;
/************************************************************************************************************
239 **                                        vmhd: Video Media Header Box
240 **
241 -------------------------------------------------------------------------------------------------------------
242 **        字段名称            |    长度(bytes)    |        有关描述
243 -------------------------------------------------------------------------------------------------------------
244 **        boxsize                |    4            |        box的长度
245 **        boxtype                |    4            |        box的类型
246 **        version                |    1            |        box版本0或1 一般为0 (以下字节数均按version=0)
247 **        flags                     |    3            |
248 **        graphics_mode          |    4            |        视频合成模式，为0时拷贝原始图像，否则与opcolor进行合成
249 **        opcolor                |    2 ×3         |        ｛red，green，blue｝
250
251 "vide"—vmhd 视频
252 "soun"— smhd 音频
253 "hint"—hmhd 忽略
254 ************************************************************************************************************/
typedef struct t_box4vmhd
{
    int graphics_mode;
} T_BOX4VMHD;
/************************************************************************************************************
261 **                                        dref: data reference box
262 **
263 -------------------------------------------------------------------------------------------------------------
264 **        字段名称            　　 |    长度(bytes)    |        有关描述
265 -------------------------------------------------------------------------------------------------------------
266 **        boxsize                |    4             |        box的长度
267 **        boxtype                |    4             |        box的类型
268 **        version                |    1             |        box版本0或1 一般为0 (以下字节数均按version=0)
269 **        flags                  |    3             |
270 **        entry count            |    4             |         "url"或"urn"表的元素个数
271 **        "url"或"urn"列表       |    不定          |
272
273 ** "dref"下会包含若干个"url"或"urn", 这些box组成一个表, 用来定位track数据. 简单的说, track可以被分成若干段,
274    每一段都可以根据"url"或"urn"指向的地址来获取数据, sample描述中会用这些片段的序号将这些片段组成一个完整的track.
275    一般情况下, 当数据被完全包含在文件中时, "url"或"urn"中的定位字符串是空的.
276 ************************************************************************************************************/
typedef struct t_box4dref
{
    int entry_count;
} T_BOX4DREF;
/************************************************************************************************************
283 **                                        stsd: Sample Description Box
284 **
285 -------------------------------------------------------------------------------------------------------------
286 **        字段名称            　　 |    长度(bytes)    |        有关描述
287 -------------------------------------------------------------------------------------------------------------
288 **        boxsize                |    4             |        box的长度
289 **        boxtype                |    4             |        box的类型
290 **        version                |    1             |        box版本0或1 一般为0 (以下字节数均按version=0)
291 **        entry count            |    4             |         "url"或"urn"表的元素个数
292
293 ** box header和version字段后会有一个entry count字段, 根据entry的个数, 每个entry会有type信息, 如"vide", "sund"等,
294    根据type不同sample description会提供不同的信息, 例如对于video track, 会有"VisualSampleEntry"类型信息,
295    对于audio track会有"AudioSampleEntry"类型信息. 视频的编码类型, 宽高, 长度, 音频的声道, 采样等信息都会出现在这个box中
296 ************************************************************************************************************/
typedef struct t_box4stsd
{
    int entry_count;
    //TODO
} T_BOX4STSD;
/************************************************************************************************************
305 **                                        stts: Time To Sample Box
306 **
307 -------------------------------------------------------------------------------------------------------------
308 **        字段名称            　　 |    长度(bytes)    |        有关描述
309 -------------------------------------------------------------------------------------------------------------
310 **        boxsize                |    4             |        box的长度
311 **        boxtype                |    4             |        box的类型
312 **        version                |    1             |        box版本，0或1，一般为0（以下字节数均按version = 0）
313 **        flags                  |    3             |
314 **        entry count            |    4             |         sample_count和sample_delta的个数
315 **        sample_count           |    4             |
316 **        sample_delta           |    4             |
317
318 ** "stts”"存储了sample的duration, 描述了sample时序的映射方法, 我们通过它可以找到任何时间的sample. "stts"可以
319    包含一个压缩的表来映射时间和sample序号, 用其他的表来提供每个sample的长度和指针. 表中每个条目提供了在同一个
320    时间偏移量里面连续的sample序号, 以及samples的偏移量. 递增这些偏移量, 就可以建立一个完整的time to sample表.
321
322    例: 说明该视频包含87帧数据(sample_count), 每帧包含512个采样(sample_delta). 总共512*87=44544个采样,
323        和我们前面mdhd box的Duration完全一致。
324        Duration/TimeScale = 44544/12288 = 3.625s, 正是我们的视频播放长度.
325        12288/512 = 24 p/s (帧率)
326 ************************************************************************************************************/
typedef struct t_box4stts_entry
{
    int sample_count;
    int sample_delta;
} T_BOX4STTS_ENTRY;
typedef struct t_box4stts
{
    int entry_count;
    T_BOX4STTS_ENTRY entrys[MAX_STTS_ENTRY_NUM];
} T_BOX4STTS;
/************************************************************************************************************
341 **                                        stss: Sync Sample Box
342 **
343 -------------------------------------------------------------------------------------------------------------
344 **        字段名称            　　 |    长度(bytes)    |        有关描述
345 -------------------------------------------------------------------------------------------------------------
346 **        boxsize                |    4             |        box的长度
347 **        boxtype                |    4             |        box的类型
348 **        version                |    1             |        box版本，0或1，一般为0（以下字节数均按version = 0）
349 **        flags                  |    3             |
350 **        entry count            |    4             |         sample_num的个数
351 **        sample_num                |    4             |
352
353 ** "stss"确定media中的关键帧. 对于压缩媒体数据, 关键帧是一系列压缩序列的开始帧, 其解压缩时不依赖以前的帧,
354    而后续帧的解压缩将依赖于这个关键帧. "stss"可以非常紧凑的标记媒体内的随机存取点, 它包含一个sample序号表,
355    表内的每一项严格按照sample的序号排列, 说明了媒体中的哪一个sample是关键帧. 如果此表不存在, 说明每一个sample
356    都是一个关键帧, 是一个随机存取点.
357 ************************************************************************************************************/
typedef struct t_box4stss_entry
{
    int sample_num;
} T_BOX4STSS_ENTRY;
typedef struct t_box4stss
{
    int entry_count;
    T_BOX4STSS_ENTRY entrys[MAX_STSS_ENTRY_NUM];
} T_BOX4STSS;
/************************************************************************************************************
371 **                                        stsc: Sample To Chunk Box
372 **
373 -------------------------------------------------------------------------------------------------------------
374 **        字段名称            　　 |    长度(bytes)    |        有关描述
375 -------------------------------------------------------------------------------------------------------------
376 **        boxsize                |    4             |        box的长度
377 **        boxtype                |    4             |        box的类型
378 **        version                |    1             |        box版本，0或1，一般为0（以下字节数均按version = 0）
379 **        flags                  |    3             |
380 **        entry count            |    4             |         entry的个数
381 **        first_chunk            |    4             |
382 **        samples_per_chunk      |    4             |
383 **        sample_des_index       |    4             |
384
385 ** 用chunk组织sample可以方便优化数据获取, 一个thunk包含一个或多个sample. "stsc"中用一个表描述了sample与chunk的映射关系,
386    查看这张表就可以找到包含指定sample的thunk, 从而找到这个sample.
387 ************************************************************************************************************/
typedef struct t_box4stsc_entry
{
    int first_chunk;
    int samples_per_chunk;
    int sample_description_index;
} T_BOX4STSC_ENTRY;
typedef struct t_box4stsc
{
    int entry_count;
    T_BOX4STSC_ENTRY entrys[MAX_STSC_ENTRY_NUM];
} T_BOX4STSC;
/************************************************************************************************************
403 **                                        stsz: Sample To Chunk Box
404 **
405 -------------------------------------------------------------------------------------------------------------
406 **        字段名称            　　 |    长度(bytes)    |        有关描述
407 -------------------------------------------------------------------------------------------------------------
408 **        boxsize                |    4             |        box的长度
409 **        boxtype                |    4             |        box的类型
410 **        version                |    1             |        box版本，0或1，一般为0（以下字节数均按version = 0）
411 **        flags                  |    3             |
412 **        sample_size            |    4             |
413 **        sample_count           |    4             |         entry的个数
414 **        entry_size             |    4             |
415
416 **  "stsz"定义了每个sample的大小, 包含了媒体中全部sample的数目和一张给出每个sample大小的表. 这个box相对来说体积是比较大的.
417 ************************************************************************************************************/
typedef struct t_box4stsz_entry
{
    int entry_size;
} T_BOX4STSZ_ENTRY;
typedef struct t_box4stsz
{
    int sample_size;
    int sample_count;
    T_BOX4STSZ_ENTRY entrys[MAX_STSZ_ENTRY_NUM];
} T_BOX4STSZ;
/************************************************************************************************************
432 **                                        stco: Chunk Offset Box
433 **
434 -------------------------------------------------------------------------------------------------------------
435 **        字段名称            　　 |    长度(bytes)    |        有关描述
436 -------------------------------------------------------------------------------------------------------------
437 **        boxsize                |    4             |        box的长度
438 **        boxtype                |    4             |        box的类型
439 **        version                |    1             |        box版本，0或1，一般为0（以下字节数均按version = 0）
440 **        flags                  |    3             |
441 **        entry_count            |    4             |
442 **        chunk_offset           |    4             |
443
444 **  "stco"定义了每个thunk在媒体流中的位置, sample的偏移可以根据其他box推算出来. 位置有两种可能, 32位的和64位的,
445     后者对非常大的电影很有用. 在一个表中只会有一种可能, 这个位置是在整个文件中的, 而不是在任何box中的.
446     这样做就可以直接在文件中找到媒体数据, 而不用解释box. 需要注意的是一旦前面的box有了任何改变, 这张表都要重新建立, 因为位置信息已经改变了.
447 ************************************************************************************************************/
typedef struct t_box4stco_entry
{
    int chunk_offset;
} T_BOX4STCO_ENTRY;
typedef struct t_box4stco
{
    int entry_count;
    T_BOX4STCO_ENTRY entrys[MAX_STCO_ENTRY_NUM];
} T_BOX4STCO;
typedef struct t_box
{
    T_BOX_HEADER boxHeader;
    unsigned char *boxData;
} T_BOX;
static void DealBox4ftyp(const T_BOX *box)
{
    int i = 0;
    int j = 0;
    int brandsNum = 0;
    T_BOX4FTYP box4ftyp = {0};
    memset(&box4ftyp, 0x0, sizeof(T_BOX4FTYP));
    memcpy(box4ftyp.major_brand, box->boxData, 4);
    box4ftyp.major_brand[MAX_FTYP_BRABDS_LEN] = '\0';
    box4ftyp.minor_version =  box->boxData[4] << 24 | box->boxData[5] << 16 | box->boxData[6] << 8 | box->boxData[7];
    brandsNum = (box->boxHeader.boxSize - MAX_BOX_SIZE_LEN - MAX_BOX_TYPE_LEN - MAX_FTYP_BRABDS_LEN - 4) / 4;
    /* 1. if not have '', 每个brands的内存是连续的, 导致打印时后面的每4个数据都会加到前面;
485        2. unsigned char brands[MAX_FTYP_BRABDS_LEN+1]; 可解决, 此时也不必加'', 但需初始化;
486        3. 因此字符串最好定义+1并赋'';
487        4. 复现: unsigned char brands[MAX_FTYP_BRABDS_LEN]
488     */
    for (i=0; i<brandsNum; i++)
    {
        memcpy(box4ftyp.compatible_brands[i].brands, box->boxData+MAX_FTYP_BRABDS_LEN+4+4*i, 4);
        box4ftyp.compatible_brands[i].brands[MAX_FTYP_BRABDS_LEN] = '\0';
    }
#ifdef PRINTF_DEBUG
    printf("tmajor_brand: %s, minor_version: %d, compatible_brands: ", box4ftyp.major_brand, box4ftyp.minor_version);
    for (i=0; i<brandsNum; i++)
    {
        if (i==brandsNum-1)
        {
            printf("%s", box4ftyp.compatible_brands[i].brands);
        }
        else
        {
            printf("%s,", box4ftyp.compatible_brands[i].brands);
        }
    }
    printf("\n");
#endif
}
static void DealBox4mvhd(const unsigned char *mvhdData)
{
    unsigned char *data = NULL;
    T_BOX4MVHD box4mvhd = {0};
    memset(&box4mvhd, 0x0, sizeof(T_BOX4MVHD));
    data = (unsigned char *)mvhdData;
    data += 4;
    box4mvhd.creation_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mvhd.modification_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mvhd.timescale = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mvhd.duration = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    //box4mvhd.rate = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    box4mvhd.rate = (data[0] << 8 | data[1]) + (data[2] << 8 | data[3]);
    data += 4;
    //box4mvhd.volume = data[0] << 8 | data[1];
    box4mvhd.volume = data[0] + data[1];
    data += 2;
    data += (MAX_MVHD_RESERVED_LEN + MAX_PRE_DEFINE_LEN + MAX_MATRIX_LEN);
    box4mvhd.next_track_id = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
#ifdef PRINTF_DEBUG
    printf("ttcreation_time: %d, modification_time: %d, timescale: %d, duration: %d, rate: %f, volume: %f, next_track_id: %d\n",
           box4mvhd.creation_time, box4mvhd.modification_time, box4mvhd.timescale, box4mvhd.duration, box4mvhd.rate, box4mvhd.volume, box4mvhd.next_track_id);
#endif
}
static void DealBox4tkhd(const unsigned char *tkhdData)
{
    unsigned char *data = NULL;
    T_BOX4TKHD box4tkhd = {0};
    memset(&box4tkhd, 0x0, sizeof(box4tkhd));
    data = (unsigned char *)tkhdData;
    box4tkhd.flags = data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4tkhd.creation_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4tkhd.modification_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4tkhd.track_id = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    data += 4; /* 4 reserved */
    box4tkhd.duration = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    data += 8; /* 8 reserved */
    box4tkhd.layer = data[0] << 8 | data[1];
    data += 2;
    box4tkhd.alternate_group = data[0] << 8 | data[1];
    data += 2;
    box4tkhd.volume = data[0] + data[1];
    data += 2;
    data += 2;
    data += 36;
    box4tkhd.width = (data[0] << 8 | data[1]) + (data[2] << 8 | data[3]);
    data += 4;
    box4tkhd.height = (data[0] << 8 | data[1]) + (data[2] << 8 | data[3]);
#ifdef PRINTF_DEBUG
    printf("tttflags: %d, creation_time: %d, modification_time: %d, track_id: %d, duration: %d, layer: %d, alternate_group: %d, volume: %f, width: %f, height: %f\n",
           box4tkhd.flags, box4tkhd.creation_time, box4tkhd.modification_time, box4tkhd.track_id, box4tkhd.duration, box4tkhd.layer, box4tkhd.alternate_group, box4tkhd.volume, box4tkhd.width, box4tkhd.height);
#endif
}
static void DealBox4dref(const T_BOX *box)
{
    // TODO
}
static void DealBox4dinf(const T_BOX *box)
{    int boxSize = 0;
    int dinfDataSize = 0;
    unsigned char *dinfData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    T_BOX drefBox = {0};
    dinfData = box->boxData;
    dinfDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (dinfDataSize > 0)
    {
        boxSize = dinfData[0] << 24 | dinfData[1] << 16 | dinfData[2] << 8 | dinfData[3];
        memcpy(boxType, dinfData+MAX_BOX_SIZE_LEN, 4);
#ifdef PRINTF_DEBUG
        printf("ttttt****BOX: Layer6****\n");
        printf("ttttttsize: %d\n", boxSize);
        printf("tttttttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_DREF))
        {
            memset(&drefBox, 0x0, sizeof(T_BOX));
            drefBox.boxHeader.boxSize = boxSize;
            memcpy(drefBox.boxHeader.boxType, boxType, strlen(boxType));
            drefBox.boxData = (unsigned char*)malloc(boxSize);
            if (drefBox.boxData)
            {
                memcpy(drefBox.boxData, dinfData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4dref((const T_BOX*)&drefBox);
                free(drefBox.boxData);
                drefBox.boxData = NULL;
            }
        }
        dinfData += boxSize;
        dinfDataSize -= boxSize;
    }
}
static void DealBox4stts(const unsigned char *sttsData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4STTS box4stts = {0};
    memset(&box4stts, 0x0, sizeof(box4stts));
    data = (unsigned char *)sttsData;
    data += 4;
    box4stts.entry_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    for (i=0; i<box4stts.entry_count; i++)
    {
        if (i == MAX_STTS_ENTRY_NUM)
        {
            break;
        }
        box4stts.entrys[i].sample_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
        box4stts.entrys[i].sample_delta = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
    }
#ifdef PRINTF_DEBUG
    printf("tttentry_count: %d, [sample_count, sample_delta]: ", box4stts.entry_count);
    if (box4stts.entry_count>MAX_STTS_ENTRY_NUM)
    {
        box4stts.entry_count = MAX_STTS_ENTRY_NUM;
    }
    for (i=0; i<box4stts.entry_count; i++)
    {
        if (i>0)
        {
            printf(", ");
        }
        printf("[%d, %d]", box4stts.entrys[i].sample_count, box4stts.entrys[i].sample_delta);
    }
    if (box4stts.entry_count==MAX_STTS_ENTRY_NUM)
    {
        printf("...(just show %d now)", MAX_STTS_ENTRY_NUM);
    }
    printf("\n");
#endif
}
static void DealBox4stss(const unsigned char *stssData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4STSS box4stss = {0};
    memset(&box4stss, 0x0, sizeof(box4stss));
    data = (unsigned char *)stssData;
    data += 4;
    box4stss.entry_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    for (i=0; i<box4stss.entry_count; i++)
    {
        if (i == MAX_STSS_ENTRY_NUM)
        {
            break;
        }
        box4stss.entrys[i].sample_num = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
    }
#ifdef PRINTF_DEBUG
    printf("tttentry_count: %d, sample_num: ", box4stss.entry_count);
    if (box4stss.entry_count>MAX_STSS_ENTRY_NUM)
    {
        box4stss.entry_count = MAX_STSS_ENTRY_NUM;
    }
    for (i=0; i<box4stss.entry_count; i++)
    {
        if (i>0)
        {
            printf(", ");
        }
        printf("%d", box4stss.entrys[i].sample_num);
    }
    if (box4stss.entry_count==MAX_STSS_ENTRY_NUM)
    {
        printf("...(just show %d now)", MAX_STSS_ENTRY_NUM);
    }
    printf("\n");
#endif
}
static void DealBox4stsc(const unsigned char *stscData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4STSC box4stsc = {0};
    memset(&box4stsc, 0x0, sizeof(box4stsc));
    data = (unsigned char *)stscData;
    data += 4;
    box4stsc.entry_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    for (i=0; i<box4stsc.entry_count; i++)
    {
        if (i == MAX_STSC_ENTRY_NUM)
        {
            break;
        }
        box4stsc.entrys[i].first_chunk = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
        box4stsc.entrys[i].samples_per_chunk = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
        box4stsc.entrys[i].sample_description_index = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
    }
#ifdef PRINTF_DEBUG
    printf("tttentry_count: %d, [first_chunk, samples_per_chunk, sample_description_index]: ", box4stsc.entry_count);
    if (box4stsc.entry_count>MAX_STSC_ENTRY_NUM)
    {
        box4stsc.entry_count = MAX_STSC_ENTRY_NUM;
    }
    for (i=0; i<box4stsc.entry_count; i++)
    {
        if (i>0)
        {
            printf(", ");
        }
        printf("[%d, %d, %d]", box4stsc.entrys[i].first_chunk, box4stsc.entrys[i].samples_per_chunk, box4stsc.entrys[i].sample_description_index);
    }
    if (box4stsc.entry_count==MAX_STSC_ENTRY_NUM)
    {
        printf("...(just show %d now)", MAX_STSC_ENTRY_NUM);
    }
    printf("\n");
#endif
}
static void DealBox4stsz(const unsigned char *stszData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4STSZ box4stsz = {0};
    memset(&box4stsz, 0x0, sizeof(box4stsz));
    data = (unsigned char *)stszData;
    data += 4;
    box4stsz.sample_size = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4stsz.sample_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    for (i=0; i<box4stsz.sample_count; i++)
    {
        if (i == MAX_STSZ_ENTRY_NUM)
        {
            break;
        }
        box4stsz.entrys[i].entry_size = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
    }
#ifdef PRINTF_DEBUG
    printf("tttsample_size: %d, sample_count: %d, [entry_size]: ", box4stsz.sample_size, box4stsz.sample_count);
    if (box4stsz.sample_count>MAX_STSZ_ENTRY_NUM)
    {
        box4stsz.sample_count = MAX_STSZ_ENTRY_NUM;
    }
    for (i=0; i<box4stsz.sample_count; i++)
    {
        if (i>0)
        {
            printf(", ");
        }
        printf("[%d]", box4stsz.entrys[i].entry_size);
    }
    if (box4stsz.sample_count==MAX_STSZ_ENTRY_NUM)
    {
        printf("...(just show %d now)", MAX_STSZ_ENTRY_NUM);
    }
    printf("\n");
#endif
}
static void DealBox4stco(const unsigned char *stcoData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4STCO box4stco = {0};
    memset(&box4stco, 0x0, sizeof(box4stco));
    data = (unsigned char *)stcoData;
    data += 4;
    box4stco.entry_count = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    for (i=0; i<box4stco.entry_count; i++)
    {
        if (i == MAX_STCO_ENTRY_NUM)
        {
            break;
        }
        box4stco.entrys[i].chunk_offset = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        data += 4;
    }
#ifdef PRINTF_DEBUG
    printf("ttentry_count: %d, [chunk_offset]: ", box4stco.entry_count);
    if (box4stco.entry_count>MAX_STCO_ENTRY_NUM)
    {
        box4stco.entry_count = MAX_STCO_ENTRY_NUM;
    }
    for (i=0; i<box4stco.entry_count; i++)
    {
        if (i>0)
        {
            printf(", ");
        }
        printf("[%d]", box4stco.entrys[i].chunk_offset);
    }
    if (box4stco.entry_count==MAX_STCO_ENTRY_NUM)
    {
        printf("...(just show %d now)", MAX_STCO_ENTRY_NUM);
    }
    printf("\n");
#endif
}
static void DealBox4stbl(const T_BOX *box)
{
    int boxSize = 0;
    int stblDataSize = 0;
    unsigned char *stblData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    stblData = box->boxData;
    stblDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (stblDataSize > 0)
    {
        boxSize = stblData[0] << 24 | stblData[1] << 16 | stblData[2] << 8 | stblData[3];
        memcpy(boxType, stblData+MAX_BOX_SIZE_LEN, 4);
#ifdef PRINTF_DEBUG
        printf("ttttt****BOX: Layer6****\n");
        printf("ttttttsize: %d\n", boxSize);
        printf("tttttttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_STTS))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, stblData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stts(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_STSS))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, stblData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stss(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_STSC))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, stblData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stsc(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_STSZ))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, stblData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stsz(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_STCO))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, stblData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stco(data);
                free(data);
                data = NULL;
            }
        }
        stblData += boxSize;
        stblDataSize -= boxSize;
    }
}
static void DealBox4mdhd(const unsigned char *mdhdData)
{
    unsigned char *data = NULL;
    T_BOX4MDHD box4mdhd = {0};
    memset(&box4mdhd, 0x0, sizeof(box4mdhd));
    data = (unsigned char *)mdhdData;
    data += 4;
    box4mdhd.creation_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mdhd.modification_time = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mdhd.timescale = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mdhd.duration = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    data += 4;
    box4mdhd.language = data[0] << 8 | data[1];
#ifdef PRINTF_DEBUG
    //printf("tttcreation_time: %d, modification_time: %d, timescale: %d, duration: %d, language: %c%c%cn",
    //box4mdhd.creation_time, box4mdhd.modification_time, box4mdhd.timescale, box4mdhd.duration, (box4mdhd.language>>10&0x1f), (box4mdhd.language>>5&0x1f), (box4mdhd.language&0x1f));
    printf("ttttcreation_time: %d, modification_time: %d, timescale: %d, duration: %d, language:%d\n",
           box4mdhd.creation_time, box4mdhd.modification_time, box4mdhd.timescale, box4mdhd.duration, box4mdhd.language);
#endif
}
static void DealBox4hdlr(const unsigned char *hdlrData)
{
    int i = 0;
    unsigned char *data = NULL;
    T_BOX4HDLR box4hdlr = {0};
    memset(&box4hdlr, 0x0, sizeof(box4hdlr));
    data = (unsigned char *)hdlrData;
    data += 4;
    data += 4;
    memcpy(box4hdlr.handler_type, data, 4);
    box4hdlr.handler_type[MAX_HANDLER_TYPE_LEN] = '\0';
    data += 4;
    data += 12;
    while ('\0' != data[i])
    {
        i++;
    }
    memcpy(box4hdlr.name, data, i);
    box4hdlr.name[MAX_HDLR_NAME_LEN] = '\0';
#ifdef PRINTF_DEBUG
    printf("tttthandler_type: %s, name: %s\n", box4hdlr.handler_type, box4hdlr.name);
#endif
}
static void DealBox4vmdhd(const unsigned char *vmdhdData)
{
    // TODO
}
static void DealBox4minf(const T_BOX *box)
{    int boxSize = 0;
    int minfDataSize = 0;
    unsigned char *minfData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    T_BOX dinfBox = {0};
    T_BOX stblBox = {0};
    minfData = box->boxData;
    minfDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (minfDataSize > 0)
    {
        boxSize = minfData[0] << 24 | minfData[1] << 16 | minfData[2] << 8 | minfData[3];
        memcpy(boxType, minfData+MAX_BOX_SIZE_LEN, 4);
#ifdef PRINTF_DEBUG
        printf("tttt********BOX: Layer5********\n");
        printf("tttttsize: %d\n", boxSize);
        printf("ttttttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_VMHD))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, minfData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4vmdhd(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_DINF))
        {
            memset(&dinfBox, 0x0, sizeof(T_BOX));
            dinfBox.boxHeader.boxSize = boxSize;
            memcpy(dinfBox.boxHeader.boxType, boxType, strlen(boxType));
            dinfBox.boxData = (unsigned char*)malloc(boxSize);
            if (dinfBox.boxData)
            {
                memcpy(dinfBox.boxData, minfData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4dinf((const T_BOX*)&dinfBox);
                free(dinfBox.boxData);
                dinfBox.boxData = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_STBL))
        {
            memset(&stblBox, 0x0, sizeof(T_BOX));
            stblBox.boxHeader.boxSize = boxSize;
            memcpy(stblBox.boxHeader.boxType, boxType, strlen(boxType));
            stblBox.boxData = (unsigned char*)malloc(boxSize);
            if (stblBox.boxData)
            {
                memcpy(stblBox.boxData, minfData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4stbl((const T_BOX*)&stblBox);
                free(stblBox.boxData);
                stblBox.boxData = NULL;
            }
        }
        minfData += boxSize;
        minfDataSize -= boxSize;
    }
}
static void DealBox4mdia(const T_BOX *box)
{    int boxSize = 0;
    int mdiaDataSize = 0;
    unsigned char *mdiaData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    T_BOX minfBox = {0};
    mdiaData = box->boxData;
    mdiaDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (mdiaDataSize > 0)
    {
        boxSize = mdiaData[0] << 24 | mdiaData[1] << 16 | mdiaData[2] << 8 | mdiaData[3];
        memcpy(boxType, mdiaData+MAX_BOX_SIZE_LEN, 4);
#ifdef PRINTF_DEBUG
        printf("ttt************BOX: Layer4************\n");
        printf("ttttsize: %d\n", boxSize);
        printf("tttttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_MDHD))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, mdiaData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4mdhd(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_HDLR))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, mdiaData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4hdlr(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_MINF))
        {
            memset(&minfBox, 0x0, sizeof(T_BOX));
            minfBox.boxHeader.boxSize = boxSize;
            memcpy(minfBox.boxHeader.boxType, boxType, strlen(boxType));
            minfBox.boxData = (unsigned char*)malloc(boxSize);
            if (minfBox.boxData)
            {
                memcpy(minfBox.boxData, mdiaData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4minf((const T_BOX*)&minfBox);
                free(minfBox.boxData);
                minfBox.boxData = NULL;
            }
        }
        mdiaData += boxSize;
        mdiaDataSize -= boxSize;
    }
}
static void DealBox4trak(const T_BOX *box)
{
    int boxSize = 0;
    int trakDataSize = 0;
    unsigned char *trakData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    T_BOX mdiaBox = {0};
    trakData = box->boxData;
    trakDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (trakDataSize > 0)
    {
        boxSize = trakData[0] << 24 | trakData[1] << 16 | trakData[2] << 8 | trakData[3];
        memcpy(boxType, trakData+MAX_BOX_SIZE_LEN, 4);
#ifdef PRINTF_DEBUG
        printf("tt****************BOX: Layer3****************\n");
        printf("tttsize: %d\n", boxSize);
        printf("ttttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_TKHD))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, trakData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4tkhd(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_MDIA))
        {
            memset(&mdiaBox, 0x0, sizeof(T_BOX));
            mdiaBox.boxHeader.boxSize = boxSize;
            memcpy(mdiaBox.boxHeader.boxType, boxType, strlen(boxType));
            mdiaBox.boxData = (unsigned char*)malloc(boxSize);
            if (mdiaBox.boxData)
            {
                memcpy(mdiaBox.boxData, trakData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4mdia((const T_BOX*)&mdiaBox);
                free(mdiaBox.boxData);
                mdiaBox.boxData = NULL;
            }
        }
        trakData += boxSize;
        trakDataSize -= boxSize;
    }
}
static void DealBox4moov(const T_BOX *box)
{
    printf("hello\n");
    int boxSize = 0;
    int moovDataSize = 0;
    unsigned char *moovData = NULL;
    unsigned char *data = NULL;
    char boxType[MAX_BOX_TYPE_LEN+1] = {0};
    T_BOX trakBox = {0};
    moovData = box->boxData;
    moovDataSize = box->boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN;
    while (moovDataSize > 0)
    {
        boxSize = moovData[0] << 24 | moovData[1] << 16 | moovData[2] << 8 | moovData[3];
        memcpy(boxType, moovData+MAX_BOX_SIZE_LEN, 4);
        boxType[MAX_BOX_TYPE_LEN] = '\0';
#ifdef PRINTF_DEBUG
        printf("t********************BOX: Layer2********************\n");
        printf("ttsize: %d\n", boxSize);
        printf("tttype: %s\n", boxType);
#endif
        if (0 == strcmp(boxType, BOX_TYPE_MVHD))
        {
            data = (unsigned char*)malloc(boxSize);
            if (data)
            {
                memcpy(data, moovData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4mvhd(data);
                free(data);
                data = NULL;
            }
        }
        else if (0 == strcmp(boxType, BOX_TYPE_TRAK))
        {
            memset(&trakBox, 0x0, sizeof(T_BOX));
            trakBox.boxHeader.boxSize = boxSize;
            memcpy(trakBox.boxHeader.boxType, boxType, strlen(boxType));
            trakBox.boxData = (unsigned char*)malloc(boxSize);
            if (trakBox.boxData)
            {
                memcpy(trakBox.boxData, moovData+MAX_BOX_SIZE_LEN+MAX_BOX_TYPE_LEN, boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
                DealBox4trak((const T_BOX*)&trakBox);
                free(trakBox.boxData);
                trakBox.boxData = NULL;
            }
        }
        moovData += boxSize;
        moovDataSize -= boxSize;
    }
}
static void DealBox(const T_BOX *box)
{
#ifdef PRINTF_DEBUG
    printf("****************************BOX: Layer1****************************\n");
    printf("tsize: %d\n", box->boxHeader.boxSize);
    printf("ttype: %s\n", box->boxHeader.boxType);
#endif
    char tmptype[10] = "";
    for(int i = 0; i < 4; ++i)
        tmptype[i] = box->boxHeader.boxType[i];
    if (0 == strcmp(tmptype, BOX_TYPE_FTYPE))
    {
        DealBox4ftyp(box);
    }
    else if (0 == strcmp(tmptype, BOX_TYPE_MOOV))
    {
        DealBox4moov(box);
    }
}
int main(int argc, char *argv[])
{
    unsigned char boxSize[MAX_BOX_SIZE_LEN] = {0};
    T_BOX box = {0};
    if (2 != argc)
    {
        printf("Usage: mp4parse **.mp4\n");
        return -1;
    }
    int fd = open(argv[1], 0);
    if (fd < 0)
    {
        printf("open file[%s] error!\n", argv[1]);
        return -1;
    }
    while (1)
    {
        memset(&box, 0x0, sizeof(T_BOX));
        if (read(fd, boxSize, 1 * 4) <= 0)
        {
            break;
        }
        box.boxHeader.boxSize = boxSize[0] << 24 | boxSize[1] << 16 | boxSize[2] << 8 | boxSize[3];
        read(fd, box.boxHeader.boxType, 1 * 4);
        box.boxHeader.boxType[MAX_BOX_TYPE_LEN] = '\0';
        box.boxData = (unsigned char*)malloc(box.boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
        if (!box.boxData)
        {
            printf("malloc data error!\n");
            break;
        }
        read(fd, box.boxData, 1 * box.boxHeader.boxSize-MAX_BOX_SIZE_LEN-MAX_BOX_TYPE_LEN);
        /* deal box data */
        DealBox(&box);
        /* free box */
        free(box.boxData);
        box.boxData = NULL;
    }
    close(fd);
    return 0;
}