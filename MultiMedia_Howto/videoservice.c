//
//  videoservice.c
//  MultiMedia_Howto
//
//  Created by fengtianyu on 2022/5/5.
//

#include "videoservice.h"
#include "libavutil/avutil.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <unistd.h>
#include <libswresample/swresample.h>

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

static int video_recoding = 0;


/// 打开编码器
/// @param width 分辨率的宽度
/// @param height 分辨率的高度
/// @param enc_ctx 编码器上下文
static void open_encoder(int width, int height, AVCodecContext **enc_ctx) {
    
    // 获取编码器
    AVCodec *codec = NULL;
    codec = avcodec_find_decoder_by_name("libx264");
    if (!codec) {
        printf("没有获取到编码器! \n");
        exit(1);
    }
    
    // 开辟编码器的上下文环境
    *enc_ctx = avcodec_alloc_context3(codec);
    if (!enc_ctx) {
        printf("开辟编码器上下文环境失败! \n");
        exit(1);
    }
    
    // 设置编码的SPS参数
    (*enc_ctx)->profile = FF_PROFILE_H264_HIGH_444; // 设置profile参数，FF_PROFILE_H264_HIGH_444:最高级别,支持的压缩特性最多 (required)
    (*enc_ctx)->level = 50; // Level是5.0 ，5.0支持的清晰度就很高，支持720P (required)
    
    // 设置编码的分辨率
    (*enc_ctx)->width = width; // (required)
    (*enc_ctx)->height = height; // (required)
    
    // 编码中GOP的设置
    (*enc_ctx)->gop_size = 250; // 设置的值小了，I帧就很多，码流就很大。设置的大了，I帧就很少，但是如果传输的时候如果I帧丢了，等下一个I帧要等的时间比较长。 (required)
    (*enc_ctx)->keyint_min = 25; // 这一组GOP中最小插入I帧的间隔。如果设置的GOP的时间特别长，在这个过程中有很多的图像有很多的变化的时候，达到这个最小值的时候也会自动的插入一个I帧 (optional)
    
    // 设置B帧的数量
    (*enc_ctx)->max_b_frames = 3; // 为了减少码流的大小，可以增加B帧。一般不超过3帧。 (optional)
    (*enc_ctx)->has_b_frames = 1; // (optional)
    
    // 设置参考帧的数量，让解码器解码的时候知道数组中有多少帧 (optional)
    (*enc_ctx)->refs = 3; // 一般为 3 4 5，参考帧越多，还原性越好，处理的越慢；不设置的话编码器有默认值
    
    // 设置输入的YUV格式 (required)
    (*enc_ctx)->pix_fmt = AV_PIX_FMT_YUVA420P;
    
    // 设置码率 (required)
    (*enc_ctx)->bit_rate = 600000; // 600 kbps，根据640*480*25计算出来码率
    
    // 设置帧率 (required)
    (*enc_ctx)->time_base = (AVRational){1, 25}; // 帧与帧之间的间隔，每帧时间25分之1秒
    (*enc_ctx)->framerate = (AVRational){25, 1};  // 帧率每秒25帧
    
    int ret = avcodec_open2((*enc_ctx), codec, NULL);
    if (ret < 0) {
        printf("编码器打开失败 \n");
        exit(1);
    }
        
}

static AVFrame* create_frame(int width, int height) {
    
    AVFrame *frame = av_frame_alloc();
    
    // 设置frame的参数
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUVA420P;
    
    // 设置buffer大小
    int ret = av_frame_get_buffer(frame, 32); // 视频里面必须是32位对齐的。
    if (ret < 0) {
        printf("开辟buffer失败\n");
        exit(1);
    }
    
    return frame;
}

void record_video(void) {
    
    // 注册设备
    avdevice_register_all();
    
    // 定义环境上下文变量
    AVFormatContext *fmt_ctx = NULL;
    
    // 定义设备的名字 0: 表示的摄像头 1: 表示的桌面
    char *device_name = "0";
    
    // 设置采集格式
    AVInputFormat *input_fmt = av_find_input_format("avfoundation");
    
    // 定义打开设备的参数
    AVDictionary *options = NULL;
    av_dict_set(&options, "video_size", "640x480", 0); // 设置分辨率
    av_dict_set(&options, "framerate", "30", 0); // 设置分辨率
    av_dict_set(&options, "pixel_format", "nv12", 0); // 设置YUV格式为nv12
    
    // 打开设备
    int open_device_result = avformat_open_input(&fmt_ctx, device_name, input_fmt, &options);
    if (open_device_result < 0) {
        printf("设备打开失败!");
        return;
    };
    
    // 如果打开设备返回的是-35,说明设备还没有准备好，sleep 1s之后进行采集即可
    sleep(1);
    
    // 定义存放读取到的数据的Packet
    AVPacket pkt;
    av_init_packet(&pkt);

    // 定义输出文件
    FILE *outfile = fopen("/Users/bytedance/Desktop/video.yuv", "wb+");
    
    // 打开编码器
    AVCodecContext *enc_ctx = NULL;
    open_encoder(VIDEO_WIDTH, VIDEO_HEIGHT, &enc_ctx);
    
    // 创建AVFrame
    AVFrame *frame = create_frame(VIDEO_WIDTH, VIDEO_HEIGHT);
    
    // 创建编码后输出的Packet
    AVPacket *newpkt = av_packet_alloc();
    if (!newpkt) {
        printf("Packet创建失败 ! \n");
        exit(1);
    }
    
    int ret = 0;
    // 读取数据
    do {
        ret = av_read_frame(fmt_ctx, &pkt);
        
        if (ret == 0) {
            // 不设置pix_format的时候，是uvyv422格式的，所以每一帧的数据长度就是 640*480*2 = 614400；
            // 当设置了 pix_format 是 nv12的时候，每一帧的数据长度 = 640*480*1.5 = 460800；
            printf("写入数据长度: %d \n", pkt.size);
            fwrite(pkt.data, 1, 460800, outfile);
            fflush(outfile);
        }
        printf("视频读取结果: %d \n", ret);
        
        // 释放packet
        av_packet_unref(&pkt);
    }while((ret == 0 || ret == -35) && video_recoding);
    
    // 关闭资源
    fclose(outfile);
    avformat_close_input(&fmt_ctx);
    printf("录制结束\n");
}

void update_video_recording_status(int is_recording) {
    video_recoding = is_recording;
}
