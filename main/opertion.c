#include "Flash.h"
#include "capture_filter.h"
#include "const.h"
#include "operation.h"
#include "WatchDog.h"

extern MainObj  main_obj;

/*******************************************************************************
* 名称：调0满位
* 输入：int i  0：调0，1：满位
* 输入：int add_addr  0：２号的系数，512:1号的系数
* 输入：const_addr 多项式系数, 目前的版本一共4个
* 输入：const_oil_array为水位相关系数，共3个（高度最大值、水位电容电压最大值和最小值）
* 输出：const_addr 从flash参数区中读取的系数
*       对应之前代码的顺序: Const_C01, Const_C00, Const_C11, Const_C10
*                           const_oil_array 为水位 常数最大值, 电压最 小/大 值 存放 buffer
* 功能：进行调0满位操作，将纯净水的系数从Flash中读取，
* 经过运算，再将新的数据写入Flash中，此过程会对1号和２号的零位满位系数均会写入
********************************************************************************/
void operation_t0t1( uint8 i, int16 add_addr, 
                     uint16 volt_ch, float temper)
{
	uint16  temp[4];
	uint16  read_data[4];	// 用于暂时写入FLASH的数据，判断是否正确写入
	uint8   j = 0;
	u8 res = 1;
	float k = 0.0;
	float t0t1_ratio = 0.0;		// 1号和２号纯净水零位满位电压的比例
	float new_vol = 0.0;
	uint16 read_count = 0;	// 记录写FLASH的次数，数值超过MAX_COUNT，即表示读写FLASH失败，FLASH故障
	u8 error_flag = 1;	// 本次读写FLASH是否故障
	u8 ak = 0;
	float const_addr[5];
	
	operation_read_flash_param( add_addr, const_addr );

	// 暂存水位、温度相关系数，不然整个扇区写入时，会擦除里面的内容
  for(j = 0;j < 4;j++)
	{
		if( 2==j )
		{
			temp[j] = (uint16)( const_addr[j]*2000);			
		}
		else if( 3==j )
		{
			temp[j] = (uint16)( const_addr[j]*10);		
		}
		else
		{
			temp[j] = (uint16) const_addr[j];       //const_addr[4]表示温度常数，此时不写入，后续可以更改一下
		}
	}
	
	// 调整系数,其中*const_addr表示温度相关系数，* const_oil_array表示水位相关系数
	if ( 0 == i )
	{
	    const_addr[0] = 0;
	    const_addr[1] = volt_ch;
	    // const_oil_array[1] = volt_ch;
	                                    //
	    temp[0] = (uint16) const_addr[0];
	    temp[1] = (uint16) const_addr[1];	
	    main_obj.const_temp_array[0] = const_addr[0];
	    main_obj.const_temp_array[1] = const_addr[1];
	}
	else if ( 1 == i )
	{
		k = volt_ch/( -1.5 * temper + 2140 );		
		const_addr[2] = 1.5 * k;
		const_addr[3] = 2140 * k;		

		temp[2] = (uint16) (const_addr[2]*2000);          //为保证存储时数据不变放大
		temp[3] = (uint16) (const_addr[3]*10);          //为保证存储时数据不变放大	
		main_obj.const_temp_array[2] = const_addr[2];
	  main_obj.const_temp_array[3] = const_addr[3];
	}
  
  // 写flash
  //将温度――电容系数4个参数写入
  //将水位油压上下限3个参数写�

	read_count = 0;
	res = 1;
	error_flag = 1;	// 默认有错误，写入正确后置0，表示正确
	while( res||error_flag)
	{
		IWDG_Feed();
		read_count++;
		res = STMFLASH_Write(FLASH_BASE_ADDR + 0x0000 + add_addr,temp,4);
		if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		IWDG_Feed();
		res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0000 + add_addr,read_data,4);
		if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		// 如果读写的数据一致，那么res、error_flag均为0，会跳出while循环
		for( ak=0;ak<4;ak++)
		{
			error_flag = 0;
			if( temp[ak]!= read_data[ak] )
			{
				error_flag = 1;	// 如果读出的数据read_data[4]和写入的数据temp[4]不一致，则读写错误,error_flag置1，否则为0
				ak = 5;
			}	
		}
		// 读写次数超出了MAX_COUNT，就会跳出循环
		if( read_count > MAX_COUNT )
		{
    		main_obj.bit_buf |= 0x04;
				printf("flash err\r\n");
				break;
		}
		
	}
	
	// PR-3
	if(add_addr == 512) // 已经写了纯净水牌号1号的参数,接下来写２号的参数
	{
		operation_read_flash_param( 0, const_addr );	
		// 暂存水位、温度相关系数，不然整个扇区写入时，会擦除里面的内容
  		for(j = 0;j < 4;j++)
		{
			if( 2==j )
			{
				temp[j] = (uint16)( const_addr[j]*2000);			
			}
			else if( 3==j )
			{
				temp[j] = (uint16)( const_addr[j]*10);		
			}
			else
			{
				temp[j] = (uint16) const_addr[j];       //const_addr[4]表示温度常数，此时不写入，后续可以更改一下
			}
		}
		
		// 写纯净水牌号0x10(２号)  的参数
		if ( 0 == i )
		{    	       
	   	 temp[0] = (uint16) 0;
	   	 temp[1] = (uint16) volt_ch;	 
		}
		else if ( 1 == i )
		{
			t0t1_ratio = 1;//系数,根据实际情况修改
			
			new_vol = (volt_ch - const_addr[1]) * t0t1_ratio + const_addr[1];//满位参数用到零位参数，所以必须先零位后满位
			k = new_vol / ( -1.5 * temper + 2140 );
			
			temp[2] = (uint16) (1.5 * k * 2000);          //为保证存储时数据不变放大
			temp[3] = (uint16) (2140 * k * 10);          //为保证存储时数据不变放大		
		}
		
		read_count = 0;
		res = 1;
		error_flag = 1;
		// 如果读写的数据一致，那么res、error_flag均为0，会跳出while循环
		while( res||error_flag)
		{
			IWDG_Feed();
			read_count++;
			res = STMFLASH_Write(FLASH_BASE_ADDR + 0x0000 + 0,temp,4);
			if(0 == res)
  		 	{
				main_obj.bit_buf &= 0xFB;
   		 	}
			IWDG_Feed();
			res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0000 + 0,read_data,4);
			if(0 == res)
  		 	{
				main_obj.bit_buf &= 0xFB;
   		 	}
			
			// 如果读写的数据一致，那么res、error_flag均为0，会跳出while循环
			for( ak=0;ak<4;ak++)
			{
				error_flag = 0;
				if( temp[ak]!= read_data[ak] )	// 如果读出的数据read_data[4]和写入的数据temp[4]不一致，则读写错误,error_flag置1，否则为0
				{
					error_flag = 1;
					ak = 5;
				}	
			}
			// 读写次数超出了MAX_COUNT，就会跳出循环
			if( read_count > MAX_COUNT )
			{
	    	main_obj.bit_buf |= 0x04;
				printf("flash err\r\n");
				break;
			}			
		}
		
	}
	else if(add_addr == 0) // 已经写了纯净水牌号２号的参数,接下来写1号的参数
	{
		operation_read_flash_param( 512, const_addr );	
		// 暂存水位、温度相关系数，不然整个扇区写入时，会擦除里面的内容
  		for(j = 0;j < 4;j++)
		{
			if( 2==j )
			{
				temp[j] = (uint16)( const_addr[j]*2000);			
			}
			else if( 3==j )
			{
				temp[j] = (uint16)( const_addr[j]*10);		
			}
			else
			{
				temp[j] = (uint16) const_addr[j];       //const_addr[4]表示温度常数，此时不写入，后续可以更改一下
			}
		}
		
		// 写纯净水牌号0x10(２号)  的参数
		if ( 0 == i )
		{    	       
	   	 	temp[0] = (uint16) 0;
	   	       temp[1] = (uint16) volt_ch;	 
		}
		else if ( 1 == i )
		{
			t0t1_ratio = 1;//系数,根据实际情况修改
			
			new_vol = (volt_ch - const_addr[1]) * t0t1_ratio + const_addr[1];
			k = new_vol / ( -1.5 * temper + 2140 );
			
			temp[2] = (uint16) (1.5 * k * 2000);          //为保证存储时数据不变放大
			temp[3] = (uint16) (2140 * k * 10);          //为保证存储时数据不变放大		
		}

		read_count = 0;
		res = 1;
		error_flag = 1;
		while( res||error_flag)
		{
			IWDG_Feed();
			read_count++;
			res = STMFLASH_Write(FLASH_BASE_ADDR + 0x0000 + 0,temp,4);
			if(0 == res)
  		 	{
				main_obj.bit_buf &= 0xFB;
   		 	}
			IWDG_Feed();
			res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0000 + 0,read_data,4);
			if(0 == res)
  		 	{
				main_obj.bit_buf &= 0xFB;
   			 }
			for( ak=0;ak<4;ak++)
			{
				error_flag = 0;
				if( temp[ak]!= read_data[ak] )
				{
					error_flag = 1;
					ak = 5;
				}	
			}

			if( read_count > MAX_COUNT )
			{
	    		main_obj.bit_buf |= 0x04;
				printf("flash err\r\n");
				break;
			}
			
		}
	
  		//printf("operation_t0t1:\r\ntemp[0] : %d\r\ntemp[1] : %d\r\ntemp[2] : %d\r\ntemp[3] : %d\r\n",temp[0],temp[1],temp[2],temp[3]);
	}
}
/*******************************************************************************
* 名称：读取存储系数  operation_read_flash_param
* 输入：int16 add_addr 存储地址，其中105G中0为２号的地址，512为1号的地址
* 输入：float * const_temp_array  存储系数，该指针参数用于存储读取出来的系数
* 功能：读取存储在FLASH中的系数，包括零位满位系数、温度补偿系数
*******************************************************************************/
void operation_read_flash_param( int16 add_addr, float* const_temp_array)
{
	u16 pBuffer[1] = {0};
	u8 res = 0;
	float real_temper_range_max=0.0,real_temper_range_min=0.0;
	u16 read_count = 0;
	

	// C10
	 res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0000 + add_addr,pBuffer,1);// 温度-起始电容电压，一次项系� 
	 read_count = 0;
	 while( (res)||(0xFFFF==pBuffer[0]))
   	 {
   	 	IWDG_Feed();
   	 	read_count++;
   	 	res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0000 + add_addr,pBuffer,1);
		 if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		if( read_count > MAX_COUNT )
		{
    		main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
			break;
		}
	 }
	 const_temp_array[0] = (float)pBuffer[0];
	 printf("operation_read_flash_param const_temp_array[0]:%f\r\n",const_temp_array[0]);
	 
	 // C00
	 res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0002 + add_addr,pBuffer,1);// 温度-起始电容电压，常数量
	read_count = 0;
	while( (res)||(0xFFFF==pBuffer[0]))
	{
		IWDG_Feed();
		read_count++;
		res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0002 + add_addr,pBuffer,1);
		if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		if( read_count > MAX_COUNT )
		{
    		main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
			break;
		}
	}
	 const_temp_array[1] = (float)pBuffer[0];
	 printf("operation_read_flash_param const_temp_array[1]:%f\r\n",const_temp_array[1]);
	 
	 // C11
	 res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0004 + add_addr,pBuffer,1);// 温度-满油电容电压，一次项系数
	 read_count = 0;
	 while((res)||(0xFFFF==pBuffer[0]))
	{
		IWDG_Feed();
		read_count++;
		 res = STMFLASH_Read(FLASH_BASE_ADDR + 0x0004 + add_addr,pBuffer,1);
		 if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		if( read_count > MAX_COUNT )
		{
    		main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
			break;
		}
	}
	 const_temp_array[2] = ((float)pBuffer[0])/2000;
	 printf("operation_read_flash_param const_temp_array[2]:%f\r\n",const_temp_array[2]);
	 
	 // C10
	 STMFLASH_Read(FLASH_BASE_ADDR + 0x0006 + add_addr,pBuffer,1);// 温度-满油电容电压，常数量
		read_count = 0;
	  while((res)||(0xFFFF==pBuffer[0]))
   	 {
   	 	IWDG_Feed();
   	 	read_count++;
		STMFLASH_Read(FLASH_BASE_ADDR + 0x0006 + add_addr,pBuffer,1);// 温度-满油电容电压，常数量
		if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
		if( read_count > MAX_COUNT )
		{
    		main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
			break;
		}
     }
	 const_temp_array[3] = ((float)pBuffer[0])/10;
	 printf("operation_read_flash_param const_temp_array[3]:%f\r\n",const_temp_array[3]);
	 
	 // Const_add
	 STMFLASH_Read(FLASH_BASE_ADDR + CONST_OFFSET,pBuffer,1); // 温度补偿值
	read_count = 0;
	  while((res)||(0xFFFF==pBuffer[0]))
    	{
    		IWDG_Feed();
    		read_count++;
		STMFLASH_Read(FLASH_BASE_ADDR + CONST_OFFSET,pBuffer,1);
		if(0 == res)
  		 {
			main_obj.bit_buf &= 0xFB;
   		 }
    		if( read_count > MAX_COUNT )
			{
	    		main_obj.bit_buf |= 0x04;
				printf("read flash err\r\n");
				break;
			}
    	}
	 const_temp_array[4] = (float)pBuffer[0]*0.1 - 50;
	 
	 // 当温度补偿值超出上下限时，对温度补偿值进行处理
	 real_temper_range_min	= -(REAL_OIL_TEMPER_MAX - REAL_OIL_TEMPER_MIN);
	 real_temper_range_max	= (REAL_OIL_TEMPER_MAX - REAL_OIL_TEMPER_MIN);
	 if(const_temp_array[4] < real_temper_range_min )
	 {
	 	const_temp_array[4] = real_temper_range_min;
	 	printf("operation_read_flash_param, const_temp_array[4] less than %f\r\n",real_temper_range_min);
	 }
	 if(const_temp_array[4] > real_temper_range_max)
	 {
	 	const_temp_array[4] = real_temper_range_max;
	 	printf("operation_read_flash_param, const_temp_array[4] large than %f\r\n", real_temper_range_max);
	 }
	 
	 printf("operation_read_flash_param const_temp_array[4]:%f\r\n",const_temp_array[4]);
	
}

/*******************************************************************************
* 名称：调整温度 temperature
* 输入：unsigned int target 目标温度值
* 输出：无
* 功能：进行调整温度操作，将温度的补偿值从Flash中读取，
* 经过运算，再将新的数据写入Flash中
*******************************************************************************/
void operation_temper( uint16 target, uint16 temper_cal, float * const_add )
{
    uint16  itemp;
	u8 res = 0;
 
 if( (target - temper_cal)>600)
 	{
		itemp = target - temper_cal + 600;        //接收到的target是一个进过加码公式对应的值
	}
	else
	{
			itemp = 0;
	}
	


	*const_add = ( float )itemp*0.1 - 60;
	
	// 写Flash
	res = STMFLASH_Write(FLASH_BASE_ADDR + CONST_OFFSET,&itemp,1);
	 if(res)
    {
    	main_obj.bit_buf |= 0x04;
		printf("write flash err\r\n");
    }
	  else
   {
	main_obj.bit_buf &= 0xFB;
   }
	printf("operation_temper,itemp : %d\r\n",itemp);
}

