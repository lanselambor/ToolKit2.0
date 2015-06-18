//  Author:Frankie.Chu
//  Date:9 April,2012
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//  Modified record:
//
/*******************************************************************************/
#include "MyTM1637.h"
#include <Arduino.h>
static int8_t TubeTabF[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};//0~9,A,b,C,d,E,F        
static int8_t TubeTabB[] = {0x3f,0x30,0x5b,0x79,0x74,0x6d,0x6f,0x38,0x7f,0x7d,0x7e,0x67,0x0f,0x73,0x4f,0x4e};//0~9,A,b,C,d,E,F

MyTM1637::MyTM1637(uint8_t Clk, uint8_t Data)
{
  Clkpin = Clk;
  Datapin = Data;
  pinMode(Clkpin,OUTPUT);
  pinMode(Datapin,OUTPUT);
}

void MyTM1637::init(void)
{
  clearDisplay();
}

int MyTM1637::writeByte(int8_t wr_data)
{
  uint8_t i,count1;   
  for(i=0;i<8;i++)        //sent 8bit data
  {
    digitalWrite(Clkpin,LOW);      
    if(wr_data & 0x01)digitalWrite(Datapin,HIGH);//LSB first
    else digitalWrite(Datapin,LOW);
    wr_data >>= 1;      
    digitalWrite(Clkpin,HIGH);
      
  }  
  digitalWrite(Clkpin,LOW); //wait for the ACK
  digitalWrite(Datapin,HIGH);
  digitalWrite(Clkpin,HIGH);     
  pinMode(Datapin,INPUT);
  
#if 0  
  while(digitalRead(Datapin))    
  { 
    count1 +=1;
    if(count1 == 200)//
    {
     pinMode(Datapin,OUTPUT);
     digitalWrite(Datapin,LOW);
     count1 =0;
    }
    pinMode(Datapin,INPUT);
  }
  pinMode(Datapin,OUTPUT);
#endif

  bitDelay();
  uint8_t ack = digitalRead(Datapin);
  if (ack == 0) 
  {
     pinMode(Datapin,OUTPUT);
     digitalWrite(Datapin,LOW);
  }
  bitDelay();
  pinMode(Datapin,OUTPUT);
  bitDelay();
  
  return ack;
}
//send start signal to TM1637
void MyTM1637::start(void)
{
  digitalWrite(Clkpin,HIGH);//send start signal to TM1637
  digitalWrite(Datapin,HIGH); 
  digitalWrite(Datapin,LOW); 
  digitalWrite(Clkpin,LOW); 
} 
//End of transmission
void MyTM1637::stop(void)
{
  digitalWrite(Clkpin,LOW);
  digitalWrite(Datapin,LOW);
  digitalWrite(Clkpin,HIGH);
  digitalWrite(Datapin,HIGH); 
}
//display function.Write to full-screen.
void MyTM1637::display(int8_t DispData[],uint8_t DispFlag)
{
  int8_t SegData[4];
  uint8_t i;
  if(DispFlag == DISPLAY_FLAG_F)
  {
      for(i = 0;i < 4;i ++)
      {
        SegData[i] = DispData[i];
      }
  }
  if(DispFlag == DISPLAY_FLAG_B)
  {
      for(i = 0;i < 4;i ++)
      {
        SegData[3 - i] = DispData[i];
      }
  }
  coding(SegData,DispFlag);
  start();          //start signal sent to TM1637 from MCU
  writeByte(ADDR_AUTO);//
  stop();           //
  start();          //
  writeByte(Cmd_SetAddr);//
  for(i=0;i < 4;i ++)
  {
    writeByte(SegData[i]);        //
  }
  stop();           //
  start();          //
  writeByte(Cmd_DispCtrl);//
  stop();           //
}
//******************************************
void MyTM1637::display(uint8_t BitAddr,int8_t DispData,uint8_t DispFlag)
{
  int8_t SegData;
  SegData = coding(DispData,DispFlag);
  start();          //start signal sent to TM1637 from MCU
  writeByte(ADDR_FIXED);//
  stop();           //
  start();          //
  if(DispFlag == DISPLAY_FLAG_F)writeByte(BitAddr|0xc0);//
  if(DispFlag == DISPLAY_FLAG_B)writeByte((3 - BitAddr)|0xc0);//
  writeByte(SegData);//
  stop();            //
  start();          //
  writeByte(Cmd_DispCtrl);//
  stop();           //
}

void MyTM1637::clearDisplay(void)
{
  display(0x00,0x7f,0x00);
  display(0x01,0x7f,0x00);
  display(0x02,0x7f,0x00);
  display(0x03,0x7f,0x00);  
}
//To take effect the next time it displays.
void MyTM1637::set(uint8_t brightness,uint8_t SetData,uint8_t SetAddr)
{
  Cmd_SetData = SetData;
  Cmd_SetAddr = SetAddr;
  Cmd_DispCtrl = 0x88 + brightness;//Set the brightness and it takes effect the next time it displays.
}

//Whether to light the clock point ":".
//To take effect the next time it displays.
void MyTM1637::point(boolean PointFlag)
{
  _PointFlag = PointFlag;
}

void MyTM1637::coding(int8_t DispData[],uint8_t DispFlag)
{
  uint8_t PointData;
  if(DispFlag == DISPLAY_FLAG_F)
  {
      if(_PointFlag == POINT_ON)PointData = 0x80;
      else PointData = 0; 
      for(uint8_t i = 0;i < 4;i ++)
      {
        if(DispData[i] == 0x7f)DispData[i] = 0x00;
        else DispData[i] = TubeTabF[DispData[i]] + PointData;
      }
  }
  if(DispFlag == DISPLAY_FLAG_B)
  {
      if(_PointFlag == POINT_ON)PointData = 0x80;
      else PointData = 0; 
      for(uint8_t i = 0;i < 4;i ++)
      {
        if(DispData[i] == 0x7f)DispData[i] = 0x00;
        else DispData[i] = TubeTabB[DispData[i]] + PointData;
      }
  }
}

int8_t MyTM1637::coding(int8_t DispData,uint8_t DispFlag)
{
  uint8_t PointData;
  if(DispFlag == DISPLAY_FLAG_F)
  {
      if(_PointFlag == POINT_ON)PointData = 0x80;
      else PointData = 0; 
      if(DispData == 0x7f) DispData = 0x00 + PointData;//The bit digital tube off
      else DispData = TubeTabF[DispData] + PointData;
  }
  if(DispFlag == DISPLAY_FLAG_B)
  {
      if(_PointFlag == POINT_ON)PointData = 0x80;
      else PointData = 0; 
      if(DispData == 0x7f) DispData = 0x00 + PointData;//The bit digital tube off
      else DispData = TubeTabB[DispData] + PointData;
  }
  return DispData;
}
void MyTM1637::bitDelay(void)
{
	delayMicroseconds(50);
}