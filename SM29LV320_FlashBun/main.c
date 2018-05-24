#include <stdio.h>
#include <csl.h>
#include <csl_irq.h>
#include <csl_chip.h>
#include <csl_emifa.h>
#include <csl_emifb.h>
#include <csl_irq.h>
#include "tistdtypes.h"
#include "SM29LV320.h"


static EMIFA_Config EmifaConfig = 
{
  EMIFA_GBLCTL_RMK
  (
    EMIFA_GBLCTL_EK2RATE_HALFCLK, //1 X EMIF input clock    EMIFA_GBLCTL_EK2RATE_FULLCLK,EMIFA_GBLCTL_EK2RATE_HALFCLK,EMIFA_GBLCTL_EK2RATE_QUARCLK  0x00000002u 
    EMIFA_GBLCTL_EK2HZ_CLK,       //eclkout2 continue output during hold
    EMIFA_GBLCTL_EK2EN_ENABLE,    //eclkout2 enable output
    EMIFA_GBLCTL_BRMODE_MRSTATUS, //bus request is memory access or refresh pending/in progress
    EMIFA_GBLCTL_NOHOLD_DISABLE,
    EMIFA_GBLCTL_EK1HZ_CLK,      //eclkout1 continue output during hold
    EMIFA_GBLCTL_EK1EN_ENABLE,   //eclkout1 enable output
    EMIFA_GBLCTL_CLK4EN_DISABLE, //clkout4 output enable
    EMIFA_GBLCTL_CLK6EN_DISABLE  //clkout6 output enable
  ),
  0xffffff33,      //32BIT SDRAM,  CE0�������ⲿSDRAM
  0x22a28a21,       //32bit ASYNC32,CE1  �첽32bit������FPGA������ͨ�š�ͼ��Ԥ������(RSetup=2,TA=2,Rstrobe=10,RHOLD=2)
  0xffffff41,      //32bit SYNC32, CE2��ͬ��32bit������EDMA��ʽ��FPGA��ȡ�˲���ͼ��ͳ��ֵ����ֵ�ָ���
  0xffffff41,      //32bit SYNC32, CE3,ͬ��32bit������EDMA��ʽ��FPGA��ȡԭʼͼ������
  EMIFA_SDCTL_RMK  //EMIFA SDRAM control
  (
    EMIFA_SDCTL_SDBSZ_4BANKS, //SDRAM bank size 4 banks
    EMIFA_SDCTL_SDRSZ_12ROW,  //row number = 12
    EMIFA_SDCTL_SDCSZ_8COL,   //column number = 8
    EMIFA_SDCTL_RFEN_ENABLE,  //SDRAM refresh enable
    EMIFA_SDCTL_INIT_YES,   //SDRAM ������ÿ��CE�ռ�󣬳�ʼ��
    EMIFA_SDCTL_TRCD_OF(2), //TRCD = (Trcd / Tcyc) - 1(��Сֵ����1)
    EMIFA_SDCTL_TRP_OF(2),  //TRP = (Trp / Tcyc) - 1(��Сֵ����1)
    EMIFA_SDCTL_TRC_OF(8),  //TRC = (TrC / Tcyc) - 1(��Сֵ����5)
  EMIFA_SDCTL_SLFRFR_DEFAULT  //self refresh mode disable
  ),
  EMIFA_SDTIM_RMK                   //EMIFA SDRAM refresh control
  (
    EMIFA_SDTIM_XRFR_DEFAULT,     //EXT TIMER default
    EMIFA_SDTIM_PERIOD_OF(1250)   //refresh period,����ֵ15.625us�����ECLKOUT1=80m��tim=1250
  ),
  EMIFA_SDEXT_RMK                    //EMIFA SDRAM extension???
  (
    EMIFA_SDEXT_WR2RD_OF(0),    //cycles between write to read command = 1,subtract 1 is 0
    EMIFA_SDEXT_WR2DEAC_OF(1),  //cycles between write to precharge = 2
    EMIFA_SDEXT_WR2WR_OF(1),   //cycles between write to write = 2(��Сֵ����0)
    EMIFA_SDEXT_R2WDQM_OF(1),  //cycles between read to bex = 2
    EMIFA_SDEXT_RD2WR_OF(0),   //cycles between read to write = 1
    EMIFA_SDEXT_RD2DEAC_OF(1),   //
    EMIFA_SDEXT_RD2RD_OF(0),     //
    EMIFA_SDEXT_THZP_OF(2),    //Troh = 3 cycle
    EMIFA_SDEXT_TWR_OF(1),     //Twr = 1 clock +6 ns
    EMIFA_SDEXT_TRRD_OF(0),    //Trrd = 12ns
    EMIFA_SDEXT_TRAS_OF(5),    //Tras = 42ns
    EMIFA_SDEXT_TCL_OF(1)     //cas latency = 3 clock
  ),
  0x00000052,  //cesec0, bit6 ���壬Synchronization clock selection bit. 0=ECLKOUT1�� 1=ECLKOut2
  0x00000052,  //cesec1��Synchronization clock:ECLKOUT2, RL=2,DPRAM�����ʱ1clk������1CLK ��CEEXT=1,CE�źŶౣ��һ������
  0x00000052,  //cesec2
  0x00000052   //cesec3
};


