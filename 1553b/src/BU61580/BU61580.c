// ��������Bu61580

#include "BU61580.h"
#include "../tool/timer.h"
#include "../tool/tracer.h"



/*
15(MSB) RX: DOUBLE BUFFER ENABLE
14 TX: EOM INT
13 TX: CIRC BUF INT
12 TX: MEMORY MANAGEMENT 2 (MM2)
11 TX: MEMORY MANAGEMENT 1 (MM1)
10 TX: MEMORY MANAGEMENT 0 (MM0)
9 RX: EOM INT
8 RX: CIRC BUF INT
7 RX: MEMORY MANAGEMENT 2 (MM2)
6 RX: MEMORY MANAGEMENT 1 (MM1)
5 RX: MEMORY MANAGEMENT 0 (MM0)
4 BCST: EOM INT
3 BCST: CIRC BUF INT
2 BCST: MEMORY MANAGEMENT 2 (MM2)
1 BCST: MEMORY MANAGEMENT 1 (MM1)
0(LSB) BCST: MEMORY MANAGEMENT 0 (MM0)
*/

#define ASUB_ADDR_CFG_BASE 0x1a0
#define BSUB_ADDR_CFG_BASE 0x220
//subaddress_buffer_scheme.bmp
#if BUFF_SIZE == 0x200
#define BU_RTSUBADDRCTRL_VAL 0xEF7B
#define ASUB_ADDR_CFG_VAL 0x0C63 //NO DBL BUFFER, 512 SIZE
#define BSUB_ADDR_CFG_VAL 0x0C63 //NO DBL BUFFER, 512 SIZE
#else //0x100
#define BU_RTSUBADDRCTRL_VAL 0xEB5A
#define ASUB_ADDR_CFG_VAL 0x0842 //NO DBL BUFFER, 256 SIZE
#define BSUB_ADDR_CFG_VAL 0x0842 //NO DBL BUFFER, 256 SIZE
#endif



// �����ڲ�16���Ĵ���
void BU_RegSetVal(Uint16 Addr,Uint16 Val )
{
  *(volatile Uint16 *)(BU_REG+(Addr<<SHIFT_POS)) =  Val;
  Timer_DelayUS(4);       //delay 4us
}

//�����ڲ��洢��ȡֵ
void BU_MemSetVal(Uint16 Addr,Uint16 Val )
{
  *(volatile Uint16 *)(BU_MEM+(Addr<<MEM_SHIFT)) = Val;
  Timer_DelayUS(4);       //delay 4us
}

//��ȡ�Ĵ���ֵ����ȴ�ģʽ����ȡ���η�����ȷֵ
Uint16 BU_RegGetVal(Uint16 Addr)
{
  Uint32 Val=0x00;
  Val=*((volatile  Uint16 *)(BU_REG+(Addr<<SHIFT_POS)));;    // ��ȴ�ģʽ����ȡ���η�����ȷֵ
  Timer_DelayUS(4);                          //��ʱ4us
  Val=*((volatile  Uint32 *)(BU_REG+(Addr<<SHIFT_POS)));    
  Timer_DelayUS(4); 
  Val = (Val>>16)&0XFFFF; 
  return Val;
}

// ��ȡ�ڴ�ֵ����ȴ�ģʽ����ȡ���η�����ȷֵ
Uint16 BU_MemGetVal(Uint16 Addr)
{
  Uint32 Val=0x00;
  
  Val=*((Uint16 *)(BU_MEM+(Addr<<MEM_SHIFT)));    // ��ȴ�ģʽ����ȡ���η�����ȷֵ
  Timer_DelayUS(4);                              //��ʱ4us
  Val=*((Uint32 *)(BU_MEM+(Addr<<MEM_SHIFT)));    
  Timer_DelayUS(4); 
  Val = (Val >> 16)&0XFFFF; 
  return Val;
}

// ��ȡ�ڴ�ֵ��������ȡ
Uint16 BU_MemGetValOne(Uint16 Addr)
{
  Uint16 Val=0x00;
  
  Val=*((Uint16 *)(BU_MEM+(Addr<<MEM_SHIFT)));    // ѭ����ȡ�����ζ�ȡ�����ϴε�ȡֵ
  Timer_DelayUS(4);                           //��ʱ4us
  
  Val = Val&0XFFFF; 
  return Val;
}

