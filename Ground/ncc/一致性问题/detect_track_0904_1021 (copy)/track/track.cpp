#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "track.hpp"

extern unsigned char *pDisp_data;
extern float xp[4];
extern signed char PNXC30[256];
extern signed char PNYC30[256];

/************************************************************************/
//双线性插值初始化函数
//参数列表依次对应

//H方向小数数据
//W方向小数数据
//H方向整数数据
//W方向整数数据
//目标信息结构体
/************************************************************************/
void Bilinear_Initial
    (
    unsigned char *CHf_initial,
    unsigned char *CWf_initial,
    unsigned short int *CHi_initial,
    unsigned short int *CWi_initial,
    const TargetData_TYPE *TargetData_Parameter
    )
{
    float ratio_h, ratio_w;
    float SampleCH, SampleCW;
    signed int hnum, wnum;

    ratio_h = TargetData_Parameter->ratio_h;
    ratio_w = TargetData_Parameter->ratio_w;

    SampleCH = TargetData_Parameter->SCenter_FH - 8 * ratio_h;
    SampleCW = TargetData_Parameter->SCenter_FW - 8 * ratio_w;

    for (hnum = 0; hnum < TEMPLATE_SIZE_H; hnum++)
    {
        CHf_initial[hnum] = (unsigned char)((SampleCH - (unsigned short int)(SampleCH)) * 16);
        CHi_initial[hnum] = (unsigned short int)SampleCH;
        SampleCH = SampleCH + ratio_h;
    }

    for (wnum = 0; wnum < TEMPLATE_SIZE_W; wnum++)
    {
        CWf_initial[wnum] = (unsigned char)((SampleCW - (unsigned short int)(SampleCW)) * 16);
        CWi_initial[wnum] = (unsigned short int)SampleCW;
        SampleCW = SampleCW + ratio_w;
    }
}


/************************************************************************/
//双线性插值初始化函数
//参数列表依次对应

//H方向小数数据
//W方向小数数据
//H方向整数数据
//W方向整数数据
//原始图像数据
//输出数据存储区
/************************************************************************/
void Bilinear_Interpolation_PN
    (
    const unsigned char *CHf_initial,
    const unsigned char *CWf_initial,
    const unsigned short int *CHi_initial,
    const unsigned short int *CWi_initial,
    const unsigned char *pSource_data,
    unsigned short int *pTemplate_data
    )
{
    signed int iH, jW;
    unsigned short int wf00, wf01, wf10, wf11;
    unsigned short int sH_i, sW_i;
    unsigned char sH_f, sW_f;
    unsigned short int aa;
    unsigned char b;
    int c;
    for (iH = 0; iH < TEMPLATE_SIZE_H; iH++)
    {
        sH_f = CHf_initial[iH];
        sH_i = CHi_initial[iH];

        for (jW = 0; jW < TEMPLATE_SIZE_W; jW++)
        {
            sW_f = CWf_initial[jW];
            sW_i = CWi_initial[jW];
            b=pSource_data[(sH_i) * FRAME_W + sW_i];
            c=FRAME_W;
            wf00 = pSource_data[(sH_i) * FRAME_W + sW_i] * (16 - sH_f) * (16 - sW_f);
            wf01 = pSource_data[(sH_i) * FRAME_W + sW_i + 1] * (16 - sH_f) * sW_f;
            b=pSource_data[(sH_i + 1) * FRAME_W + sW_i];
            wf10 = pSource_data[(sH_i + 1) * FRAME_W + sW_i] * sH_f * (16 - sW_f);
            wf11 = pSource_data[(sH_i + 1) * FRAME_W + sW_i + 1] * sH_f * sW_f;
            aa = ((wf00 + wf01 + wf10 + wf11) >> 1);
            *pTemplate_data++ = ((wf00 + wf01 + wf10 + wf11) >> 1); // *128 = *2^7, (min = 0*128 max = 255*128)
        }
    }
}


/************************************************************************/
//高斯分布随机数产生函数
//参数列表依次对应

//随机数数据存储区
/************************************************************************/
unsigned char Gaussian_Distribution
    (
    float *random_matrix
    )
{
    unsigned int k, i;
    unsigned int temp_sum;
    // Seed the random-number generator with the current time so that
    // the numbers will be different every time we run.
    srand( (unsigned)time( NULL ) );

    for (k = 0; k < NUM_PSEUDORANDOM; k++)
    {
        temp_sum = 0;
        for (i = 0; i < 12; i++)
        {
            temp_sum += rand()%(32767 + 1);
        }
        random_matrix[k] = (float)(temp_sum) / 32767 - 6.0;
    }
    return 1;
}


/************************************************************************/
//正样本计算函数
//参数列表依次对应

//H方向随机数
//W方向随机数
//H方向小数数据
//W方向小数数据
//H方向整数数据
//W方向整数数据
//原始图像数据
//正样本数据存储区
//目标信息结构体
/************************************************************************/
void affine_training_posi
    (
    const float *RANDOM_POSIH32,
    const float *RANDOM_POSIW32,
    unsigned char *CHf_initial,
    unsigned char *CWf_initial,
    unsigned short int *CHi_initial,
    unsigned short int *CWi_initial,
    const unsigned char *pSource_data,
    unsigned short int *posi_templates,
    TargetData_TYPE *TargetData_Parameter
    )
{
    unsigned short int *posi_templates_line;
    float Center_H, Center_W;
    float SampleSH, SampleSW;
    signed int jnum;

    Center_H = TargetData_Parameter->TCenter_H;
    Center_W = TargetData_Parameter->TCenter_W;

    SampleSH = TargetData_Parameter->TSize_H;
    SampleSW = TargetData_Parameter->TSize_W;

    TargetData_Parameter->ratio_h = (SampleSH) / TEMPLATE_SIZE_H;
    TargetData_Parameter->ratio_w = (SampleSW) / TEMPLATE_SIZE_W;

    for (jnum = 0; jnum < NUM_POSITIVE; jnum++)
    {
        posi_templates_line = posi_templates + jnum * NUM_TEMPLATE_PIXEL;

        TargetData_Parameter->SCenter_FH = Center_H + RANDOM_POSIH32[jnum];
        TargetData_Parameter->SCenter_FW = Center_W + RANDOM_POSIW32[jnum];

        Bilinear_Initial
            (
            CHf_initial,
            CWf_initial,
            CHi_initial,
            CWi_initial,
            TargetData_Parameter
            );


        Bilinear_Interpolation_PN
            (
            CHf_initial,
            CWf_initial,
            CHi_initial,
            CWi_initial,
            pSource_data,
            posi_templates_line
            );
    }
}

