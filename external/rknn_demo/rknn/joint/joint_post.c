#include "joint_post.h"

int postProcessCPM(float *output, float *result, int point_num)
{
	for (int i = 0; i < point_num; i++) {
        // printf("i=%d\n", i);
        if (i >= point_num) break;
		float val_x_sum = 0;
		float val_y_sum = 0;
		float val_sum = 0;
        int val_count = 0;
		for (int j = 0; j < 96*96; j++) {
			if (output[i*96*96 + j] > 0.3) {
                // LOGI("i=%d j=%d val=%f\n", i, j, output[i*96*96 + j]);
				int x = j % 96;
				int y = j / 96;
				float val = output[i*96*96 + j];
				val_x_sum += val * x;
				val_y_sum += val * y;
				val_sum += val;
                val_count++;
			}
		}
     //   printf("i=%d val_x_sum=%f val_y_sum=%f val_sum=%f val_count=%d\n", i, val_x_sum, val_y_sum, val_sum, val_count);
        if (val_sum != 0 && val_count >= 20) {
            float coor_x = val_x_sum / val_sum;
            float coor_y = val_y_sum / val_sum;
     //       printf("coor x=%f y=%f\n", coor_x, coor_y);
            result[i * 2] = coor_x;
            result[i * 2 + 1] = coor_y;
        } else {
            result[i * 2] = 0;
            result[i * 2 + 1] = 0;
        }
	}
    return 0;
}