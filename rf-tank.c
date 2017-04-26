#include <mega128.h>
#include <delay.h>                                     
// M14_5_22_BLUETOOTH_LED....newtc 128모듈과 블루투스 모듈간의 통신

unsigned char rx = 8, shoot1, shoot2, a = 0;//rx=0이면 초기상태에서 PA0의LED가 꺼지지않음
unsigned int tim0_cnt, mot_pos, change;
unsigned int rc_time[3] = {54, 150, 210};   //0도, 90도 180도
unsigned char count; 

void shutdown()
{    
    while(1)
    {   
        PORTC = 0x00;
        PORTG = 0x00;
        delay_ms(1000);
        PORTG = 0x01;
        delay_ms(250);
    }
}

int swap(int a)
{
    if(a == 0)
    {
        a = 1;
        return(a);
    }
    
    else if(a == 1)
    {
        a = 0;
        return(a);
    }
}

void main()
{
DDRA = 0xff; //모터구동
DDRB = 0xff; // 수신인터럽트확인...
DDRC = 0xff;
DDRE = 0xEf;
DDRF = 0xff;
DDRG = 0xff;
DDRD = 0x00;   

UCSR0A = 0x00;
UCSR0B = 0b10010000; // 수신 인터럽트 허용, 수신 기능 인에이블
UCSR0C = 0b00000110; // 전송 데이터를 8비트로
UBRR0H = 0;
UBRR0L = 103;  // BAUD RATE 9600 16.MHz
SREG |= 0x80; // LED off

//이론치 (256-251) x 8분주 x 1/16us = 7us, 실측치 10us
    TIMSK = 0x01;
    TCCR0 = 0x02;                                                  
    TCNT0 = 238;
 
    
    EIMSK = 0b00000011;
    EICRA = 0b00001010;
    
    tim0_cnt = 0;
    PORTF = 0xff;
    PORTC = 0x00;
    
    while(1)
    {  
        PORTG = 0x01;
        if(a)
        {
            if(tim0_cnt >= 2000)
            {
                tim0_cnt = 0;
                PORTF = 0xff;
                change++;
                if(change == 100)
                {
                    mot_pos = (mot_pos + 1) % 3;
                    change = 0;
                }                
            }
            if(tim0_cnt >= rc_time[mot_pos]) PORTF = 0x00;
        }   
    }
}

interrupt [USART0_RXC] void RX_interrupt(void)
{
rx = UDR0; // 수신된 데이터가 저장된 UDR0 송수신 데이터 레지스터를 rx에 대입
PORTB = ~PORTB; // ISR로 들어오는것 LED로 확인
if(rx == 0X01)                                  //전진버튼
    {   
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        delay_us(200);
        PORTC.4 = 1;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 1;                   
        rx = 8;
    }   
    if(rx == 0X02)                             //후진버튼
    {
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        delay_us(200);
        PORTC.4 = 0;
        PORTC.5 = 1;
        PORTC.6 = 1;
        PORTC.7 = 0;                    
        rx = 8;                                             
    }   
    if(rx == 0x03)                             //우회전버튼
    { 
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        delay_us(100);
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 1;                      
        rx = 8;
    }              
    if(rx == 0x04)                             //좌회전버튼
    {
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        delay_us(100);
        PORTC.4 = 1;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        rx = 8;
    } 
    if(rx == 0x05)                             //정지버튼
    {
        PORTC.4 = 0;
        PORTC.5 = 0;
        PORTC.6 = 0;
        PORTC.7 = 0;
        rx = 8;
    }
    if(rx == 0x06)                            //발사버튼        
    {   
        PORTE.1 = 1;
        for(shoot1 = 1; shoot1 < 2;shoot1++)
        {
            for(shoot2 = 1; shoot2 < 6; shoot2++)
            {
                PORTF = 0x20;
                delay_us(13);
                PORTF = 0x00;
                delay_us(13);
            }
            delay_ms(8);
        }
        PORTE.1 = 0;
         rx = 8; 
    }
    if(rx == 0x07)      //서브모터버튼
    {
        a = swap(a);
        rx = 8;                   
    }    
}

interrupt [TIM0_OVF] void timer_int0(void)
{
    tim0_cnt++;
    TCNT0 = 238;
}

interrupt [EXT_INT0] void external_int0(void)
{   
    PORTB = ~PORTB;                                                                                                             
    count++;
    PORTC = count;
    PORTG = 0x00;
    delay_ms(100);
    PORTG = 0x01;
                                                           
    if(count == 5)
    {   
        PORTC = count;
        shutdown();
    }
    EIFR = 0x01;                  
}

interrupt [EXT_INT1] void external_int1(void)
{  
    shutdown();
}