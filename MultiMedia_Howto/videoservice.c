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

static int video_recoding = 0;

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
//    av_new_packet(&pkt, 4096);

    // 定义输出文件
    FILE *outfile = fopen("/Users/bytedance/Desktop/video.yuv", "wb+");
    
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
