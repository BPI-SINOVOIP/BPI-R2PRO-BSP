#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <sys/time.h>

#include "rknn_api.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;

template <class T>
void get_top_n(T* prediction, int prediction_size, size_t num_results,
               float threshold, std::vector<std::pair<float, int>>* top_results,
               bool input_floating) {
  // Will contain top N results in ascending order.
  std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>,
                      std::greater<std::pair<float, int>>>
      top_result_pq;

  const long count = prediction_size;  // NOLINT(runtime/int)
  for (int i = 0; i < count; ++i) {
    float value;
    if (input_floating)
      value = prediction[i];
    else
      value = prediction[i] / 255.0;
    // Only add it if it beats the threshold and has a chance at being in
    // the top N.
    if (value < threshold) {
      continue;
    }

    top_result_pq.push(std::pair<float, int>(value, i));

    // If at capacity, kick the smallest value out.
    if (top_result_pq.size() > num_results) {
      top_result_pq.pop();
    }
  }

  // Copy to output vector and reverse into descending order.
  while (!top_result_pq.empty()) {
    top_results->push_back(top_result_pq.top());
    top_result_pq.pop();
  }
  std::reverse(top_results->begin(), top_results->end());
}

int ReadLabelsFile(const string& file_name,
                            std::vector<string>* result,
                            size_t* found_label_count) {
  std::ifstream file(file_name);
  if (!file) {
    std::cerr << "Labels file " << file_name << " not found\n";
    return -1;
  }
  result->clear();
  string line;
  while (std::getline(file, line)) {
    result->push_back(line);
  }
  *found_label_count = result->size();
  const int padding = 16;
  while (result->size() % padding) {
    result->emplace_back();
  }
  return 0;
}

int main(int argc, char** argv)
{
    const char *img_path = "/tmp/dog.jpg";
    const char *model_path = "/tmp/mobilenet_v1-tf.rknn";
    const char *lable_path = "/tmp/labels.txt";
    const int output_elems = 1001;

    const int img_width = 224;
    const int img_height = 224;
    const int img_channels = 3;

    const int input_index = 0;      // node name "input"
    const int output_index = 0;     // node name "MobilenetV1/Predictions/Reshape_1"

    // Load image
    cv::Mat img = cv::imread(img_path, 1);
    if(!img.data) {
        printf("cv::imread %s fail!\n", img_path);
        return -1;
    }
    if(img.cols != img_width || img.rows != img_height)
        cv::resize(img, img, cv::Size(img_width, img_height), (0, 0), (0, 0), cv::INTER_LINEAR);

    //BGR->RGB
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    // Load model
    FILE *fp = fopen(model_path, "rb");
    if(fp == NULL) {
        printf("fopen %s fail!\n", model_path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    void *model = malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", model_path);
        free(model);
        return -1;
    }

    // Start Inference
    rknn_input inputs[1];
    rknn_output outputs[1];
    rknn_tensor_attr output0_attr;

    int ret = 0;
    rknn_context ctx = 0;

    ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM | RKNN_FLAG_COLLECT_PERF_MASK);
    if(ret < 0) {
        printf("rknn_init fail! ret=%d\n", ret);
        goto Error;
    }

    output0_attr.index = 0;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &output0_attr, sizeof(output0_attr));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto Error;
    }

    inputs[0].index = input_index;
    inputs[0].buf = img.data;
    inputs[0].size = img_width * img_height * img_channels;
    inputs[0].pass_through = false;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    ret = rknn_inputs_set(ctx, 1, inputs);
    if(ret < 0) {
        printf("rknn_input_set fail! ret=%d\n", ret);
        goto Error;
    }

    ret = rknn_run(ctx, nullptr);
    if(ret < 0) {
        printf("rknn_run fail! ret=%d\n", ret);
        goto Error;
    }

    outputs[0].want_float = true;
    outputs[0].is_prealloc = false;
    ret = rknn_outputs_get(ctx, 1, outputs, nullptr);
    if(ret < 0) {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        goto Error;
    }

    // Process output
    if(outputs[0].size == output0_attr.n_elems * sizeof(float))
    {
        const size_t num_results = 5;
        const float threshold = 0.001f;

        std::vector<std::pair<float, int>> top_results;
        get_top_n<float>((float*)outputs[0].buf, output_elems,
                           num_results, threshold, &top_results, true);

        std::vector<string> labels;
        size_t label_count;
        if (!ReadLabelsFile(lable_path, &labels, &label_count)) {
            for (const auto& result : top_results) {
                const float confidence = result.first;
                const int index = result.second;
                std::cout << confidence << ": " << index << " " << labels[index] << "\n";
            }
        }
    }
    else
    {
        printf("rknn_outputs_get fail! get output_size = [%d], but expect %u!\n",
            outputs[0].size, (uint32_t)(output0_attr.n_elems * sizeof(float)));
    }

    rknn_outputs_release(ctx, 1, outputs);

    // performance query
    {
        rknn_perf_run perf_run;
        ret = rknn_query(ctx, RKNN_QUERY_PERF_RUN, &perf_run, sizeof(perf_run));
        if(ret < 0) {
            printf("rknn_query fail! ret=%d\n", ret);
            goto Error;
        }
        printf("perf_run.run_duration = %ld us\n", perf_run.run_duration);

        rknn_perf_detail perf_detail;
        ret = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail, sizeof(perf_detail));
        if(ret < 0) {
            printf("rknn_query fail! ret=%d\n", ret);
            goto Error;
        }
        if(perf_detail.data_len > 0 && perf_detail.perf_data)
            printf("perf_run.perf_data = %s\n", perf_detail.perf_data);
        else
            printf("please enable RKNN_FLAG_COLLECT_PERF_MASK flag in rknn_init!!!\n");
    }

Error:
    if(ctx > 0)         rknn_destroy(ctx);
    if(model)           free(model);
    if(fp)              fclose(fp);
    return 0;
}
