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
//#include "lib"

void greeting() {
    
    printf("ok! you call c function success. \n");
    
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "ok! avutil success call ! \n");

}

// 定义一个变量，指示当前是否在录制中
static int recoding = 0;

void record() {
    
    // 注册设备
    avdevice_register_all();
    
    // 定义环境上下文变量
    AVFormatContext *fmt_ctx = NULL;
    // 定义 devicename 参数，指示从哪个设备读取数据； ':0'，冒号前面表示为视频设备；冒号后面代表音频设备，0表示第一个音频设备
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
    
    // 打开一个文件，用于将读取到的数据写入
    FILE *outfile = fopen("/Users/bytedance/Desktop/audio.pcm", "wb+");

    // 定义变量存储 读取音频数据的结果
    int read_result = 0;
    
    do {
        // 读取音频数据，并将数据放入到 packet中
        read_result = av_read_frame(fmt_ctx, &pkt);
        
        // 读取到音频数据之后，将数据写入到文件中
        if (read_result == 0) {
            printf("数据大小是 %d \n", pkt.size); // 这里打印出来显示是 2048，也就是一帧的音频数据大小是2048
            fwrite(pkt.data, pkt.size, 1, outfile);
            fflush(outfile);
        }

        // 释放packet的内容
        av_packet_unref(&pkt);
    }while((read_result == 0 || read_result == -35) && recoding == 1);

    printf("录制结束了 \n");
    
    // 关闭文件
    fclose(outfile);
    
    // 释放上下文环境
    avformat_close_input(&fmt_ctx);
}

void updateRecodeState(int recodeState) {
    recoding = recodeState;
}
