
/*********************************************************************************************
�������� ���� 1602 ��ʾ ���Գ���
��д�ˣ� ���� yuwx.net��
��дʱ�䣺����2011��6��18��
Ӳ��֧�֣�����STC12C5A60S2 12MHz���壨���ɻ�����8051��Ƭ����
�ӿ�˵��������
�޸���־������
      1.ʵ��DB1ֱ�ӿ���Lcd1602,��ʾ��ʱ�䣬�Զ���ͼ��V1.0. 2014/04/22 01:58:00 TIME: 2014.04.21	
      2.���Ӻ���Ч��.P3.2 ����LCM2402��RS������,��Ϊ1.3��P3.2����Ϊ�ⲿ�����ж��ź���  TIME:2014.05.04    
      3. ��Ӵ���ͨѶ�������Ժ�DB1,����ң�أ������룩
	2014.05.05 01:42 pc->DB1 ���ڽ���Ok
/*********************************************************************************************/

#include <DB1.h>
//#include "lcd.h"

#define MAX_STRING_LEN 16
unsigned char talbe1[MAX_STRING_LEN]=" [www.yuwx.net] ";
//unsigned char talbe1[MAX_STRING_LEN]="0123456789";
unsigned char talbe2[MAX_STRING_LEN]="LCD1602 test ok!";

unsigned char code CDIS1[13]={" Red Ctrl"}; //{" Red Control"};
unsigned char code CDIS2[13]={" IR-CODE --H"};

unsigned char IrValue[6] = {0};
unsigned char Time = 0;
unsigned int nPushCounts = 0;//������������
unsigned char uType = 0;//0:��ʾ���� 1:��ʾ����

//void DELAY_MS (unsigned int a);

typedef unsigned char      uint8;          // �޷���8λ���ͱ��� //

/********************************************************************************************
// ���Ŷ��� // ��ʹ���߸���ʵ�ʸ��ģ�
/********************************************************************************************/
#define		LCM2402_DB0_DB7		P2			// ����LCM2402����������

sbit IRIN	      = P3 ^ 2;					//�����ź���
sbit LCM2402_RS   = P1 ^ 3;					// ����LCM2402��RS������
sbit LCM2402_RW   = P3 ^ 3;					// ����LCM2402��RW������
sbit LCM2402_E    = P3 ^ 4;					// ����LCM2402��E������
sbit LCM2402_Busy = P2 ^ 7;					// ����LCM2402�Ĳ�æ�ߣ���LCM2402_DB0_DB7������

sbit LED_IR = P1 ^ 7; //������Ժ���LED

data unsigned char TIME_DD,TIME_MO,TIME_YY,TIME_WW,TIME_HH,TIME_MM,TIME_SS;//�����ա��¡��ꡢ�ܡ�ʱ���֡�����¶ȴ����
data bit DAY_BIT = 0;//�������ӱ�־λ���������ڽ�λ��������
data unsigned char DIS_BIT = 0; //������Ϣ���л���ʾ
data unsigned char cou  = 0;     // �������,��10msʱ���ź��ۼӵ�1s 
/********************************************************************************************
// ����LCM2402ָ� // ����ϸ��������ֲᣩ
/********************************************************************************************/
#define			CMD_clear		0x01             // �����Ļ
#define			CMD_back		0x02             // DDRAM����λ
#define			CMD_dec1		0x04             // �����AC��ָ�룩��1������д
#define			CMD_add1		0x06             // �����AC��ָ�룩��1������д
#define			CMD_dis_gb1		0x0f             // ����ʾ_�����_�������˸
#define			CMD_dis_gb2		0x0e             // ����ʾ_�����_�ع����˸
#define			CMD_dis_gb3		0x0c             // ����ʾ_�ع��_�ع����˸
#define			CMD_OFF_dis		0x08             // ����ʾ_�ع��_�ع����˸
#define			CMD_set82		0x38             // 8λ����_2����ʾ
#define			CMD_set81		0x30             // 8λ����_1����ʾ���ϱ��У�
#define			CMD_set42		0x28             // 4λ����_2����ʾ
#define			CMD_set41		0x20             // 4λ����_1����ʾ���ϱ��У�
#define			lin_1			0x80             // 4λ����_1����ʾ���ϱ��У�
#define			lin_2			0xc0             // 4λ����_1����ʾ���ϱ��У�

