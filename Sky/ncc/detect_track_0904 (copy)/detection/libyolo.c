#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "option_list.h"
#include "network.h"
#include "parser.h"
#include "region_layer.h"
#include "utils.h"
#include "libyolo.h" 

typedef struct {
	char darknet_path[1024];
	char **names;
	float nms;
	box *boxes;
	float **probs;
	network net;
} yolo_obj;

box get_detection_info(image im, int num, float thresh, box *boxes, float **probs, int classes)
{
    box obj_box;
//	int i = 0;
//    int tagNum = 0;
//    int x = 0, y = 0, w = 0, h = 0;  // flag = 0,
//	float maxprob = 0.0;
//    box obj_box;

//	for(i = 0; i < num; ++i){
//        int cls = max_index(probs[i], classes);
//        float prob = probs[i][cls];
//		if(prob > thresh){
//            tagNum = tagNum + 1;
//            if(prob > maxprob){
//				maxprob = prob;
//                obj_box = boxes[i];
//            }
//        }
//    }

//    if(tagNum > 0) {
//        int left  = (obj_box.x-obj_box.w/2.)*im.w;
//        int right = (obj_box.x+obj_box.w/2.)*im.w;
//        int top   = (obj_box.y-obj_box.h/2.)*im.h;
//        int bot   = (obj_box.y+obj_box.h/2.)*im.h;

//        if(left < 0) left = 0;
//        if(right > im.w-1) right = im.w-1;
//        if(top < 0) top = 0;
//        if(bot > im.h-1) bot = im.h-1;

//        x = (left + right) / 2;
//        y = (top + bot) / 2;
//        w = right - left + 1;
//        h = bot - top + 1;
//    } else {
//		x = 0;
//        y = 0;
//        w = 0;
//        h = 0;
//    }

    int i = 0;
    int tagNum = 0;
    int x = 0, y = 0, w = 0, h = 0;  // flag = 0,
    float maxprob = 0.0;

    for(i = 0; i < num; ++i){
        int cls = max_index(probs[i], classes);
        float prob = probs[i][cls];
        if(prob > thresh){
            box b = boxes[i];

            int left  = (b.x-b.w/2.)*im.w;
            int right = (b.x+b.w/2.)*im.w;
            int top   = (b.y-b.h/2.)*im.h;
            int bot   = (b.y+b.h/2.)*im.h;

            if(left < 0) left = 0;
            if(right > im.w-1) right = im.w-1;
            if(top < 0) top = 0;
            if(bot > im.h-1) bot = im.h-1;

            detection_info *info = (detection_info *)malloc(sizeof(detection_info));
//            strncpy(info->name, names[cls], sizeof(info->name));
            info->left = left;
            info->right = right;
            info->top = top;
            info->bottom = bot;
            info->prob = prob;
//            list_insert(output, info);

            if(prob > maxprob){
                maxprob = prob;
                //printf("Preditect：%d %d %d %d\n" ,left, right, top, bot);
//				flag = 1;
                x = (left + right) / 2;
                y = (top + bot) / 2;
                w = right - left + 1;
                h = bot - top + 1;
            }
            tagNum = tagNum + 1;
        }
    }
    if(tagNum == 0){
//		flag = 2;
        x = 0;
        y = 0;
    }

    obj_box.x = x;
    obj_box.y = y;
    obj_box.h = h;
    obj_box.w = w;
    return obj_box;
}

yolo_handle yolo_init(char *darknet_path, char *datacfg, char *cfgfile, char *weightfile)
{
	yolo_obj *obj = (yolo_obj *)malloc(sizeof(yolo_obj));
	if (!obj) return NULL;
	memset(obj, 0, sizeof(yolo_obj));

    char cur_dir[1024];
    strncpy(obj->darknet_path, darknet_path, sizeof(obj->darknet_path));
    getcwd(cur_dir, sizeof(cur_dir));
    chdir(darknet_path);
	list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "./darknet/data/voc.names");
	obj->names = get_labels(name_list);

	obj->net = parse_network_cfg(cfgfile);
	if(weightfile){
		load_weights(&obj->net, weightfile);
	}
	set_batch_network(&obj->net, 1);
	srand(2222222);

	int j;
	obj->nms=.4;

	layer l = obj->net.layers[obj->net.n-1];
	obj->boxes = calloc(l.w*l.h*l.n, sizeof(box));
	obj->probs = calloc(l.w*l.h*l.n, sizeof(float *));
	for(j = 0; j < l.w*l.h*l.n; ++j) obj->probs[j] = calloc(l.classes + 1, sizeof(float *));
	chdir(cur_dir);

	return (yolo_handle)obj;
}

