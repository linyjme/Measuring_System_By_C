#include "ad.h"
#include "types.h"

extern uint16  ad_out[2][12];
/*******************************************************************************
* 名称：数据采集
* 输入：通道序号
* 输出：float 进行排序滤波以后的结果
* 功能：采集对应通道的数据，并进行排序滤波，每组10个
* 数据进行排序，舍去最小的4个数和最大的2个数，仅取中
* 间的4个值进行平均。再乘以1000，单位为mV.
 ********************************************************************************/
float capture_filter_exec( uint8 channel )
{
	//uint8  xdata ad_addr;
	uint8   i, j, k;
	float   ftemp;
	float   ad_data_hex[10]; // AD模块测量十组原始数据放在数组中
	float   ad_data_dec[10]; // AD模块测量十组十进制数据放在数组中

    // 采集10组数据，转换成10进制
	for ( i=0; i<10; i++ )
	{
		ad_data_hex[i] = Get_Adc_Average(channel);    // 读取AD转换结果
		ad_data_dec[i] = ad_data_hex[i]*2.5/4096;	// 转换为十进制,2.63V为实际测到的VREF0电压
	}

	// 排序
	for ( j=0; j<=9; j++ )
	{
		for( k=0; k<9-j; k++ )
		{
			if( ad_data_dec[k] > ad_data_dec[k+1] )
			{
				ftemp = ad_data_dec[k];
				ad_data_dec[k]   = ad_data_dec[k+1];
				ad_data_dec[k+1] = ftemp;
			}
		}
	}

	// 取中间四个数据求取平均值，最小的4个和最大的2个舍去
	ftemp = 0;
	for ( i=4; i<=7; i++ )
	{
		ftemp += ad_data_dec[i];
	}

	ftemp = ftemp*250 + 0.5;     //   ftemp=ftemp*1000/4

	return ftemp;
}

/*******************************************************************************
* 名称：有效值判断
* 输入：unsigned int min     数据的下限，超过即为该值即为无效数据
* 输入：unsigned int max     数据的上限，超过即为该值即为无效数据
* 输入：unsigned int step    两次数据的最大差值，超过即为该值即为无效数据
* 输入：unsigned int compare 需要进行判断的数据
* 输入：unsigned int target  目标值
* 输出：无
* 返回值: uint8  有效状态
* ( 0:上下限有效，差值有效， 1:上下限有效，差值无效，
*   2:上下限无效，差值有效， 3:上下限无效，差值无效 )
* 功能：对数据进行有效值判断，先上下限判断，超出上下限
* 即为无效数据；再进行差值判断，与目标值作比较，差值超
* 出最大差值范围则判断为无效数据，同时将有效状态信息输出
*******************************************************************************/
uint8 capture_filter_judge( uint16 min, uint16 max, uint16 step, uint16 compare, uint16 target )
{
    uint8   ret = 0;
    uint16 tmp1;

    // 先得出需要判断的数据和目标值的差值的绝对值
	if ( compare > target )
	{
		tmp1 = compare - target;
	}
	else
	{
		tmp1 = target - compare;
	}


    if ( ( compare < max ) && ( compare > min ) )
    {
        if (  tmp1 < step  )                                    //( compare <= target ) &&
        {
            ret = 0; // 上下限有效, 差值有效
        }
        else
        {
            ret = 1; // 上下限有效, 差值无效
        }

    }
    else // if ( ( compare < max ) || ( compare > min ) )
    {
        if (  tmp1 < step )                           //( compare <= target ) &&
        {
            ret = 2; // 上下限无效, 差值有效
        }
        else
        {
            ret = 3; // 上下限无效, 差值无效
        }
    }

	return ret;
}

/*******************************************************************************
* 函数功能：滤波函数
* 输入：unsigned int channel 通道序号
* 输入：unsigned int loading 需要滤波的值
* 输出：
* 返回值：unsigned int 滤波结果
* 功能：将新的有效值添加入数组中，删除最早的数据，将
*       数组中的10个数据求均值。
******************************************************************************/
uint16 capture_filter_filter( uint16 channel, uint16 loading )
{
	int8  i;

    // 新数据放到数组中最后一个元素位置.
	ad_out[channel][11] = loading;

	// 将ad_out_0[i+1]依次赋值给ad_out_0[i]，数组最后一个舍去
	for ( i=1; i<11; i++ )
	{
		ad_out[channel][i] = ad_out[channel][i+1];
	}

	// 将ad_out_0[]中的前10个数求取平均值
	ad_out[channel][0] = 0;
	for ( i=1; i<11; i++ )
	{
		ad_out[channel][0] = ad_out[channel][0] + ad_out[channel][i];
	}

	ad_out[channel][0] = (uint16)(ad_out[channel][0]/10);

	//printf("ad_out[0] : %d\r\nad_out[1] : %d\r\nad_out[2] : %d\r\nad_out[3] : %d\r\nad_out[4] : %d\r\nad_out[5] : %d\r\nad_out[6] : %d\r\nad_out[7] : %d\r\nad_out[8] :%d\r\nad_out[9] : %d\r\nad_out[10] : %d\r\nad_out[11]: %d\r\n"
	//,ad_out[channel][0],ad_out[channel][1],ad_out[channel][2],
	//	ad_out[channel][3],ad_out[channel][4],ad_out[channel][5],ad_out[channel][6],
	//	ad_out[channel][7],ad_out[channel][8],ad_out[channel][9],ad_out[channel][10],ad_out[channel][11]);

	return ad_out[channel][0];
}

/*******************************************************************************
* 函数功能：得到指定通道的平均值(已经在数据元素位置0放置好)
* 输入：unsigned int channel 通道序号
* 输出：无
* 返回值：平均值
******************************************************************************/
uint16 capture_filter_get_average( uint16 channel )
{
    return ad_out[channel][0];
}

/*******************************************************************************
* 函数功能：得到指定通道的最新数据
* 输入：unsigned int channel 通道序号
* 输出：无
* 返回值：最新数据
******************************************************************************/
uint16 capture_filter_get_last_data( uint16 channel )
{
    return ad_out[channel][11];
}

