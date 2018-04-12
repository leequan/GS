#include "dataAssociation.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//the status of target label
extern char tagStatus[10];

//init data
//AREA *curTag: the area of target detection in the current frame
//AREA *befTag: the area of target detection in the before frame
//char *tagStatus: the status of target label
void initData(AREA *curTag, AREA *befTag, char *tagStatus)
{
    int i;
    //the number of target detection in the current frame
    for(i = 0; i < 20; i++){
        curTag[i].rect.x = 0;
        curTag[i].rect.y = 0;
        curTag[i].rect.w = 0;
        curTag[i].rect.h = 0;
        curTag[i].holdNum = 5;
        curTag[i].intNum = 0;
        curTag[i].IP = 0;
        curTag[i].markFlag = 0;
    }
    //the number of target detection in the before frame
    for(i = 0; i < 40; i++){
        befTag[i].rect.x = 0;
        befTag[i].rect.y = 0;
        befTag[i].rect.w = 0;
        befTag[i].rect.h = 0;
        befTag[i].holdNum = 5;
        befTag[i].intNum = 0;
        befTag[i].IP = 0;
        befTag[i].markFlag = 0;
    }
    for(i = 0; i < 10; i++){
        tagStatus[i] = 0;
    }
}

//data conversion
//box* obj_rect: the boxes of target detection in the current frame
//int obj_num: the number of target detection in the current frame
//AREA *curTag: the aera of target detection in the current frame
int dataConversion(box* obj_rect, int obj_num, AREA *curTag)
{
    int i;
    for(i = 0; i < obj_num; i++){
        curTag[i].rect = obj_rect[i];
        curTag[i].holdNum = 5;
        curTag[i].intNum = 0;
        curTag[i].IP = 0;
        curTag[i].markFlag = 0;
    }
    return obj_num;
}

//data association
//AREA *bef: the area of target detection in the before frame
//int befNum: the number of target detection in the before frame
//AREA *cur: the area of target detection in the current frame
//int curNum: the number of target detection in the current frame
//box *obj_rect: the box of target detection in the current frame
//char into_detect: into the detect flag
int dataAssociation(AREA *bef, int befNum, AREA *cur, int curNum, box *obj_rect, char into_detect)
{
    int i, j, k;
    char flag = 1;//match flag
    int newNum = befNum;//matched number
    int cur_centerX, cur_centerY;//current target center
    int bef_centerX, bef_centerY;//before target center

    //frist frame
    if(into_detect == 1){
        i = 0;
        curNum = dataConversion(obj_rect, curNum, cur);
        for(j = 0; j < curNum; j++){
            if(cur[j].markFlag == 0){
                bef[i] = cur[j];
                i++;
            }
        }
        return i;
    }
    //after the second frame
    else{
        curNum = dataConversion(obj_rect, curNum, cur);
    }

    //clear IP
    for(i = 0; i < befNum; i++){
        bef[i].holdNum--;//no matched,keep num minus one
        if(bef[i].holdNum <= 0){
            tagStatus[bef[i].IP - 1] = 0;//clear target status
            bef[i].IP = 0;//clear target IP

            bef[i] = bef[befNum - 1];//target area conversion
            befNum = befNum > 0 ? befNum - 1 : 0;
            i = i > 0 ? i - 1 : 0;
        }
    }

    newNum = befNum;

    //target match
    for(i = 0; i < curNum; i++){
        cur_centerX = cur[i].rect.x + (cur[i].rect.w + 1) / 2;
        cur_centerY = cur[i].rect.y + (cur[i].rect.h + 1) / 2;
        for(j = 0; j < befNum; j++){
            if(bef[j].markFlag == 1)continue;
            bef_centerX = bef[j].rect.x + (bef[j].rect.w + 1) / 2;
            bef_centerY = bef[j].rect.y + (bef[j].rect.h + 1) / 2;
            //matched conditions
            flag = !cur[i].markFlag;
            flag = flag && (!cur[i].intNum);
            flag = abs(cur_centerX - bef_centerX) > 15 ? 0 : flag;
            flag = abs(cur_centerY - bef_centerY) > 15 ? 0 : flag;

            //match success
            if(flag){
                cur[i].markFlag = 1;//matched success, set 1
                cur[i].intNum = bef[j].intNum + 1;//matched,intconnectted num plus one
                k = bef[j].IP ;
                bef[j] = cur[i];//data conversion
                bef[j].IP = k;
                break;
            }
        }
        //match failed
        if(cur[i].markFlag == 0){
            bef[newNum] = cur[i];
            newNum++;
        }
    }

    //delete
    for(i = 0; i< befNum; i++){
        bef[i].markFlag = 0;//after matched success, set 0
        if(bef[i].holdNum <= 0){
            bef[i] = bef[newNum - 1];//target area conversion
            newNum = newNum > 0 ? newNum - 1 : 0;
        }
        befNum = newNum < befNum ? befNum - 1 : befNum;
    }
    befNum = newNum;

    //label
    for(i = 0; i < befNum; i++){
        //label conditions
        if(bef[i].intNum > 4 && bef[i].IP == 0){
            for(j = 0; j < 10; j++){
                if(tagStatus[j] == 0){
                    bef[i].IP = j + 1;
                    tagStatus[j] = 1;
                    break;
                }
            }
        }
    }
    return befNum;
}
