//************************************************
#include <SoftwareSerial.h>
#include<stdlib.h>
#include <string.h>
SoftwareSerial SIM900(2,3);
#include "SIM900.h"
#include "sms.h"
SMSGSM sms;
char smsbuffer[10]; //biến tin nhắn nhận
char n[20]="+841695821150"; //SĐT người gửi - chủ
char pos; //biến vị trí tin nhắn trong sim
char pos1;
char *p; //biến lệnh của người gửi
char *p1;
char *p2;
char command;
String test;
int count=0;
boolean admin=false;
boolean bomm = false;
boolean checkSms=false;
int dem=0;
//********************************************
#define Analog 5
//*****************************************************
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11 
#define Bom 5
#define Van 6
DHT dht(DHTPIN, DHTTYPE);

//Mái che
int value;
#define Analog3 4
#define manChe 8
int doAmMacDinh=50;//độ ẩm mặc định cần phải tưới
//******************************************
int doAmDatCanTuoi=50;
int tat=0;
int bat=0;
void setup() 
{
 Serial.begin(9600);
 dht.begin();
 pinMode(Bom,OUTPUT);
 pinMode(Van,OUTPUT);
 digitalWrite(Bom,HIGH);
 digitalWrite(Van,HIGH);
 pinMode(manChe,OUTPUT);
 Serial.println("GSM Shield testing.");
  if (gsm.begin(2400)) 
      Serial.println("\nstatus=READY");
  else Serial.println("\nstatus=IDLE"); 
}
//****************************************************
char *Reply(int Doamdat,int Doamkhongkhi,int Nhietdo)
{  
  char q[10],w[10],e[10];
  char DO_AM[30]="Do am dat: ";
  char DO_AM_KK[30]="Do am khong khi: ";
  char NHIET_DO[30]="Nhiet do: ";
  char result[160]="\0";
  itoa(Doamdat,q,10);//chuyển đổi doAm về chuổi DO_AM
  itoa(Doamkhongkhi,w,10);
  itoa(Nhietdo,e,10);
  strcat(result,DO_AM); //nối biến DO_AM vào sau biến result->kết quả là: "Độ ẩm: 123" 
  strcat(result,q);
  strcat(result,"%\n");//xuống dòng
  strcat(result,DO_AM_KK);
  strcat(result,w);
  strcat(result,"%\n");
  
  strcat(result,NHIET_DO);
  strcat(result,e);
  strcat(result,"*C");
  return result;
}
//Hàm trả tin nhắn cho người dùng khi không tưới đk
char *Reply1(int doAmMacDinh)
{  
  char q[10];
  char result[160]="\0";
  char content[50]="Do am can tuoi da thay doi thanh ";
  itoa(doAmMacDinh,q,10);
  strcat(result,content);
  strcat(result,q);
  strcat(result,"%");
  return result;
}
char *Reply2()
{  
  char content[160]="Do am dat da du.De tiep tuc tuoi,soan YES";
  return content;
}