Uint16 BU_GetStatusRegister()
{
	return BU_RegGetVal(BU_RTSTATUSWORD);
}
void BU_SetServiceRequest(bool request)
{
	BU_RegSetVal(BU_CFG1, 0x8D80);
}
#define ASEND_CMD_BASE 0x160



//����BU61580��MILSTD1553B�������.pdf
//acehg.pdf
b32 BU_Initialize(){
    u32 i = 0;
	//printf("BU_Initialize\n");
	BU_RegSetVal(BU_RST,5);//R3:reset(0) and interrupt reset(2) ,page 31, page 42
	BU_RegSetVal(BU_CFG3,0x8000);//R7:#3������ǿģʽ(15)
	
	BU_MemSetVal(0x100,STACK_A_ADDR); //��ʼ����ջAָ��λ�� 	page 42
	BU_MemSetVal(0x104,STACK_B_ADDR); //��ʼ����ջBָ��λ��   page 42
	for(i = 0; i < 0xff; i ++){
		BU_MemSetVal(STACK_A_ADDR + i, 0);//��ʼ����ջA,����
		BU_MemSetVal(STACK_B_ADDR + i, 0);//��ʼ����ջB,����
	}
	for(i = 0; i < 32; i ++){//��ʼ��lookup table
		BU_MemSetVal(0x140 + i, ARECV_BUFF_BASE);//����A���ӵ�ַ0-31�������0x0400��ʼ�ĵ�ַ��
		BU_MemSetVal(0x160 + i, ASEND_BUFF_BASE);//����A���ӵ�ַ0-31�������0x0800��ʼ�ĵ�ַ��
		BU_MemSetVal(0x180 + i, ABCST_BUFF_BASE);//����A�չ㲥�ӵ�ַ0-31�������0x0c00��ʼ�ĵ�ַ�� 
		BU_MemSetVal(0x1a0 + i, ASUB_ADDR_CFG_VAL);//����A�ӵ�ַ0-31����������Ϊ256 
		BU_MemSetVal(0x1c0 + i, BRECV_BUFF_BASE);//����B���ӵ�ַ0-31�������0x0600��ʼ�ĵ�ַ�� 
		BU_MemSetVal(0x1e0 + i, BSEND_BUFF_BASE);//����B���ӵ�ַ0-31�������0x0a00��ʼ�ĵ�ַ��
		BU_MemSetVal(0x200 + i, BBCST_BUFF_BASE);//����B�չ㲥�ӵ�ַ0-31�������0x0e00��ʼ�ĵ�ַ��
		BU_MemSetVal(0x220 + i, BSUB_ADDR_CFG_VAL);//����B�ӵ�ַ0-31����������Ϊ256
	}
	BU_RegSetVal(BU_RTSUBADDRCTRL,BU_RTSUBADDRCTRL_VAL); //R4 ���û�������С
	//step13 Busy bit lookup table
	for(i = 0; i < 8 ; i++)      // All Busy bit lookup table set 'no busy' 8  
	{
	    BU_MemSetVal(0x240 + i, 0); 
	}
	for(i = 0; i < 8; i++){
		BU_MemSetVal(0x108 + i, 0xFFFF);
	}
	//step14 Command illegalization Table,  δ�����ָ��Ϊ�Ƿ��� P157,Table80
    for(i = 0;i<256;i++)                // 0x300-0x3ff�Ƿ�
    {
        BU_MemSetVal(0x300 + i, 0xFFFF); //��ʼ���Ƚ�����ָ����Ϊ�Ƿ�
    }
	#define BUG_CFG2_VAL 0xBa97
	BU_RegSetVal(BU_CFG2, BUG_CFG2_VAL);//R2:  (0xB817/
					//Enhanced interrupts(bit15=1)          
					//RAM Parity Enable(bit14=0)
					//BUSY LOOK UP TABLE ENABLE(bit13=1)
					//RX SA DOUBLE BUFFER ENABLE(bit12=1)              
					
					//OVERWRITE INVALID DATA(bit11=1)
					//256-WORD BOUNDER Disable(bit10=0) 
					//TTR2 (bit9) //TTR1(bit8)              
					
					//TTR0 (bit7)                                 //CLEAR TIME TAG ON SYNCHRONIZE(bit6=0 NOT )
					//LOAD TIME TAG ON SYNCHRONIZE(bit5=0 NOT)    //INTERRUPT STATUS AUTO CLEAR(bit4=1)
					
					//InteruptType:(Level bit3=1;Pluse bit3=0)   //CLEAR SERVICE REQ.(bit2=1)
					//ENHANCED RT MEMORY MANG.(bit1=1)           //SEPARATE BROADCAST DATA(bit0=1)
	BU_RegSetVal(BU_CFG3, 0x8089);//R7 (0x805D)
                 //ENHANCE MODE ENABLE(bit15=1)                  
                 
                 //ILLEGALIZATION DISABLE(bit7)             //OVERRIDE MODE T/R ERROR(bit6=1)
                 //Alternate RT STATUS WORD enable(bit5=0) //ILLEGAL RX TRANSFER Disable(bit4=1)
                 
                 //BUSY RX TRANSFER DISABLE(bit3 =1)    //RTFAIL-FLAG WRAP ENABLE(bit2 =1)
                 //1553A MODE CODES ENABLE(bit2 = 0)    //ENHANCED MODE CODE HANDLING(bit0=1)
	BU_RegSetVal(BU_CFG4, 0x1000); //R8 (0X2100)
					//Mode Command Override Busy(bit13)
					//Expand BC Control Word Enable(bit12)
					//1ST Retry Alt/Same Bus* (bit8)
	BU_RegSetVal(BU_CFG5, 0x0854);//R9 (0x088e)
					// 16MHz(bit15=0)       
					
					// EXPANDED CROSSING ENABLED(bit11=1)
					
					// BroadCast Disabled(bit7) //RT Addr latch/Transspant(bit6)
					// RT Address4(bit5) //RT Address3(bit4)

					// RT Address2(bit3) //RT Address1(bit2)
                    // RT Address0(bit1)    // Program RT Address to 7(parity bit =0)
	BU_RegSetVal(BU_RTSTATUSWORD, 0x0000);//Re 
							
	BU_RegSetVal(BU_CFG1, 0x8F80); //R1: Enhanced RT Mode,  (0x8D80/0x8F80)
					//Active Area B/A (bit13),
					//s10~s8 (bit11~8=1111)
					//s6(bit7)
	BU_RegSetVal(BU_TIMETAG, 0); //R5
	BU_RegSetVal(BU_INTSTATUS,0); //R6
	#define BU_INTMASK_VAL 0x72B7
	BU_RegSetVal(BU_INTMASK,0);//R0:��ʼ���ж����μĴ���
							//EndOfMsg(0),RTModeCode(1),FmtErr(2),
							//SubAddr(4),RTBuffRollOver(5),TimeTagRollOver(6),RTAddrParityErr(7)
							//HandShake(9),
							//RTCmdSackRollOver(12),RTTransmitTimeout(13),RAMParityErrr(14)
	//IRQ_map(IRQ_EVT_EXTINT4,12);
	//IRQ_reset(IRQ_EVT_EXTINT4);
	//IRQ_disable(IRQ_EVT_EXTINT4);
	
	return vTrue;
}