/*4. ��ʾ�������*/
void DisplayIRCode();
void DelayMs(unsigned int x);   //0.14ms��� 0us
void IrInit();//��ʼ������

//��ʾʮ���Ƽ���
void DisplayTenCode(int nNumber);
//��ʾ16���Ƽ���
void DisplayHexCode(int nNumber);
//���ô���
void UsartConfiguration();

//����������
unsigned char ReadSerial();
//��ʾ��������
void DisplaySerialCode(unsigned char uCode);

//�򴮿ڷ���һ���ַ� 
void send_char_com(unsigned char ch);
//�򴮿ڷ���һ���ַ���,���Ȳ��ޡ�//Ӧ�ã�send_string_com("d9887321$"); 
void send_string_com(unsigned char *str);

void TestSerial();
//���Ժ��ⰴ��������
void IrKeyTest();

/********************************************************************************************
// ��LCMæ���� [�ײ�Э��] // �����еײ�Э�鶼�����ע��
// LCM2402��æ����LCM2402����æ״̬�����������ȴ�����æ״̬ //
/********************************************************************************************/
void LCM2402_TestBusy(void){
   	LCM2402_DB0_DB7 = 0xff;		//�豸��״̬
   	LCM2402_RS = 0;
   	LCM2402_RW = 1;
   	LCM2402_E = 1;
   	while(LCM2402_Busy);		//�ȴ�LCM��æ
   	LCM2402_E = 0;				//
}
/********************************************************************************************
// дָ����� //
// ��LCM2402д���� ��������Ҫ1��ָ�����ڲ��� //
/********************************************************************************************/
void LCM2402_WriteCMD(uint8 LCM2402_command) { 
  	LCM2402_TestBusy();
  	LCM2402_DB0_DB7 = LCM2402_command;
  	LCM2402_RS = 0;
  	LCM2402_RW = 0;
  	LCM2402_E = 1;
  	LCM2402_E = 0;
}
/********************************************************************************************
// д���ݳ��� //
// ��LCM2402д���� //
/********************************************************************************************/
void LCM2402_WriteData(uint8 LCM2402_data){ 
    LCM2402_TestBusy();
	LCM2402_DB0_DB7 = LCM2402_data;
  	LCM2402_RS = 1;
  	LCM2402_RW = 0;
  	LCM2402_E = 1;
  	LCM2402_E = 0;
}
/********************************************************************************************
// ��ӡ�ַ������� // ������������ָ�뺯����
// ��LCM����һ���ַ���,����48�ַ�֮�� 
// ��һ��λ�� 0x00~0x17  �ڶ���λ�� 0x40~0x57 
// Ӧ�þ�����print(0x80,"doyoung.net"); //�ڵ�һ�е�һλ���������Ҵ�ӡdoyoung.net�ַ���
/********************************************************************************************/
void print(uint8 a,uint8 *str){
	LCM2402_WriteCMD(a | 0x80);
	while(*str != '\0'){
		LCM2402_WriteData(*str++);
	}
	*str = 0;
}
/********************************************************************************************
// ��ӡ���ַ����� // 
// ��һ��λ�� 0x00~0x17  �ڶ���λ�� 0x40~0x57 
// ��LCM����һ���ַ�,��ʮ�����ƣ�0x00����ʾ 
// Ӧ�þ�����print(0xc0,0x30); //�ڵڶ��е�һλ����ӡ�ַ���0��
/********************************************************************************************/
void print2(uint8 a,uint8 t){
		LCM2402_WriteCMD(a | 0x80);
		LCM2402_WriteData(t);
}
/********************************************************************************************
// ����С���� //
// ��д��8���������ַ���д��������CGRAM����ֱ����ȡ��ʾ��
// �ַ����巽����ο������ֲ� 
/********************************************************************************************/
uint8 code Xword[]={
    0x18,0x18,0x07,0x08,0x08,0x08,0x07,0x00,        //�棬���� 0x00
    0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,        //һ������ 0x01
    0x00,0x00,0x00,0x0e,0x00,0xff,0x00,0x00,        //�������� 0x02
    0x00,0x00,0xff,0x00,0x0e,0x00,0xff,0x00,        //�������� 0x03
    0x00,0x00,0xff,0xf5,0xfb,0xf1,0xff,0x00,        //�ģ����� 0x04
    0x00,0xfe,0x08,0xfe,0x0a,0x0a,0xff,0x00,        //�壬���� 0x05
    0x00,0x04,0x00,0xff,0x00,0x0a,0x11,0x00,        //�������� 0x06
    0x00,0x1f,0x11,0x1f,0x11,0x11,0x1f,0x00,        //�գ����� 0x07
};

