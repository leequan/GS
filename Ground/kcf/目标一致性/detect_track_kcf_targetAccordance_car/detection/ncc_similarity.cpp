#include "ncc_similarity.h"
#include "detection.h"

// 取候选区域框中最大ncc的目标函数
box select_obj_box_all(Mat frame, Mat obj_img, detection_info **info, int num, int xx, int yy,int &detect_flag)
{
    box selected_box_big_ncc;
//在候选框内判断检测结果的ncc
 box obj_box;
// box aa;
    detection_info **info1 = (detection_info **)malloc(sizeof(detection_info));
    int tagnum = 0;
    //候选框的边界判断
    Region2 sub_rect2;
    sub_rect2.x = xx - sub_rect2.width/2.0;
    sub_rect2.y = yy - sub_rect2.height/2.0;
    tune_coordinate(&(sub_rect2.x), &(sub_rect2.width), frame.cols);
    tune_coordinate(&(sub_rect2.y), &(sub_rect2.height), frame.rows);

    for (int i = 0; i < num; i++)
    {
        printf("infoleft= %d, inforight= %d,infotop= %d, infobottom= %d \n",info[i]->left,info[i]->right,info[i]->top,info[i]->bottom);
         if ((info[i]->right<sub_rect2.x)||(info[i]->left>sub_rect2.x+256)||(info[i]->top>sub_rect2.y+256)||(info[i]->bottom<sub_rect2.y) )
        {

            continue;
        }
        else
        {
            info1[tagnum] = info[i];
            printf("info1left= %d, info1right= %d,info1top= %d, info1bottom= %d \n",info1[tagnum]->left,info1[tagnum]->right,info1[tagnum]->top,info1[tagnum]->bottom);
            tagnum++;
        }
    }

    if (tagnum == 0) {
        obj_box.x = -2;
        obj_box.y = -1;
        obj_box.w = 0;
        obj_box.h = 0;
        detect_flag++;
        printf("detect_flg = %d \n",detect_flag);
        return obj_box;
    }

    else
    {
    // printf("tag num: %d \n", tagnum);

//计算最大ncc
     selected_box_big_ncc = select_obj_box_big_ncc(frame, obj_img, info1, tagnum,xx, yy);

    return selected_box_big_ncc;
    }


//在全局图像范围内判断检测结果的ncc
//    selected_box_big_ncc = select_obj_box_big_ncc(frame, obj_img, info, num, xx, yy);
//    return selected_box_big_ncc;
}
//计算最大ncc　函数
box select_obj_box_big_ncc(Mat img, Mat obj_img, detection_info **info1, int tagnum, int xx, int yy)
{

    Mat tag[tagnum], tag1[tagnum];
    float maxncc = 0.0;
    int max_index = 0;
    box select_obj_box_big_ncc;
    float nccvalue[tagnum];
    double average[tagnum],  variance[tagnum];

    Mat obj_tag = obj_img;
    double obj_average = 0.0;
    double obj_variance = 0.0;
    av(obj_tag,obj_average, obj_variance);

   for(int i = 0; i<tagnum; i++)
   {
        Rect center_rect(info1[i]->left, info1[i]->top, info1[i]->right-info1[i]->left, info1[i]->bottom-info1[i]->top);
        printf("left= %d, right= %d \n",info1[i]->left,info1[i]->right);
        tag[i] = img(center_rect);
        resize(tag[i], tag1[i], Size(obj_img.cols, obj_img.rows));
        av(tag1[i],average[i], variance[i]);
        nccvalue[i] = ncc_similarity(obj_tag,tag1[i], obj_average,  obj_variance,  average[i],  variance[i]);
        if (nccvalue[i] > maxncc)
        {
            maxncc = nccvalue[i];
            max_index = i;
        }
   }

   select_obj_box_big_ncc.x = (info1[max_index]->left + info1[max_index]->right) / 2;
   select_obj_box_big_ncc.y = (info1[max_index]->top + info1[max_index]->bottom) / 2;
   select_obj_box_big_ncc.h = info1[max_index]->right - info1[max_index]->left;
   select_obj_box_big_ncc.w = info1[max_index]->bottom - info1[max_index]->top;

   return select_obj_box_big_ncc;

}
//计算框的均值和标准差
    void av(Mat tagimg, double &average, double &variance)
    {
        average = 0;
        for (int y=0;y <tagimg.rows;y++)
        {
            for (int x=0;x <tagimg.cols;x++)
            {
                average += tagimg.at<uchar>(x,y);
             }
        }
        average /= (tagimg.cols*tagimg.rows);

        variance = 0;
        for (int y=0;y <tagimg.rows;y++)
        {
            for (int x=0;x <tagimg.cols;x++)
            {
                int temp = tagimg.at<uchar>(x,y);
                variance += (temp - average)*(temp - average);
             }
        }
        variance /= (tagimg.cols*tagimg.rows);
        variance = sqrt(variance);
    }
//计算ncc
  double  ncc_similarity(Mat obj_tag,Mat tag1,double obj_average, double obj_variance, double average, double variance)
     {
         int sum_pixel = 0, sum_pixel1 = 0;
            int factor_num, factor_num1;
            for(int i = 0; i < tag1.rows; i++){
                for(int j = 0; j < tag1.cols; j++){
                    sum_pixel += obj_tag.at<uchar>(j,i);
                    sum_pixel1 += tag1.at<uchar>(j,i);
                }
             }
                 factor_num = 1073741824 / sum_pixel;//2^30=1073741824
                 factor_num1 = 1073741824 / sum_pixel1;//2^30=1073741824
                 double  sum = 0.0;
                 for(int i = 0; i < tag1.rows; i++){
                     for(int j = 0; j < tag1.cols; j++){
                         uchar u = (obj_tag.at<uchar>(j,i)*factor_num)>>15;
                         uchar v = (tag1.at<uchar>(j,i)*factor_num1)>>15;
                         sum += abs((u - obj_average) * (v - average)/(obj_variance*variance));
                     }
                 }

            double ncc_value = sum/(obj_tag.cols*obj_tag.rows);

         return ncc_value;
     }
