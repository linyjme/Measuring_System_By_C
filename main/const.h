#ifndef __CONST_H
#define __CONST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"
	
#define BOX1             0
#define BOX2             1
#define BOX3             0
#define BOX4             0	

#define FLASH_BASE_ADDR  0x8030000     // 存储系数的初始地址
#define OIL_TYPE_OFFSET  0x1000     // 存储纯净水水类型号的FLASH偏移地址
#define CONST_OFFSET     0x0800     // 存储温度补偿值的FLASH偏移地址
#define MAX_COUNT 	10	// 最多可以尝试读写flash的次数
#define STATE_TIMER 1000
	
	
#define  CONST_T3   (12.778)    // 计算温度时，三次项系数
#define  CONST_T2   (-4.6554)   // 计算温度时，二次项系数
#define  CONST_T1   (95.168)	// 计算温度时，一次项系数
#define  CONST_T0   (-128.96)   // 计算温度时，常数量

#define	 HU_MIN	    300   	    // 水位电压的下限-50
#define	 HU_MAX    2400	    // 水位电压的上限+80
#define	 TU_MIN	    500   	    // 温度电压的下限-50
#define	 TU_MAX	   1800   	    // 水位电压的上限-50
#define	 TU_CHA	    5		    // 两次温度电压的最大差值
#define	 HU_CHA	    5		    // 两次水位电压的最大差值

#define	 VER_L     0x05         // 软件版本号小数点后，BCD码
#define	 VER_H     0x02	        // 软件版本号小数点前，BCD码
#define	 VER_YEAR  0x18         // 水量传感器版本时间年（20XX），BCD码
#define	 VER_MON   0x12         // 水量传感器版本时间月份，BCD码
#define	 VER_DAY   0x10	        // 水量传感器版本时间日期，BCD码

#define	 REAL_OIL_TEMPER_MIN		(-55)   	    // 真实水位温度最小值
#define	 REAL_OIL_TEMPER_MAX		(125)   	    // 真实水位温度最大值

#define	 CONVER_OIL_TEMPER_MIN	(50)   	    // 转换后的水位温度最小值
#define	 CONVER_OIL_TEMPER_MAX	(1850)   	    // 转换后的水位温度最大值


//需要减去的零点体积
#define    WEIGHT_ZERO_1   9.8
#define    WEIGHT_ZERO_2   10.4
#define    WEIGHT_ZERO_3   9.8
#define    WEIGHT_ZERO_4   6.3

//满点体积保持不变
#define    WEIGHT_ZERO_rp3_1   967
#define    WEIGHT_ZERO_rp3_2   633
#define    WEIGHT_ZERO_rp3_3   967
#define    WEIGHT_ZERO_rp3_4   636

#define    WEIGHT_ZERO_rp5_1   990
#define    WEIGHT_ZERO_rp5_2   648
#define    WEIGHT_ZERO_rp5_3   990
#define    WEIGHT_ZERO_rp5_4   653

//需要减去的零点体积
#define    WEIGHT_ZERO_PROOF_1   9.8
#define    WEIGHT_ZERO_PROOF_2   10.4
#define    WEIGHT_ZERO_PROOF_3   9.8
#define    WEIGHT_ZERO_PROOF_4   6.3

//满点体积保持不变
#define    WEIGHT_ZERO_PROOF_rp3_1   967
#define    WEIGHT_ZERO_PROOF_rp3_2   633
#define    WEIGHT_ZERO_PROOF_rp3_3   967
#define    WEIGHT_ZERO_PROOF_rp3_4   636

#define    WEIGHT_ZERO_PROOF_rp5_1   990
#define    WEIGHT_ZERO_PROOF_rp5_2   648
#define    WEIGHT_ZERO_PROOF_rp5_3   990
#define    WEIGHT_ZERO_PROOF_rp5_4   653


//1号水箱
#if   BOX1
#define  VOLUME_MIN  12.458		// 零位点的纯净水体积
#define  VOLUME_MAX  1250.169	// 最大纯净水体积
#define  PROOF_MAX   27.24		// 特殊材料的最大体积

#define  CONST_6_W3   (0.0000384421)   //计算体积时，三次项系数
#define  CONST_6_W2   (-0.093529721)   //计算体积时，二次项系数
#define  CONST_6_W1   (74.62195189)    //计算体积时，一次项系数
#define  CONST_6_W0   (-19427.03231)   //计算体积时，常数量