uint8 code HeartWord[]={
	0x03,0x07,0x0f,0x1f,0x1f,0x1f,0x1f,0x1f,
    0x18,0x1E,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
    0x07,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
    0x10,0x18,0x1c,0x1E,0x1E,0x1E,0x1E,0x1E,
    0x0f,0x07,0x03,0x01,0x00,0x00,0x00,0x00,
	0x1f,0x1f,0x1f,0x1f,0x1f,0x0f,0x07,0x01,
	0x1f,0x1f,0x1f,0x1f,0x1f,0x1c,0x18,0x00,                     
	0x1c,0x18,0x10,0x00,0x00,0x00,0x00,0x00};//��ͼ��
	
	
/*
void CgramWrite(void) 
{	// װ��CGRAM //
    uint8 i;
	LCM2402_WriteCMD(0x06);			// CGRAM��ַ�Զ���1
	LCM2402_WriteCMD(0x40);			// CGRAM��ַ��Ϊ00��
    for(i=0;i<64;i++) {
		LCM2402_WriteData(Xword[i]);// ������д������
    }
}
*/
	
//void CgramWrite(void) 
void CgramWrite_byType(uint8 type)
{	// װ��CGRAM //
    uint8 i;
	LCM2402_WriteCMD(0x06);			// CGRAM��ַ�Զ���1
	LCM2402_WriteCMD(0x40);			// CGRAM��ַ��Ϊ00��
    for(i=0;i<64;i++) {
		if(1 == type)
		{
			LCM2402_WriteData(Xword[i]);// ������д������
		}
    	else if(2 == type)
		{
			LCM2402_WriteData(HeartWord[i]);// ������д������
		}
		else{
			//
		}
    }
}
/********************************************************************************************
// LCM2402��ʼ�� //��ʹ���߿��Զ��壬�� * �ų����б��뱣�������޸ģ�
/********************************************************************************************/
void LCM2402_Init(void){
  	LCM2402_WriteCMD(CMD_set82);	//* ��ʾģʽ���ã���ʾ2�У�ÿ���ַ�Ϊ5*7������
  	LCM2402_WriteCMD(CMD_clear);	//  ��ʾ����
  	LCM2402_WriteCMD(CMD_back);		//* ����ָ��ָ���1�е�1���ַ�λ��
  	LCM2402_WriteCMD(CMD_add1);		//  ��ʾ����ƶ����ã����ֲ������������
  	LCM2402_WriteCMD(CMD_dis_gb3); 	//  ��ʾ����������ã���ʾ������꿪����˸��
	//CgramWrite();					//  ��CGRAMд���Զ����ַ�
	//CgramWrite_byType(2);
}
/********************************************************************************************/
//			������LCM2402��������			//
/*********************************************************************************************/

