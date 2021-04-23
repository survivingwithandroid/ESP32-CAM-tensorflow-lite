#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#define MODEL_IMAGE_WIDTH   28
#define MODEL_IMAGE_HEIGHT  28
#define NUM_CHANNELS         1 // BW
#define kCategory  10
const char* kCategoryLabels[kCategory] = {"T-shirt/top", "Trouser", "Pullover", "Dress", "Coat",
               "Sandal", "Shirt", "Sneaker", "Bag", "Ankle boot"};
#endif