/**********************************************************/
 int DO_AM_DAT()
{
 float ana=analogRead(Analog);
 float anal=ana-223;
 float analog=anal/800;
 float doAmDat=100-(analog*100);
 int test=(int)doAmDat;
  return test;
}
 int DO_AM_KHONG_KHI()
{
  int h=dht.readHumidity();
  return h;
}
 int NHIET_DO_KHONG_KHI()
{
  int t=dht.readTemperature();
  return t;
}
/*****************************************************************************VÒNG LẶP TRONG MẠCH*****************************************************************************/
void loop() 
{  
  
  
  
  int DO_AM_DAT1=DO_AM_DAT();
  Serial.println(DO_AM_DAT1);
   /**Phần xử lý tin nhắn**/
  //Đọc tin nhắn
  pos=sms.IsSMSPresent(SMS_UNREAD);
  Serial.println((int)pos);
  Serial.print("POS=");
  Serial.println((int)pos);
  smsbuffer[0]='\0';
  sms.GetSMS((int)pos,n,20,smsbuffer,160);
  p=strstr(smsbuffer,"STATUS");//tìm kiếm chuổi STATUS trong nội dung tin nhắn
    if(p)//nếu mà nội dung tin nhắn là STATUS 
    {
      sms.SendSMS(n, Reply(DO_AM_DAT(),DO_AM_KHONG_KHI(),NHIET_DO_KHONG_KHI()));
   
    }
    else//ngược lại nếu nội dung tin nhắn ko phải là STATUS
    {
        
          p=strstr(smsbuffer,"WATERING");//thì tìm kiếm chuổi WATERING trong nội dung tin nhắn
          p1=strstr(smsbuffer,"STOP");
          p2=strstr(smsbuffer,"YES");
          if(p)//nếu mà nội dung tin nhắn là WATERING 
          {  
              //Trước khi tưới phải kiểm tra xem điều kiện độ ẩm đất có hợp lý hay ko
              if(DO_AM_DAT1<=doAmMacDinh)
              {
               sms.SendSMS(n,"Dang tuoi...");
               admin=true;
               digitalWrite(Bom,LOW);//bơm nước
               digitalWrite(Van,LOW);  
              }
              else
              {  
            
                sms.SendSMS(n,Reply2());
              }
          }
          else if(p1)//Ngược lại nếu nội dung tin nhắn là STOP
          {    sms.SendSMS(n,"Da dung.");
               admin=false;
               digitalWrite(Bom,HIGH);//bơm nước
               digitalWrite(Van,HIGH); 
          }
          else if(p2)//nếu nội dung tin nhắn là YES có nghĩa là người dùng đã xác nhận yêu cầu tưới mặc dù độ ẩm đất đã đủ.
          {
               sms.SendSMS(n,"Dang tuoi...");
               admin=true;
               digitalWrite(Bom,LOW);//bơm nước
               digitalWrite(Van,LOW);  
          }
          else//ngược lại nếu nội dung tin nhắn ko phải là WATERING và không phải là STOP
          {
             
                p=strstr(smsbuffer,"CHANGE");//thì tìm kiếm chuổi CHANGE trong nội dung tin nhắn dùng để thay đổi nhiệt độ
              if(p)//nếu mà nội dung tin nhắn là CHANGE 
              {  
                int Length=sizeof(smsbuffer);//lấy độ dài của chuổi tin nhắn
                char changeNhietDo[163]="\0";//biến để lấy ra giá trị nhiệt độ từ nội dung tin nhắn thay đổi nhiệt độ
                strncpy(changeNhietDo,smsbuffer+7,2);//cắt giá trị nhiệt độ từ trong nội dung chuổi tin nhắn
                doAmDatCanTuoi=atoi(changeNhietDo);//gán doAmDatCanTuoi bằng với giá trị nhiệt độ cần thay đổi trong nội dung tin nhắn
                doAmMacDinh=doAmDatCanTuoi;
                sms.SendSMS(n,Reply1(doAmMacDinh));
              }
         }
          
    }
    
  
    /**Thông báo đến người dùng xem tin nhắn đã gửi thành công hay chưa**/
    
     /**Phần xử lý việc bơm nước**/
    if (admin ==true)
    {
      count+=1;
      if(count==20)
      {
        Reset();
      } 
    }
    else
   { //Phần tưới tự động
    if(DO_AM_DAT1<=doAmMacDinh)//mỗi lần muốn tưới phải kiểm tra xem độ ẩm có lớn hơn 50% chưa(tức là đất phải khô mới tưới)
       {   
           bomm=true;
           digitalWrite(Bom,LOW);
           digitalWrite(Van,LOW); 
       }
   if (bomm ==true)
    {
      count+=1;
      if(count==20)
      {
        Reset1();
      } 
    }
   }
     
  /**Phần xử lý mái che**/
/*  value= analogRead(Analog3);
  if(value<=100)
  {
    bat+=1;
    if (bat==5)
    {
    digitalWrite(manChe,HIGH);
    }
    if(bat>5) 
    {
      bat=0;
    }
  }
  else
  {
      tat+=1;
      if (tat==5)
      {
      digitalWrite(manChe,LOW);
      }
       if(tat>5) 
      {
      tat=0;
      }
  }
  */
   value= analogRead(Analog3);
  if(value<=100)
  {
    digitalWrite(manChe,HIGH);
  }
  else
  {
      digitalWrite(manChe,LOW);
  }
    delay(1000);
}



/**============================================================================CÁC HÀM XỬ LÝ========================================================**/
//Hàm reset
void Reset()
{  
    sms.SendSMS(n,"Da tuoi thanh cong.");
    digitalWrite(Bom,HIGH);
    digitalWrite(Van,HIGH); 
    count=0;
    admin=false;
}

void Reset1()
{  
    digitalWrite(Bom,HIGH);
    digitalWrite(Van,HIGH); 
    count=0;
    bomm=false;
}