/*********************************************************************************************/
bit IsLeapYear(void){    //���㱾���Ƿ������� 
	unsigned int a;
	a = 2000+TIME_YY;//��2000��ʾ����������
	if((a%4==0 && a%100!=0)||(a%400==0)){ //����ļ��㹫ʽ
			return 1;//�����귵��1 
		}else{ 
 			return 0;//�������귵��0 
		}
} 
/**********************************************************************************************/	
void month_day(void){  
	unsigned char mon_day[]={31,28,31,30,31,30,31,31,30,31,30,31};
	TIME_DD++;//���1
	TIME_WW++;//����ֵ��1
	if(TIME_WW > 7){
		TIME_WW = 1;//ʱ��ֵ�޶�
	}
	if(TIME_DD > mon_day[TIME_MO-1]){//������Ƿ���ڵ��µ����ֵ
		if(IsLeapYear()&&TIME_MO==2){ //���㱾���Ƿ��������2�·� 
			TIME_DD = 29;//�������������2�£�����Ϊ29
		}else{
			TIME_DD = 1; //����͵���1
			TIME_MO++;//�¼�1
			if(TIME_MO > 12){
				TIME_MO = 1; //����´���12���µ���1
				TIME_YY++;//���1 �����������޻��ӣ�
			}
		}
	}
} 
/**********************************************************************************************/	
void init (void){ //�ϵ��ʼ��
	TMOD = 0x11;         // ��ʱ/������0,1�����ڷ�ʽ1   
    TH0 = 0x3c;          // Ԥ�ò���50msʱ���ź�   
    TL0 = 0xb0;   
    EA = 1;              // �����ж�   
    ET0 = 1;             // ��ʱ/������0�����ж�   
    TR0 = 1;             // ���ն�ʱ/������0   
////
	TIME_DD = 4; //ʱ�����״�ʹ�õ�ֵ��֮�����EEPROM�Զ���¼��һ���ֵ
	TIME_MO	= 5; //��ʼʱ�䣺2009��5��18����һ��20ʱ13��40��
	TIME_YY = 14;
	TIME_WW = 7;
	TIME_HH	= 20;
	TIME_MM = 30;
	TIME_SS = 0;
}
/********************************************************************************************
//��ʾ��Ŀ ʱ�䲿�� �ڵ�һ��ȫ����ʾʱ��
*********************************************************************************************/    
void RealTime_Display(void){
	    print(0x80,"20");
	    print2(0x82,TIME_YY/10+0x30);
	    print2(0x83,TIME_YY%10+0x30);
		print(0x84,"/");            // ��ʾ��
		//
	    print2(0x85,TIME_MO/10+0x30);
	    print2(0x86,TIME_MO%10+0x30);
		print(0x87,"/");            // ��ʾ��
		//
	    print2(0x88,TIME_DD/10+0x30);
	    print2(0x89,TIME_DD%10+0x30);

		print(0x8b,"[");            // ��ʾ[
		print2(0x8c,TIME_WW%10); //����
		print(0x8d,"]");            // ��ʾ]

	    print2(0x40,TIME_HH/10+0x30);//Сʱ
	    print2(0x41,TIME_HH%10+0x30);
		print(0x42,":");            // ��ʾcgram��һ����ģ"��"
		//
	    print2(0x43,TIME_MM/10+0x30);//����
	    print2(0x44,TIME_MM%10+0x30);
		print(0x45,".");            // ��ʾcgram��һ����ģ"."
		//
	    print2(0x46,TIME_SS/10+0x30);//��
	    print2(0x47,TIME_SS%10+0x30);
		//
}