void yolo_cleanup(yolo_handle handle)
{
	yolo_obj *obj = (yolo_obj *)handle;
	if (obj) {
		layer l = obj->net.layers[obj->net.n-1];
		free(obj->boxes);
		free_ptrs((void **)obj->probs, l.w*l.h*l.n);
		free(obj);
	}
}

box yolo_detect(yolo_handle handle, image im, float thresh, float hier_thresh)
{
    yolo_obj *obj = (yolo_obj *)handle;
	clock_t time0 = clock();
	image sized = letterbox_image(im, obj->net.w, obj->net.h);
	printf("Resize image in %f seconds.\n", sec(clock()-time0));

	float *X = sized.data;
	clock_t time = clock();
	network_predict(obj->net, X);
	printf("Prediction in %f seconds.\n", sec(clock()-time));
	
	layer l = obj->net.layers[obj->net.n-1];
	get_region_boxes(l, im.w, im.h, obj->net.w, obj->net.h, thresh, obj->probs, obj->boxes, NULL, 0, 0, hier_thresh, 1);
	if (obj->nms) do_nms_obj(obj->boxes, obj->probs, l.w*l.h*l.n, l.classes, obj->nms);

//	list *output = make_list();
    box detect_box = get_detection_info(im, l.w*l.h*l.n, thresh, obj->boxes, obj->probs, l.classes);
//	detection_info **info = (detection_info **)list_to_array(output);
//	*num = output->size;

//	free_list(output);
	// free_image(im);
	free_image(sized);
    return detect_box;
}

box yolo_test(yolo_handle handle, char *filename, float thresh, float hier_thresh)
{
	clock_t time = clock();
    
    yolo_obj *obj = (yolo_obj *)handle;
    
    time = clock();
	char input[256];
	strncpy(input, filename, sizeof(input));
	image im = load_image_color(input,0,0);
    printf("Read image in %f seconds.\n", sec(clock()-time));
    
    time = clock();
	image sized = letterbox_image(im, obj->net.w, obj->net.h);
    printf("Resize image in %f seconds.\n", sec(clock()-time));

	float *X = sized.data;
	time = clock();
	network_predict(obj->net, X);
	printf("Prediction in %f seconds.\n", sec(clock()-time));
	
	layer l = obj->net.layers[obj->net.n-1];
	get_region_boxes(l, im.w, im.h, obj->net.w, obj->net.h, thresh, obj->probs, obj->boxes, NULL, 0, 0, hier_thresh, 1);
	if (obj->nms) do_nms_obj(obj->boxes, obj->probs, l.w*l.h*l.n, l.classes, obj->nms);
    box detect_box = get_detection_info(im, l.w*l.h*l.n, thresh, obj->boxes, obj->probs, l.classes);

	free_image(sized);
    return detect_box;
}

//detection_info **yolo_test(yolo_handle handle, char *filename, float thresh, float hier_thresh, int *num)
//{
//	yolo_obj *obj = (yolo_obj *)handle;

//	char input[256];
//	strncpy(input, filename, sizeof(input));
//	image im = load_image_color(input,0,0);
//	image sized = letterbox_image(im, obj->net.w, obj->net.h);

//	float *X = sized.data;
//	clock_t time;
//	time=clock();
//	network_predict(obj->net, X);
//	printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));

//	layer l = obj->net.layers[obj->net.n-1];
//	get_region_boxes(l, im.w, im.h, obj->net.w, obj->net.h, thresh, obj->probs, obj->boxes, NULL, 0, 0, hier_thresh, 1);
//	if (obj->nms) do_nms_obj(obj->boxes, obj->probs, l.w*l.h*l.n, l.classes, obj->nms);

//	list *output = make_list();
//	get_detection_info(im, l.w*l.h*l.n, thresh, obj->boxes, obj->probs, l.classes, obj->names, output);
//	detection_info **info = (detection_info **)list_to_array(output);
//	*num = output->size;

//	free_list(output);
//	free_image(im);
//	free_image(sized);

//	return info;
//}
