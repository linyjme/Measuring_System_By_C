/*******************************************************************************

                       main.C

*******************************************************************************/
#include <stdio.h>
#include "string.h"
#include "math.h"
#include "const.h"
#include "capture_filter.h"
#include "operation.h"
#include "delay.h"
#include "Pwm.h"
#include "Ad.h"
#include "Usart.h"
#include "WatchDog.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h" 
#include "flash.h"

MainObj  main_obj; 
uint16  ad_out[2][12]={0};
uint8  wd_flag = 0;
enum state{STATE0,STATE1,STATE2,STATE3,STATE4,STATE5,STATE6};

static void mcu_init(void);
void send_data( void );
static void power_check( void );
static void oil_vol_capture(void);
static void oil_temper_capture(void);
static void handle_command( void );
static void handle_status(void);
static void handle_send_data( void );
static void handle_oil_type( void );

static void sensor_data_filter(void);
static void handle_weight(void);
//static void handle_prooftank_weight(void);	// 特殊水箱的质量处理

u8 STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
u8 STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

 int main( void )
{
	SCB->VTOR = FLASH_BASE | 0x10000; /* Vector Table Relocation in Internal FLASH. */
	mcu_init();	        // MCU初始化	
	
  printf("program starting...\r\n");

	memset( &main_obj , 0 , sizeof( MainObj ) ); // 初始化结构体
	
	main_obj.work_status = 0x00;
	main_obj.bit_buf = 0xF0;		// 默认flash无故障，其他均默认有故障
	main_obj.oil_state = STATE0;

	// 处理纯净水水类型号及特殊水箱曲线
	handle_oil_type();
	      
    // 从flash参数区中读取相关参数.
	operation_read_flash_param( main_obj.oil_type_addr, main_obj.const_temp_array );

	// 上电自检
	power_check();

	// 主循环
	for(;;)
	{
		// 周期自检
		power_check();
		        
		// 采集水位电压
		oil_vol_capture();
		
		// 采集纯净水温度电压
		oil_temper_capture();
		
		// 喂狗
		if(wd_flag == 1)
		{
			IWDG_Feed();
			wd_flag = 0;
		}
		
		// 处理飞控计算机数据
		handle_command();
		
		// 处理状态
		handle_status();
		
		// 处理发送的数据
		handle_send_data(); 
		
		// 喂狗
		if(wd_flag == 1)
		{
			IWDG_Feed();
			wd_flag = 0;
		} 
		printf("main_obj.oil_type:%x\r\n",main_obj.oil_type);
		printf("main_obj.tank_type:%x\r\n",main_obj.tank_type);
	}
	
}

/*******************************************************************************
* 名称：单片机初始化
* 功能：初始化单片机
*******************************************************************************/
static void mcu_init(void)
{
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart2_init(9600);
	uart1_init(115200);
	Adc_Init();
	TIM2_Int_Init(999,7199);  //Tout = ((999+1)*(7199+1)) /72us = 100ms
	IWDG_Init(4,625);	//Tout = ((4*2^prer)*rlr) / 40ms = 1000ms = 1s
	TIM3_PWM_Init(3000,0);	 //不分频。PWM频率=72000000/900=80Khz   /3000
	TIM_SetCompare1(TIM3,1500);
}

/*******************************************************************************
* 名称：读取纯净水水类型号和特殊曲线类型
* 功能：在flash中读取纯净水水类型号
*******************************************************************************/
static void handle_oil_type( void )
{
    u8 pBuffer[2] = {0};
    u8 res = 0;
    u8 error_count = 0;		// 读写flash计数
    const u8 default_oil_type = 0x10;// 默认纯净水水类型号１号(0x10)
    const u8 default_tank_type = 0x10;// 默认为非特殊水箱类型(0x10)
    
    //读取纯净水水类型号 
    res = STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
    if(res)
    {
    	main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
    }
    else
   {
		main_obj.bit_buf &= 0xFB;
   }
    main_obj.oil_type = pBuffer[0];
    printf("handle_oil_type,type is %x\r\n", main_obj.oil_type);
      
    // 0x10表示为１号,0x20表示２号
		error_count = 0;
		while( ( 0x10 != main_obj.oil_type ) && ( 0x20 != main_obj.oil_type ) )
		{
			IWDG_Feed();
			error_count++;
			if(error_count > MAX_COUNT) // 超过次数MAX_COUNT，退出
			{
			 	 main_obj.oil_type = default_oil_type;
	    			main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("handle_oil_type: write flash error max, quit and set %d\r\n", default_oil_type);
				break;
			}
			printf("oil type is neither 0x10 nor 0x20, set to 0x10\r\n");
			pBuffer[0] = default_oil_type;
			res = STMFLASH_Write(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
			if(res)
  			{
  				main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("write flash err\r\n");
  			}
			 else
 			  {
				main_obj.bit_buf &= 0xFB;
   			  }
			IWDG_Feed();
			res = STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
			
			if(res)
    			{
    				main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("read flash err\r\n");
    			}
			 else
  			 {
				main_obj.bit_buf &= 0xFB;
  			 }
			
			main_obj.oil_type = pBuffer[0];
			printf("handle_oil_type,oil type is %x\r\n", main_obj.oil_type);
			if(main_obj.oil_type != default_oil_type) // 若不是默认值，则进入下一轮循环
			{
				main_obj.oil_type = 0xff;	
			}
		}
    
		IWDG_Feed();
		
		if( 0x20 == main_obj.oil_type )
		{
			main_obj.oil_type_addr = 0;		// ２号的存储偏移地址
		}
		else
		{
			main_obj.oil_type_addr = 512;  // １号的存储偏移地址
		}
		
		
	//读取特殊水箱类型	
	res = STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET + 2,(u16 *)pBuffer,1);
    if(res)
    {
    	main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
    }
    else
   {
	main_obj.bit_buf &= 0xFB;
   }
    main_obj.tank_type = pBuffer[0];

    if(( 0x10 != main_obj.tank_type ) && ( 0x20 != main_obj.tank_type ))
    {
    		pBuffer[0] = default_tank_type;
			IWDG_Feed();
		res = STMFLASH_Write(FLASH_BASE_ADDR + OIL_TYPE_OFFSET + 2,(u16 *)pBuffer,1);
		if(res)
  		{
  			main_obj.bit_buf |= 0x04;	// FLASH故障
			printf("write flash err\r\n");
  		}
		 else
   		{
			main_obj.bit_buf &= 0xFB;
   		}
		IWDG_Feed();
		res = STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET + 2,(u16 *)pBuffer,1);	
		if(res)
    		{
    			main_obj.bit_buf |= 0x04;	// FLASH故障
			printf("read flash err\r\n");
    		}
		 else
  		 {
			main_obj.bit_buf &= 0xFB;
  		 }
		IWDG_Feed();
		main_obj.tank_type = pBuffer[0];
		
		if(main_obj.tank_type != default_tank_type) 
		{
			main_obj.bit_buf |= 0x04;	// FLASH故障
			main_obj.tank_type = default_tank_type;
			printf("read flash err\r\n");
		}
		 else
   		{
			main_obj.bit_buf &= 0xFB;
   		}
    }
    
     // 0x10表示为非特殊水箱,0x20表示特殊水箱
		error_count = 0;
		while( ( 0x10 != main_obj.tank_type ) && ( 0x20 != main_obj.tank_type ) )
		{
			IWDG_Feed();
			error_count++;
			if(error_count > MAX_COUNT) // 超过次数MAX_COUNT，退出
			{
			 	 main_obj.tank_type = default_tank_type;
	    			main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("handle_tank_type: write flash error max, quit and set %d\r\n", default_tank_type);
				break;
			}
			printf("oil type is neither 0x10 nor 0x20, set to 0x10\r\n");
			pBuffer[0] = default_tank_type;
			res = STMFLASH_Write(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
			if(res)
  			{
  				main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("write flash err\r\n");
  			}
			 else
 			  {
				main_obj.bit_buf &= 0xFB;
   			  }
			IWDG_Feed();
			res = STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
			
			if(res)
    			{
    				main_obj.bit_buf |= 0x04;	// FLASH故障
				printf("read flash err\r\n");
    			}
			 else
  			 {
				main_obj.bit_buf &= 0xFB;
  			 }
			
			main_obj.tank_type = pBuffer[0];
			printf("handle_tank_type,oil type is %x\r\n", main_obj.tank_type);
			if(main_obj.tank_type != default_tank_type) // 若不是默认值，则进入下一轮循环
			{
				main_obj.tank_type = 0xff;	
			}
		}
    
    main_obj.oil_type_send = main_obj.oil_type|(main_obj.tank_type>>4);
    
    main_obj.pre_oil_type = main_obj.oil_type_send;
	    
}

