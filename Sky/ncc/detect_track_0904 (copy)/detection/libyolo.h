#ifndef LIBYOLO_H
#define LIBYOLO_H
#include "image.h"

typedef void* yolo_handle;

typedef struct {
	char name[32];
	int left;
	int right;
	int top;
	int bottom;
	float prob;
} detection_info;

box get_detection_info(image im, int num, float thresh, box *boxes, float **probs, int classes);
yolo_handle yolo_init(char *darknet_path, char *datacfg, char *cfgfile, char *weightfile);
void yolo_cleanup(yolo_handle handle);
box yolo_detect(yolo_handle handle, image im, float thresh, float hier_thresh);
box yolo_test(yolo_handle handle, char *filename, float thresh, float hier_thresh);
//detection_info **yolo_test(yolo_handle handle, char *filename, float thresh, float hier_thresh, int *num);

#endif // LIBYOLO_H