/************************************************************************/
//负样本计算函数
//参数列表依次对应

//H方向随机数
//W方向随机数
//H方向小数数据
//W方向小数数据
//H方向整数数据
//W方向整数数据
//原始图像数据
//负样本数据存储区
//目标信息结构体
/************************************************************************/
void affine_training_nega
    (
    const float *RANDOM_NEGAH64,
    const float *RANDOM_NEGAW64,
    unsigned char *CHf_initial,
    unsigned char *CWf_initial,
    unsigned short int *CHi_initial,
    unsigned short int *CWi_initial,
    const unsigned char *pSource_data,
    unsigned short int *nega_templates,
    TargetData_TYPE *TargetData_Parameter
    )
{
    unsigned short int *nega_templates_line;
    signed int jnum;
    signed short int DataL, DataR, DataT, DataB;
    signed short int Center_H, Center_W;
    signed short int SampleCH, SampleCW;
    unsigned char Size_H, Size_W;

    Center_H = TargetData_Parameter->TCenter_H;
    Center_W = TargetData_Parameter->TCenter_W;

    Size_H = TargetData_Parameter->TSize_H;
    Size_W = TargetData_Parameter->TSize_W;

    TargetData_Parameter->ratio_h = ((float)(Size_H)) / TEMPLATE_SIZE_H;
    TargetData_Parameter->ratio_w = ((float)(Size_W)) / TEMPLATE_SIZE_W;

    DataL = (Size_W >> 1) + IMALEW;
    DataR = IMARIW - (Size_W >> 1);
    DataT = (Size_H >> 1) + IMATOW;
    DataB = IMABOW - (Size_H >> 1);

    for (jnum = 0; jnum < NUM_NEGATIVE; jnum++)
    {
        nega_templates_line = nega_templates + jnum * NUM_TEMPLATE_PIXEL;

        SampleCH = Center_H + (signed short int)(RANDOM_NEGAH64[jnum] * Size_H);
        SampleCW = Center_W + (signed short int)(RANDOM_NEGAW64[jnum] * Size_W);

        SampleCH = (SampleCH < DataT) ? DataT : (SampleCH > DataB) ? DataB : SampleCH;
        SampleCW = (SampleCW < DataL) ? DataL : (SampleCW > DataR) ? DataR : SampleCW;

        TargetData_Parameter->SCenter_FH = (float)SampleCH;
        TargetData_Parameter->SCenter_FW = (float)SampleCW;

        Bilinear_Initial
            (
            CHf_initial,
            CWf_initial,
            CHi_initial,
            CWi_initial,
            TargetData_Parameter
            );

        Bilinear_Interpolation_PN
            (
            CHf_initial,
            CWf_initial,
            CHi_initial,
            CWi_initial,
            pSource_data,
            nega_templates_line
            );
    }

}


/************************************************************************/
//粒子样本计算函数
//参数列表依次对应

//粒子数据特征信息存储区
//粒子数据存储区
//随机数存储区
//仿射系数数据
//H方向小数数据
//W方向小数数据
//H方向整数数据
//W方向整数数据
//原始图像数据
//目标信息结构体
/************************************************************************/

//void affine_training_particle
//    (
//    float *particle_data,
//    unsigned short int *particle_templates,
//    const int *CandiH,
//    const int *CandiW,
//    unsigned char *CHf_initial,
//    unsigned char *CWf_initial,
//    unsigned short int *CHi_initial,
//    unsigned short int *CWi_initial,
//    const unsigned char *pSource_data,
//    TargetData_TYPE *TargetData_Parameter
//    )
//{
//    unsigned short int *particle_templates_line;
//    float SampleSH, SampleSW;

//    signed int inum, jnum;
//    signed short int Center_H, Center_W;
//    signed short int SampleCH, SampleCW;
//    unsigned char Size_H, Size_W;

//    Center_H = TargetData_Parameter->TCenter_H;
//    Center_W = TargetData_Parameter->TCenter_W;

//    Size_H = TargetData_Parameter->TSize_H;
//    Size_W = TargetData_Parameter->TSize_W;


//    for (inum = 0; inum < 2 * RADIUS +1 ;inum++)
//    {
//        for (jnum = 0; jnum < 2 * RADIUS +1 ;jnum++)
//        {
//            particle_templates_line = particle_templates + (inum * (2 * RADIUS + 1) + jnum ) * NUM_TEMPLATE_PIXEL;

//            SampleCH = Center_H + CandiH[inum];
//            SampleCW = Center_W + CandiW[jnum];
//            SampleSW = Size_H;
//            SampleSH = Size_W;

//            TargetData_Parameter->ratio_h = (SampleSH) / TEMPLATE_SIZE_H;
//            TargetData_Parameter->ratio_w = (SampleSW) / TEMPLATE_SIZE_W;

//            TargetData_Parameter->SCenter_FH = (float)SampleCH;
//            TargetData_Parameter->SCenter_FW = (float)SampleCW;

//            particle_data[inum * (2 * RADIUS +1) + jnum] = SampleCH;
//            particle_data[NUM_PARTICLE + inum * (2 * RADIUS +1) + jnum] = SampleCW;
//            particle_data[2 * NUM_PARTICLE + inum * (2 * RADIUS +1) + jnum] = (SampleSH);
//            particle_data[3 * NUM_PARTICLE + inum * (2 * RADIUS +1) + jnum] = (SampleSW);

//            Bilinear_Initial
//                (
//                CHf_initial,
//                CWf_initial,
//                CHi_initial,
//                CWi_initial,
//                TargetData_Parameter
//                );

//            Bilinear_Interpolation_PN
//                (
//                CHf_initial,
//                CWf_initial,
//                CHi_initial,
//                CWi_initial,
//                pSource_data,
//                particle_templates_line
//                );