/*******************************************************************************
* 名称：数据发送
* 功能：处理数据并发送
*******************************************************************************/

void send_data( void )
{
	uint8  send_buffer[15];
	uint8 i;
	
	main_obj.weight_send = 222;	
	send_buffer[0] = 0xAA;
	send_buffer[1] = 0x55;
	send_buffer[2] = main_obj.bit_buf;   
	send_buffer[3] = 0x0f;	    
	send_buffer[4] =main_obj.weight_send&0x00FF; 
	send_buffer[5] = main_obj.weight_send>>8;  
	send_buffer[6] = main_obj.height_send&0x00FF;
	send_buffer[7] = main_obj.height_send>>8;         
	send_buffer[8] = main_obj.vol_oil_send&0x00FF; 
	send_buffer[9] = main_obj.vol_oil_send>>8;
	send_buffer[10]= main_obj.temper_send&0x00FF;           
	send_buffer[11]= main_obj.temper_send>>8;
	send_buffer[12] = main_obj.work_status;
	send_buffer[13] = main_obj.oil_type_send;
	send_buffer[14] = 0;
	
	for ( i=0; i<14; i++ ) // 计算校验和
	{
		send_buffer[14] += send_buffer[i];
	}
	send_buffer[14] = send_buffer[14]&0xff;
	send_buffer[14] = 256 - send_buffer[14];
	USART_ClearFlag(USART2,USART_FLAG_TC);
	for ( i=0; i<15; i++ )  // 将send_data数组中24个数据传输至UART
	{
		USART_SendData(USART2, send_buffer[i]);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);	
		//IWDG_Feed();                   			
	}
}

/*******************************************************************************
* 名称：周期自检
* 功能：对电源电源进行周期自检
*******************************************************************************/
static void power_check( void )
{
	uint16  itemp;
  uint8 cpu_test1 = 1;
  uint8 cpu_test2 = 2;
  uint8 cpu_test3 = 3;
	
	itemp = ( uint16 )( capture_filter_exec(ADC_Channel_12 ) * 0.02 ); // 采集5VBIT,
	printf("power 5v check :%d\r\n",itemp);	
	if ( ( itemp>46 ) && ( itemp < 54 ) )
	{
		main_obj.bit_buf &= 0xBF;     // 电压在正常范围内		
		main_obj.bit_count[0] = 0;
	}
	else
	{		
		main_obj.bit_count[0]++;
		if( 99 < main_obj.bit_count[0] )
    {
			main_obj.bit_count[0] = 100;
			main_obj.bit_buf |= 0x40;     // 供电故障
    }

	}
	
	itemp = ( uint16 )( capture_filter_exec( ADC_Channel_13 ) * 0.02 );   // 采集3.3VBIT
	printf("power 3.3v check :%d\r\n",itemp);
	
	if ( ( itemp > 30 ) && ( itemp < 37 ) )
	{
		main_obj.bit_buf &= 0x7F;     // 电压在正常范围内
		main_obj.bit_count[1] = 0;
	}
	else
	{
	main_obj.bit_count[1]++;
	if( 99 < main_obj.bit_count[1] )
  {
		main_obj.bit_count[1] = 100;
		main_obj.bit_buf |= 0x80;     // 电源模块故障
  }
	}

	//对cpu进行自检
	if(cpu_test1 + cpu_test2 == cpu_test3)
	{
		main_obj.bit_buf &= 0xfe;
	}
	else
	{
		main_obj.bit_buf |= 0x01;
	}
}

