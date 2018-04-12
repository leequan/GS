#ifndef TRACK
#define TRACK


/* user-defined */
typedef char                CHAR;
typedef signed char         SINT8;
typedef unsigned char       UINT8;
typedef signed short int    SINT16;
typedef unsigned short int  UINT16;
typedef signed int          SINT32;
typedef unsigned int        UINT32;

typedef struct
{
    float SCenter_FH;
    float SCenter_FW;
    float ratio_h;
    float ratio_w;
    signed short int TCenter_H;
    signed short int TCenter_W;
    unsigned char TSize_H;
    unsigned char TSize_W;
    unsigned char TSizeHalf_H;
    unsigned char TSizeHalf_W;
    unsigned char StartKalman;  // initialization

    unsigned char TrackStatus;  // initialization ״̬
    unsigned char track_state;  // initialization״̬
}TargetData_TYPE;


/* Global Parameter */
#define DEBUG_MODE ( 0 )

#define FRAME_W ( 720 )
#define FRAME_H ( 576 )

#define RADIUS (10)

#define HALF_FRAME_H ( 576 )

/* 0 -- 255 8X */
#define NUM_POSITIVE ( 48 )

/* 0 -- 255 8X */
#define NUM_NEGATIVE ( 192 )

/* 0 -- 512 8X */
#define NUM_PARTICLE ( (2 * RADIUS +1 ) * (2 * RADIUS +1) )

// new parameter
#define TH_TARGETSIZE ( 100 )

/* 0 -- 65535 */
#define NUM_PSEUDORANDOM ( 3000 )//2560

/* 0 <= UPDATE_CYCLE <= PATH_COHERENCE_NUM */
#define UPDATE_CYCLE ( 6 )
#define PATH_COHERENCE_NUM ( 15 )


#define EPS ( 2.2204e-16)

/* 0 -- 1 */
#define UPDATE_RATE ( 0.90f )


#define BEFORE_UPDATE_RATE ( 922 )
#define AFTER_UPDATE_RATE ( 102 )

#define BEFORE_UR ( 115 )
#define AFTER_UR ( 13 )

#define SCALE_SIG ( 0.20f )


#define IMALEW ( 30 )
#define IMARIW ( 690 )
#define IMATOW ( 16 )
#define IMABOW ( 272 )

#define IMALE ( 104 )
#define IMARI ( 616 )
#define IMATO ( 24 )
#define IMABO ( 264 )


/* 0 -- 255 8X */
#define TEMPLATE_SIZE_H ( 16 )
#define TEMPLATE_SIZE_W ( 16 )


/* 0 -- 65535 8X */
/* TEMPLATE_SIZE_H * TEMPLATE_SIZE_W */
#define NUM_TEMPLATE_PIXEL ( 256 )

/* TEMPLATE_SIZE_H >> 1 4X */
/* TEMPLATE_SIZE_W >> 1 4X */
#define TEMPLATE_HALF_H ( 8 )
#define TEMPLATE_HALF_W ( 8 )

#define SIZE_POSITIVE ( NUM_POSITIVE * NUM_TEMPLATE_PIXEL )
#define SIZE_NEGATIVE ( NUM_NEGATIVE * NUM_TEMPLATE_PIXEL )
#define SIZE_PARTICLE ( NUM_PARTICLE * NUM_TEMPLATE_PIXEL )

#define BIT_LEVEL ( 128 )

#define SPACING ( 1 )
/* (256 / BIT_LEVEL) = 2^SPACING = 2^1 */

#define SPACEAREA ( 512 )
/* 128 * 4 = 512 */



const float RANDOM_POSIH32[48] =
{
    -1.22025  ,0.05243   ,0.56542   ,0.08070   ,1.04369   ,-0.46882  ,
    0.35345   ,-1.03603  ,0.07767   ,0.79726   ,-0.62909  ,2.73354   ,
    -0.47887  ,0.97499   ,1.16541   ,0.24058   ,1.27495   ,-0.08741  ,
    0.99396   ,-1.00419  ,-1.04571  ,0.33195   ,-1.90684  ,0.69719   ,
    -1.54380  ,0.86708   ,-0.38624  ,-0.79646  ,0.41781   ,-0.85435  ,
    2.74847   ,0.43398   ,-0.82709  ,0.49790   ,-0.79383  ,-0.55906  ,
    0.54466   ,0.61988   ,1.10720   ,-1.12142  ,1.56104   ,-0.24235  ,
    -1.92013  ,0.75296   ,-0.77023  ,0.09319   ,0.66352   ,1.61988
};

