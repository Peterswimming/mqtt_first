#include "flash.h"
#include "string.h"

#define SPI_FLASH			SPI1

#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)

#define SPI_BAUD			SPI_BaudRatePrescaler_2

#define SF_CS_GPIO			GPIOA
#define SF_CS_PIN			GPIO_Pin_4

#define CS_LOW()       SF_CS_GPIO->BRR = SF_CS_PIN

/* 片选口线置高不选中 */
#define SF_CS_HIGH()      SF_CS_GPIO->BSRR = SF_CS_PIN

//#define	W25QXX_CS 

static uint8_t sf_SendByte(uint8_t _ucValue);

static void bsp_CfgSPIForSFlash(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	ENABLE_SPI_RCC();

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI_FLASH, ENABLE);				/* 使能SPI  */
	sf_SendByte(0xff);
}

static uint8_t sf_SendByte(uint8_t _ucValue)
{
	/* 等待上个数据未发送完毕 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);

	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}

void W25QXX_Init(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
		
		/*PA.5:SCK PA.6:MISO PA.7:MOSI*/
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;//CS
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	
		SF_CS_HIGH();
		bsp_CfgSPIForSFlash();
}

u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	CS_LOW();				    
	sf_SendByte(0x90);//发送读取ID命令	    
	sf_SendByte(0x00); 	    
	sf_SendByte(0x00); 	    
	sf_SendByte(0x00); 	 			   
	Temp|=sf_SendByte(0xFF)<<8;  
	Temp|=sf_SendByte(0xFF);	 
	SF_CS_HIGH();				    
	return Temp;
}   		 

u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	CS_LOW();                            //使能器件   
	sf_SendByte(W25X_ReadStatusReg); //发送读取状态寄存器命令    
	byte=sf_SendByte(0Xff);          //读取一个字节  
	SF_CS_HIGH();                            //取消片选     
	return byte;   
} 
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(u8 sr)   
{   
	CS_LOW();                            //使能器件   
	sf_SendByte(W25X_WriteStatusReg);//发送写取状态寄存器命令    
	sf_SendByte(sr);               	//写入一个字节  
	SF_CS_HIGH();                            //取消片选     	      
}   
//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
	CS_LOW();                          	//使能器件   
    sf_SendByte(W25X_WriteEnable); 	//发送写使能  
	SF_CS_HIGH();                           	//取消片选     	      
} 
//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
	CS_LOW();                            //使能器件   
  sf_SendByte(W25X_WriteDisable);  //发送写禁止指令    
	SF_CS_HIGH();                            //取消片选     	      
} 	

//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	CS_LOW();                            	//使能器件   
    sf_SendByte(W25X_ReadData);         	//发送读取命令   
    sf_SendByte((u8)((ReadAddr)>>16));  	//发送24bit地址    
    sf_SendByte((u8)((ReadAddr)>>8));   
    sf_SendByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=sf_SendByte(0XFF);   	//循环读数  
    }
	SF_CS_HIGH();  				    	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(const u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    W25QXX_Write_Enable();                  	//SET WEL 
	CS_LOW();                            	//使能器件   
    sf_SendByte(W25X_PageProgram);      	//发送写页命令   
    sf_SendByte((u8)((WriteAddr)>>16)); 	//发送24bit地址    
    sf_SendByte((u8)((WriteAddr)>>8));   
    sf_SendByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)sf_SendByte(pBuffer[i]);//循环写数  
	SF_CS_HIGH();                            	//取消片选 
	W25QXX_Wait_Busy();					   		//等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(const u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(const u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
   	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_Erase_Sector(secpos);		//擦除这个扇区
			for(i=0;i<secremain;i++)	   		//复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  				//指针偏移
			WriteAddr+=secremain;				//写地址偏移	   
		   	NumByteToWrite-=secremain;			//字节数递减
			if(NumByteToWrite>4096)secremain=4096;//下一个扇区还是写不完
			else secremain=NumByteToWrite;		//下一个扇区可以写完了
		}	 
	};	 
}
//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                 	 	//SET WEL 
    W25QXX_Wait_Busy();   
  	CS_LOW();                            	//使能器件   
    sf_SendByte(W25X_ChipErase);        	//发送片擦除命令  
	SF_CS_HIGH();                            	//取消片选     	      
	W25QXX_Wait_Busy();   				   		//等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
//	//监视falsh擦除情况,测试用   
// 	printf("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  	//SET WEL 	 
    W25QXX_Wait_Busy();   
  	CS_LOW();                            	//使能器件   
    sf_SendByte(W25X_SectorErase);      	//发送扇区擦除指令 
    sf_SendByte((u8)((Dst_Addr)>>16));  	//发送24bit地址    
    sf_SendByte((u8)((Dst_Addr)>>8));   
    sf_SendByte((u8)Dst_Addr);  
	SF_CS_HIGH();                            	//取消片选     	      
    W25QXX_Wait_Busy();   				   		//等待擦除完成
}  
//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);  		// 等待BUSY位清空
}  
//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	CS_LOW();                           	 	//使能器件   
    sf_SendByte(W25X_PowerDown);        //发送掉电命令  
	SF_CS_HIGH();                            	//取消片选     	      
//    delay_us(3);                               //等待TPD  
}   
//唤醒
void W25QXX_WAKEUP(void)   
{  
  	CS_LOW();                            	//使能器件   
    sf_SendByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
	SF_CS_HIGH();                            	//取消片选     	      
//    delay_us(3);                            	//等待TRES1
}   