/*******************************************************************************
* 名称：水位电压采集
* 功能：采集水位电压以及相应水位故障位
*******************************************************************************/
static void oil_vol_capture(void)
{
	uint8 judge;
	uint16 itemp;
	
	// 第0通道, 水位
	itemp = ( uint16 )capture_filter_exec( ADC_Channel_10 ); // 第一次滤波，滤波深度1/10
	//printf("vol_oil_f : %f\r\n",capture_filter_exec( ADC_Channel_10 ));
	judge = capture_filter_judge( HU_MIN, HU_MAX,HU_CHA, itemp,capture_filter_get_average(0) );//上下限以及步长判断
	//参数依次为：水位最小值、最大值、差值允许的最大值、需要比较的值、理想值（目标值）
	
	switch( judge )
	{
		case 0:             //上下限有效，差值有效
		{
			printf("oil vol is valid\r\n");
			main_obj.bit_count[2] = 0;
			main_obj.step_count[0] = 0;
			main_obj.bit_buf &= 0xEF;			 
			break;
		}
		case 1:             //上下限有效，差值无效
		{
			main_obj.bit_buf &= 0xEF;
			main_obj.step_count[0]++;
			if ( main_obj.step_count[0] < 10 )		// 差值无效，不计入误差判断
			{
			itemp = capture_filter_get_average( 0 );
			}
			else if( 99 < main_obj.step_count[0] )
			{
				 main_obj.step_count[0] = 100;
			}
			break;
		}
		case 2:             //上下限无效，差值有效 	
		case 3:             //上下限无效，差值无效
		{
			main_obj.bit_count[2]++;
			if ( main_obj.bit_count[2] > 99 )
			{
				main_obj.bit_buf |= 0x10;
				main_obj.bit_count[2] = 100;
			}
		else
		{
			itemp = capture_filter_get_average( 0 );
		}
		break;
		}
		default:
		{
		break;
		}
	}
	
	main_obj.vol_oil = capture_filter_filter( 0, itemp );
	printf("vol_oil_filter1 : %d\r\n",main_obj.vol_oil );
}

/*******************************************************************************
* 名称：纯净水温度采集
* 功能：采集纯净水温度以及相应的故障位
*******************************************************************************/
static void oil_temper_capture(void)
{
	uint16 itemp;
	uint8 judge;
	itemp = ( uint16 )capture_filter_exec( ADC_Channel_11 );
	//printf("vol_temper_f : %f\r\n",capture_filter_exec( ADC_Channel_11 ));
	judge = capture_filter_judge( TU_MIN, TU_MAX, TU_CHA, itemp, capture_filter_get_average( 1 ) );
	switch( judge )
	{
		case 0:						   //上下限有效，差值有效
		{
			printf("temper vol is valid\r\n");
			main_obj.bit_buf &= 0xDF;
			main_obj.bit_count[3] = 0;
			main_obj.temper_error_flag = 0;
			break;
		}
		case 1:							//上下限有效，差值无效，不计入故障判断
		{
			main_obj.bit_buf &= 0xDF;
			main_obj.step_count[1]++;
			main_obj.temper_error_flag = 0;
			if ( main_obj.step_count[1] < 10 )
			{
				itemp = capture_filter_get_average( 1 );
			}
			else if( 99 < main_obj.step_count[1] )
		    {
		        main_obj.step_count[1] = 100;
		    }
			break;
		}
		case 2:							  //上下限无效，差值有效
		case 3:							  //上下限无效，差值无效
		{
			main_obj.bit_count[3]++;
			if ( main_obj.bit_count[3] > 99 )
			{
				main_obj.bit_buf |= 0x20;
				main_obj.temper_error_flag = 1;
				main_obj.bit_count[3] = 100;
			}
			else
			{
				itemp = capture_filter_get_average( 1 );
			}
			break;
		}
		default:
		{
			break;
		}
	}

	main_obj.vol_temper = capture_filter_filter( 1, itemp );
	printf("vol_temper_filter1 : %d\r\n",main_obj.vol_temper );
}