void BU_disableIrq(){
	BU_RegSetVal(BU_INTMASK,0);
	//IRQ_disable(IRQ_EVT_EXTINT4);
}
void BU_enableIrq(){
	BU_RegSetVal(BU_INTMASK,BU_INTMASK_VAL);
	//IRQ_enable(IRQ_EVT_EXTINT4);
}

#define RTADDR 10
#define A_SEND(subAddr,len)  BU_MemSetVal(ASEND_CMD_BASE + subAddr, (RTADDR << 11) | (1 << 10) | (subAddr<<5)|len)

void BU_SetBusy(bool isBcst,bool isTransmit,u8 subAddr){
	u16 busyAddr = 0x240 | (isBcst ? 4 : 0) | (isTransmit ? 2 : 0) | ((subAddr & 0x10) ? 1 : 0);
	//u16 bitMask = 1 << (subAddr & 0xF);
	BU_MemSetVal(busyAddr,0);
}

void BU_SendOne(u32 subAddr){
	printf("send\r\n");
	Timer_DelayUS(4);
	BU_MemSetVal(ASEND_BUFF_BASE,0x1234);
	A_SEND(subAddr,1);
	Timer_DelayUS(1000000);
}

void BU_SetRspSync(u16 val){
	BU_MemSetVal(0x0110 + 0x11,val);
}
void BU_EnableModeInterupt(ModeType type,u16 modeCode){
    u16 idx = modeCode & 0x1F;
	u16 offset = type << 1 + idx >> 4;
	u16 val = BU_MemGetVal(0x108 + offset);
	BU_MemSetVal(0x108 + offset, val | (1 << idx));
}
void BU_DisableModeInterupt(ModeType type,u16 modeCode){
	u16 idx = modeCode & 0x1F;
	u16 offset = type << 1 + idx >> 4;
	u16 val = BU_MemGetVal(0x108 + offset);
	BU_MemSetVal(0x108 + offset, val & (~(1 << idx)));
}

    //0100H�Ƕ�ջָ��A�ĵ�ַ��
	//0104H�Ƕ�ջָ��B��ַ��
	//0000-00FFH��256�ֵĶ�ջA�ռ䣬Ҳ��1553B������Ϣ����������ÿ������ռ4���֣�һ���ɴ���64����������
				//��������4���ֱַ��ǿ�״̬�֡�ʱ������������ݿ�ָ�롢���������֡�     
	//0108-010FH��ģʽ����ѡ���жϱ�����ÿһ�����ա����͡��㲥���ա��㲥���͵�ģʽ����ʱ���Ƿ�����жϡ�
		//0x108:interrupts for receive mode codes 0-15
		//0x109:interrupts for receive mode codes 16-31
		//0x10a:interrupts for transmit mode codes 0-15
		//0x10b:interrupts for transmit mode codes 16-31
		//0x10c:interrupts for broadcast receive mode codes 0-15
		//0x10d:interrupts for broadcast receive mode codes 16-31
		//0x10e:interrupts for broadcast transmit mode codes 0-15
		//0x10f:interrupts for broadcast transmit mode codes 16-31
	//0110-013FH��ģʽ��������ݵĹ̶���ַ����RT���յ�һ��ģʽ����ʱ���������ڶ�Ӧλ�õ������Զ����ظ�BC��
	//0140-01BFH��01C0-023FH�ֱ���RT��A��B�����ַ��ѯ������A���򣬵�ַ��Ϊ4���֣�
		//0140-015FH�ֱ��Ӧ�ӵ�ַ0���ӵ�ַ31�Ľ������ݻ��������׵�ַ��
		//0160-017FH�ֱ��Ӧ�ӵ�ַ0���ӵ�ַ31�ķ������ݻ��������׵�ַ��
		//0180-019FH�ֱ��Ӧ�ӵ�ַ0���ӵ�ַ31�㲥ģʽ���ݻ��������׵�ַ��
		//01A0һ01BFH�ֱ��Ӧ�ӵ�ַ0���ӵ�ַ31���ӵ�ַ�����֣������������ݻ������Ĵ�С�͹�����ʽ��
	             //�磺����Ϣģʽ��˫����ģʽ��ѭ������ģʽ��
	//0240-0247H��æλ��ѯ������ÿһλ�ֱ��Ӧ�ӵ�ַ0���ӵ�ַ31��æ״̬��0��ʾ��æ��1��ʾæ��
	            //��RT�յ�����Ϣ��Ӧ���ӵ�ַΪæʱ������BC�˷���״̬�ֵ�æλ��1��
	//0300-03FFH�ǷǷ���ָ���ѯ����Ϊ4���֣�
		//0300-033FH��Ӧ�㲥���շǷ������
		//0340-037FH��Ӧ�㲥���ͷǷ������
		//0380-03BFH��Ӧ�ǹ㲥ʽ���շǷ������
		//03C0-03FFH��Ӧ�ǹ㲥ʽ���ͷǷ������
		//��RT�յ��Ƿ���������ʱ������BC���ض�Ӧ��״̬�֡�      
	//0260-02FFH��0400-0FFFHΪ���ݻ�������
				//�ⲿ�ֿռ�������ӵ�ַ0���ӵ�ַ31���ӵ�ַ����������ÿ���ӵ�ַ��Ӧ�����ݻ������ռ�Ĵ�С�͹�����ʽ��

