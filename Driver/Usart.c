#include "Usart.h"
//#include "sys.h"
#include "stm32f10x_usart.h"
#include <stdio.h>
#include "const.h"

extern  MainObj  main_obj;

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (unsigned char) ch);
    while (!(USART1->SR & USART_FLAG_TXE));
    return (ch);
}

int GetKey (void)
{
     while (!(USART1->SR & USART_FLAG_RXNE));
     return ((int)(USART1->DR & 0x1FF));
}


//开串口 并执行初始化
//8位数据位 无校验 1位起始位/1位停止位 允许收发中断	宏定义BAUDRATE设定波特率 低优先级中断
void uart2_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2时钟

	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10

  //Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Odd;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART2, ENABLE);                    //使能串口1

}

void uart1_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART2时钟

	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10

  //Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1

}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断
	{
		 Res =USART_ReceiveData(USART2);	//读取接收到的数据

		 switch ( main_obj.cur_pos )
	    {
	        case 0:
	        {
			    main_obj.recv_buf[main_obj.cur_pos] = Res;
				main_obj.check_sum= 0;
				main_obj.check_sum += Res;
	            if ( 0xAA == Res )
			    {
				    main_obj.cur_pos = 1;
		 	    }
			    else
			    {
			   	    main_obj.cur_pos = 0;
			   	}
			    break;
			}
	        case 1:
	        {
			    main_obj.recv_buf[main_obj.cur_pos] = Res;

	            if ( 0x55 == Res )
			    {
				    main_obj.cur_pos = 2;
				     main_obj.check_sum += Res;
		 	    }
				// 防止数据数据帧头、检验和同时为0xAA导致的丢包（Modified by Eric，2018.11.6）
				else if( 0xAA == Res )
				{
					 main_obj.cur_pos = 1;	// 此时检验和不需要更新（Modified by Eric，2018.11.6）
				}
			    else
			    {
			   	    main_obj.cur_pos = 0;
			   	}
			    break;
			}
	        case 2:
	        case 3:
	        case 4:
	        case 5:
		  case 6:
		  	case 7:
			{
	            		main_obj.recv_buf[main_obj.cur_pos] = Res;
		        	main_obj.cur_pos++;
		       	main_obj.check_sum += Res;
	            break;
	        	}
		    case 8:
		    {
			    		main_obj.recv_buf[8] = Res;		                // 如果接收到的第8个数据等于校验和，则表示接收数据成功;

			    main_obj.check_sum = main_obj.check_sum & 0xFF; // 校验和为第2个数到第6个数的和与0xFF
			    main_obj.check_sum = 256 - main_obj.check_sum;
			 		if ( (Res==main_obj.check_sum  )&&( 0x09 ==main_obj.recv_buf[3]))	            // 检验和正确的同时保证字计数字为0x09
		      {

							main_obj.uart_flag   = 1;				  // 同时，串口数据接收标识 uart_flag 置1

							// 连续收到3次同样的指令才认为该纯净水水类型号有效
							if( main_obj.oil_type_counter == 0)
							{
								main_obj.temp_oil_type = main_obj.recv_buf[4];
							}
							if( main_obj.temp_oil_type==main_obj.recv_buf[4])
							{
								main_obj.oil_type_counter++;
								if( main_obj.oil_type_counter>=3)
								{
									main_obj.oil_type_target = main_obj.recv_buf[4];	// 水类型号指令有效
									printf("main_obj.oil_type_target is valid ,value is %d! \r\n",main_obj.oil_type_target);
								}
							}
							else		// 如果两次受到的数据不一致，开始重新计数
							{
								main_obj.oil_type_counter = 0;
							}

							// 连续收到3次同样的指令才认为该水箱曲线有效
							if( main_obj.tank_type_counter == 0)
							{
								main_obj.temp_tank_type = main_obj.recv_buf[5];
							}
							if( main_obj.temp_tank_type==main_obj.recv_buf[5])
							{
								main_obj.tank_type_counter++;
								if( main_obj.tank_type_counter>=3)
								{
									main_obj.tank_type_target = main_obj.recv_buf[5];	// 水箱曲线有效
									printf("main_obj.tank_type_target is valid ,value is %d! \r\n",main_obj.tank_type_target);
								}
							}
							else	// 如果两次受到的数据不一致，开始重新计数
							{
								main_obj.tank_type_counter = 0;
							}

							// 连续收到3次同样的指令才认为工作指令有效
							if( main_obj.command_status_counter == 0)
							{
								main_obj.temp_command_status = main_obj.recv_buf[6];
							}
							if( main_obj.temp_command_status==main_obj.recv_buf[6])
							{
								main_obj.command_status_counter++;
								if( main_obj.command_status_counter>=3)
								{
									main_obj.command_status = main_obj.recv_buf[6];	// 工作指令有效
									printf("main_obj.command_status is valid ,value is %d! \r\n",main_obj.command_status);
								}
							}
							else
							{
								main_obj.command_status_counter = 0;	// 两次数据不一致，开始重新计数
							}


							// 连续收到3次同样的指令才认为该水箱-水类型号确认信息有效
							if( main_obj.type_tank_confirm_counter == 0)
							{
								main_obj.temp_type_tank_confirm = main_obj.recv_buf[7];
							}
							if( main_obj.temp_type_tank_confirm==main_obj.recv_buf[7])
							{
								main_obj.type_tank_confirm_counter++;
								if( main_obj.type_tank_confirm_counter>=3)
								{
									main_obj.type_tank_confirm = main_obj.recv_buf[7];	// 水箱-水类型号确认信息有效
									printf("main_obj.type_tank_confirm is valid ,value is %d! \r\n",main_obj.type_tank_confirm);

								}
							}
							else
							{
								main_obj.type_tank_confirm_counter = 0;	// 两次数据不一致，开始重新计数
							}
							// 调温度
							if( main_obj.command_status==0x33)
							{
								main_obj.cur_pos++;
								break;
							}
			    }
		        main_obj.cur_pos = 0;
		        main_obj.check_sum = 0;
		        break;
		    }
			case 9:
			{
				main_obj.recv_buf[9] = Res;
				main_obj.cur_pos++;
				break;
			}
			case 10:
			{
				main_obj.recv_buf[10] = Res;
				main_obj.temper_target = main_obj.recv_buf[9]*256 + main_obj.recv_buf[10];
				main_obj.cur_pos = 0;
				main_obj.check_sum = 0;
				break;
			}
            default:
                {
		        main_obj.cur_pos = 0;
		        main_obj.check_sum= 0;
		        break;
		    }
	    }


	}

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
		 Res =USART_ReceiveData(USART1);	//读取接收到的数据

		 switch ( main_obj.cur_pos )
	    {
	        case 0:
	        {
			    main_obj.recv_buf[main_obj.cur_pos] = Res;

	            if ( 0xAA == Res )
			    {
				    main_obj.cur_pos = 1;
		 	    }
			    else
			    {
			   	    main_obj.cur_pos = 0;
			   	}
			    break;
			}
	        case 1:
	        {
			    main_obj.recv_buf[main_obj.cur_pos] = Res;
	            if ( 0x55 == Res )
			    {
				    main_obj.cur_pos = 2;
		 	    }
			    else
			    {
			   	    main_obj.cur_pos = 0;
			   	}
			    break;
			}
	        case 2:
			{
	            		main_obj.recv_buf[main_obj.cur_pos] = Res;
		        	main_obj.cur_pos++;
	            break;
	        	}
		    case 3:
		    {
			  main_obj.recv_buf[main_obj.cur_pos] = Res;		                // 如果接收到的第7个数据等于校验和，则表示接收数据成功;
			  main_obj.height = main_obj.recv_buf[2]*256 + main_obj.recv_buf[3];

		        main_obj.cur_pos   = 0;

		        break;
		    }
            default:
                {
		        main_obj.cur_pos = 0;

		        break;
		    }
	    }


	}

}