#define  CONST_5_W3   (0.0000094085)   //计算体积时，三次项系数
#define  CONST_5_W2   (-0.01975593)    //计算体积时，二次项系数
#define  CONST_5_W1   (12.17791334)   //计算体积时，一次项系数
#define  CONST_5_W0   (-1820.096949)   //计算体积时，常数量

#define  CONST_4_W3   (0.0000043139)   //计算体积时，三次项系数
#define  CONST_4_W2   (-0.008475403)    //计算体积时，儿次项系数
#define  CONST_4_W1   (3.862023191)   //计算体积时，一次项系数
#define  CONST_4_W0   (220.7072494)   //计算体积时，常数量

#define  CONST_3_W3   (-0.0000000018)   //计算体积时，三次项系数
#define  CONST_3_W2   (-0.0000907193)    //计算体积时，二次项系数
#define  CONST_3_W1   ( -1.564592473)   //计算体积时，一次项系数
#define  CONST_3_W0   (1390.49385)   //计算体积时，常数量

#define  CONST_2_W3   (0.0000040545)   //计算体积时，三次项系数
#define  CONST_2_W2   (-0.003353257)    //计算体积时，二次项系数
#define  CONST_2_W1   (-0.690282731)   //计算体积时，一次项系数
#define  CONST_2_W0   (1312.408037)   //计算体积时，常数量

#define  CONST_1_W3   (0.0000048361)   //计算体积时，三次项系数
#define  CONST_1_W2   (-0.003183062)    //计算体积时，二次项系数
#define  CONST_1_W1   (-0.7851664910 )   //计算体积时，一次项系数
#define  CONST_1_W0   (1320.275386)   //计算体积时，常数量


#define  CONST_PROOF_6_W2   (0.00008287)   //计算体积时，二次项系数
#define  CONST_PROOF_6_W1   (-0.15573399)   //计算体积时，一次项系数
#define  CONST_PROOF_6_W0   (73.43722164)    //计算体积时，常数量

#define  CONST_PROOF_5_W2   (0.00008667)   //计算体积时，二次项系数
#define  CONST_PROOF_5_W1   (- 0.16241932)    //计算体积时，一次项系数
#define  CONST_PROOF_5_W0   (76.35122823)   //计算体积时，常数量

#define  CONST_PROOF_4_W2   (0.00000467)   //计算体积时，二次项系数
#define  CONST_PROOF_4_W1   (-0.04270281)    //计算体积时，一次项系数
#define  CONST_PROOF_4_W0   (32.63393348)   //计算体积时，常数量

#define  CONST_PROOF_3_W2   (-0.00000591)   //计算体积时，二次项系数
#define  CONST_PROOF_3_W1   (-0.03219374)    //计算体积时，一次项系数
#define  CONST_PROOF_3_W0   (30.03064475)   //计算体积时，常数量

#define  CONST_PROOF_2_W2   (-0.00001096)   //计算体积时，二次项系数
#define  CONST_PROOF_2_W1   (-0.02930142)    //计算体积时，一次项系数
#define  CONST_PROOF_2_W0   (29.56138899)   //计算体积时，常数量

#define  CONST_PROOF_1_W2   (-0.00003546)   //计算体积时，二次项系数
#define  CONST_PROOF_1_W1   (-0.02196103)    //计算体积时，一次项系数
#define  CONST_PROOF_1_W0   (29.01208132 )   //计算体积时，常数量

#endif

//2号水箱
#if   BOX2
#define  VOLUME_MIN  13.261			// 零位点的纯净水体积
#define  VOLUME_MAX  823.651		// 最大纯净水体积
#define  PROOF_MAX   17.31		// 特殊材料的最大体积

#define  CONST_7_W3   (0.0000197216)   //计算体积时，二次项系数
#define  CONST_7_W2   (-0.042143145)    //计算体积时，一次项系数
#define  CONST_7_W1   (28.48976477)   //计算体积时，常数量
#define  CONST_7_W0   (-5881.893117)   //计算体积时，常数量

#define  CONST_6_W3   (0.0000083366)   //计算体积时，二次项系数
#define  CONST_6_W2   (-0.019218743)    //计算体积时，一次项系数
#define  CONST_6_W1   (13.68825098)   //计算体积时，常数量
#define  CONST_6_W0   (-2883.242168)   //计算体积时，常数量

