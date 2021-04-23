#include <app_machine_learning.h>

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"


#include "esp_log.h"
#include "esp_camera.h"
#include "model_config.h"
#include "image_util.h"
#include "model.h"


// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;

// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
float* model_input_buffer = nullptr;
}  // namespace

static const char *TAG = "app_ml";


void init_ml_module() {

  
  
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
   static tflite::MicroErrorReporter micro_error_reporter;
   error_reporter = &micro_error_reporter;
  
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  
  model = tflite::GetModel(_mnist_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Here we define the model 
  //static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
  //static tflite::ops::micro::AllOpsResolver micro_mutable_op_resolver; 
  static tflite::AllOpsResolver micro_mutable_op_resolver;
  // ----------------------
  /*micro_mutable_op_resolver.AddQuantize();
  micro_mutable_op_resolver.AddConv2D();
  micro_mutable_op_resolver.AddMaxPool2D();
  micro_mutable_op_resolver.AddReshape();
  micro_mutable_op_resolver.AddFullyConnected();
  micro_mutable_op_resolver.AddSoftmax();
  micro_mutable_op_resolver.AddDequantize();
*/
  /*
  micro_mutable_op_resolver.AddShape();
  micro_mutable_op_resolver.AddStridedSlice();
  micro_mutable_op_resolver.AddPack();
 

  micro_mutable_op_resolver.AddMaxPool2D();
 
  micro_mutable_op_resolver.AddDequantize();*/
    // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  interpreter =  new tflite::MicroInterpreter(
      model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
   
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter -> Report("--- Error allocating tensor arena ----");
  }
 model_input = interpreter->input(0);
 model_input_buffer = model_input->data.f;

 // ESP_LOGI(TAG, "Model size %d", model_input->dims->size);
}
/*
void normalise_image_buffer(float* dest_image_buffer,uint8* imageBuffer, int size )
{
  for (int i=0; i < size; i++)
  {
    dest_image_buffer[i] = imageBuffer[i]/255.0f;
    
  }
}*/

void classify_image(camera_fb_t *fb) {

  //Serial.println("Classify image...");
  // allocate space for cropped image
  int img_size = MODEL_IMAGE_WIDTH * MODEL_IMAGE_HEIGHT * NUM_CHANNELS;
  uint8_t * tmp_buffer = (uint8_t *) malloc(img_size);
  //Serial.println("TMP Buffer");
  ESP_LOGI(TAG, "TMP Buffer");
  // resize image
  image_resize_linear(tmp_buffer,fb->buf,MODEL_IMAGE_HEIGHT,MODEL_IMAGE_WIDTH,NUM_CHANNELS,fb->width,fb->height);
  ESP_LOGI(TAG, "Image resized");
 

  // normalize image
  ESP_LOGI(TAG, "Size: %d", sizeof(tmp_buffer));
  for (int i=0; i < img_size; i++)
  {
   //printf("Data %i", tmp_buffer[i]);
   //(interpreter->input(0))->data.f[i] = tmp_buffer[i] / 255.0f;
   model_input_buffer[i] = tmp_buffer[i] / 255.0f;
   //normalise_image_buffer( (interpreter->input(0))->data.f, tmp_buffer, img_size);
  }

  free(tmp_buffer);
   ESP_LOGI(TAG, "Invoking interpreter");
  //Serial.println("Invoking interpreter");
  if (kTfLiteOk != interpreter->Invoke()) 
	{
				error_reporter->Report("Error");
  }

  ESP_LOGI(TAG, "Showing results");
  TfLiteTensor* output = interpreter->output(0);
				
				for (int i=1; i < kCategory; i++)
				{
          ESP_LOGI(TAG, "Label=%s, Prob=%f",kCategoryLabels[i], output->data.f[i] );
        
 				}

   esp_camera_fb_return(fb);
   fb = NULL;
   ESP_LOGI(TAG, "Free Image");

}