//        }
//    }

//}
void affine_training_particle
    (
    float *particle_data,
    unsigned short int *particle_templates,
    const float *random_matrix,
    const int *CandiH,
    const int *CandiW,
    unsigned char *CHf_initial,
    unsigned char *CWf_initial,
    unsigned short int *CHi_initial,
    unsigned short int *CWi_initial,
    const unsigned char *pSource_data,
    TargetData_TYPE *TargetData_Parameter
    )
{
    unsigned short int *particle_templates_line;
    float SampleSH, SampleSW;
    float ratio_h_w;
    signed int inum, jnum;
    signed short int Center_H, Center_W;
    signed short int SampleCH, SampleCW;
    unsigned char Size_H, Size_W;

    int Dia = 2 * RADIUS +1;
    float *randomMatrix_particle;
    unsigned short int min_index, step_index, max_index;
    float SW[NUM_PARTICLE],SH[NUM_PARTICLE];


    randomMatrix_particle = (float *)malloc(NUM_PARTICLE * sizeof( float ));
    if ( NULL == randomMatrix_particle )
    {
        printf( "File %s \nLine %d \n", __FILE__, __LINE__ );
    }

    // random data
    min_index = rand() % (NUM_PARTICLE);
    step_index = (NUM_PSEUDORANDOM - min_index) / ( NUM_PARTICLE);
    max_index = NUM_PARTICLE * step_index + min_index;

    for (inum = min_index; inum < max_index; inum += step_index)
    {
        randomMatrix_particle[(inum - min_index) / step_index] = random_matrix[inum];
    }

    // target
    Center_H = TargetData_Parameter->TCenter_H;
    Center_W = TargetData_Parameter->TCenter_W;
    Size_H = TargetData_Parameter->TSize_H;
    Size_W = TargetData_Parameter->TSize_W;
    ratio_h_w = ((float)Size_H) / Size_W;
   // printf(" In CenterX,CenterY=%d,%d,H,W=%d,%d \n",Center_W,Center_H,Size_H,Size_W);
    //  Scale No Or Yes ?
    if( Size_H * Size_W <= TH_TARGETSIZE )
    {
        //printf("Scale 1\n");
        for(inum = 0; inum < NUM_PARTICLE; inum++ )
        {
            SW[inum] = Size_W;
            SH[inum] = Size_H;
        }
        
    }
    else
    {
       // printf("Scale 2\n");
        for(inum = 0; inum < NUM_PARTICLE; inum++ )
        {
            SW[inum] = Size_W + randomMatrix_particle[inum];
            SH[inum] = SW[inum] * ratio_h_w;
            //printf("SW ,SH = %f,%f \n",SW[inum],SH[inum] );
        }
        
    }

    for (inum = 0; inum < Dia; inum++)
    {
        for (jnum = 0; jnum < Dia; jnum++)
        {
            particle_templates_line = particle_templates + (inum * Dia + jnum) * NUM_TEMPLATE_PIXEL;

            SampleCH = Center_H + CandiH[inum];
            SampleCW = Center_W + CandiW[jnum];
            SampleSW = SW[inum * Dia + jnum];
            SampleSH = SH[inum * Dia + jnum];

            //printf("SampleCH,SampleCW = %d,%d, W,H = %f,%f \n",SampleCH,SampleCW,SampleSW ,SampleSH);
            
            TargetData_Parameter->ratio_h = (SampleSH) / TEMPLATE_SIZE_H;
            TargetData_Parameter->ratio_w = (SampleSW) / TEMPLATE_SIZE_W;

            TargetData_Parameter->SCenter_FH = (float)SampleCH;
            TargetData_Parameter->SCenter_FW = (float)SampleCW;

            particle_data[inum * Dia + jnum ] = SampleCH;
            particle_data[NUM_PARTICLE + inum * Dia + jnum] = SampleCW;
            //printf("SampleCW , index, particle_data = %d,%d,%f \n", 
                //SampleCW,NUM_PARTICLE + inum * Dia + jnum,particle_data[NUM_PARTICLE + inum * Dia + jnum] );
            
            particle_data[2 * NUM_PARTICLE + inum * Dia + jnum ] = (SampleSH);
            particle_data[3 * NUM_PARTICLE + inum * Dia + jnum ] = (SampleSW);

            Bilinear_Initial
                (
                CHf_initial,
                CWf_initial,
                CHi_initial,
                CWi_initial,
                TargetData_Parameter
                );

            Bilinear_Interpolation_PN
                (
                CHf_initial,
                CWf_initial,
                CHi_initial,
                CWi_initial,
                pSource_data,
                particle_templates_line
                );
        }

    }

    free(randomMatrix_particle);
}


/************************************************************************/
//显示函数
//参数列表依次对应