//dguidehg.pdf  1-12

//1553A MODE CODES ENABLED = 0 : 
   //both subaddresses 0 and 31 to be mode code subaddresses
   //RT recognizes and responds to all MIL-STD-1553B mode codes, including those with or without, Data Words.
   //RT will decode for the MIL-STD-1553B "Transmit Status" and "Transmit Last Command" mode codes 
   //and will not update its internal RT Status Word Register as a result of
   //these commands, with the exception of setting the Message Error bit if the command is illegalized

//CLEAR SERVICE REQUEST: If this bit is logic "0," the Service Request RT Status Word bit may
	//controlled only by the host processor software. If the bit is logic "1," the Service Request bit may still be
	//set and cleared under software (register) control. In addition, the SERVICE REQUEST* Configuration
	//Register (#1) bit will automatically clear (go to logic 1) after the ACE RT has responded to a Transmit
	//Vector Word mode code command. That is, if the CLEAR SERVICE REQUEST bit is set to 1 while
	//SERVICE REQUEST* is set to 0, the ACE RT will respond with the Service Request Status bit set for all
	//commands until the RT responds to a Transmit Vector Word command. In this instance, the ACE will
	//respond with the Service Request still set in the Status Word for this message. Following this message,
	//SERVICE REQUEST* in the Configuration Register automatically clears to a logic "1." It stays logic "1"
	//(cleared) for subsequent messages until it is reasserted to a logic "0" by the host processor.

