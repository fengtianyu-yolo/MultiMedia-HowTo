//
//  audioservice.c
//  MultiMedia_Howto
//
//  Created by fengtianyu on 2022/2/27.
//

#include "audioservice.h"
#include "libavutil/avutil.h"

void greeting() {
    printf("ok! you call c function success. \n");
    
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "ok! avutil success call ! \n");
}