//目标信息结构体
//原始图像数据
//灰度值
/************************************************************************/
void display_rectangle
    (
    const TargetData_TYPE *TargetData_Parameter,
    unsigned char *image_data,
    const unsigned char gray_value
    )
{
    signed int hi, wj, hm, wn, numi, numj;
    signed short int gateLE, gateRI, gateTO, gateBO;
//    signed short int focusLE, focusRI, focusTO, focusBO;
//    signed short int borderLE, borderRI, borderTO, borderBO;
    signed short int Center_H, Center_W;
    signed short int FocusHalf_H, FocusHalf_W;
    unsigned char SizeHalf_H, SizeHalf_W;

    Center_H = TargetData_Parameter->TCenter_H;
    Center_W = TargetData_Parameter->TCenter_W;

    SizeHalf_H = TargetData_Parameter->TSizeHalf_H;
    SizeHalf_W = TargetData_Parameter->TSizeHalf_W;

    FocusHalf_H = TargetData_Parameter->TSizeHalf_H - 3;
    FocusHalf_W = TargetData_Parameter->TSizeHalf_W - 3;

    gateLE = Center_W - SizeHalf_W; // [0 718]
    gateRI = Center_W + SizeHalf_W - 1; // [1 719]

    gateLE = (gateLE < 0) ? (0) : (gateLE >= (FRAME_W - 1)) ? (FRAME_W - 2) : (gateLE);
    gateRI = (gateRI < 1) ? (1) : (gateRI >= FRAME_W) ? (FRAME_W - 1) : (gateRI);

    gateTO = Center_H - SizeHalf_H; // [0 286]
    gateBO = Center_H + SizeHalf_H - 1; // [1 287]

    gateTO = (gateTO < 0) ? (0) : (gateTO >= (HALF_FRAME_H - 1)) ? (HALF_FRAME_H - 2) : (gateTO);
    gateBO = (gateBO < 1) ? (1) : (gateBO >= HALF_FRAME_H) ? (HALF_FRAME_H - 1) : (gateBO);


//    focusLE = Center_W - FocusHalf_W; // [0 718]
//    focusRI = Center_W + FocusHalf_W - 1; // [1 719]

//    focusLE = (focusLE < 0) ? (0) : (focusLE >= (FRAME_W - 1)) ? (FRAME_W - 2) : (focusLE);
//    focusRI = (focusRI < 1) ? (1) : (focusRI >= FRAME_W) ? (FRAME_W - 1) : (focusRI);

//    focusTO = Center_H - FocusHalf_H; // [0 286]
//    focusBO = Center_H + FocusHalf_H - 1; // [1 287]

//    focusTO = (focusTO < 0) ? (0) : (focusTO >= (HALF_FRAME_H - 1)) ? (HALF_FRAME_H - 2) : (focusTO);
//    focusBO = (focusBO < 1) ? (1) : (focusBO >= HALF_FRAME_H) ? (HALF_FRAME_H - 1) : (focusBO);


//    borderLE = Center_W - 8; // [0 718]
//    borderRI = Center_W + 8; // [1 719]

//    borderLE = (borderLE < 0) ? (0) : (borderLE >= (FRAME_W - 1)) ? (FRAME_W - 2) : (borderLE);
//    borderRI = (borderRI < 1) ? (1) : (borderRI >= FRAME_W) ? (FRAME_W - 1) : (borderRI);

//    borderTO = Center_H - 8; // [0 286]
//    borderBO = Center_H + 8; // [1 287]

//    borderTO = (borderTO < 0) ? (0) : (borderTO >= (HALF_FRAME_H - 1)) ? (HALF_FRAME_H - 2) : (borderTO);
//    borderBO = (borderBO < 1) ? (1) : (borderBO >= HALF_FRAME_H) ? (HALF_FRAME_H - 1) : (borderBO);


    for ( hi = gateTO; hi <= gateBO; hi++ )
    {
        image_data[hi * FRAME_W + gateLE] = gray_value;
        image_data[hi * FRAME_W + gateRI] = gray_value;
    }

    for ( wj = gateLE; wj <= gateRI; wj++ )
    {
        image_data[gateTO * FRAME_W + wj ] = gray_value;
        image_data[gateBO * FRAME_W + wj ] = gray_value;
    }

    //for ( hm = focusTO; hm <= focusBO; hm++ )
    //{
    //    image_data[ hm * FRAME_W + Center_W] = gray_value;
    //}

   // for ( wn = focusLE; wn <= focusRI; wn++ )
   // {
   //     image_data[ Center_H * FRAME_W + wn] = gray_value;
   // }

//    for ( numi = borderTO; numi <= borderBO; numi++ )
//    {
//        image_data[numi * FRAME_W + borderLE] = gray_value;
//        image_data[numi * FRAME_W + borderRI] = gray_value;
//    }

//    for ( numj = borderLE; numj <= borderRI; numj++ )
//    {
//        image_data[borderTO * FRAME_W + numj ] = gray_value;
//        image_data[borderBO * FRAME_W + numj ] = gray_value;
//    }

}

/************************************************************************/
//显示函数
//参数列表依次对应

//目标H方向坐标
//目标W方向坐标
//原始图像数据
//灰度值
/************************************************************************/
void display_particle
    (
    const signed short int Center_H,
    const signed short int Center_W,
    unsigned char *image_data,
    const unsigned char gray_value
    )
{
    signed int hi, wj;
    signed short int gateLE, gateRI, gateTO, gateBO;
    signed char SizeHalf_H, SizeHalf_W;

    SizeHalf_H = 1;
    SizeHalf_W = 1;

    gateLE = Center_W - SizeHalf_W; // [0 718]
    gateRI = Center_W + SizeHalf_W; // [1 719]

    gateLE = (gateLE < 0) ? (0) : (gateLE >= (FRAME_W - 1)) ? (FRAME_W - 2) : (gateLE);
    gateRI = (gateRI < 1) ? (1) : (gateRI >= FRAME_W) ? (FRAME_W - 1) : (gateRI);

    gateTO = Center_H - SizeHalf_H; // [0 286]
    gateBO = Center_H + SizeHalf_H; // [1 287]

    gateTO = (gateTO < 0) ? (0) : (gateTO >= (HALF_FRAME_H - 1)) ? (HALF_FRAME_H - 2) : (gateTO);
    gateBO = (gateBO < 1) ? (1) : (gateBO >= HALF_FRAME_H) ? (HALF_FRAME_H - 1) : (gateBO);

    for ( hi = gateTO; hi <= gateBO; hi++ )
    {
        image_data[hi * FRAME_W + gateLE] = gray_value;
        image_data[hi * FRAME_W + gateRI] = gray_value;
    }

    for ( wj = gateLE; wj <= gateRI; wj++ )
    {
        image_data[gateTO * FRAME_W + wj] = gray_value;
        image_data[gateBO * FRAME_W + wj] = gray_value;
    }

}

/************************************************************************/
//边界显示函数
//参数列表依次对应