//ENHANCED MODE CODE HANDLING (bit 0 of Configuration Register #3) is not used, the pointers for receive subaddresses 0 and 31 (for
	//Synchronize with Data messages) generally get loaded with the same pointer value.
	//Similarly, the Lookup Table addresses for transmit subaddresses 0 and 31 (for Transmit Vector
	//Word messages) generally get loaded with the same pointer value.
	//If ENHANCED MODE CODE HANDLING is enabled. Data Words for these mode codes
	//are stored in locations 0111 (for Synchronize with data) and 0130 (for Transmit Vector Word).
    //Data Words for mode codes are stored in address locations 0110-013F, and
	//interrupt requests for individual mode codes may be enabled by means of a table in address locations 0108-010F. 
	//The desired mode code may be selected by setting the appropriate bit in the Mode Code
	//Selective Interrupt Table (address range 0108-010F).). When the specific mode code message
	//has been completed, the RT MODE CODE bit of the Interrupt Status Register will return logic "1".


	//BU-61580��������ʱ������Ӧ���ӵ�ַ���������͵���ʼ���趨�Ĳ�ѯ���ַ�У�
	//ͬʱ�����ӵ�ַ��Ӧ��ʸ������Ϊ��1��������RT���������и��¡�
	//��Ϊ�ӵ�ַ������λ�����˷��ͺͽ��ղ����жϣ��������жϴ�������У�
	//���������뷢���ӵ�ַ��ͬʱ������BU-61580�ѽ���Ҫ���͵����ݷ��͵�1553B�����У�
	//��ʱ�����жϷ�������н��÷����ӵ�ַ��ʸ�����塰0����
	//BC�����ԵĲ�ѯʸ���֣�ֻ�ж�Ӧ�ӵ�ַ��ʸ����Ϊ��1����BC�Ŵ�RT�н�����ȡ�ߡ�

	//����RT�����з���������Ϣ��Ȩ��,����RT��BC��RT��RT��RT��Ϊ����Դ����Ϣ��ʽ����֯,��RT�ϴ����ݵĹ�
	//����Ƚ������ݽ�Ϊ���ӡ����߿�������֯�������Ϣ��Ϊ��������Ϣ�ͷ���������Ϣ��
	//��RT��ϵͳ�������ϴ������������������Ƚ����������BU-61580����Ӧ�ӵ�ַ�������ݻ�����,
	//���÷������ݰ�����,������Ӧ���ӵ�ַ"����æ"λ����ȴ�BCȡ�����ݡ�����Ӧ���ӵ�ַ���������ж�ʱ,
	//��������������ϴ�,�����������жϴ�������н��������ݰ�������1���ڸ���Ϊ��ʱ��"����æ"λ������"1"
	//���ڸ���BC��ǰʱ��RT��ϵͳû�и������ݡ�
	//���ڷ���������Ϣ,BC��ԭ����������Ϣ�ļ�Ъ����Ҫ��RT����ʸ���ֵ�ģʽ���
	//��BC�Ե���10Hz��Ƶ����RT����ѯ��ָ��,����ѯ��RT�����¼�����ͽ�ʸ������λ,
	//BC����RT���͵�ʸ��������֯��Ϣ������¼���RT���ж��¼�������Ӧ����ʸ���ָ�λ��

