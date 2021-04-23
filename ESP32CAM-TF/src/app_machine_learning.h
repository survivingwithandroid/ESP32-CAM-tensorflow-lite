#include "esp_camera.h"
#ifndef _APP_MACHINE_LEARNING_H
#define _APP_MACHINE_LEARNING_H
#ifdef __cplusplus
extern "C" {
#endif


void init_ml_module();
void classify_image(camera_fb_t *fb);

#ifdef __cplusplus
}
#endif
#endif