//左边界坐标
//右边界坐标
//上边界坐标
//下边界坐标
//原始图像数据
//灰度值
/************************************************************************/
void display_boundary
    (
    const signed short int DataL,
    const signed short int DataR,
    const signed short int DataT,
    const signed short int DataB,
    unsigned char *image_data,
    const unsigned char gray_value
    )
{
    signed int hi, wj;
    signed short int gateLE, gateRI, gateTO, gateBO;

    gateLE = DataL; // [0 718]
    gateRI = DataR; // [1 719]

    gateLE = (gateLE < 0) ? (0) : (gateLE >= (FRAME_W - 1)) ? (FRAME_W - 2) : (gateLE);
    gateRI = (gateRI < 1) ? (1) : (gateRI >= FRAME_W) ? (FRAME_W - 1) : (gateRI);

    gateTO = DataT; // [0 286]
    gateBO = DataB; // [1 287]

    gateTO = (gateTO < 0) ? (0) : (gateTO >= (HALF_FRAME_H - 1)) ? (HALF_FRAME_H - 2) : (gateTO);
    gateBO = (gateBO < 1) ? (1) : (gateBO >= HALF_FRAME_H) ? (HALF_FRAME_H - 1) : (gateBO);

    for ( hi = gateTO; hi <= gateBO; hi++ )
    {
        image_data[hi * FRAME_W + gateLE] = gray_value;
        image_data[hi * FRAME_W + gateRI] = gray_value;
    }

    for ( wj = gateLE; wj <= gateRI; wj++ )
    {
        image_data[gateTO * FRAME_W + wj] = gray_value;
        image_data[gateBO * FRAME_W + wj] = gray_value;
    }

}


/************************************************************************/
//归一化函数
//参数列表依次对应

//样本数据存储区
//样本数目---归一化使用
/************************************************************************/
void norm_vector
    (
    unsigned short int *template_matrix,
    const unsigned short int num_template
    )
{
    signed int inum, jnum, knum;
    signed int factor_num;
    unsigned int sum_pixel;

    for (inum = 0; inum < num_template; inum++)
    {
        sum_pixel = 1;

        for (jnum = 0; jnum < NUM_TEMPLATE_PIXEL; jnum++)
        {
            sum_pixel += template_matrix[inum * NUM_TEMPLATE_PIXEL + jnum];
        }

        factor_num = 1073741824/sum_pixel;  // 2^30 = 1073741824

        for (knum = 0; knum < NUM_TEMPLATE_PIXEL; knum++)
        {
            template_matrix[inum * NUM_TEMPLATE_PIXEL + knum] = ((template_matrix[inum * NUM_TEMPLATE_PIXEL + knum] * factor_num) >> 15);
        // template_matrix[inum * NUM_TEMPLATE_PIXEL + knum] = _mpylir(template_matrix[inum * NUM_TEMPLATE_PIXEL + knum], factor_num);
        }
    }

}

/************************************************************************/
//负样本初始化函数
//参数列表依次对应

//负样本均值
//负样本数据存储区
//负样本方差
/************************************************************************/
void nega_templates_initial
    (
    unsigned short int *nega_mean,
    const unsigned short int *nega_templates,
    unsigned int *nega_sigma
    )
{
    signed int numpixel, jnum, nnum;
    unsigned int meanvalue_nega;
    unsigned int sigmavalue_nega;
    signed short int tempdata;

    for (numpixel = 0; numpixel < NUM_TEMPLATE_PIXEL; numpixel++)
    {
        sigmavalue_nega = 0;
        meanvalue_nega = 0;

        for (jnum = 0; jnum < NUM_NEGATIVE; jnum++)
        {
            meanvalue_nega += nega_templates[jnum * NUM_TEMPLATE_PIXEL + numpixel];
        }
        meanvalue_nega = meanvalue_nega / NUM_NEGATIVE;  // Q15

    /*	for (nnum = 0; nnum < NUM_NEGATIVE; nnum++)
        {
            tempdata = (nega_templates[nnum * NUM_TEMPLATE_PIXEL + numpixel] - meanvalue_nega);
            sigmavalue_nega += tempdata * tempdata;
        }
        // 2^28
        nega_sigma[numpixel] = (sigmavalue_nega << 4) / NUM_NEGATIVE;*/
        nega_mean[numpixel] = meanvalue_nega;
    }

}


/************************************************************************/
//分类器更新函数
//参数列表依次对应

//负样本数据存储区
//负样本均值
//正样本均值
//负样本方差
//正样本方差
/************************************************************************/
void classifier_update
    (
    const unsigned short int *nega_templates,
    unsigned short int *nega_mean,
    unsigned short int *posi_mean,
    unsigned int *nega_sigma,
    unsigned int *posi_sigma
    )
{
    signed int numpixel, jnum, nnum;
    unsigned int meanvalue_nega;
    unsigned int sigmavalue_nega;
    signed short int tempdata;
    unsigned int sigmavalue_nega_data;
    unsigned int transferdata;

    for (numpixel = 0; numpixel < NUM_TEMPLATE_PIXEL; numpixel++)
    {
        sigmavalue_nega = 0;
        meanvalue_nega = 0;
        for (jnum = 0; jnum < NUM_NEGATIVE; jnum++)
        {
            meanvalue_nega += nega_templates[jnum * NUM_TEMPLATE_PIXEL + numpixel];
        }
        meanvalue_nega = meanvalue_nega / NUM_NEGATIVE;  //////Q15

        for (nnum = 0; nnum < NUM_NEGATIVE; nnum++)
        {
            tempdata = (nega_templates[nnum * NUM_TEMPLATE_PIXEL + numpixel] - meanvalue_nega);
            sigmavalue_nega += tempdata * tempdata;
        }

        sigmavalue_nega_data = (sigmavalue_nega << 4) / NUM_NEGATIVE;

        nega_mean[numpixel] = ((BEFORE_UPDATE_RATE * nega_mean[numpixel] + AFTER_UPDATE_RATE * meanvalue_nega) >> 10);
        transferdata = ((BEFORE_UR * nega_sigma[numpixel] + AFTER_UR * sigmavalue_nega_data) >> 7);

        //transferdata = ((_mpy32ll(BEFORE_UR, nega_sigma[numpixel])) + (_mpy32ll(AFTER_UR, sigmavalue_nega_data))) >> 7;

        nega_sigma[numpixel] = transferdata;

    }

}


/************************************************************************/
//分类器函数
//参数列表依次对应


