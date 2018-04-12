note:
  This is detect and kcf track for car groud with GPU.
  The Makefile file is mainly modified by yangdong.
  Add #include "tagEdgeExtraction.cpp" in main.cpp.
  Delete #include "track.hpp" 
  "box detect_image(Mat frame, char *frame_file, double thresh, double hier_thresh, TargetData_TYPE TargetData_Parameter);" 
  "box select_obj_box_all(Mat frame, detection_info **info, int num,TargetData_TYPE TargetData_Parameter);"
  "box select_obj_box_big_ncc(Mat img, detection_info **info1, int tagnum, TargetData_TYPE TargetData_Parameter);" in detection.h