static EMIFB_Config MyEmifbConfig = 
{
  EMIFB_GBLCTL_RMK
  (
    EMIFB_GBLCTL_EK2RATE_HALFCLK, //1 X EMIF input clock
    EMIFB_GBLCTL_EK2HZ_CLK,   //eclkout2 continue output during hold
    EMIFB_GBLCTL_EK2EN_ENABLE,  //eclkout2 enable output
    EMIFB_GBLCTL_BRMODE_MRSTATUS, //bus request is memory access or refresh pending/in progress
    EMIFB_GBLCTL_NOHOLD_ENABLE,
    EMIFB_GBLCTL_EK1HZ_CLK,   //eclkout1 continue output during hold
    EMIFB_GBLCTL_EK1EN_ENABLE //eclkout1 enable output
  ),
  0x22a28a12, // ce0, ASYNC16,BU61580 
  0x22a28a02, // ce1, ASYNC8��FLASH
  0x22a28a22,
  0x22a28a22,
  EMIFB_SDCTL_DEFAULT,
  EMIFB_SDTIM_DEFAULT,
  EMIFB_SDEXT_DEFAULT,
  0x00000002,
  0x00000002,
  0x00000002,
  0x00000002
};

//#define FLASH_BOOTLOADER

#define FLASH_BASE    0x64000000                              // Flash��ַ
#define CODE_ADDR     0x64010000                              // �û������ַ
#define ODDPIX_ADDR   0x64020000                              // ����������
#define PARA_ADDR1     0x64030000                              // ��Ԫ������ַ
#define PARA_ADDR2     0x64040000                              // ��Ԫ������ַ
#define PARA_ADDR3     0x64050000                              // ��Ԫ������ַ
#define FPGA_PROM      0x64060000

#define WriteCode 1   // �Ƿ�ͬʱ��д�����������������
#define WriteOddPix 0 

Uint32 * pt=(Uint32 *)0x00;  

unsigned int binBuffer[0x12300];  // 511188 =499.20703125
#pragma DATA_SECTION(binBuffer, ".codebuf")

extern far void vectors();

void main()
{
  FILE *fp;
  Uint16 i=0;  
  Uint16 nError=0;
  Uint32 LengthOfFile = 0;
  Uint32 testVar=0;
  Uint32 WrFlashAddr=0x00;

  CSL_init();                   //��ʼ��CSL
  EMIFA_config(&EmifaConfig);
  EMIFB_config(&MyEmifbConfig); //����EMIFA
 // IRQ_setVecs(vectors);        //�����ж�ʸ�����ַ
 // IRQ_nmiEnable();             //NMIʹ��
 // IRQ_globalEnable();          //�����ж�
  
  WrFlashAddr = FLASH_BASE;
//  FLASH_erase(WrFlashAddr, FLASH_SECTORSIZE);
  
  ////////////////////////////////////////////////////////
  // ��ȡ����д�����������
 //#ifdef FLASH_BOOTLOADER
  printf("FLASH_erase_chip ...\r\n");
  FLASH_erase_chip();
  printf("open C6416_Bootload ...\r\n");
  if((fp=fopen("..\\bin\\zlg.bin","rb"))==NULL)//C6416_Bootload.bin Flash.bin kjxm_top.bin
  {
      printf("open file fail");
      return;
  }
  printf("Read File Begin \n");
  
  pt = &binBuffer[0];
  while(!feof(fp))
  { 
      fread(pt,4,1,fp);
      pt++;
      LengthOfFile = LengthOfFile+4;
	  if((LengthOfFile % 0x1000) == 0)
	  printf("Program read 0x%x\n",LengthOfFile);
  }   
  fclose(fp);
  LengthOfFile = LengthOfFile-4;

  printf("Program Begin\n");
  if(LengthOfFile) 
  {
      //FLASH_erase(WrFlashAddr, LengthOfFile);//FLASH_SECTORSIZE); // 64k*64sectors/1024 = 4M byte
	  printf("FLASH_erase done\n");
      FLASH_write((Uint32)&binBuffer[0],WrFlashAddr,LengthOfFile);
	  printf("FLASH_write done\n");
      nError = Verify_Flash((Uint32)&binBuffer[0],WrFlashAddr,LengthOfFile);
	  printf("Verify_Flash 0x%x Error:%d...\n",LengthOfFile,nError); 
  }
  printf("Program Finished\n"); 
  printf("Code Date Verify Error :%8x\n",nError);
  while(1);
  ////////////////////////////////////////////////////////////////
  WrFlashAddr = CODE_ADDR;
  printf("open zlg ...\r\n");
  if((fp=fopen("zlg.bin","rb"))==NULL)
  {
      printf("open file fail");
      return;
  }
  printf("Read File Begin \n");
  
  pt = &binBuffer[0];
  while(!feof(fp))
  { 
      fread(pt,4,1,fp);
      pt++;
      LengthOfFile = LengthOfFile+4;
	  if((LengthOfFile % 0x1000) == 0)
	  printf("Program read 0x%x\n",LengthOfFile);
    }   
  fclose(fp);
  LengthOfFile = LengthOfFile-4;

  printf("Program Begin\n");
  if(LengthOfFile) 
  {
      FLASH_erase(WrFlashAddr, LengthOfFile);//FLASH_SECTORSIZE); // 64k*64sectors/1024 = 4M byte
      FLASH_write((Uint32)&binBuffer[0],WrFlashAddr,LengthOfFile);
      nError = Verify_Flash((Uint32)&binBuffer[0],WrFlashAddr,LengthOfFile);
	  printf("writing 0x%x Error:%d...\n",LengthOfFile,nError); 
  }
  printf("Program Finished\n"); 
  printf("Code Date Verify Error :%8x\n",nError);
  while(1);
}
//http://blog.sina.com.cn/s/blog_66eaee8f0102w5cz.html
//http://blog.csdn.net/mdqaq/article/details/53784851
//http://www.go-gddq.com/wap.aspx?cid=6&cp=3&nid=1138378&p=1&sp=6276
//http://blog.163.com/philly_li/blog/static/39267405201021213347718/
//http://blog.csdn.net/lcydhr/article/details/43272427
//http://www.codeforge.cn/article/6040