//Busy bit lookup table
  //bit address:  15 14 13 12 11 10  9  8  7  6  5  4  3      2                    1    0
  //binary value:  0  0  0  0  0  0  1  0  0  1  0  0  0  BROADCAST/OWN ADDRESS*  T/R*  SA4   //0x240 ~ 0x247

  //C language u32 subAddrBusyFlag = (((u32)*(u16*)(0x240 | BROADCAST/OWN ADDRESS*<<2 | T/R*<<1 + 1)) << 16 ) | (*(u16*)(0x240 | BROADCAST/OWN ADDRESS*<<2 | T/R*<<1))

  //bit address :  15    14    13   12   11   10   9    8    7    6    5    4    3    2    1    0      
  //suaddr 0 ~ 3:  SA15 SA14 SA13 SA12 SA11 SA10 SA9  SA8  SA7  SA6  SA5  SA4  SA3  SA2  SA1  SA0      //0X240 for OWN ADDRESS*, R*, SA4(0)
  //suaddr 0 ~ 3:  SA31 SA30 SA29 SA28 SA27 SA26 SA25 SA24 SA23 SA22 SA21 SA20 SA19 SA18 SA17 SA16     //0X240 for OWN ADDRESS*, R*, SA4(1)


  //TRANSMIT VECTOR WORD (T/R* = 1; 10000)
	//MESSAGE SEQUENCE = TRANSMIT VECTOR WORD----STATUS/VECTOR WORD
		//The ACE transmits a Status Word followed by a Vector Word. If the ENHANCED MODE CODES
		//are enabled (bit 15 in Configuration Register #3 set to logic "1") the contents of the Vector Word
		//are obtained from RAM location 120 (hex). If ENHANCED MODE CODES are not enabled, the
		//single word data block in the shared RAM that is referenced by the lookup table pointer for transmit
		//subaddress 00000 or 11111.
		//ERROR CONDITIONS
			//1. Invalid Command. No response, command ignored.
			//2. Correct Command Followed by Data Word. No Status response. Set Message Error bit
				//(Status Word), High Word Count (BIT Word).
			//3. T/R* bit set to zero, no Data Word. No Status response. Set Message Error bit (Status
				//Word), and Low Word Count (BIT Word).
			//4. T/R* bit set to zero plus one Data Word. The ACE will respond with Status. The Data
				//Word will be stored in RAM location 0110 (or single-word data block for subaddress 0000 or 1111).
			//5. Zero T/R* bit and Broadcast Address, no Data Word. No Status response. Set Message
				//Error and Broadcast Command Received bits (Status Word), and Low Word Count (BIT word).
			//6. Zero T/R* bit and Broadcast Address, plus one Data Word. No Status response. Set
				//Broadcast Command Received bits (Status Word).
			//7. Broadcast Address. No Status response. Set Message Error and Broadcast Command
				//Received bits (Status Word), Command Word Contents Error (BIT word).

//data_buffer_management.bmp
//1553B_message.jpg
//memory map : page 230

//RT PSEUDO CODE EXAMPLE