#define  CONST_5_W3   (-0.0000000001)   //计算体积时，二次项系数
#define  CONST_5_W2   (-0.0000573992)    //计算体积时，一次项系数
#define  CONST_5_W1   (-0.990866131)   //计算体积时，常数量
#define  CONST_5_W0   (864.9319887)   //计算体积时，常数量

#define  CONST_4_W3   (0)   //计算体积时，二次项系数
#define  CONST_4_W2   (-0.0000576614)    //计算体积时，一次项系数
#define  CONST_4_W1   (-0.990689627)   //计算体积时，常数量
#define  CONST_4_W0   (864.8918796)   //计算体积时，常数量

#define  CONST_3_W3   (0.0000000001)   //计算体积时，二次项系数
#define  CONST_3_W2   (-0.0000577844)    //计算体积时，一次项系数
#define  CONST_3_W1   (-0.99064574)   //计算体积时，常数量
#define  CONST_3_W0   (864.886003)   //计算体积时，常数量

#define  CONST_2_W3   (0.0000007013)   //计算体积时，二次项系数
#define  CONST_2_W2   (-0.000470092)    //计算体积时，一次项系数
#define  CONST_2_W1   (-0.9111354050)   //计算体积时，常数量
#define  CONST_2_W0   (859.8628164)   //计算体积时，常数量

#define  CONST_1_W3   (0.000002484)   //计算体积时，二次项系数
#define  CONST_1_W2   (-0.003022264)    //计算体积时，一次项系数
#define  CONST_1_W1   (-0.441046012)   //计算体积时，常数量
#define  CONST_1_W0   (836.4875726)   //计算体积时，常数量


// 特殊材料体积计算
#define  CONST_PROOF_7_W2   (0.00012929)   //计算体积时，二次项系数
#define  CONST_PROOF_7_W1   (-0.22222115)    //计算体积时，一次项系数
#define  CONST_PROOF_7_W0   (95.43008551)   //计算体积时，常数量

#define  CONST_PROOF_6_W2   (0.00001952)   //计算体积时，二次项系数
#define  CONST_PROOF_6_W1   (-0.04698846)   //计算体积时，一次项系数
#define  CONST_PROOF_6_W0   (25.50367957)    //计算体积时，常数量

#define  CONST_PROOF_5_W2   (0.00001584)   //计算体积时，二次项系数
#define  CONST_PROOF_5_W1   (-0.04426179)    //计算体积时，一次项系数
#define  CONST_PROOF_5_W0   (25.50053769)   //计算体积时，常数量

#define  CONST_PROOF_4_W2   (0.00000801)   //计算体积时，二次项系数
#define  CONST_PROOF_4_W1   (-0.03210028)    //计算体积时，一次项系数
#define  CONST_PROOF_4_W0   (21.01773248)   //计算体积时，常数量

#define  CONST_PROOF_3_W2   (-0.00000456)   //计算体积时，二次项系数
#define  CONST_PROOF_3_W1   (-0.02050025)    //计算体积时，一次项系数
#define  CONST_PROOF_3_W0   (18.40291898)   //计算体积时，常数量

#define  CONST_PROOF_2_W2   (-0.00000616 )   //计算体积时，二次项系数
#define  CONST_PROOF_2_W1   (-0.02067493)    //计算体积时，一次项系数
#define  CONST_PROOF_2_W0   (18.52974540 )   //计算体积时，常数量

#define  CONST_PROOF_1_W2   (-0.00007114)   //计算体积时，二次项系数
#define  CONST_PROOF_1_W1   (-0.0035647)    //计算体积时，一次项系数
#define  CONST_PROOF_1_W0   (17.43012303 )   //计算体积时，常数量
#endif

//3号水箱
#if   BOX3
#define  VOLUME_MIN  12.458		// 零位点纯净水体积
#define  VOLUME_MAX  1250.169	// 最大纯净水体积
#define  PROOF_MAX   27.24		// 特殊材料的最大体积

#define  CONST_6_W3   (0.0000384421)   //计算体积时，二次项系数
#define  CONST_6_W2   (-0.093529721)   //计算体积时，一次项系数
#define  CONST_6_W1   (74.62195189)    //计算体积时，常数量
#define  CONST_6_W0   (-19427.03231)   //计算体积时，常数量

#define  CONST_5_W3   (0.0000094085)   //计算体积时，二次项系数
#define  CONST_5_W2   (-0.01975593)    //计算体积时，一次项系数
#define  CONST_5_W1   (12.17791334)   //计算体积时，常数量
#define  CONST_5_W0   (-1820.096949)   //计算体积时，常数量