/*******************************************************************************
* 名称：指令响应
* 功能：响应机电管理计算机或PC的指令，进行零位满位、调温度、纯净水水类型号曲线切换
*******************************************************************************/
static void handle_command( void )
{
    uint16 addr;
    u8 pBuffer[2] ={0};
    u8 TmpBuffer[2] = {0};
    u8 res = 0;
	
    // main_obj.uart_flag = 1表示接收到满足通信协议的指令，响应后该指令无效
    if ( 1 == main_obj.uart_flag )
   {
   	switch(main_obj.oil_state )
		{
			case STATE0:	// STATE0为初始状态，可以响应纯净水水类型号、水箱类型的指令
			{
				printf("Enter STATE0! \r\n");
				if( main_obj.type_tank_confirm==0x00 )
				{
					// 如果收到水类型号为１号 0x10、水箱为非特殊0x10，则进入状态1
					if( (main_obj.oil_type_target==0x10 )&&(main_obj.tank_type_target==0x10))
					{
						main_obj.oil_type_send = 0x11;	// 反馈的信息
						main_obj.oil_state = STATE1;
						main_obj.state_counter = 0;
						printf("Switch STATE0 to State1! \r\n");
					}
					// 如果收到水类型号为１号 0x10、水箱为特殊0x20，则进入状态2
					else if((main_obj.oil_type_target==0x10 )&&(main_obj.tank_type_target==0x20))
					{
						main_obj.oil_type_send = 0x12;	// 反馈的信息
						main_obj.oil_state = STATE2;
						main_obj.state_counter = 0;	// 开始计数，该状态只能保持一段时间，该时间段确认信息才能生效
						printf("Switch STATE0 to State2! \r\n");
					}
					// 如果收到水类型号为２号 0x20、水箱为非特殊0x10，则进入状态3
					else if((main_obj.oil_type_target==0x20 )&&(main_obj.tank_type_target==0x10))
					{
						main_obj.oil_type_send = 0x21;
						main_obj.oil_state = STATE3;
						main_obj.state_counter = 0;// 开始计数，该状态只能保持一段时间，该时间段确认信息才能生效
						printf("Switch STATE0 to State3! \r\n");
					}
					// 如果收到水类型号为２号0x20、水箱为特殊0x20，则进入状态4
					else if((main_obj.oil_type_target==0x20 )&&(main_obj.tank_type_target==0x20))
					{
						main_obj.oil_type_send = 0x22;
						main_obj.oil_state = STATE4;
						main_obj.state_counter = 0; // 开始计数，该状态只能保持一段时间，该时间段确认信息才能生效
						printf("Switch STATE0 to State4! \r\n");
					}
				}
				break;
			}
			case STATE1:
			{
				printf("Enter State1! \r\n");
				// 状态1中，当确认信息为0x11时，表示更改水类型号和曲线一致，指令有效
				if( main_obj.type_tank_confirm == 0x11 )
				{
					main_obj.oil_type_send = 0x11;
					main_obj.oil_type = 0x10;
					main_obj.tank_type = 0x10;
					main_obj.pre_oil_type = main_obj.oil_type_send;
					main_obj.oil_cmd_valid = 1;
					printf("Switch STATE0 to State1 succeed! \r\n");
				}
				// 状态1中，当确认信息为0x00且水类型号、曲线一致时，继续等等确认信息，等待时间有效
				else if( main_obj.type_tank_confirm == 0x00 )
				{
					if((main_obj.oil_type_target==0x10 )&&(main_obj.tank_type_target==0x10))
					{
						// 超时则退回状态0
						if( main_obj.state_counter>=STATE_TIMER )
						{
							main_obj.oil_state = STATE0;
							main_obj.oil_type_send = main_obj.pre_oil_type;
							printf("waiting too long ,return to STATE0! \r\n");
						}
					}
					else	//当水类型号、曲线不一致时，直接退回状态0
					{
						main_obj.oil_state = STATE0;
						main_obj.oil_type_send = main_obj.pre_oil_type;
						printf("command unmatched in STATE1! \r\n");
					}
				}
				else
				{
					// 确认信息错误时，直接退回状态0
					main_obj.oil_state = STATE0;
				}
				break;
			} 
			case STATE2:
			{
				printf("Enter State2! \r\n");
				// 确认指令有效
				if( main_obj.type_tank_confirm == 0x12 )
				{
					main_obj.oil_type_send = 0x12;
					main_obj.oil_type = 0x10;
					main_obj.tank_type = 0x20;
					main_obj.oil_cmd_valid = 1;
					main_obj.pre_oil_type = main_obj.oil_type_send;
					printf("Switch STATE0 to State2 succeed! \r\n");
					break;
				}
					// 状态2中，当确认信息为0x00且水类型号、曲线一致时，继续等等确认信息，等待时间有效
				else if( main_obj.type_tank_confirm == 0x00 )
				{
					if((main_obj.oil_type_target==0x10 )&&(main_obj.tank_type_target==0x20))
					{
						if( main_obj.state_counter>=STATE_TIMER )
						{
							main_obj.oil_state = STATE0;
							main_obj.oil_type_send = main_obj.pre_oil_type;
							printf("waiting too long ,State2 return to STATE0! \r\n");
						}
					}
					else	//当水类型号、曲线不一致时，直接退回状态0
					{
						main_obj.oil_state = STATE0;
						main_obj.oil_type_send = main_obj.pre_oil_type;
						printf("unmatched command in STATE2! \r\n");
					}
				}
				else
				{
					// 确认信息错误时，直接退回状态0
					main_obj.oil_state = STATE0;
					main_obj.oil_type_send = main_obj.pre_oil_type;
				}
				break;
			}
			case STATE3:
			{
				printf("Enter State3! \r\n");
				// 确认指令有效
				if( main_obj.type_tank_confirm == 0x21 )
				{
					main_obj.oil_type_send = 0x21;
					main_obj.oil_type = 0x20;
					main_obj.tank_type = 0x10;
					main_obj.pre_oil_type = main_obj.oil_type_send;
					main_obj.oil_cmd_valid = 1;
					printf("Switch STATE0 to State3 succeed! \r\n");
				}
				else if( main_obj.type_tank_confirm == 0x00 )
				{
					// 当水类型号、曲线一致时，继续等等确认信息，等待时间有效
					if((main_obj.oil_type_target==0x20 )&&(main_obj.tank_type_target==0x10))
					{
						if( main_obj.state_counter>=STATE_TIMER )
						{
							main_obj.oil_state = STATE0;
							main_obj.oil_type_send = main_obj.pre_oil_type;
							printf("waiting too long ,State3 return to STATE0! \r\n");
						}
					}
					else	//当水类型号、曲线不一致时，直接退回状态0
					{
						main_obj.oil_state = STATE0;
						main_obj.oil_type_send = main_obj.pre_oil_type;
						printf("unmatched command in STATE3! \r\n");
					}
				}
				else
				{
						main_obj.oil_state = STATE0;// 确认信息错误时，直接退回状态0
						main_obj.oil_type_send = main_obj.pre_oil_type;
				}
				break;
			}
			case STATE4:
			{
				printf("Enter State4! \r\n");
				// 确认指令有效
				if( main_obj.type_tank_confirm == 0x22 )
				{
					main_obj.oil_type_send = 0x22;
					main_obj.oil_type = 0x20;
					main_obj.tank_type = 0x20;
					main_obj.pre_oil_type = main_obj.oil_type_send;
					main_obj.oil_cmd_valid = 1;	//	水类型号-曲线更改信息有效
					printf("Switch STATE0 to State4 succeed! \r\n");
				}
				else if( main_obj.type_tank_confirm == 0x00 )
				{
					// 当水类型号、曲线一致时，继续等等确认信息，等待时间有效
					if((main_obj.oil_type_target==0x20 )&&(main_obj.tank_type_target==0x20))
					{
						if( main_obj.state_counter>=STATE_TIMER )
						{
							main_obj.oil_state = STATE0;
							main_obj.oil_type_send = main_obj.pre_oil_type;
							printf("waiting too long ,State4 return to STATE0! \r\n");
						}
					}
					else	//当水类型号、曲线不一致时，直接退回状态0
					{
						main_obj.oil_state = STATE0;
						main_obj.oil_type_send = main_obj.pre_oil_type;
						printf("unmatched command in STATE4! \r\n");
					}
				}
				else
				{
					main_obj.oil_state = STATE0;	// 确认信息错误时，直接退回状态0
					main_obj.oil_type_send = main_obj.pre_oil_type;
				}
				break;
			}
			default:
				break;
				
	}

	// 水类型号-曲线更改信息有效
	if( main_obj.oil_cmd_valid )
	{
		//把纯净水水类型号写入flash
		pBuffer[0] = main_obj.oil_type;
		IWDG_Feed();
		res = STMFLASH_Write(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)pBuffer,1);
		if(res)
		{
			main_obj.bit_buf |= 0x04;
			printf("write flash err\r\n");
		}
		 else
			{
			main_obj.bit_buf &= 0xFB;
			}
		IWDG_Feed();
		STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET,(u16 *)TmpBuffer,1);
		if(res)
		{
			main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
		}
		 else
			{
			main_obj.bit_buf &= 0xFB;
			}
		IWDG_Feed();
		if(!(TmpBuffer[0] == pBuffer[0] && TmpBuffer[1] == pBuffer[1]))
		{
			main_obj.bit_buf |= 0x04;
			printf("write flash err\r\n");
		}
		 else
			{
			main_obj.bit_buf &= 0xFB;
			}
	
		if( 0x20 == main_obj.oil_type )
		{
			main_obj.oil_type_addr = 0;		// ２号的存储偏移地址
		}
		else
		{
			main_obj.oil_type_addr = 512;  // １号的存储偏移地址
		}
				// 重新从flash参数区中读取相关参数.
		operation_read_flash_param( main_obj.oil_type_addr, main_obj.const_temp_array );
	
		//把特殊水箱类型写入flash
		pBuffer[0] = main_obj.tank_type;
		IWDG_Feed();
		res = STMFLASH_Write(FLASH_BASE_ADDR + OIL_TYPE_OFFSET + 2,(u16 *)pBuffer,1);
		if(res)
		{
			main_obj.bit_buf |= 0x04;
			printf("write flash err\r\n");
		}
		 else
			{
			main_obj.bit_buf &= 0xFB;
			}
		IWDG_Feed();
		STMFLASH_Read(FLASH_BASE_ADDR + OIL_TYPE_OFFSET + 2,(u16 *)TmpBuffer,1);
		if(res)
		{
			main_obj.bit_buf |= 0x04;
			printf("read flash err\r\n");
		}
		else
		{
			main_obj.bit_buf &= 0xFB;
		}
		IWDG_Feed();
		if(!(TmpBuffer[0] == pBuffer[0] && TmpBuffer[1] == pBuffer[1]))
		{
			main_obj.bit_buf |= 0x04;
			printf("write flash err\r\n");
		}
		 else
			{
			main_obj.bit_buf &= 0xFB;
			}
	
		operation_read_flash_param( main_obj.oil_type_addr, main_obj.const_temp_array );
	
		printf("oil type : %x\r\n",main_obj.oil_type);
		main_obj.oil_cmd_valid = 0;
	}
			
			//接收到机电的零位满位信息
			// １号(0x10）的偏移地址为512，２号(0x20）的偏移地址为0
			if( 0x20 == main_obj.oil_type )
			{
				addr = 0;		// ２号的存储偏移地址
			}
			else
			{
				addr = 512;  // １号的存储偏移地址
			}

			main_obj.t0_flag  = 0;       // 初始化调0标志
			main_obj.t1_flag  = 0;       // 初始化满位标志
			main_obj.temper_flag = 0;    // 初始化调温度标志

			switch ( main_obj.command_status )
			{
				case 0x00:    // 发送正常工作指令
				{
					main_obj.t0_flag = 0;
					main_obj.t1_flag = 0;
					main_obj.temper_flag= 0;
					break;
				}
				case 0x0f:    // 对方发送零位指令过来
				{
					printf("begin to adjust to zero\r\n");
					operation_t0t1( 0, addr, main_obj.vol_oil_send, 
					                main_obj.temper);
					main_obj.t0_flag = 1;
					break;
				}
				case 0xf0:    // 对方发送满位指令过来
				{
					printf("begin to adjust to full\r\n");
					operation_t0t1( 1, addr, main_obj.vol_oil_send, 
													main_obj.temper);					
					main_obj.t1_flag = 1;
					break;
				}
				case 0x33:   // 对方发送调温度指令过来
				{
					printf("begin to adjust temper\r\n");
					operation_temper( main_obj.temper_target, main_obj.temper_org, ( float * )&main_obj.const_temp_array[4] );
					main_obj.temper_flag = 1;
					break;
				}
				default:
				{
					break;
				}
			}
			main_obj.uart_flag = 0;
	}
}