//负样本均值
//正样本均值
//粒子样本数据存储区
//定点后验概率存储区
//负样本方差
//正样本方差
//调试使用参数---浮点后验概率存储区
/************************************************************************/
void ratio_classifier
    (
    const unsigned short int *nega_mean,
    const unsigned short int *posi_mean,
    const unsigned short int *particle_templates,
//	signed int *map_templates,
//	const unsigned int *nega_sigma,
//	const unsigned int *posi_sigma,
    float *float_map_templates
    )
{
    signed int inum, jnum;
    signed int square_posi, square_nega;
    signed int ireci_negas, ireci_posis;
    signed int transferdata;
    unsigned int transfer_negas, transfer_posis;
    signed short int diff_posi, diff_nega;

    //*******************************
    float ncc_negas, ncc_posis;
    unsigned int  corr_negas, corr_posis;
    unsigned int  norm1_negas, norm1_posis;
    unsigned int  norm2;

    for (jnum = 0; jnum < NUM_PARTICLE; jnum++)
    {
        corr_posis=0;
        corr_negas=0;
        norm1_posis=0;
        norm1_negas=0;
        norm2=0;
        for (inum = 0; inum < NUM_TEMPLATE_PIXEL; inum++)
        {
            corr_posis+=posi_mean[inum]*particle_templates[jnum * NUM_TEMPLATE_PIXEL + inum];
            corr_negas+=nega_mean[inum]*particle_templates[jnum * NUM_TEMPLATE_PIXEL + inum];
            norm1_posis+=posi_mean[inum]*posi_mean[inum];
            norm1_negas+=nega_mean[inum]*nega_mean[inum];
            norm2+=particle_templates[jnum * NUM_TEMPLATE_PIXEL + inum]*particle_templates[jnum * NUM_TEMPLATE_PIXEL + inum];
        }
        ncc_posis=1-((float)corr_posis/sqrt((float)norm1_posis*(float)norm2)+1)/2.0;
        ncc_negas=1-((float)corr_negas/sqrt((float)norm1_negas*(float)norm2)+1)/2.0;

        float_map_templates[jnum]=ncc_negas/(ncc_negas+ncc_posis);
    }

}

/************************************************************************/
//最大置信度计算函数
//参数列表依次对应

//置信度数据存储区
//置信度数据存储区备份
//目标信息结构体
//调试参数---浮点置信度数据存储区
/************************************************************************/
unsigned short int max_address
    (
    //const signed int *like_lihood,
    //signed int *apriority_value,
    float *max_value,
    TargetData_TYPE *TargetData_Parameter,
    const float *float_like_lihood
    )
{
    float value_sum, value_norm;
    signed int jnum, inum, knum, mnum;
    //float max_value;
    signed int transferdata;
    unsigned short int max_value_num;

    float fmax_value;
    signed int fnum;
    unsigned short int fmax_value_num;
    float floatvalue_sum, floatvalue_norm;

    max_value[0]=0;
    for (inum = 0; inum < NUM_PARTICLE; inum++)
    {
        if (float_like_lihood[inum] > max_value[0])
        {
            max_value[0] = float_like_lihood[inum];
            max_value_num = inum;
        }
    }

    return max_value_num;
}

/************************************************************************/
//置信度定点函数
//参数列表依次对应

//后验概率数据存储区
//置信度数据存储区
/************************************************************************/
void MAP
    (
    signed int *map_templates,
    signed int *like_lihood
    )
{
    signed int inum, jnum;
    signed int temp_sum;
    signed int *map_templates_line;

    for (inum = 0; inum < NUM_PARTICLE; inum++)
    {

        temp_sum = 0;
        map_templates_line = map_templates + inum * NUM_TEMPLATE_PIXEL;

        for (jnum = 0; jnum < NUM_TEMPLATE_PIXEL; jnum++)
        {
            temp_sum += *map_templates_line++;
        }
        *like_lihood++ = (temp_sum);
    }

}

/************************************************************************/
//调试函数---置信度浮点函数
//参数列表依次对应

//后验概率数据存储区
//置信度数据存储区
/************************************************************************/
void MAP_float
    (
    float *map_templates,
    float *like_lihood
    )
{
    signed int inum, jnum;
    float temp_sum;
    float *map_templates_line;

    for (inum = 0; inum < NUM_PARTICLE; inum++)
    {

        temp_sum = 0;
        map_templates_line = map_templates + inum * NUM_TEMPLATE_PIXEL;

        for (jnum = 0; jnum < NUM_TEMPLATE_PIXEL; jnum++)
        {
            temp_sum += *map_templates_line++;
        }
        *like_lihood++ = (temp_sum);
    }

}

/************************************************************************/
//调试函数---仿射系数更新函数
//参数列表依次对应

//仿射系数数据
//H方向中心坐标存储区
//W方向中心坐标存储区
//目标跟踪状态显示
/************************************************************************/
void affsig_update
    (
    float *affsig,
    const signed short int *hc_record,
    const signed short int *wc_record,
    const unsigned char track_state
    )
{
    signed int inum, jnum;
    signed int standard_h, standard_w;
    signed short int mean_h, mean_w;
    signed short int tempdata_h, tempdata_w;
    signed char radius_minh, radius_maxh, radius_minw, radius_maxw;

    radius_minh = 2;
    radius_maxh = 2;
    radius_minw = 5;
    radius_maxw = 10;

    mean_h = 0;
    mean_w = 0;
    standard_h = 0;
    standard_w = 0;

    for (inum = 0; inum < UPDATE_CYCLE; inum++)
    {
        mean_h += hc_record[inum];
        mean_w += wc_record[inum];
    }

    mean_h = mean_h / UPDATE_CYCLE;
    mean_w = mean_w / UPDATE_CYCLE;

    for(jnum = 0; jnum < UPDATE_CYCLE; jnum++)
    {
        tempdata_h = (hc_record[jnum] - mean_h);
        tempdata_w = (wc_record[jnum] - mean_w);
        standard_h += tempdata_h * tempdata_h;
        standard_w += tempdata_w * tempdata_w;
    }

    standard_h = sqrt((float)standard_h / UPDATE_CYCLE);
    standard_w = sqrt((float)standard_w / UPDATE_CYCLE);


    if (track_state)
    {
        //affsig[1] = (standard_h > 8) ? (standard_h) : (standard_h + 1);
        //affsig[0] = (standard_w > 8) ? (standard_w) : (standard_w + 1);
        affsig[0] = (standard_w < radius_minw) ? radius_minw : (standard_w > radius_maxw) ? radius_maxw : standard_w;
        affsig[1] = (standard_h < radius_minh) ? radius_minh : (standard_h > radius_maxh) ? radius_maxh : standard_h;
    }
    else
    {
        affsig[0] = (standard_w < 8) ? 8 : (standard_w > 11) ? 11 : standard_w;
        affsig[1] = (standard_h < 3) ? 3 : (standard_h > 8) ? 8 : standard_h;
    }

    //affsig[1] = affsig[1]/2;
}


