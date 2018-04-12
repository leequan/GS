#include "track.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


unsigned int posi_sigma[NUM_TEMPLATE_PIXEL];
unsigned int nega_sigma[NUM_TEMPLATE_PIXEL];
unsigned short int posi_templates[SIZE_POSITIVE];
unsigned short int nega_templates[SIZE_NEGATIVE];
unsigned short int particle_templates[SIZE_PARTICLE];
unsigned short int posi_mean[NUM_TEMPLATE_PIXEL];
unsigned short int nega_mean[NUM_TEMPLATE_PIXEL];
unsigned short int CHi_initial[TEMPLATE_SIZE_H];
unsigned short int CWi_initial[TEMPLATE_SIZE_W];
unsigned char CHf_initial[TEMPLATE_SIZE_H];
unsigned char CWf_initial[TEMPLATE_SIZE_W];
float float_map_templates[NUM_PARTICLE];
float particle_data[4 * NUM_PARTICLE];
float random_mtx[NUM_PSEUDORANDOM];
float ncc_maxc[1]={0};
float ncc_mean=0;
float ncc_forw=1;
int small_ncc_num=0;
unsigned short int max_value_num;
float affsig[3] = {4, 4, 0.20};
// new add
float radioWH;

int nccx[20]={0};
int nccy[20]={0};
int x,y,sumx=0,sumy=0;
int lastx,lasty;
int num_frame=1;
TargetData_TYPE TargetData_Parameter;

unsigned char flagtrackerror;
unsigned char num_still = 0;

int CandiH[2*RADIUS + 1],CandiW[2*RADIUS + 1];
void init()
{
    int i;
    for(i=0;i<2*RADIUS + 1;i++)
    {
        CandiH[i]=-RADIUS+i;
        CandiW[i]=-RADIUS+i;
    }
}