void tiem0(void) interrupt 1{   // T/C0�жϷ������(����50msʱ���ź�)   
    cou++;                      // ���������1   
    if(cou > 19){                 // ����ֵ��100(1s)   
        cou = 0;               // �����������   
        TIME_SS++;                 // ���������1(��λ10ms*100=1s)   
        if(TIME_SS > 59){          // �����ֵ��60   
           
            TIME_SS = 0;           // �����������   
            TIME_MM++;             // �ּ�������1(��λ60s=1m)  
            if(TIME_MM > 59){      // �ּ�����60   
                TIME_MM = 0;       // �ּ���������   
                TIME_HH++;         // ʱ��������1(��λ60m=1h)   
                if(TIME_HH > 23){  // ʱ������23   
                    TIME_HH = 0;   // ʱ����������
					DAY_BIT = 1;	//�����ӱ�־λ 
				}  
            }   
        }   
    }   
    TH0 = 0x3c;                // ���ö�ʱ����   
    TL0 = 0xb0;   
}
/*********************************************************************************************/


/********************************************************************************************/
/*
// ��ʾʱ�Ӻ��� //
void main (void)
{
	init();//��ʼ��                           
	LCM2402_Init();//LCM2402��ʼ��                           
	while(1)
	{		
		RealTime_Display();    	
//		print(0x80,"DoYoung Studio"); //�ڵ�һ�е�һλ���������Ҵ�ӡdoyoung.net�ַ���
//		print(0x40,"www.DoYoung.net"); //�ڵ�һ�е�һλ���������Ҵ�ӡdoyoung.net�ַ���

		if(DAY_BIT == 1)
		{ 
			//��������Ƿ���£�������㹫��
			month_day();//���㹫������	
			DAY_BIT = 0;//������ɺ����ڱ����־λ��0
		}
	}
}
*/
/********************************************************************************************/    

/*********************************************************************************************
�����������뼶CPU��ʱ����
��  �ã�DELAY_MS (?);
��  ����1~65535����������Ϊ0��
����ֵ����
��  ����ռ��CPU��ʽ��ʱ�������ֵ��ͬ�ĺ���ʱ��
��  ע��Ӧ����1T��Ƭ��ʱi<600��Ӧ����12T��Ƭ��ʱi<125
/*********************************************************************************************/
void Delay1ms (unsigned int a) //��� 0us
{
	unsigned int i;
	while( a-- != 0)
	{
		for(i = 0; i < 600; i++);
	}
}

void DisplayLove();

/*******************************************************************************
��������������
��  �ã���
��  ������
����ֵ����
��  ��������ʼ��������ѭ��
��  ע��
*******************************************************************************/
void main()
{	
	uint8 i = 0;
	unsigned char uCodeSerial = 0;
	unsigned int nPushCountsPre = 0;//��¼֮ǰ�ļ��������ڲ�����ʱ������
	
	init();//��ʼ��                           
	LCM2402_Init();//LCM2402��ʼ��	
	IrInit();//��ʼ������
	
/* 1.��ʾtest */
	print(0x80, talbe1);
	print(0x40, talbe2);	
	Delay1ms(1000);	
	LCM2402_WriteCMD(CMD_clear);//����
	
/*2. ��ʾLove! */
	DisplayLove();
	
	Delay1ms(1000);	
	LCM2402_WriteCMD(CMD_clear);	//����

/*. ��ʾ�������ǰ���ַ���*/
	//CDIS1
	print(0x80, CDIS1);
	print(0x40, CDIS2);	
	Delay1ms(1000);		
	//LCM2402_WriteCMD(CMD_clear);//����
	
	//д���Զ���ͼ��
	CgramWrite_byType(1);
	
	//��������
	UsartConfiguration();
	
	while(1)
	{		
		//print(0x80,"DoYoung Studio"); //�ڵ�һ�е�һλ���������Ҵ�ӡdoyoung.net�ַ���
		//print(0x40,"www.DoYoung.net"); //�ڵ�һ�е�һλ���������Ҵ�ӡdoyoung.net�ַ���		
		//print2(0x4a, TIME_SS % 7);		

		/*3. ��ʾʱ��Realtime */
		/*
		RealTime_Display();
		
		if(DAY_BIT == 1)
		{ 
			//��������Ƿ���£�������㹫��
			month_day();//���㹫������	
			DAY_BIT = 0;//������ɺ����ڱ����־λ��0
		}
		*/
		
		/*4. ��ʾ�������*/
		DisplayIRCode();
		
		//����������
		uCodeSerial = ReadSerial();
		
		if(0 == uType)
		{
			DisplayHexCode((int)nPushCounts);
			DisplayTenCode((int)nPushCounts);
			
			//send_string_com("�¶�:");	//��ʾ����
			//send_char_com(i/10+0x30);	//��ʾ�¶� ʮλ
			//send_char_com(i%10+0x30);	//��ʾ�¶� ��λ
			//send_char_com(0x54);
			//send_string_com("��");	//��C
			
			if(nPushCountsPre != nPushCounts)
			{
				//���Ժ��ⰴ��������
				IrKeyTest();
			}			
			
			//Delay1ms(500);
		}
		else if(1 == uType)
		{
			/*5. ��ʾ�������� */
			DisplaySerialCode(uCodeSerial);
			//Delay1ms(1000);
		}
		else{
			//
		}
		
		nPushCountsPre = nPushCounts;
		
		Delay1ms(10);
	}
}

