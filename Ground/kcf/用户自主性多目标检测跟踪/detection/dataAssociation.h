#ifndef DATAASSOCIATION_H
#define DATAASSOCIATION_H

#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "darknet.h"

typedef struct
{
    box rect;//target left-top coordinate,width,height
    int intNum;//interconnection num
    char holdNum;//keep num
    char markFlag;//target mark
    int IP;//target label
}AREA;

//init data
void initData(AREA *curTag, AREA *befTag, char *tagStatus);
//data coversion
int dataConversion(box* obj_rect, int obj_num, AREA *curTag);
//data association
int dataAssociation(AREA *bef, int befNum, AREA *cur, int curNum, box *obj_rect, char detect_flag);


#endif // DATAASSOCIATION_H