/*******************************************************************************
* 名称：反馈工作状态
* 功能：根据收到的机电管理计算机或PC的指令和产品操作，反馈指令
*******************************************************************************/
static void handle_status(void)
{
    if ( ( 0 == main_obj.t0_flag ) && ( 0 == main_obj.t1_flag ) && ( 0 == main_obj.temper_flag ) )
		{
			switch( main_obj.command_status )
			{
				case 0x00:
				{
					printf("working status\r\n");
					main_obj.work_status = 0x00;
					break;
				}
				case 0x0f:
				{
					printf("adjust to zero failed\r\n");
					main_obj.work_status = 0x39;	// 零位失败
					break;
				}
				case 0xf0:
				{
					printf("adjust to full failed\r\n");
					main_obj.work_status = 0x49;	// 满位失败
					break;
				}
				case 0x33:
				{
					printf("adjust temper failed\r\n");
					main_obj.work_status = 0x99;					
					break;
				}
				default:
				{
					break;
				}
			}
		}
		else if ( ( 1 == main_obj.t0_flag ) && ( 0 == main_obj.t1_flag ) && ( 0 == main_obj.temper_flag ) )
		{
			printf("adjust to zero success\r\n");
			main_obj.work_status = 0x30;            // 零位成功
			//	main_obj.height_send   = HEIGHT_ZERO;    // 零位时发送的高度
			//	main_obj.weight_send   = WEIGHT_MIN;     // 零位时的质量

		}
		else if ( ( main_obj.t0_flag == 0 ) && ( main_obj.t1_flag == 1 ) && ( main_obj.temper_flag == 0 ) )
		{
			printf("adjust to full success\r\n");
			main_obj.work_status = 0x40; // 满位成功
			//	main_obj.height_send = HEIGHT_FULL;        //满位时发送的高度
			//	main_obj.weight_send = WEIGHT_MAX;     // 满位时发送的质量

		}
		else if( ( main_obj.t0_flag == 0 ) && ( main_obj.t1_flag == 0 ) && ( main_obj.temper_flag == 1 ) )
		{
			printf("adjust temper success\r\n");
			main_obj.work_status = 0x88;   // 调温度成功

			main_obj.temper_send = main_obj.temper_target;      //temper为实际温度,发送的是加码了的温度，所以需要接口文件的解码公式
		}
}