const float RANDOM_POSIW32[48] =
{
    -0.37624  ,-1.95393  ,0.07779   ,0.79471   ,0.80218   ,-0.04659  ,
    -1.25680  ,-0.42696  ,1.79093   ,-0.44250  ,1.53380   ,0.16744   ,
    -1.41265  ,0.28972   ,-0.90871  ,-0.65405  ,-0.55119  ,-0.34782  ,
    0.42844   ,-0.63269  ,0.50571   ,-1.63541  ,-0.04040  ,0.16880   ,
    -1.55180  ,-0.14537  ,1.31620   ,0.13544   ,0.81988   ,0.35732   ,
    -1.51298  ,-0.22977  ,-0.83198  ,2.31563   ,0.54096   ,1.97657   ,
    -0.13791  ,-0.00558  ,-0.18559  ,0.24645   ,-1.19662  ,1.00483   ,
    0.62545   ,0.21348   ,-0.00713  ,0.93525   ,-0.35023  ,-0.05083
};



const float RANDOM_NEGAH64[192] =
{
    0.23599   ,-1.27596  ,0.61270   ,0.39532   ,-0.49769  ,-0.68783  ,2.36522   ,0.64745   ,1.33955   ,0.20872   ,0.51202   ,-0.04399  ,
    -0.63005  ,2.68303   ,0.55300   ,1.03064   ,0.65212   ,0.24519   ,-2.27510  ,0.41547   ,-0.29635  ,-0.90483  ,-0.72580  ,-0.42185  ,
    1.34188   ,1.81794   ,-1.45174  ,0.93450   ,0.16023   ,0.63291   ,-0.58171  ,-0.44910  ,0.71744   ,0.16673   ,1.68940   ,-0.58263  ,
    0.77945   ,0.69637   ,-0.03882  ,-0.78966  ,0.00633   ,-0.85493  ,-0.09097  ,1.19482   ,0.54051   ,-0.96769  ,-0.34788  ,1.34115   ,
    0.87514   ,0.32099   ,1.06243   ,0.87680   ,-0.41489  ,0.03622   ,1.77102   ,2.73038   ,0.56430   ,2.72923   ,-0.79026  ,-1.31990  ,
    0.27187   ,1.43714   ,0.92393   ,0.66112   ,0.15676   ,-0.50003  ,1.33729   ,0.05405   ,-0.63271  ,-0.07545  ,2.18424   ,0.71634   ,
    0.43399   ,-1.09224  ,-0.40492  ,-1.00696  ,1.78487   ,-0.00870  ,1.20325   ,0.39705   ,-0.23150  ,1.68285   ,-1.20603  ,-0.09212  ,
    -0.21919  ,-0.32080  ,-0.36496  ,0.17434   ,-0.15261  ,0.45828   ,0.62009   ,0.59802   ,-1.78074  ,-1.71360  ,-0.61963  ,0.04066   ,
    -0.63052  ,0.78233   ,0.30241   ,-0.57413  ,-0.05053  ,-0.25736  ,-0.57076  ,0.99144   ,0.77684   ,-0.56438  ,0.39468   ,0.43692   ,
    0.15377   ,-0.18016  ,0.89675   ,0.54752   ,-0.36227  ,0.21671   ,0.17887   ,-0.11018  ,0.56049   ,-0.15395  ,0.24112   ,-0.29191  ,
    1.75529   ,0.82526   ,-0.53420  ,-0.10065  ,-1.51442  ,-0.75813  ,-2.22199  ,0.00063   ,0.40432   ,0.85978   ,-1.63939  ,-0.28383  ,
    0.18117   ,0.68775   ,1.42537   ,0.03776   ,0.14956   ,1.52389   ,2.00993   ,0.01148   ,-1.73891  ,0.21916   ,-0.95098  ,0.07143   ,
    0.77416   ,-0.23511  ,0.60924   ,0.27746   ,0.17359   ,-0.60352  ,0.36065   ,-0.30682  ,-0.23855  ,-0.32734  ,-0.13000  ,-0.44377  ,
    -0.85952  ,-0.51599  ,-0.70716  ,-1.20044  ,0.34037   ,1.11392   ,-0.39179  ,-0.34463  ,-0.57483  ,-1.18500  ,0.56506   ,2.22333   ,
    -0.04600  ,0.07595   ,-1.91947  ,-1.22513  ,2.37425   ,0.40388   ,-1.68476  ,0.50175   ,0.15780   ,0.72306   ,-0.79638  ,1.68654   ,
    -0.50508  ,1.07236   ,0.27015   ,0.62976   ,1.37695   ,0.14123   ,-0.49494  ,-0.93363  ,-0.20052  ,-0.88335  ,-0.60387  ,-0.83817
};