void DisplayLove()
{
	uint8 i = 0;
	
	CgramWrite_byType(2);
	
	print(0x81, "I ");
		
	for(i=0; i<8; i++)
	{
		if(i<4)
		{
			print2(0x83+i, i);
		}
		else
		{
			print2(0x43+(i-4), i);
		}			
	}
	
	print(0x47, "Luo Ying!");
}

/*4. ��ʾ�������*/
void DisplayIRCode()
{
	unsigned char uAdrr = 0x00;
	unsigned char uValue = 0x00;
	IrValue[4]=IrValue[2]>>4;	 	 	//��λ
	IrValue[5]=IrValue[2]&0x0f;		//��λ	

	if(IrValue[4]>9)
	{
		//LcdWriteCom(0xc0+0x09);			//������ʾλ��
		//LcdWriteData(0x37+IrValue[4]);	//����ֵת��Ϊ����ʾ��ASCII��
		uValue = 0x37+IrValue[4];
	}
	else
	{
		//LcdWriteCom(0xc0+0x09);
		//LcdWriteData(IrValue[4]+0x30);	//����ֵת��Ϊ����ʾ��ASCII��
		uValue = 0x30+IrValue[4];
	}
	uAdrr = 0xc0 + 0x09;
	print2(uAdrr, uValue);
	
	if(IrValue[5]>9)
	{
		//LcdWriteCom(0xc0+0x0a);
		//LcdWriteData(IrValue[5]+0x37);		//����ֵת��Ϊ����ʾ��ASCII��
		uValue = 0x37+IrValue[5];
	}
	else
	{
		//LcdWriteCom(0xc0+0x0a);
		//LcdWriteData(IrValue[5]+0x30);		//����ֵת��Ϊ����ʾ��ASCII��
		uValue = 0x30+IrValue[5];
	}
	uAdrr = 0xc0 + 0x0a;
	print2(uAdrr, uValue);	
}

//��ʾʮ���Ƽ���
void DisplayTenCode(int nNumber)
{
	unsigned char uBai = 0x00;//��λ
	unsigned char uGe = 0x00;//��λ
	unsigned char uShi = 0x00;//ʮλ
	unsigned char uAdrr = 0x00;
	unsigned char uValue = 0x00;
	
	//�����ʾ������������,ʮ������ʾ
	uBai = nNumber/100;
	uShi = (nNumber%100)/10;
	uGe = nNumber % 10;
	
	if(uBai > 9)
	{
		uBai = 0;//����999������0��ʼ����
	}
	uAdrr = 0xc0 + 0x0d;
	uValue = 0x30 + uBai;
	print2(uAdrr, uValue);
	
	uAdrr = 0xc0 + 0x0e;
	uValue = 0x30 + uShi;
	print2(uAdrr, uValue);	
	
	uAdrr = 0xc0 + 0x0f;
	uValue = 0x30 + uGe;
	print2(uAdrr, uValue);
}

