#include "detection.h"


Detection::Detection(char *darknet_path, char *datacfg, char *cfgfile, char *weightfile)
{
    yolo = yolo_init(darknet_path, datacfg, cfgfile, weightfile);

    obj_box.x = -1;
    obj_box.y = -1;
    obj_box.h = 0;
    obj_box.w = 0;
    
    sub_rect.x = -1;
    sub_rect.y = -1;
}

Detection::~Detection()  { yolo_cleanup(yolo); }

box Detection::get_obj_box() const  { return obj_box; }

box Detection::get_sub_box() const  { return sub_box; }

Region Detection::get_sub_rect() const  { return sub_rect; }


void Detection::ipl_into_image(IplImage* src, image im)
{
    unsigned char *data = (unsigned char *)src->imageData;
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    int step = src->widthStep;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
}


image Detection::ipl_to_image(IplImage* src)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image out = make_image(w, h, c);
    ipl_into_image(src, out);
    return out;
}


box Detection::select_obj_box(detection_info **info, int num)
{
    box selected_box;
    float maxprob = 0.0;
    int max_index = 0;

    for (int i = 0; i < num; i++)
    {
        if (info[i]->prob > maxprob)
        {
            maxprob = info[i]->prob;
            max_index = i;
        }
    }

    selected_box.x = (info[max_index]->left + info[max_index]->right) / 2;
    selected_box.y = (info[max_index]->top + info[max_index]->bottom) / 2;
    selected_box.h = info[max_index]->right - info[max_index]->left;
    selected_box.w = info[max_index]->bottom - info[max_index]->top;

    return selected_box;
}


// detect with image object in memory
box Detection::detect_image(Mat frame, double thresh, double hier_thresh)
{
    // box detect_box;
    int num;
    
    // convert mat to image
    IplImage img_ipl = IplImage(frame);
    image img = ipl_to_image(&img_ipl);

    detection_info **info = yolo_detect(yolo, img, thresh, hier_thresh, &num);
    if (num == 0) {
        obj_box.x = -1;
        obj_box.y = -1;
        obj_box.w = 0;
        obj_box.h = 0;
        return get_obj_box();
    }

    obj_box = select_obj_box(info, num);


    return get_obj_box();
}

// detect with disk frame file
box Detection::detect_image(Mat frame, char *frame_file, double thresh, double hier_thresh)
{
    int num = 0;                        // the number of detected boxes
    float *feature_map = NULL;
    int map_size = 0;

    imwrite(frame_file, frame);

    detection_info **info = yolo_test(yolo, frame_file, thresh, hier_thresh, &num, &feature_map, &map_size);

    // no boxes
    if (num == 0) {
        obj_box.x = -1;
        obj_box.y = -1;
        obj_box.w = 0;
        obj_box.h = 0;
        return get_obj_box();
    }

    obj_box = select_obj_box(info, num);

    return get_obj_box();
}


// tune coordinate of subrect
void tune_coordinate(int *x0, int *x_range, const int &x_limit)
{
    // both sides overstep at x-axis
    if (*x_range >= x_limit)
    {
        *x_range = x_limit;
        *x0 = 0;
    }
    // left/top overstep
    else if (*x0 < 0)
    {
        *x0 = 0;
    }
    // right/bottom overstep
    else if (*x0+*x_range > x_limit)
    {
        *x0 = x_limit - *x_range;
    }
    // just in the range
    // else *x0 = *x0
}


void Detection::new_subrect(int cx, int cy, int im_cols, int im_rows)
{
    sub_rect.x = cx - sub_rect.width/2.0;
    sub_rect.y = cy - sub_rect.height/2.0;

    // cout << sub_rect.x << ", " << sub_rect.y << endl;

    tune_coordinate(&(sub_rect.x), &(sub_rect.width), im_cols);
    tune_coordinate(&(sub_rect.y), &(sub_rect.height), im_rows);

    // cout << sub_rect.x << ", " << sub_rect.y << endl;
    // cout << sub_rect.width << ", " << sub_rect.height << endl;
}


// convert a box from a sub-region to the whole range
void Detection::subox_to_objbox()
{
    obj_box.x = sub_box.x + sub_rect.x;
    obj_box.y = sub_box.y + sub_rect.y;
    obj_box.h = sub_box.h;
    obj_box.w = sub_box.w;
}


// convert a box from the whole range to a sub-region
void Detection::objbox_to_subox()
{
    sub_box.x = obj_box.x - sub_rect.x;
    sub_box.y = obj_box.y - sub_rect.y;
    sub_box.h = obj_box.h;
    sub_box.w = obj_box.w;
}


box Detection::detect(Mat frame, char *frame_file, double thresh, double hier_thresh,
                    int obj_track_x, int obj_track_y)
{
    cout << "Detect at (" << obj_track_x << ", " << obj_track_y << ")" << endl;
    
    // new detection, get a objcet box and sub-region coordinates
    if (obj_track_x == -1 && obj_track_y == -1)
    {
        obj_box = detect_image(frame, frame_file, thresh, hier_thresh);
    }
    
    // detect in a region at (obj_track_x, obj_track_y)
    else if (obj_track_x != -1 && obj_track_y != -1)
    {
        new_subrect(obj_track_x, obj_track_y, frame.cols, frame.rows);
        Rect rect(sub_rect.x, sub_rect.y, sub_rect.width, sub_rect.height);
        Mat sub_img = frame(rect);
        sub_box = detect_image(sub_img, frame_file, thresh, hier_thresh);
        
        subox_to_objbox();
    }
    
    else
    {
        cout << "Ooooops, wrong sub-region center coordinates!" << endl;
                
        obj_box.x = -1;
        obj_box.y = -1;
        obj_box.h = 0;
        obj_box.w = 0;
    }
    
    return get_obj_box();
}