const float RANDOM_NEGAW64[192] =
{
    -0.83517  ,0.61704   ,0.28938   ,-0.87056  ,-0.10667  ,0.33188   ,-0.48223  ,-1.03442  ,-0.96914  ,-0.61859  ,0.01135   ,2.94909   ,
    -0.04688  ,-1.14669  ,-1.07646  ,0.32753   ,-0.27886  ,1.47251   ,-1.63329  ,-0.65477  ,-1.49692  ,-0.40418  ,-0.86649  ,-0.94267  ,
    -0.98843  ,-0.37444  ,-0.61868  ,1.05593   ,0.28740   ,-1.45904  ,-1.83015  ,0.94927   ,2.28783   ,-2.15649  ,1.28228   ,0.22261   ,
    0.38471   ,-0.11272  ,0.08809   ,1.42296   ,0.68648   ,-1.07524  ,-0.25277  ,0.60606   ,-1.44494  ,0.20205   ,1.29009   ,-0.58080  ,
    1.39545   ,1.62338   ,0.21411   ,0.19441   ,0.35846   ,-0.36463  ,0.22127   ,-0.29617  ,1.58262   ,0.30356   ,0.80338   ,-0.27385  ,
    1.48955   ,-0.02756  ,-0.32128  ,1.91529   ,-0.30054  ,0.71647   ,2.12568   ,0.16304   ,1.61199   ,-0.47324  ,0.80988   ,-1.00557  ,
    0.52014   ,-0.22579  ,0.52786   ,1.08903   ,-0.30375  ,0.50664   ,0.52202   ,-0.48281  ,0.61339   ,0.56839   ,0.43306   ,-0.24405  ,
    -0.87977  ,-0.78442  ,0.11727   ,-0.21566  ,0.03369   ,1.28163   ,-0.28667  ,-0.24553  ,-2.34724  ,-0.23713  ,-0.72016  ,-0.65898  ,
    0.60963   ,2.43658   ,0.05832   ,-0.19521  ,-1.75578  ,0.74954   ,0.49423   ,1.07714   ,-2.25984  ,0.90149   ,0.00485   ,1.13007   ,
    -0.75863  ,-0.20779  ,0.41231   ,0.14784   ,0.06114   ,-1.39812  ,0.92758   ,1.57240   ,-0.42034  ,-0.27520  ,0.75469   ,0.45845   ,
    0.93149   ,-0.81481  ,0.24255   ,-1.62505  ,1.02619   ,2.07834   ,0.44876   ,-0.75624  ,-0.79385  ,0.06687   ,-2.42475  ,1.14581   ,
    0.05425   ,-1.39341  ,-0.89385  ,-0.36357  ,-1.94452  ,0.54581   ,1.41667   ,-0.93905  ,0.01697   ,1.04582   ,0.79480   ,-0.77369  ,
    0.26565   ,1.87729   ,-0.11027  ,0.08705   ,0.45359   ,-0.79702  ,1.88509   ,-1.01272  ,-0.48401  ,0.47552   ,-0.59435  ,-1.32253  ,
    -1.41300  ,-1.08510  ,-0.30165  ,-0.01936  ,-0.96323  ,-1.58611  ,-1.48469  ,-1.32392  ,-0.74071  ,-2.06486  ,2.00014   ,-0.49218  ,
    -0.46575  ,-0.91877  ,-0.03644  ,-1.90243  ,-0.23335  ,1.19247   ,0.41315   ,0.08306   ,-0.52794  ,-0.84994  ,0.72534   ,-0.38639  ,
    0.40807   ,0.96798   ,-0.63441  ,-0.07923  ,-1.40821  ,1.28972   ,-0.62482  ,-0.27872  ,0.13674   ,0.08254   ,-0.36873  ,-0.28248

};


char track(unsigned char* pSource_data,char o,int dx,int dy,int ww,int hh);
void Weighted_Distribution
    (
    unsigned short int *weightedD,
    signed short int   *stateParameter
    );


void Weighted_Histogram
    (
    const unsigned char      *pGray_data,
    unsigned int             *hist_template,
    const unsigned short int *weightedD,
    const signed short int   *stateParameter
    );


void Histogram_Data
    (
    const unsigned char      *pGray_data,
    const unsigned short int *weightedD,
    unsigned int             *hist_particle,
    const signed short int TDataL,
    const signed short int TDataR,
    const signed short int TDataT,
    const signed short int TDataB,
    const signed short int divisor
    );


signed short int Distancer
    (
    const unsigned int *hist_template,
    const unsigned int *hist_particle
    );


void Bilinear_Initial
    (
    unsigned char *CHf_initial,
    unsigned char *CWf_initial,
    unsigned short int *CHi_initial,
    unsigned short int *CWi_initial,
    const TargetData_TYPE *TargetData_Parameter
    );


void Bilinear_Interpolation_PN
    (
    const unsigned char *CHf_initial,
    const unsigned char *CWf_initial,
    const unsigned short int *CHi_initial,
    const unsigned short int *CWi_initial,
    const unsigned char *pSource_data,
    unsigned short int *pTemplate_data
    );