//��ʾ16���Ƽ���
void DisplayHexCode(int nNumber)
{
	unsigned char uHex01 = 0x00;//ʮ�����Ƶ�λ
	unsigned char uHex02 = 0x00;//ʮ�����Ƹ�λ
	unsigned char uAdrr = 0x00;
	unsigned char uValue = 0x00;
	
	//ʮ��������ʾ
	uAdrr = 0x80 + 0x0e;//��һ�е�14���ַ�
	uHex01 = nNumber % 16;
	uHex02 = nNumber / 16 ;
	if(uHex02 > 9)//��д��λ
	{
		uValue = 0x37 + uHex02;
	}
	else
	{
		uValue = 0x30 + uHex02;
	}	
	print2(uAdrr, uValue);
	
	uAdrr = 0x80 + 0x0f;//��һ�е�15���ַ�
	if(uHex01 > 9)
	{
		uValue = 0x37 + uHex01;
	}
	else
	{
		uValue = 0x30 + uHex01;
	}	
	print2(uAdrr, uValue);
}

/*******************************************************************************
* ������         : DelayMs()
* ��������		   : ��ʱ
* ����           : x
* ���         	 : ��
*******************************************************************************/
void DelayMs(unsigned int x)   //0.14ms��� 0us
{
	unsigned char i;
	while(x--)
	{
		//for (i = 0; i<13; i++){}
		for(i=0; i<83; i++){}//600*0.14 = 84
	}
}

/*******************************************************************************
* ������         : IrInit()
* ��������		   : ��ʼ�������߽���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void IrInit()
{
	int n = 4;
	
	IT0=1;//�½��ش���
	EX0=1;//���ж�0����
	EA=1;	//�����ж�

	IRIN=1;//��ʼ���˿�
	
	//����LED�����Ƿ�����	
	while(n > 0)
	{
		LED_IR = 1;
		Delay1ms(300);
		LED_IR = 0;
		Delay1ms(300);
		
		--n;
	}	
}

/*******************************************************************************
* ������         : ReadIr()
* ��������		   : ��ȡ������ֵ���жϺ���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void ReadIr() interrupt 0
{
	unsigned char j,k;
	unsigned int err;
	Time=0;					 
	DelayMs(70);

	if(IRIN==0)		//ȷ���Ƿ���Ľ��յ���ȷ���ź�
	{		
		err=1000;				//1000*10us=10ms,����˵�����յ�������ź�
		/*������������Ϊ����ѭ���������һ������Ϊ�ٵ�ʱ������ѭ������ó�������ʱ
		�������������*/	
		while((IRIN==0)&&(err>0))	//�ȴ�ǰ��9ms�ĵ͵�ƽ��ȥ  		
		{
			DelayMs(1);
			err--;
		}		
		
		if(IRIN==1)			//�����ȷ�ȵ�9ms�͵�ƽ
		{
			err=500;
			while((IRIN==1)&&(err>0))		 //�ȴ�4.5ms����ʼ�ߵ�ƽ��ȥ
			{
				DelayMs(1);
				err--;
			}
			for(k=0;k<4;k++)		//����4������
			{				
				for(j=0;j<8;j++)	//����һ������
				{
					err=60;		
					while((IRIN==0)&&(err>0))//�ȴ��ź�ǰ���560us�͵�ƽ��ȥ
					{
						DelayMs(1);
						err--;
					}
					err=500;
					while((IRIN==1)&&(err>0))	 //����ߵ�ƽ��ʱ�䳤�ȡ�
					{
						DelayMs(1);//0.14ms
						Time++;
						err--;
						if(Time>30)
						{
							EX0=1;
							return;
						}
					}
					IrValue[k]>>=1;	 //k��ʾ�ڼ�������
					if(Time>=8)			//����ߵ�ƽ���ִ���565us����ô��1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//����ʱ��Ҫ���¸�ֵ							
				}
			}
			
			//���Ժ���LED����״̬
			LED_IR = ~LED_IR;
			nPushCounts++;
			uType = 0;//��������ʾ
		}
		if(IrValue[2]!=~IrValue[3])
		{
			return;
		}
	}			
}