#define  CONST_4_W3   (0.0000043139)   //计算体积时，二次项系数
#define  CONST_4_W2   (-0.008475403)    //计算体积时，一次项系数
#define  CONST_4_W1   (3.862023191)   //计算体积时，常数量
#define  CONST_4_W0   (220.7072494)   //计算体积时，常数量

#define  CONST_3_W3   (-0.0000000018)   //计算体积时，二次项系数
#define  CONST_3_W2   (-0.0000907193)    //计算体积时，一次项系数
#define  CONST_3_W1   ( -1.564592473)   //计算体积时，常数量
#define  CONST_3_W0   (1390.49385)   //计算体积时，常数量

#define  CONST_2_W3   (0.0000040545)   //计算体积时，二次项系数
#define  CONST_2_W2   (-0.003353257)    //计算体积时，一次项系数
#define  CONST_2_W1   (-0.690282731)   //计算体积时，常数量
#define  CONST_2_W0   (1312.408037)   //计算体积时，常数量

#define  CONST_1_W3   (0.0000048361)   //计算体积时，二次项系数
#define  CONST_1_W2   (-0.003183062)    //计算体积时，一次项系数
#define  CONST_1_W1   (-0.7851664910 )   //计算体积时，常数量
#define  CONST_1_W0   (1320.275386)   //计算体积时，常数量

// 特殊材料体积计算
#define  CONST_PROOF_6_W2   (0.00008287)   //计算体积时，二次项系数
#define  CONST_PROOF_6_W1   (-0.15573399)   //计算体积时，一次项系数
#define  CONST_PROOF_6_W0   (73.43722164 )    //计算体积时，常数量

#define  CONST_PROOF_5_W2   (0.00008667)   //计算体积时，二次项系数
#define  CONST_PROOF_5_W1   (-0.16241932)    //计算体积时，一次项系数
#define  CONST_PROOF_5_W0   (76.35122823)   //计算体积时，常数量

#define  CONST_PROOF_4_W2   (0.00000467)   //计算体积时，二次项系数
#define  CONST_PROOF_4_W1   (-0.04270281)    //计算体积时，一次项系数
#define  CONST_PROOF_4_W0   (32.63393348)   //计算体积时，常数量

#define  CONST_PROOF_3_W2   (-0.00000591)   //计算体积时，二次项系数
#define  CONST_PROOF_3_W1   (-0.03219374)    //计算体积时，一次项系数
#define  CONST_PROOF_3_W0   (30.03064475)   //计算体积时，常数量

#define  CONST_PROOF_2_W2   (-0.00001096 )   //计算体积时，二次项系数
#define  CONST_PROOF_2_W1   (-0.02930142)    //计算体积时，一次项系数
#define  CONST_PROOF_2_W0   (29.56138899  )   //计算体积时，常数量

#define  CONST_PROOF_1_W2   (-0.00003546)   //计算体积时，二次项系数
#define  CONST_PROOF_1_W1   (-0.02196103)    //计算体积时，一次项系数
#define  CONST_PROOF_1_W0   (29.01208132 )   //计算体积时，常数量

#endif

//4号水箱
#if     BOX4
#define  VOLUME_MIN  8.051		// 最小纯净水体积
#define  VOLUME_MAX  823.651	// 满位点的纯净水体积
#define  PROOF_MAX   18.03		// 特殊材料的最大体积

#define  CONST_6_W3   (0.0000499275)   //计算体积时，二次项系数
#define  CONST_6_W2   (-0.116216142)    //计算体积时，一次项系数
#define  CONST_6_W1   (89.03305511)   //计算体积时，常数量
#define  CONST_6_W0   (-22375.28856)   //计算体积时，常数量

#define  CONST_5_W3   (0.0000083366)   //计算体积时，二次项系数
#define  CONST_5_W2   (-0.019218743)    //计算体积时，一次项系数
#define  CONST_5_W1   (13.68825098)   //计算体积时，常数量
#define  CONST_5_W0   (-2883.242168)   //计算体积时，常数量

#define  CONST_4_W3   (0.0000000002)   //计算体积时，二次项系数
#define  CONST_4_W2   (-0.0000580383)    //计算体积时，一次项系数
#define  CONST_4_W1   (-0.990440226)   //计算体积时，常数量
#define  CONST_4_W0   (864.8377031)   //计算体积时，常数量

