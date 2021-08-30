#include <Arduino.h>
#include <SoftwareSerial.h>      // 採用SoftwareSerial程式庫
#include <DFRobotDFPlayerMini.h> // 採用DFRobotDFPlayerMini程式庫
#include <Adafruit_NeoPixel.h>
#define DEBUG true

#define LED_PIN 6   // LED 腳位定義
#define LED_COUNT 8 // 燈條上的LED數量
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//*************看這邊**************************************************
const int number_of_music = 2;   //歌曲數目，放16首歌就要寫16，99首歌就寫99。
int list[number_of_music] = {0};  //播放清單初始化
//********************************************************************

//按鈕腳位定義
#define next 2 // 按一下音量變大，長按下一首歌
#define prev 3 // 按一下音量變小，長按上一首歌
#define Play 4 // 按一下 暫停/撥放
#define mode 5 // 按一下切換模式 循環撥放/單曲重複/隨機撥放
//////////////////////////////////////////////////

//偵測播放腳位定義
#define idle 12 // 偵測播放狀態 撥放中LOW，暫停中HIGH
////////////////////////////////////////////////

//變數宣告
int playing = 0;
bool Flag = 0;
byte vol = 15;
byte R = 255;
byte G = 0;
byte B = 0;
int num;
bool Flag1;
int state = 0, curr = -1;
/////////////////////////

SoftwareSerial mySoftwareSerial(10, 11);     // mySoftwareSerial(RX, TX), 宣告軟體序列傳輸埠
                                             // 用來與DFPlayerMini通訊用

DFRobotDFPlayerMini myDFPlayer;         //宣告MP3 Player
//void printDetail(uint8_t type, int value);  

void rainbow(int wait, int j) // 彩虹酷炫燈
{
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) 
  {
    for (int i = 0; i < j; i++) // For each pixel in strip...
    {     
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
  }
}

void random_list(int listptr[], int curr) // 產生隨機撥放列表
{
  bool Flag[number_of_music] = {false}; // 陣列初始化
  randomSeed(analogRead(A0));     // 隨機種子
  Flag[listptr[curr] - 1] = true; // 紀錄哪首歌已經出現在清單內了，哪些還沒
  listptr[0] = listptr[curr];     // 把現在正在撥放的歌放在 listptr[0] 位置
  int i = 1;
  while (i < number_of_music) // 迴圈 number_of_music - 1 次
  {
    int randNumber = random(number_of_music); // 產生隨機數 0 ~ number_of_music - 1
    if (Flag[randNumber] == false) // 如果該歌曲不在 list 裡面
    {
      listptr[i] = randNumber + 1; // 將該歌曲加入 list 裡面
      Flag[randNumber] = true;     // 紀錄該歌曲出現
      i++; // 下一個index
    }
  }
  Serial.println("random list :");
  for (i = 0; i < number_of_music; i++) // 印出 list
  {
    Serial.println(listptr[i]);
  }
  return;
}

void reset_list(int listptr[], int curr)  // 產生循環撥放列表
{
  listptr[0] = listptr[curr]; // 把現在正在撥放的歌放在 listptr[0] 位置
  for (int i = 1; i < number_of_music; i++) // 迴圈 number_of_music - 1 次
  {
    listptr[i] = (listptr[i - 1] + 1) % number_of_music; // 按照歌曲順序放入 list
    if (listptr[i] == 0)
      listptr[i] = number_of_music; // 第 0 首就是第 number_of_music 首
  }
  Serial.println("reset_list :");
  for (int i = 0; i < number_of_music; i++)  // 印出 list
  {
    Serial.println(listptr[i]);
  }
}

void single_list(int listptr[], int curr) // 產生單曲重複撥放列表
{
  listptr[0] = listptr[curr]; // 把現在正在撥放的歌放在 listptr[0] 位置 
  for (int i = 1; i < number_of_music; i++)
  {
    listptr[i] = listptr[0]; // 全部都放同一首歌
  }
  Serial.println("single_list :");
  for (int i = 0; i < number_of_music; i++)  // 印出 list
  {
    Serial.println(listptr[i]);
  }
}

void printDetail(uint8_t type, int value) // 印出詳情
{
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return;
}