char track(unsigned char* pSource_data,char o,int dx,int dy,int ww,int hh)
{
    int i;
    TargetData_Parameter.track_state = 1;  // initialization
    TargetData_Parameter.TrackStatus = 1;  // initialization
    Gaussian_Distribution
    (
        random_mtx
        );
    init();
    if(o==1)
    {
        affine_training_particle(particle_data,particle_templates,random_mtx,CandiH,CandiW,CHf_initial,CWf_initial,CHi_initial,CWi_initial,pSource_data,&TargetData_Parameter);
        norm_vector(particle_templates,NUM_PARTICLE);
        ratio_classifier(nega_mean,posi_mean,particle_templates,float_map_templates);
        max_value_num = max_address(ncc_maxc,&TargetData_Parameter,float_map_templates);
        //printf("max_value_num = %d \n",max_value_num);
        x=particle_data[max_value_num+NUM_PARTICLE] + 0.5-TargetData_Parameter.TCenter_W;
        y=particle_data[max_value_num] + 0.5-TargetData_Parameter.TCenter_H ;

        TargetData_Parameter.TCenter_H = particle_data[max_value_num] + 0.5;
        TargetData_Parameter.TCenter_W = particle_data[NUM_PARTICLE + max_value_num] + 0.5;
        TargetData_Parameter.TSize_H = particle_data[2 * NUM_PARTICLE + max_value_num] + 0.5;
        TargetData_Parameter.TSize_W =  TargetData_Parameter.TSize_H * radioWH;
        TargetData_Parameter.TSizeHalf_H = (TargetData_Parameter.TSize_H >> 1);
        TargetData_Parameter.TSizeHalf_W = (TargetData_Parameter.TSize_W >> 1);


        // printf(" CenterX = %d, CenterY = %d \n",TargetData_Parameter.TCenter_W,TargetData_Parameter.TCenter_H);
        // printf("2 X = %d, Y = %d \n",x,y);
        num_frame++;
        //printf("num_frame = %d \n",num_frame);
        lastx = nccx[(num_frame-2)%20];
        lasty = nccy[(num_frame-2)%20];
        nccx[(num_frame-2)%20]=x;
        nccy[(num_frame-2)%20]=y;

        display_rectangle (&TargetData_Parameter,pSource_data,255);
        printf("--[%d,%d]--\r\n",TargetData_Parameter.TCenter_W,TargetData_Parameter.TCenter_H);

        if((TargetData_Parameter.TCenter_H < 25)||(TargetData_Parameter.TCenter_H > FRAME_H - 25)||(TargetData_Parameter.TCenter_W < 25)||(TargetData_Parameter.TCenter_W > FRAME_W - 25))
           return 0;

        flagtrackerror = 0;

        // 1. track error 1
        if(2 <= num_frame && num_frame <= 25)
        {
            if(ncc_forw-ncc_maxc[0]<0.2)
            {
                 small_ncc_num=0;
                 ncc_forw = ncc_maxc[0];
            }
            else
            {
                small_ncc_num++;
            }

            if(small_ncc_num > 5)
            {
                printf("1~~~~~~~~~~~ \n");
                flagtrackerror = 1;
                small_ncc_num = 0;
            }

        }
        else
        {  // 2. track error 2
             sumx=0;
             sumy=0;
             for(i = num_frame-1; i < num_frame + 11; i++)
             {
                 sumx += nccx[i%20];
                 sumy += nccy[i%20];
             }
             if((abs(x-sumx/20)>5)||(abs(y-sumy/20)>5))
             {
                  printf("2~~~~~~~~~~~ \n");
                  flagtrackerror = 1;
             }
        }

        // 3. track error 3
        if( ncc_maxc[0] < 0.5 )
        {
            printf("3~~~~~~~~~~~ \n");
            flagtrackerror = 1;
        }
        // 4. track error 4
        if ( x == 0 && y == 0)
            num_still++;
        else
            num_still = 0;

        if ( num_still >=5 )
        {
            printf("4~~~~~~~~~~~ \n");
            flagtrackerror = 1;
            num_still = 0;
        }

        if ( flagtrackerror == 1 )
        {
           nccx[(num_frame-2)%20] = lastx;
           nccy[(num_frame-2)%20] = lasty;
           return 0;
        }
        else
           return 1;

        // update negative module
        if (num_frame % 10 == 0)
        {
            affine_training_nega(RANDOM_NEGAH64,RANDOM_NEGAW64,CHf_initial,CWf_initial,CHi_initial,CWi_initial,pSource_data,nega_templates,&TargetData_Parameter);
            norm_vector(nega_templates,NUM_NEGATIVE);
            nega_templates_initial(nega_mean,nega_templates,nega_sigma);
        }
    }
    else
    {
        printf("right here...\n");
        num_frame=1;
        //printf("num_frame = %d \n",num_frame);
        TargetData_Parameter.TCenter_H = dy;
        TargetData_Parameter.TCenter_W = dx;
        TargetData_Parameter.TSize_H = hh;
        TargetData_Parameter.TSize_W = ww;
        TargetData_Parameter.TSizeHalf_H = (TargetData_Parameter.TSize_H >> 1);
        TargetData_Parameter.TSizeHalf_W = (TargetData_Parameter.TSize_W >> 1);

        radioWH = 1.0 * TargetData_Parameter.TSizeHalf_W / TargetData_Parameter.TSizeHalf_H;

        affine_training_posi(RANDOM_POSIH32,RANDOM_POSIW32,CHf_initial,CWf_initial,CHi_initial,CWi_initial,pSource_data,posi_templates,&TargetData_Parameter);
        affine_training_nega(RANDOM_NEGAH64,RANDOM_NEGAW64,CHf_initial,CWf_initial,CHi_initial,CWi_initial,pSource_data,nega_templates,&TargetData_Parameter);
        norm_vector(posi_templates,NUM_POSITIVE);
        norm_vector(nega_templates,NUM_NEGATIVE);
        posi_templates_initial(posi_mean,posi_templates,posi_sigma);
        nega_templates_initial(nega_mean,nega_templates,nega_sigma);

        display_rectangle (&TargetData_Parameter,pSource_data,0);
        printf("detect CenterX = %d, CenterY = %d, h = %d, w = %d \n",dx,dy,hh,ww);

        return 1;
    }
}