/*******************************************************************************
* 名称：数据滤波
* 功能：对采集的温度电压、水位电压进行滤波，得到纯净水温度和水位高度
*******************************************************************************/
static void sensor_data_filter(void)
{
    uint8 i;
    float itemp,sum;
    float height_zero,height_full;
    
    // 温度数据滤波
    if ( main_obj.temper_count < TEMP_VOL_NUM )
    {
        // 使用最新数据更新
        main_obj.temper_vol_array[main_obj.temper_count] = capture_filter_get_last_data(1);

        main_obj.temper_count++;
    }
    else
    {
        for( i=0; i<main_obj.temper_count-1; i++ )
        {
            main_obj.temper_vol_array[i] = main_obj.temper_vol_array[i+1];
        }

        // 使用最新数据更新
        main_obj.temper_vol_array[main_obj.temper_count-1] = capture_filter_get_last_data(1);

    } 
          
    sum = 0.0;
    for( i=0; i<main_obj.temper_count; i++ )
    {
        sum = sum + main_obj.temper_vol_array[i];
    }
    
    itemp = sum/main_obj.temper_count;
    main_obj.vol_temper_send = (uint16)itemp;
    printf("vol_temper_send_filter2 : %d\r\n",main_obj.vol_temper_send);
    itemp = itemp * 0.001;    // 毕竟send_buffer[16]中的单位为mv，oil_temper单位变为v
    
    // 温度计算
    main_obj.temper      =  CONST_T3*itemp*itemp*itemp + CONST_T2*itemp*itemp +
                            CONST_T1*itemp + CONST_T0 + main_obj.const_temp_array[4];
							
    if(main_obj.temper < REAL_OIL_TEMPER_MIN)
    {
    	main_obj.temper = REAL_OIL_TEMPER_MIN;
			printf("temper : less than %d\r\n", REAL_OIL_TEMPER_MIN);
    }
    if(main_obj.temper > REAL_OIL_TEMPER_MAX)
    {
    	main_obj.temper = REAL_OIL_TEMPER_MAX;
			printf("temper : large than %d\r\n", REAL_OIL_TEMPER_MAX);
    }
    if(main_obj.temper_error_flag == 1)
    {
    	main_obj.temper = 20;
    }
    // 未加补偿值的温度                        
    main_obj.temper_org  =  ( main_obj.temper - main_obj.const_temp_array[4] )*10 + 600;
	
    if(main_obj.temper_org < CONVER_OIL_TEMPER_MIN)
    {
    	main_obj.temper_org = CONVER_OIL_TEMPER_MIN;
			printf("main_obj.temper_org : less than %d\r\n", CONVER_OIL_TEMPER_MIN);
    }
    if(main_obj.temper_org > CONVER_OIL_TEMPER_MAX)
    {
    	main_obj.temper_org = CONVER_OIL_TEMPER_MAX;
			printf("main_obj.temper_org : large than %d\r\n", CONVER_OIL_TEMPER_MAX);
    }

    // 加了补偿值的温度
    main_obj.temper_send =  main_obj.temper * 10 + 600.5;     // temper为实际温度,发送的是加码了的温度，所以需要接口文件的解码公式
    
    // 根据纯净水水类型号和纯净水温度确定纯净水密度
    if(main_obj.oil_type == 0x10)
    main_obj.density = ((float)main_obj.temper * ( -0.748 ) + 794.4)*0.001;
    else if (main_obj.oil_type == 0x20)
    main_obj.density = (-0.0001*( float )main_obj.temper*( float )main_obj.temper + ( float )main_obj.temper*(-0.7787) + 830.34)*0.001;
    else
    main_obj.density = ((float)main_obj.temper * ( -0.748 ) + 794.4)*0.001;		
    printf("temper : %f\r\n",main_obj.temper);
   
    // 水位数据滤波 
    if ( main_obj.data_count < OIL_VOL_NUM )
    {
         // 使用最新数据更新
        main_obj.oil_vol_array[main_obj.data_count] = capture_filter_get_last_data(0);
        main_obj.data_count++;
    }
    else
    {
        for( i=0; i<main_obj.data_count-1; i++ )
        {
            main_obj.oil_vol_array[i] = main_obj.oil_vol_array[i+1];
        }
        main_obj.oil_vol_array[main_obj.data_count-1] = capture_filter_get_last_data(0);

    }
    
    sum = 0.0;
    for( i=0; i<main_obj.data_count; i++ )
    {
        sum = sum + main_obj.oil_vol_array[i];
    }
    
    itemp = sum/main_obj.data_count;
    main_obj.vol_oil_send = (uint16)itemp;
        
    printf("vol_oil_send_filter2 : %f\r\n",itemp);


		height_zero = HEIGHT_ZERO;
		height_full = HEIGHT_FULL;
		
    // 高度输出结果
    // 电压-温度-高度公式
    itemp = height_zero + ( height_full - height_zero) * ( itemp - main_obj.const_temp_array[0] * main_obj.temper - main_obj.const_temp_array[1] ) /
           ( ( main_obj.const_temp_array[0] - main_obj.const_temp_array[2] ) * main_obj.temper + main_obj.const_temp_array[3] - main_obj.const_temp_array[1] );

		printf("const_temp_array[0] : %f\r\nconst_temp_array[1] : %f\r\nconst_temp_array[2] : %f\r\nconst_temp_array[3] : %f\r\n",main_obj.const_temp_array[0],
		main_obj.const_temp_array[1],main_obj.const_temp_array[2],main_obj.const_temp_array[3]);
    // 实际高度
    main_obj.height = itemp/10;
    //main_obj.height = main_obj.height + CONST_R_3 * main_obj.height * main_obj.height * main_obj.height + CONST_R_2 * main_obj.height * main_obj.height + CONST_R_1 * main_obj.height + CONST_R_0;

    // 发送的加码高度
    main_obj.height_send = ( uint16 )itemp;
    printf("height_send : %d\r\n",main_obj.height_send);
   
}