/*******************************************************************************
* �� �� ��         :UsartConfiguration()
* ��������		   :���ô���
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void UsartConfiguration()
{
	/* //PZ init
	SCON=0X50;			//����Ϊ������ʽ1
	TMOD=0X20;			//���ü�����������ʽ2
	PCON=0X80;			//�����ʼӱ�
	TH1=0XF3;		    //��������ʼֵ���ã�ע�Ⲩ������4800��
	TL1=0XF3;
	TR1=1;			  //�򿪼�����
*/
	//Delay1ms(200); //
	//P1M0 = 0x01;//ǿ��?
	
	TMOD = 0x20;	//��ʱ��T/C1������ʽ2
	SCON = 0x50;	//���ڹ�����ʽ1�������ڽ��գ�SCON = 0x40 ʱ��ֹ���ڽ��գ�
	TH1 = 0xF3;	//��ʱ����ֵ��8λ����
	TL1 = 0xF3;	//��ʱ����ֵ��8λ����
	PCON = 0x80;	//�����ʱ�Ƶ�����α��䲨����Ϊ2400��
	TR1 = 1;	//��ʱ������
}

//����������
unsigned char ReadSerial()
{
	unsigned char receiveData = 0x00;
	if(RI == 1)				//�鿴�Ƿ���յ�����
	{
		receiveData = SBUF;	//��ȡ����
		RI = 0;				//�����־λ
		
		uType = 1;//��������ʾ		
		//���յ���������ָʾ��
		LED_IR = ~LED_IR;
		
		Delay1ms(20);
	}
	
	return receiveData;
}

//��ʾ��������
void DisplaySerialCode(unsigned char uCode)
{
	if(uCode != 0)
	{
		//LcdWriteCom(0xC0);
		//--��Ϊһ�ν���ֻ�ܽ��յ�8λ���ݣ����Ϊ255������ֻ����ʾ��λ֮��--//
		//LcdWriteData('0' + (receiveData / 100));      // ��λ
		//LcdWriteData('0' + (receiveData % 100 / 10)); // ʮλ
		//LcdWriteData('0' + (receiveData % 10));		  // ��λ
		DisplayHexCode((int)uCode);
		DisplayTenCode((int)uCode);
	}
}

//�򴮿ڷ���һ���ַ� 
void send_char_com(unsigned char ch)
{
	SBUF=ch;
	//while(TI == 0);
	//TI=0;
}

//�򴮿ڷ���һ���ַ���,���Ȳ��ޡ�//Ӧ�ã�send_string_com("d9887321$"); 
void send_string_com(unsigned char *str)
{
	while (*str != '\0')
	{
		send_char_com(*str);
		*str++; 
	}
	*str = 0;
}

/* void TestSerial()
{
	send_char_com(0xDD);
	send_char_com(0xDD);	
	send_char_com(0xDD);
	send_char_com(0xDD);	
	
	for(i=1; i<5; i++)
	{
		send_char_com(IrValue[i]);
	}
	
	send_char_com(0xFF);
	send_char_com(0xFF);
	send_char_com(0xFF);
	send_char_com(0xFF);
} */

//���Ժ��ⰴ��������
void IrKeyTest()
{
	//unsigned char uCode = (IrValue[4] << 4) + (IrValue[5] & 0x0f);
	/*
	unsigned char i = 0;
	for(i=0; i<128; i++)
	{
		send_char_com(i);
	}	
	*/
	
	unsigned char uCode = IrValue[2];
	
	switch(uCode)
	{
		case 0x05:
		{
			send_string_com("�¶�:");	//��ʾ����
			break;
		}
		default:
		{
			//send_char_com(uCode);
			//send_string_com("FFFF");
			send_char_com(uCode);
			//send_string_com("DDDD");
			break;
		}			
	}
	
	//Delay1ms(2000);
}