void Bilinear_Interpolation_center
    (
    const unsigned char *pSource_data,
    unsigned short int *pTemplate_data,
    const TargetData_TYPE *TargetData_Parameter
    );


void Bilinear_Interpolation_particle
    (
    const unsigned char *pSource_data,
    unsigned short int *pTemplate_data,
    const TargetData_TYPE *TargetData_Parameter
    );


unsigned char Gaussian_Distribution
    (
    float *random_matrix
    );



void display_rectangle
    (
    const TargetData_TYPE *TargetData_Parameter,
    unsigned char *image_data,
    const unsigned char gray_value
    );



void display_particle
    (
    const signed short int Center_H,
    const signed short int Center_W,
    unsigned char *image_data,
    const unsigned char gray_value
    );


void display_boundary
    (
    const signed short int DataL,
    const signed short int DataR,
    const signed short int DataT,
    const signed short int DataB,
    unsigned char *image_data,
    const unsigned char gray_value
    );

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
    );

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
    );

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
    );

//void affine_training_particle
//    (
//    float *particle_data,
//    unsigned short int *particle_templates,
//    const float *random_matrix,
//    const float *affsig,
//    unsigned char *CHf_initial,
//    unsigned char *CWf_initial,
//    unsigned short int *CHi_initial,
//    unsigned short int *CWi_initial,
//    const unsigned char *pSource_data,
//    TargetData_TYPE *TargetData_Parameter
//    );

void norm_vector
    (
    unsigned short int *template_matrix,
    const unsigned short int num_template
    );

void ones_initial
    (
    float *vector_sigma
    );

void classifier_update
    (
    const unsigned short int *nega_templates,
    unsigned short int *nega_mean,
    unsigned short int *posi_mean,
    unsigned int *nega_sigma,
    unsigned int *posi_sigma
    );

void nega_templates_initial
    (
    unsigned short int *nega_mean,
    const unsigned short int *nega_templates,
    unsigned int *nega_sigma
    );


unsigned short int max_address
    (
    //const signed int *like_lihood,
    //signed int *apriority_value,
    float *max_value,
    TargetData_TYPE *TargetData_Parameter,
    const float *float_like_lihood
    );

void MAP_float
    (
    float *map_templates,
    float *like_lihood
    );

void ratio_classifier
    (
    const unsigned short int *nega_mean,
    const unsigned short int *posi_mean,
    const unsigned short int *particle_templates,
    //signed int *map_templates,
    //const unsigned int *nega_sigma,
    //const unsigned int *posi_sigma,
    float *float_map_templates
    );


void MAP
    (
    signed int *map_templates,
    signed int *like_lihood
    );


void affsig_update
    (
    float *affsig,
    const signed short int *hc_record,
    const signed short int *wc_record,
    const unsigned char track_state
    );


void posi_templates_initial
    (
    unsigned short int *posi_mean,
    const unsigned short int *posi_templates,
    unsigned int *posi_sigma
    );


void center_update
    (
    signed short int *hc_record,
    signed short int *wc_record,
    const TargetData_TYPE *TargetData_Parameter
    );


void mean_std_apriority
    (
    signed int *apriority_value,
    signed int *mean_value,
    signed int *standard_value,
    signed int *gap_value
    );


void apriority_judge
    (
    signed int *apriority_value,
    unsigned char *num_apriority
    );

// void running_forecast
// 	(
// 	TargetData_TYPE *TargetData_Parameter,
// 	signed int *Backup_Target_W,
// 	signed int *Backup_Target_H,
// 	signed int *hcode,
// 	signed int *wcode,
// 	signed int *hcode_velocity,
// 	signed int *wcode_velocity,
// 	unsigned char *unsuccess_num
// 	);

void running_forecast
    (
    TargetData_TYPE *TargetData_Parameter,
    float *Backup_Target_W,
    float *Backup_Target_H,
    float *Backup_Target_VW,
    float *Backup_Target_VH,
    unsigned char *unsuccess_num,
    const signed int track_frame
    );

signed char road_map
    (
    signed int *hcode_velocity,
    signed int *wcode_velocity,
    signed int *hmean_velocity,
    signed int *wmean_velocity,
    signed int *hcode,
    signed int *wcode,
    signed int *Backup_Target_W,
    signed int *Backup_Target_H
    );


void update_similar_apriority
    (
    signed int *similar_apriority,
    signed int *apriority_value
    );


void compute_margin
    (
    const signed int *similar_apriority,
    signed int *apriority_max6,
    signed int *apriority_min6,
    signed int *apriority_max5,
    signed int *apriority_min5,
    const signed int track_frame
    );
#endif // TRACK