/*******************************************************************************
* 名称：纯净水质量计算
* 功能：计算纯净水质量
*******************************************************************************/
static void handle_weight(void)
{
    float weight;
    float volume;
    float proof_volume = 0.0;
    int tank_para = 0;

	
   // 如果为特殊水箱则使用特殊水箱的曲线，否则默认使用非特殊水箱
 	if( 0x20==main_obj.tank_type )
 	{
 		tank_para = 1;
 	}
 	else
 	{
 		tank_para = 0;
 	} 
 	 
	if(main_obj.height > CONST_HEIGHT_TOP)
	{
	    volume = VOLUME_MIN;
	}
	else if(main_obj.height <= CONST_HEIGHT_BOTTOM)
	{
		volume = VOLUME_MAX - tank_para*PROOF_MAX;
	}
	else if((main_obj.height > CONST_HEIGHT_BOTTOM) &&( main_obj.height <= CONST_HEIGHT_TOP ))     // 考虑误差，防止过度传感器过度报警
	{  
	    if((main_obj.height > CONST_HEIGHT_BOTTOM) &&(main_obj.height <= CONST_HEIGHT_1))
        {
          volume = CONST_1_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_1_W2 * main_obj.height* main_obj.height + CONST_1_W1* main_obj.height + CONST_1_W0;
	    		
	    		// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_1_W2 * main_obj.height* main_obj.height + CONST_PROOF_1_W1* main_obj.height + CONST_PROOF_1_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;	
	    }
	
	    else if((main_obj.height > CONST_HEIGHT_1) &&(main_obj.height <= CONST_HEIGHT_2))
        {
          volume = CONST_2_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_2_W2 * main_obj.height* main_obj.height + CONST_2_W1* main_obj.height + CONST_2_W0;
        	
        	// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_2_W2 * main_obj.height* main_obj.height + CONST_PROOF_2_W1* main_obj.height + CONST_PROOF_2_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
        }
	
	   else if((main_obj.height >= CONST_HEIGHT_2) &&(main_obj.height <= CONST_HEIGHT_3))
        {
          volume = CONST_3_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_3_W2 * main_obj.height* main_obj.height + CONST_3_W1* main_obj.height + CONST_3_W0;
        	// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，否则为0
	    		proof_volume = tank_para*( CONST_PROOF_3_W2 * main_obj.height* main_obj.height + CONST_PROOF_3_W1* main_obj.height + CONST_PROOF_3_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
        }
	
	    else if((main_obj.height > CONST_HEIGHT_3) &&(main_obj.height <= CONST_HEIGHT_4))
        {
          volume = CONST_4_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_4_W2 * main_obj.height* main_obj.height + CONST_4_W1* main_obj.height + CONST_4_W0;
	    		
	    		// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_4_W2 * main_obj.height* main_obj.height + CONST_PROOF_4_W1* main_obj.height + CONST_PROOF_4_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
	    }
	
	    else if((main_obj.height > CONST_HEIGHT_4) &&(main_obj.height <= CONST_HEIGHT_5))
        {
          volume = CONST_5_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_5_W2 * main_obj.height* main_obj.height + CONST_5_W1* main_obj.height + CONST_5_W0;
        	
        	// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_5_W2 * main_obj.height* main_obj.height + CONST_PROOF_5_W1* main_obj.height + CONST_PROOF_5_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
        }
		
	//传感器2	
				#if BOX2
	 else if((main_obj.height > CONST_HEIGHT_5) &&(main_obj.height <= CONST_HEIGHT_6))
        {
         volume = CONST_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_6_W2 * main_obj.height* main_obj.height + CONST_6_W1* main_obj.height + CONST_6_W0;
          
          // 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
        }
      else if((main_obj.height >= CONST_HEIGHT_6) &&(main_obj.height <= CONST_HEIGHT_TOP))
       {
         volume = CONST_7_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_7_W2 * main_obj.height* main_obj.height + CONST_7_W1* main_obj.height + CONST_7_W0;
       	
       	  // 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_7_W2 * main_obj.height* main_obj.height + CONST_PROOF_7_W1* main_obj.height + CONST_PROOF_7_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
       }
			 #endif

	//传感器4	
			 #if BOX4
	 else if((main_obj.height > CONST_HEIGHT_5) &&(main_obj.height <= CONST_HEIGHT_TOP))
        {
          volume = CONST_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_6_W2 * main_obj.height* main_obj.height + CONST_6_W1* main_obj.height + CONST_6_W0;
        	
        	// 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
        
        }
				#endif
      
//传感器1  
    #if BOX1				
		else if((main_obj.height >= CONST_HEIGHT_5) &&(main_obj.height <= CONST_HEIGHT_TOP))
	    {
				volume = CONST_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_6_W2 * main_obj.height* main_obj.height + CONST_6_W1* main_obj.height + CONST_6_W0;
	    
	        // 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，否则为0
	    		proof_volume = tank_para*( CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0);
	    		//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
	    }
	  #endif

//传感器3  
    #if BOX3				
		else if((main_obj.height >= CONST_HEIGHT_5) &&(main_obj.height <= CONST_HEIGHT_TOP))
	    {
				volume = CONST_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_6_W2 * main_obj.height* main_obj.height + CONST_6_W1* main_obj.height + CONST_6_W0;
	            	
	       // 当为特殊水箱时，tank_para为1，特殊材料体积proof_volume才会大于0，此时才会减去特殊材料的体积
	    		proof_volume = tank_para*( CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0);
	    	//　当为非特殊材料时，proof_volume为0
	    		volume = volume - proof_volume;
	    }
	  #endif

	}
		// 体积计算的时候需要减去零位点以下的体积，保证零位点体积为0，且体积数据连续
		volume = volume - VOLUME_MIN;
		
		if( volume < 0 )
		{
			volume = 0;
		}
	
	  weight = volume * main_obj.density;
		
		// 四舍五入，减小误差
	  main_obj.weight_send = ( uint16 )(weight+0.5);
	
    printf("weight_send : %d\r\n",main_obj.weight_send);	
}

#if 0
/*******************************************************************************
* 名称：特殊水箱的纯净水质量计算
* 功能：计算特殊水箱的纯净水质量
*******************************************************************************/
static void handle_prooftank_weight(void)
{
	float weight;
	float volume;
	
 	if(main_obj.oil_type == 0x10)
 	{
 		main_obj.WEIGHT_MAX_1 = (WEIGHT_ZERO_PROOF_rp3_1); 
		main_obj.WEIGHT_MAX_2 = (WEIGHT_ZERO_PROOF_rp3_2);
		main_obj.WEIGHT_MAX_3 = (WEIGHT_ZERO_PROOF_rp3_3) ;
		main_obj.WEIGHT_MAX_4 = (WEIGHT_ZERO_PROOF_rp3_4) ;	
 	}
	else if(main_obj.oil_type == 0x20)
	{
		main_obj.WEIGHT_MAX_1 = (WEIGHT_ZERO_PROOF_rp5_1); 
		main_obj.WEIGHT_MAX_2 = (WEIGHT_ZERO_PROOF_rp5_2) ;
		main_obj.WEIGHT_MAX_3 = (WEIGHT_ZERO_PROOF_rp5_3) ;
		main_obj.WEIGHT_MAX_4 = (WEIGHT_ZERO_PROOF_rp5_4) ;	
	}
	  
	if(main_obj.height > CONST_HEIGHT_PROOF_TOP)
	{
	    main_obj.weight_send = 0x00;
	}
	else if(main_obj.height <= CONST_HEIGHT_PROOF_BOTTOM)
	{
		#if BOX1
	   main_obj.weight_send = main_obj.WEIGHT_MAX_1;
		#endif
		#if BOX2
		main_obj.weight_send = main_obj.WEIGHT_MAX_2;
		#endif
		#if BOX3
		main_obj.weight_send = main_obj.WEIGHT_MAX_3;
		#endif
		#if BOX4
		main_obj.weight_send = main_obj.WEIGHT_MAX_4;
		#endif
	}
	else if((main_obj.height > CONST_HEIGHT_PROOF_BOTTOM) &&( main_obj.height <= CONST_HEIGHT_PROOF_TOP ))     // 考虑误差，防止过度传感器过度报警
	{
	    if((main_obj.height > CONST_HEIGHT_PROOF_BOTTOM) &&(main_obj.height <= CONST_HEIGHT_PROOF_1))
        {
          volume = CONST_PROOF_1_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_1_W2 * main_obj.height* main_obj.height + CONST_PROOF_1_W1* main_obj.height + CONST_PROOF_1_W0;
	    }
	    else if((main_obj.height > CONST_HEIGHT_PROOF_1) &&(main_obj.height <= CONST_HEIGHT_PROOF_2))
        {
          volume = CONST_PROOF_2_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_2_W2 * main_obj.height* main_obj.height + CONST_PROOF_2_W1* main_obj.height + CONST_PROOF_2_W0;
        }
	
	   else if((main_obj.height >= CONST_HEIGHT_PROOF_2) &&(main_obj.height <= CONST_HEIGHT_PROOF_3))
        {
          volume = CONST_PROOF_3_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_3_W2 * main_obj.height* main_obj.height + CONST_PROOF_3_W1* main_obj.height + CONST_PROOF_3_W0;
        }
	
	    else if((main_obj.height > CONST_HEIGHT_PROOF_3) &&(main_obj.height <= CONST_HEIGHT_PROOF_4))
        {
          volume = CONST_PROOF_4_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_4_W2 * main_obj.height* main_obj.height + CONST_PROOF_4_W1* main_obj.height + CONST_PROOF_4_W0;
	    }
	
	    else if((main_obj.height > CONST_HEIGHT_PROOF_4) &&(main_obj.height <= CONST_HEIGHT_PROOF_5))
        {
          volume = CONST_PROOF_5_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_5_W2 * main_obj.height* main_obj.height + CONST_PROOF_5_W1* main_obj.height + CONST_PROOF_5_W0;
        }
		
	//传感器2	
				#if BOX2
			else if((main_obj.height > CONST_HEIGHT_PROOF_5) &&(main_obj.height <= CONST_HEIGHT_PROOF_6))
        {
         volume = CONST_PROOF_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0;
        }
      else if((main_obj.height >= CONST_HEIGHT_PROOF_6) &&(main_obj.height <= CONST_HEIGHT_PROOF_TOP))
       {
         volume = CONST_PROOF_7_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_7_W2 * main_obj.height* main_obj.height + CONST_PROOF_7_W1* main_obj.height + CONST_PROOF_7_W0;
       }
			 #endif

	//传感器4	
			 #if BOX4
	 		else if((main_obj.height > CONST_HEIGHT_PROOF_5) &&(main_obj.height <= CONST_HEIGHT_PROOF_6))
        {
          volume = CONST_PROOF_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0;
        }
			else if((main_obj.height >= CONST_HEIGHT_PROOF_6) &&(main_obj.height <= CONST_HEIGHT_PROOF_TOP))
        {
         volume = 0;
        }
				#endif
      
//传感器1  
    #if BOX1				
		else if((main_obj.height >= CONST_HEIGHT_PROOF_5) &&(main_obj.height <= CONST_HEIGHT_PROOF_TOP))
	    {
				volume = CONST_PROOF_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0;
	    }
	  #endif

//传感器3  
    #if BOX3				
		else if((main_obj.height >= CONST_HEIGHT_PROOF_5) &&(main_obj.height <= CONST_HEIGHT_PROOF_TOP))
	    {
				volume = CONST_PROOF_6_W3 * main_obj.height * main_obj.height* main_obj.height + CONST_PROOF_6_W2 * main_obj.height* main_obj.height + CONST_PROOF_6_W1* main_obj.height + CONST_PROOF_6_W0;
	    }
	  #endif
			
		weight = volume * main_obj.density;
			
		#if BOX1
	  weight = weight - WEIGHT_ZERO_PROOF_1;
		#endif
		#if BOX2
		weight = weight - WEIGHT_ZERO_PROOF_2;
		#endif
		#if BOX3
		weight = weight - WEIGHT_ZERO_PROOF_3;
		#endif
		#if BOX4
		weight = weight - WEIGHT_ZERO_PROOF_4;
		#endif
			
	  main_obj.weight_send = ( uint16 )weight;
		}
	
	
    printf("weight_send : %d\r\n",main_obj.weight_send);	
}
#endif

/*******************************************************************************
* 名称：纯净水质量计算
* 功能：对纯净水质量进行处理
*******************************************************************************/
static void handle_send_data( void )
{	    
        
	   if(main_obj.fliter_flag == 1)  
	   {
	   	 sensor_data_filter();
	 	
	 	 	handle_weight();
	 	 
		 	main_obj.fliter_flag = 0;
	   }
		 
	    

	    printf("\r\n");	


}