#define  CONST_3_W3   (0)   //计算体积时，二次项系数
#define  CONST_3_W2   (-0.0000576735)    //计算体积时，一次项系数
#define  CONST_3_W1   (-0.990678033)   //计算体积时，常数量
#define  CONST_3_W0   (864.8890753)   //计算体积时，常数量

#define  CONST_2_W3   (0.000000503)   //计算体积时，二次项系数
#define  CONST_2_W2   (-0.000358188)    //计算体积时，一次项系数
#define  CONST_2_W1   (-0.931676437)   //计算体积时，常数量
#define  CONST_2_W0   (861.085296)   //计算体积时，常数量

#define  CONST_1_W3   (0.0000029553)   //计算体积时，二次项系数
#define  CONST_1_W2   (-0.003081545)    //计算体积时，一次项系数
#define  CONST_1_W1   (-0.439441587)   //计算体积时，常数量
#define  CONST_1_W0   (836.4914506)   //计算体积时，常数量


// 特殊材料体积计算
#define  CONST_PROOF_6_W2   (0.00013083)   //计算体积时，二次项系数
#define  CONST_PROOF_6_W1   (-0.22529222)   //计算体积时，一次项系数
#define  CONST_PROOF_6_W0   (96.93366225)    //计算体积时，常数量

#define  CONST_PROOF_5_W2   (0.00007225)   //计算体积时，二次项系数
#define  CONST_PROOF_5_W1   (-0.13245342)    //计算体积时，一次项系数
#define  CONST_PROOF_5_W0   (60.13962430)   //计算体积时，常数量

#define  CONST_PROOF_4_W2   (0.00000649)   //计算体积时，二次项系数
#define  CONST_PROOF_4_W1   (-0.03297427)    //计算体积时，一次项系数
#define  CONST_PROOF_4_W0   (22.45510417 )   //计算体积时，常数量

#define  CONST_PROOF_3_W2   (0.00000123)   //计算体积时，二次项系数
#define  CONST_PROOF_3_W1   (-0.02622775)    //计算体积时，一次项系数
#define  CONST_PROOF_3_W0   (20.27914333)   //计算体积时，常数量

#define  CONST_PROOF_2_W2   (-0.00000555 )   //计算体积时，二次项系数
#define  CONST_PROOF_2_W1   (-0.02185765)    //计算体积时，一次项系数
#define  CONST_PROOF_2_W0   (19.56246342)   //计算体积时，常数量

#define  CONST_PROOF_1_W2   (-0.00006322)   //计算体积时，二次项系数
#define  CONST_PROOF_1_W1   (-0.00289367)    //计算体积时，一次项系数
#define  CONST_PROOF_1_W0   (18.13277650 )   //计算体积时，常数量

#endif

#define  OIL_VOL_NUM    (40)
#define  TEMP_VOL_NUM     (20)

//传感器是正装的，测试点在上面，高度均是以测试点作为基准零点

//1号传感器
#if     BOX1 
#define  HEIGHT_ZERO     8740        // 零位时高度(单位：0.1mm)
#define  HEIGHT_FULL     1320       // 满位时高度(单位：0.1mm)
#endif 

//2号传感器
#if     BOX2
#define  HEIGHT_ZERO     8240         // 零位时高度(单位：0.1mm)
#define  HEIGHT_FULL     870        // 满位时高度(单位：0.1mm)

#endif

//3号传感器
#if     BOX3
#define  HEIGHT_ZERO     8740         // 零位时高度(单位：0.1mm)
#define  HEIGHT_FULL     1320        // 满位时高度(单位：0.1mm)

#endif

//4号传感器
#if     BOX4
#define  HEIGHT_ZERO    8310         // 零位时高度(单位：0.1mm)
#define  HEIGHT_FULL    870        // 满位时高度(单位：0.1mm)

#endif


//1号传感器
#if     BOX1
#define   CONST_HEIGHT_BOTTOM           71  
#define   CONST_HEIGHT_1           		  151
#define   CONST_HEIGHT_2           		  298
#define   CONST_HEIGHT_3            		  605
#define   CONST_HEIGHT_4            		  750
#define   CONST_HEIGHT_5                     825
#define   CONST_HEIGHT_TOP                 874

#endif


//2号传感器
#if     BOX2
#define   CONST_HEIGHT_BOTTOM          25
#define   CONST_HEIGHT_1           		  101
#define   CONST_HEIGHT_2           		  251
#define   CONST_HEIGHT_3            	  401
#define   CONST_HEIGHT_4            	  601
#define   CONST_HEIGHT_5                    751
#define   CONST_HEIGHT_6                   801
#define   CONST_HEIGHT_TOP                824

