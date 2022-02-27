//
//  audioservice.c
//  MultiMedia_Howto
//
//  Created by fengtianyu on 2022/2/27.
//

#include "audioservice.h"
#include "libavutil/avutil.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <unistd.h>

void greeting() {
    
    printf("ok! you call c function success. \n");
    
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "ok! avutil success call ! \n");
    
    // 注册设备
    avdevice_register_all();
    
    // 定义环境上下文变量
    AVFormatContext *fmt_ctx = NULL;
    // 定义 devicename 参数: ':0'，冒号前面表示为视频设备；冒号后面代表音频设备，0表示第一个音频设备
    char *devicename = ":0";
    // 定义 参数选项 参数
    AVDictionary *options = NULL;
    
    // 设备的采集格式
    AVInputFormat *input_format = av_find_input_format("avfoundation");
    
    // 打开音频输入设备
    int open_result = avformat_open_input(&fmt_ctx, devicename, input_format, &options);
    if (open_result == 0) {
        printf("设备成功打开 \n");
    }
    
    // 定义数据包变量，读取的音频数据就放到这个数据包中
    AVPacket pkt;
    // 初始化数据包
    av_init_packet(&pkt);
    // 延迟1s，不然设备还没有准备好
    sleep(1);
    
    // 打开文件
    FILE *outfile = fopen("/User/Desktop/audio.pcm", "wb+");
    
    // 读取音频数据，并将数据放入到 packet中
    int read_result = av_read_frame(fmt_ctx, &pkt);
    if (read_result == 0) {
        printf("音频数据读取成功了 \n");
    }
    // 将音频数据写入文件
    fwrite(pkt.data, pkt.size, 1, outfile);
    // 立即将缓冲区的文件内容写入到文件中
    fflush(outfile);
    // 关闭文件
    fclose(outfile);
    
    // packet
    av_packet_unref(&pkt);
    // 释放上下文环境
    avformat_close_input(&fmt_ctx);
    
}
