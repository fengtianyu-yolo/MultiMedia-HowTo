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
#include <libswresample/swresample.h>

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
    
    /* 音频数据读取相关变量的定义 */
    // 定义环境上下文变量
    AVFormatContext *fmt_ctx = NULL;
    // 定义 devicename 参数，指示从哪个设备读取数据； ':0'，冒号前面表示为视频设备；冒号后面代表音频设备，0表示第一个音频设备
    char *devicename = ":0";
    // 定义 参数选项 参数
    AVDictionary *options = NULL;
    
    // 设备的采集格式
    AVInputFormat *input_format = av_find_input_format("avfoundation");
    
    /* 重采样相关变量的定义 */
    // 定义变量 存储输入缓冲区的地址
    uint8_t **src_data = NULL;
    // 定义 输入缓冲区大小
    int src_linesize = 0;
    // 创建输入缓冲区
    av_samples_alloc_array_and_samples(
                                       &src_data, // 输入的缓冲区地址
                                       &src_linesize, // 输入的大小
                                       1, // 输入的声道数
                                       512, // 采样数
                                       AV_SAMPLE_FMT_FLT, // 输入的位数 32位
                                       0);
    
    // 定义输出缓冲区的地址
    uint8_t **dst_data = NULL;
    // 定义输出缓冲区的大小
    int dst_linesize = 0;
    // 创建输出缓冲区
    av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, 1, 512, AV_SAMPLE_FMT_S16, 0);
    
    
    // 创建重采样上下文
    SwrContext *swr_ctx = NULL;
    
    // 初始化重采样上下文
    swr_ctx = swr_alloc_set_opts(
                                 NULL,
                                 AV_CH_LAYOUT_MONO, // 输出的声道格式，单声道，在中间
                                 AV_SAMPLE_FMT_S16, // 输出的采样格式 16位的
                                 48000, // 输出的采样率
                                 AV_CH_LAYOUT_MONO, // 输入的声道格式，单声道，在中间位置的布局格式
                                 AV_SAMPLE_FMT_FLT, // 输入的采样格式 32位的
                                 48000, // 输入的采样率 32位的
                                 0, // Log 相关的参数
                                 NULL // Log 相关的参数
                                 );
    swr_init(swr_ctx);
    /* -END 重采样相关变量定义完成 END- */
    
    /* 编码相关变量定义 */
    // 获取编码器
    AVCodec *codec = avcodec_find_encoder_by_name("libfdk_aac");
    // 创建编码上下文
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;
    codec_ctx->sample_rate = 48000;
    codec_ctx->bit_rate = 64000;
    
    // 音频的输入数据
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        printf("编码输入数据没有创建成功 \n");
        exit(0);
    }
    frame->nb_samples = 512; // 采样数
    frame->format = AV_SAMPLE_FMT_S16;
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    av_frame_get_buffer(frame, 0);
    if (!frame->buf[0]) {
        printf("编码输入缓冲区没有创建成功 \n");
        exit(0);
    }
    
    
    // 编码后的音频数据
    AVPacket *new_pkt = av_packet_alloc();
    if (!new_pkt) {
        printf("编码输出数据没有创建成功 \n");
    }
    
    int codec_open_result = avcodec_open2(codec_ctx, codec, NULL);
    if (codec_open_result < 0) {
        printf("编码器打开结果: %d \n", codec_open_result);
        exit(0);
    }
    
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
    FILE *outfile = fopen("/Users/bytedance/Desktop/audio.aac", "wb+");
    FILE *originfile = fopen("/Users/bytedance/Desktop/origin_audio.pcm", "wb+");

    // 定义变量存储 读取音频数据的结果
    int read_result = 0;
    
    do {
        // 读取音频数据，并将数据放入到 packet中
        read_result = av_read_frame(fmt_ctx, &pkt);
        
        // 读取到音频数据之后，将数据写入到文件中
        if (read_result == 0) {
            printf("数据大小是 %d \n", pkt.size); // 这里打印出来显示是 2048，也就是一帧的音频数据大小是2048
            
            // 把数据拷贝到输入缓冲区
            memcpy((void *)src_data[0], (void *)pkt.data, pkt.size);
            
            // 重采样
            swr_convert(
                        swr_ctx, // 重采样的上下文
                        dst_data, // 重采样的输出缓冲区的数据
                        512, // 输出的采样数
                        (const uint8_t **)src_data, // 输入的缓冲区的数据
                        512 // 输入的采样数
                        );

            // 将重采样的数据拷贝到输入编码区
            memcpy((void *)frame->data[0], dst_data[0], dst_linesize);
            
            // 将重采样后的一帧数据放到编码器缓冲区
            int ret = avcodec_send_frame(codec_ctx, frame);
            
            while(ret >= 0) {
                // 从编码器输出缓冲区取编码后的数据
                ret = avcodec_receive_packet(codec_ctx, new_pkt);
                if (ret < 0) {
                    // 输出缓冲区的数据读取完了
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        printf("编码后的数据读取完了 \n");
                        break;
                    } else {
                        exit(0);
                    }
                }
            }
            
            fwrite(new_pkt->data, 1, new_pkt->size, outfile);
            fwrite(pkt.data, 1, pkt.size, originfile);
            
            printf("编码的数据大小 = %d \n", new_pkt->size);

            fflush(outfile);
            fflush(originfile);
        }

        // 释放packet的内容
        av_packet_unref(&pkt);
    }while((read_result == 0 || read_result == -35) && recoding == 1);

    printf("录制结束了 \n");
    
    // 关闭文件
    fclose(outfile);
    fclose(originfile);
    
    // 释放输入缓冲区
    if (src_data){
        av_freep(&src_data[0]);
    }
    av_freep(&src_data);
    
    // 释放 输出缓冲区
    if (dst_data) {
        av_freep(&dst_data[0]);
    }
    av_freep(&dst_data);
    
    // 释放重采样上下文
    swr_free(&swr_ctx);
    
    // 释放编码输入和输出缓冲区
    av_frame_free(&frame);
    av_packet_free(&new_pkt);
    
    // 释放上下文环境
    avformat_close_input(&fmt_ctx);
}

void updateRecodeState(int recodeState) {
    recoding = recodeState;
}