/************************************************************************/
//正样本初始化函数
//参数列表依次对应

//正样本均值
//正样本数据存储区
//正样本方差
/************************************************************************/
void posi_templates_initial
    (
    unsigned short int *posi_mean,
    const unsigned short int *posi_templates,
    unsigned int *posi_sigma
    )
{
    signed int numpixel, inum, mnum;
    unsigned int meanvalue_posi;
    unsigned int sigmavalue_posi;
    signed short int tempdata;

    for (numpixel = 0; numpixel < NUM_TEMPLATE_PIXEL; numpixel++)
    {
        meanvalue_posi = 0;
        sigmavalue_posi = 0;

        for (inum = 0; inum < NUM_POSITIVE; inum++)
        {
            meanvalue_posi += posi_templates[inum * NUM_TEMPLATE_PIXEL + numpixel];
        }
        meanvalue_posi = meanvalue_posi / NUM_POSITIVE;  // Q15

    /*	for (mnum = 0; mnum < NUM_POSITIVE; mnum++)
        {
            tempdata = (posi_templates[mnum * NUM_TEMPLATE_PIXEL + numpixel] - meanvalue_posi);
            sigmavalue_posi += tempdata * tempdata;
        }
        // 2^28
        posi_sigma[numpixel] = (sigmavalue_posi << 4) / NUM_POSITIVE; // Q30 + 4*/
        posi_mean[numpixel] = meanvalue_posi;  // 2^15
    }

}

/************************************************************************/
//仿射系数更新函数
//参数列表依次对应

//H方向中心坐标存储区
//W方向中心坐标存储区
//目标信息结构体
/************************************************************************/
void center_update
    (
    signed short int *hc_record,
    signed short int *wc_record,
    const TargetData_TYPE *TargetData_Parameter
    )
{
    signed int inum;

    for (inum = (PATH_COHERENCE_NUM - 1); inum > 0; inum--)
    {
        hc_record[inum] = hc_record[inum - 1];
        wc_record[inum] = wc_record[inum - 1];
    }

    hc_record[0] = TargetData_Parameter->TCenter_H;
    wc_record[0] = TargetData_Parameter->TCenter_W;

}




/************************************************************************/
//置信度排序函数
//参数列表依次对应

//置信度存储数据
//置信度排序第六个最大值
//置信度排序第六个最小值
//置信度排序第五个最大值
//置信度排序第五个最小值
//跟踪运行帧数
/************************************************************************/
void compute_margin
    (
    const signed int *similar_apriority,
    signed int *apriority_max6,
    signed int *apriority_min6,
    signed int *apriority_max5,
    signed int *apriority_min5,
    const signed int track_frame
    )
{
    signed int inum, jnum, knum;
    signed int num_apriority;
    signed int *ptemp_apriority;

    signed int temp_max, temp_min;

    num_apriority = (track_frame > 241) ? (241 - 5) : (track_frame - 5);

    ptemp_apriority = (signed int *)malloc((num_apriority) * 4);
    if ( NULL == ptemp_apriority )
    {
        printf( "File %s \nLine %d \n", __FILE__, __LINE__ );
    }

    for (knum = 0; knum < num_apriority; knum++)
    {
        ptemp_apriority[knum] = similar_apriority[knum + 5];
    }

    for (inum = 0; inum < 6; inum++)
    {
        for (jnum = num_apriority - 1; jnum > inum; jnum--)
        {
            if ( ptemp_apriority[jnum] < ptemp_apriority[jnum - 1] )
            {
                temp_max = ptemp_apriority[jnum];
                ptemp_apriority[jnum] = ptemp_apriority[jnum - 1];
                ptemp_apriority[jnum - 1] = temp_max;
            }
        }
    }

    for (inum = 1; inum < 7; inum++)
    {
        for (jnum = 0; jnum < num_apriority - inum; jnum++)
        {
            if ( ptemp_apriority[jnum] > ptemp_apriority[jnum + 1] )
            {
                temp_max = ptemp_apriority[jnum];
                ptemp_apriority[jnum] = ptemp_apriority[jnum + 1];
                ptemp_apriority[jnum + 1] = temp_max;
            }
        }
    }

    *apriority_max6 = ptemp_apriority[num_apriority - 6];
    *apriority_min6 = ptemp_apriority[5];
    *apriority_max5 = ptemp_apriority[num_apriority - 5];
    *apriority_min5 = ptemp_apriority[4];

    free(ptemp_apriority);
}


/************************************************************************/
//置信度更新数据
//参数列表依次对应

//置信度存储数据空间
//当前置信度值
/************************************************************************/
void update_similar_apriority
    (
    signed int *similar_apriority,
    signed int *apriority_value
    )
{

    signed int inum;

    for (inum = 240; inum > 0; inum--)
    {
        similar_apriority[inum] = similar_apriority[inum - 1];
    }

    similar_apriority[0] = apriority_value[0];
}


/************************************************************************/
//权重分布函数
//参数列表依次对应

//权重值数据存储区
//目标状态信息
/************************************************************************/
void Weighted_Distribution
    (
    unsigned short int *weightedD,
    signed short int *stateParameter
    )

