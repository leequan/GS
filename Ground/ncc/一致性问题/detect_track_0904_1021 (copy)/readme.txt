Note:
  1.Detection and Ncc tracking for car groud on CPU;
  2.Add ncc_similarity method to match target what tacking failed then detection again;
  3.First,mainly with detection target what maxprop before tracking win;Second,during tracking phase save track_target information every frame;Third,with detection all target on whole image and co-option target on 256x256 area to match;Finally,if no-match for five consecutive frames with detection target what maxprop to track.
