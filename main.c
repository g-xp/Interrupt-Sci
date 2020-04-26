/*
 * main.c
 *
 *  Created on: 2019-9-3
 *      Author: Administrator
 */

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "string.h"
interrupt void sciaTxFifoIsr(void);
interrupt void sciaRxFifoIsr(void);
void scia_fifo_init(void);
void scia_xmit(int a);
void scia_msg(char *msg);
void delay_loop(void);
void delay_loop1(void);
void Gpio_selet(void);
char m[9],x;
char *msg;
int number1=1,number2=1;
char *numbertochar(int number);
char str1[10];

void main(void)
{

	InitSysCtrl();
	Gpio_selet();
	InitSciaGpio();
	DINT;
	InitPieCtrl();
	IER=0x0000;
	IFR=0X0000;
	InitPieVectTable();
	EALLOW;
	PieVectTable.SCIRXINTA=&sciaRxFifoIsr;
	PieVectTable.SCITXINTA=&sciaTxFifoIsr;
	EDIS;
	scia_fifo_init();
	PieCtrlRegs.PIECTRL.bit.ENPIE=1;
	PieCtrlRegs.PIEIER9.bit.INTx1=1;
	PieCtrlRegs.PIEIER9.bit.INTx2=1;
	PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT3
	PieCtrlRegs.PIEIER9.bit.INTx4=1;     // PIE Group 9, INT4
	IER=0x100;
	EINT;
	GpioDataRegs.GPCDAT.all=0x00000007;
	while(1)
	{
		delay_loop1();
		GpioDataRegs.GPCTOGGLE.all=0x00000007;
		delay_loop1();
		GpioDataRegs.GPCTOGGLE.all=0x00000007;
		SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;
		if(number1<101&&number2<101)
		{
			number1++;
			number2++;
		}
		else
		{
			number1=1;
			number2=1;
		}

	}
}

interrupt void sciaTxFifoIsr(void)
{
    Uint16 i;
    msg="01";
    scia_msg(msg);
    msg=numbertochar(number1);
    for(i=0;msg[i]!='\0';i++)
    {
    	scia_xmit(msg[i]);
    }
    x='e';
    scia_xmit(x);
    msg="02";
    scia_msg(msg);
    msg=numbertochar(number2);
    for(i=0;msg[i]!='\0';i++)
    {
    	scia_xmit(msg[i]);
    }
    x='e';
    scia_xmit(x);
//    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;//�������FIFO�жϱ�־λ���ڴ˴��������ᵼ�·���FIFO�ж�һֱ���������Ҫ�رղ���
    PieCtrlRegs.PIEACK.bit.ACK9=1;
//    delay_loop();
}

interrupt void sciaRxFifoIsr(void)
{
        Uint16 i;
        for(i=0;i<9;i++)
        {
        	if(i<8)
                m[i]=SciaRegs.SCIRXBUF.all;
        	else
        		m[i]='\n';
        }
        if(m[7]=='1')
        	msg = "operator1 success \n";
        else if(m[7]=='2')
        	msg = "operator2 success \n";
        else
        	msg = "operator3 success \n";
        scia_msg(msg);
//        scia_msg(m);

        SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;
//        ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;
        PieCtrlRegs.PIEACK.bit.ACK9=1;
}

void scia_fifo_init()
{
	//SCIͨ�ſ��ƼĴ���
	SciaRegs.SCICCR.all =0x0007;   //SCI���ݳ��ȿ���λ111��8λ����
								   //����ģʽ����ֹѭ���Լ�ģʽ
								   //��������żУ��
								   //һλֹͣλ
   //SCI���ƼĴ���1
   SciaRegs.SCICTL1.all =0x0003;   // SCI����ʹ�ܣ�SCI����ʹ��
                                   // Disable RX ERR, SLEEP, TXWAKE
//   ScibRegs.SCICTL2.bit.TXINTENA =1;
//   ScibRegs.SCICTL2.bit.RXBKINTENA =1;
   SciaRegs.SCIHBAUD    =0x0001;
   SciaRegs.SCILBAUD    =0x00E7;//���ò�����Ϊ9600

   //SCI����FIFO�Ĵ���SCIFFTX
   SciaRegs.SCIFFTX.bit.SCIFFENA=1;//ʹ��FIFO
   SciaRegs.SCIFFTX.bit.SCIRST=1;//SCI��λ��־λ��1.SCI���պͷ���FIFO���ܼ�������
   SciaRegs.SCIFFTX.bit.TXFFIENA=1;//��ʹ�ܷ���FIFO�ж�
   SciaRegs.SCIFFTX.bit.TXFFIL=0x01;//12��FIFOʹ��
   SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;//����FIFO�ж������־λ��1�����TXFFINTλ
   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=0;//SCI����FIFO��λ��0����λ����FIFOָ��

   //SCI����FIFO�Ĵ���SCIFFRX
   SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;//SCI����FIFO��������־λ��1�����RXFFOVF
   SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;//����FIFO�ж������־λ
   SciaRegs.SCIFFRX.bit.RXFIFORESET=0;//SCI����FIFO��λ��0����λ����FIFOָ��
   SciaRegs.SCIFFRX.bit.RXFFIENA=1;//����FIFO�ж�ʹ��λ��
   SciaRegs.SCIFFRX.bit.RXFFIL=0x08;//12��FIFOʹ��
   //SCI FIFO���ƼĴ���SCIFFCT
   SciaRegs.SCIFFCT.all=0x00;
   // SCI���ƼĴ���1
   SciaRegs.SCICTL1.bit.SWRESET=1;//Relinquish SCI from Reset


   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
   SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}

void scia_xmit(int a)//�����ַ����ʹ���
{
    while (SciaRegs.SCICTL2.bit.TXRDY == 0) {}
    SciaRegs.SCITXBUF=a;

}

void scia_msg(char * msg)//�����ַ�������
{
    Uint16 len;
    Uint16 i;
    len=(strlen(msg));
    for(i=0;i<len;i++)
    {
            scia_xmit(msg[i]);
    }
//   return 0;
}

void delay_loop()
{
    Uint32      i;
	Uint32      j;
	for(i=0;i<32;i++)
    for (j = 0; j < 100000; j++) {}
}

void delay_loop1()
{
    Uint32      i;
	Uint32      j;
	for(i=0;i<32;i++)
    for (j = 0; j < 100000; j++) {}
}

void Gpio_selet()
{
	EALLOW;
	GpioCtrlRegs.GPCMUX1.bit.GPIO64=0;
	GpioCtrlRegs.GPCMUX1.bit.GPIO65=0;
	GpioCtrlRegs.GPCMUX1.bit.GPIO66=0;
	GpioCtrlRegs.GPCDIR.bit.GPIO64=1;
	GpioCtrlRegs.GPCDIR.bit.GPIO65=1;
	GpioCtrlRegs.GPCDIR.bit.GPIO66=1;
	EDIS;
}

char  *numbertochar(int number)
{
	int i = 0,j=0;
	char str[10];
	while (number)
	{
		str[i] = number % 10 + '0';
		number /= 10;
		i++;
	}
	for (i = i - 1; i >= 0; i--)
	{
		str1[j] = str[i];
		j++;
	}
	str1[j] = '\0';
	return str1;
}