{
    signed int hi, wj;
    signed int centerTW, centerTH ;
    unsigned int sumw32;
    unsigned int sumw_temp;
    signed short int TW, TH;
    signed short int diff_hc, diff_wc;
    signed short int distdata;
    signed short int bandw;
    unsigned short int weightemp;

    centerTW = stateParameter[4];
    centerTH = stateParameter[5];
    TW = (stateParameter[4] << 1);
    TH = (stateParameter[5] << 1);

    sumw32 = 0;
    bandw = 4194304/(TW * TW + TH * TH); // 2^22 = 4194304
    // [192, 5408]

    for ( hi = 0; hi < TH; hi++ )
    {
        for ( wj = 0; wj < TW; wj++ )
        {
            diff_hc = (hi - centerTH);
            diff_wc = (wj - centerTW);
            distdata = (diff_hc) * (diff_hc) + (diff_wc) * (diff_wc);

            weightemp = 16384 - ((distdata * bandw) >> 8);

            *weightedD++ = weightemp;
            sumw32 += weightemp;
        }
    }

    sumw_temp = (sumw32 >> 7);
    *(stateParameter + 6) = 4194304/sumw_temp;
}


/************************************************************************/
//权重直方图函数
//参数列表依次对应

//原始图像数据
//直方图数据存储区
//权重值数据存储区
//目标状态信息
/************************************************************************/
void Weighted_Histogram
    (
    const unsigned char  *pGray_data,
    unsigned int       *hist_template,
    const unsigned short int *weightedD,
    const signed short int *stateParameter
    )

{
    unsigned int hi, wj, mk;
    signed short int divisor;
    unsigned short int TDataL, TDataR, TDataT, TDataB;
    unsigned char  bin;

    TDataL = *(stateParameter) - *(stateParameter + 4);
    TDataR = *(stateParameter) + *(stateParameter + 4) - 1;
    TDataT = *(stateParameter + 2) - *(stateParameter + 5);
    TDataB = *(stateParameter + 2) + *(stateParameter + 5) - 1;
    divisor = stateParameter[6];

    for( hi = TDataT; hi <= TDataB; hi++ )
    {
        for( wj = TDataL; wj <= TDataR; wj++ )
        {
            bin = (pGray_data[hi * FRAME_W + wj] >> SPACING);
            *(hist_template + bin) += *weightedD++;
        }
    }

    for ( mk = 0; mk < BIT_LEVEL; mk += 8 )
    {
        *(hist_template + mk) = ((*(hist_template + mk) * divisor) >> 15);
        *(hist_template + mk + 1) = ((*(hist_template + mk + 1) * divisor) >> 15);
        *(hist_template + mk + 2) = ((*(hist_template + mk + 2) * divisor) >> 15);
        *(hist_template + mk + 3) = ((*(hist_template + mk + 3) * divisor) >> 15);
        *(hist_template + mk + 4) = ((*(hist_template + mk + 4) * divisor) >> 15);
        *(hist_template + mk + 5) = ((*(hist_template + mk + 5) * divisor) >> 15);
        *(hist_template + mk + 6) = ((*(hist_template + mk + 6) * divisor) >> 15);
        *(hist_template + mk + 7) = ((*(hist_template + mk + 7) * divisor) >> 15);
    }
}


/************************************************************************/
//权重直方图函数
//参数列表依次对应

//原始图像数据
//权重值数据存储区
//直方图数据存储区
//目标数据左边界
//目标数据右边界
//目标数据上边界
//目标数据下边界
//仅仅是一个除数，归一化使用的
/************************************************************************/
void Histogram_Data
    (
    const unsigned char  *pGray_data,
    const unsigned short int *weightedD,
    unsigned int       *hist_particle,
    const signed short int TDataL,
    const signed short int TDataR,
    const signed short int TDataT,
    const signed short int TDataB,
    const signed short int divisor
    )

{
    unsigned int hi, wj, mk;
    unsigned char  bin;

    for( hi = TDataT; hi <= TDataB; hi++ )
    {
        for( wj = TDataL; wj <= TDataR; wj++ )
        {
            bin = (pGray_data[hi * FRAME_W + wj] >> SPACING);
            *(hist_particle + bin) += *weightedD++;
        }
    }

    for ( mk = 0; mk < BIT_LEVEL; mk += 8 )
    {
        *(hist_particle + mk) = ((*(hist_particle + mk) * divisor) >> 15);
        *(hist_particle + mk + 1) = ((*(hist_particle + mk + 1) * divisor) >> 15);
        *(hist_particle + mk + 2) = ((*(hist_particle + mk + 2) * divisor) >> 15);
        *(hist_particle + mk + 3) = ((*(hist_particle + mk + 3) * divisor) >> 15);
        *(hist_particle + mk + 4) = ((*(hist_particle + mk + 4) * divisor) >> 15);
        *(hist_particle + mk + 5) = ((*(hist_particle + mk + 5) * divisor) >> 15);
        *(hist_particle + mk + 6) = ((*(hist_particle + mk + 6) * divisor) >> 15);
        *(hist_particle + mk + 7) = ((*(hist_particle + mk + 7) * divisor) >> 15);
    }

}


/************************************************************************/
//相似度计算函数
//参数列表依次对应

//初始目标权重直方图
//备选目标权重直方图
/************************************************************************/
signed short int Distancer
    (
    const unsigned int *hist_template,
    const unsigned int *hist_particle
    )

{
    unsigned int nk, temp_val0, temp_val1, temp_val2, temp_val3;
    signed int similar_sum = 0;

    for ( nk = 0; nk < BIT_LEVEL; nk += 4 )
    {
        temp_val0 = (*(hist_particle + nk)) * (*(hist_template + nk));
        temp_val1 = (*(hist_particle + nk + 1)) * (*(hist_template + nk + 1));
        temp_val2 = (*(hist_particle + nk + 2)) * (*(hist_template + nk + 2));
        temp_val3 = (*(hist_particle + nk + 3)) * (*(hist_template + nk + 3));

        similar_sum += floor(sqrt(temp_val0) + 0.5);
        similar_sum += floor(sqrt(temp_val1) + 0.5);
        similar_sum += floor(sqrt(temp_val2) + 0.5);
        similar_sum += floor(sqrt(temp_val3) + 0.5);

        //similar_sum += _IQ0sqrt(temp_val0);
        //similar_sum += _IQ0sqrt(temp_val1);
        //similar_sum += _IQ0sqrt(temp_val2);
        //similar_sum += _IQ0sqrt(temp_val3);
    }

    return (similar_sum);
}




