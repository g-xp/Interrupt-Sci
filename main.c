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
//    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;//清楚发送FIFO中断标志位。在此处如果清零会导致发送FIFO中断一直产生，因此要关闭才行
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
	//SCI通信控制寄存器
	SciaRegs.SCICCR.all =0x0007;   //SCI数据长度控制位111，8位数据
								   //空闲模式，禁止循环自检模式
								   //不允许奇偶校验
								   //一位停止位
   //SCI控制寄存器1
   SciaRegs.SCICTL1.all =0x0003;   // SCI发送使能，SCI接收使能
                                   // Disable RX ERR, SLEEP, TXWAKE
//   ScibRegs.SCICTL2.bit.TXINTENA =1;
//   ScibRegs.SCICTL2.bit.RXBKINTENA =1;
   SciaRegs.SCIHBAUD    =0x0001;
   SciaRegs.SCILBAUD    =0x00E7;//设置波特率为9600

   //SCI发送FIFO寄存器SCIFFTX
   SciaRegs.SCIFFTX.bit.SCIFFENA=1;//使能FIFO
   SciaRegs.SCIFFTX.bit.SCIRST=1;//SCI复位标志位，1.SCI接收和发送FIFO功能继续工作
   SciaRegs.SCIFFTX.bit.TXFFIENA=1;//不使能发送FIFO中断
   SciaRegs.SCIFFTX.bit.TXFFIL=0x01;//12级FIFO使能
   SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;//发送FIFO中断清除标志位，1，清除TXFFINT位
   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=0;//SCI发送FIFO复位，0，复位发送FIFO指针

   //SCI接收FIFO寄存器SCIFFRX
   SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;//SCI接收FIFO溢出清除标志位，1，清除RXFFOVF
   SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;//接收FIFO中断清除标志位
   SciaRegs.SCIFFRX.bit.RXFIFORESET=0;//SCI接收FIFO复位，0，复位接收FIFO指针
   SciaRegs.SCIFFRX.bit.RXFFIENA=1;//接收FIFO中断使能位，
   SciaRegs.SCIFFRX.bit.RXFFIL=0x08;//12级FIFO使能
   //SCI FIFO控制寄存器SCIFFCT
   SciaRegs.SCIFFCT.all=0x00;
   // SCI控制寄存器1
   SciaRegs.SCICTL1.bit.SWRESET=1;//Relinquish SCI from Reset


   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
   SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}

void scia_xmit(int a)//单个字符发送代码
{
    while (SciaRegs.SCICTL2.bit.TXRDY == 0) {}
    SciaRegs.SCITXBUF=a;

}

void scia_msg(char * msg)//发送字符串代码
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