#endif


//3号传感器
#if     BOX3
#define   CONST_HEIGHT_BOTTOM           71  
#define   CONST_HEIGHT_1           		  151
#define   CONST_HEIGHT_2           		  298
#define   CONST_HEIGHT_3            		  605
#define   CONST_HEIGHT_4            		  750
#define   CONST_HEIGHT_5                     825
#define   CONST_HEIGHT_TOP                 874

#endif


//4号传感器
#if BOX4
#define   CONST_HEIGHT_BOTTOM           25  
#define   CONST_HEIGHT_1           		   111
#define   CONST_HEIGHT_2                     251
#define   CONST_HEIGHT_3                     551
#define   CONST_HEIGHT_4            	   751
#define   CONST_HEIGHT_5                     801
#define   CONST_HEIGHT_TOP                 831

#endif

typedef struct MainObj
{
    uint8  uart_flag;               // MCU串口接收标志位，当发生接收中断时，中断函数来赋值，main函数并不赋值
    uint8  type_tank_flag;   //纯净水水类型号特殊水箱标志位
    uint8  type_tank_confirm_flag;//纯净水水类型号确认标志位
    uint8  clear_feedback_flag;//清除反馈标志位
    uint8  oil_type;                // 读取传感器FLASH中的纯净水水类型号
    uint8  oil_type_target;          // 接收到的目标纯净水水类型号
    uint8  oil_tank_target;
    uint8  tank_type_target;
    uint8  pre_oil_type_target;          // 接收到的目标纯净水水类型号
    uint8  pre_oil_tank_target;
    uint8  oil_type_send;		// 发送的水类型号-水箱信息
    
    uint8  command_status;          // 传感器工作状态
    uint8  cur_pos;                 // 接收指令在数组的位置
    uint8  data_count;              //
    uint8  temper_count; 
    uint8  bit_buf;                 // 故障字定义
    uint8  work_status;             // 传感器工作状态
    uint8  uart_timer;
    uint8  timer_count;
    uint8  send_count;
    uint8  clear_feedback_count;

    uint8  oil_state;
    
    uint8  bit_count[4];            //
    uint8  step_count[2];
    uint8  t0_flag;               // 定义零位变量标识
    uint8  t1_flag;			  	// 定义满位变量标识
    uint8  temper_flag;
    uint8  fliter_flag;
    uint8  tank_type;	// 水箱类型
    uint8  type_tank_confirm;	// 水类型号-水箱确认更改信息
    uint8  pre_oil_type;
    uint8  oil_cmd_valid;
    
    uint8  temp_command_status;
    uint8  temp_oil_type;
    uint8  temp_tank_type;
    uint8  temp_type_tank_confirm;
    
    uint16 type_tank_confirm_counter;
    uint16 tank_type_counter;
    uint16 command_status_counter;
    uint16 oil_type_counter;
    uint16 state_counter;
    
    uint16 oil_type_addr;
    uint16 vol_oil;
    uint16 vol_temper;            // 纯净水温度电压
    uint16 vol_temper_send;             //
    uint16 vol_oil_send;            // 发送的
    uint16 weight_send;                 // 发送的体积数据
    uint16 height_send;                 // 
    uint16 temper_org;                //  
    uint16 temper_send;
    uint16 temper_target;             // 目标温度
    
    uint8 check_sum;				// 检验和从16位改为8位
    
    float height;
    float temper;
	float density;					// 密度
    float const_temp_array[5];     // 温度-起始电容电压，一次项系数
                                    // 温度-起始电容电压，常数量
                                    // 温度-满油电容电压，一次项系数
                                    // 温度-满油电容电压，常数量
                                    // 温度补偿值

    // uint16 const_oil_array[3];      // 水位高度最大值（常数）
                                    // 水位电压最小值
                                    // 水位电压最大值
    uint8  recv_buf[11];             // 接收到的数据
    uint16 oil_vol_array[OIL_VOL_NUM];
                                    // 水位电压数组
    uint16 temper_vol_array[TEMP_VOL_NUM];
                                    // 温度电压数组
    float WEIGHT_MAX_1;
    float WEIGHT_MAX_2;
    float WEIGHT_MAX_3;
    float WEIGHT_MAX_4;
    
    uint8 temper_error_flag ;
    
	
} MainObj;

#ifdef __cplusplus
}
#endif

#endif // __CONST_H
