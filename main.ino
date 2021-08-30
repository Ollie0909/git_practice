
void setup()
{
  pinMode(next, INPUT); // 按鈕腳位輸入模式
  pinMode(prev, INPUT); // 按鈕腳位輸入模式
  pinMode(Play, INPUT); // 按鈕腳位輸入模式
  pinMode(mode, INPUT); // 按鈕腳位輸入模式
  pinMode(idle, INPUT); // 播放狀態腳位輸入模式
  pinMode(LED_PIN, OUTPUT); // LED 控制腳位輸出模式

  strip.begin();             // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();             // Turn OFF all pixels
  strip.setBrightness(10);   // set Brightness to 10

  Serial.begin(115200);          // 定義Serial傳輸速率115200bps
  mySoftwareSerial.begin(9600);  // 定義mySoftwareSerial傳輸速率9600bps, DFPlayerMini的通訊速率為9600bps.

  Serial.println(F("DFRobot DFPlayer Mini Demo"));   // 印出DFRobot DFPlayer Mini Demo字串到Serial通訊埠
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));    // 以下用法依此類通, 不再贅述喔

  if (!myDFPlayer.begin(mySoftwareSerial))           // 如果DFPlayer Mini回應不正確.
  { //Use softwareSerial to communicate with mp3.   // 印出下面3行字串
    Serial.println(F("Unable to begin:"));         // 然後程式卡死.
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));  // 如果DFPlayer Mini回應正確.印出"DFPlayer Mini online."

  myDFPlayer.setTimeOut(500); // 設定通訊逾時為500ms
  rainbow(3, 8); // 彩虹酷炫燈
  strip.clear();
//  for (int i = 0; i < vol * 8 / 30; i++) {
//    strip.setPixelColor(i, strip.Color(255,   0,   0)); //  Set pixel's color (in RAM)
//    strip.show();                                       //  Update strip to match
//  }
  //----Set volume----
  myDFPlayer.volume(vol);    // 設定音量, 範圍0~30
  //myDFPlayer.volumeUp();   // 增加音量
  //myDFPlayer.volumeDown(); // 降低音量

  myDFPlayer.EQ(DFPLAYER_EQ_ROCK); // 等化器搖滾模式

  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  myDFPlayer.enableDAC(); //Enable On-chip DAC

 // myDFPlayer.play(1);  // 播放第1首音樂

  //播放清單初始化 list[] = {1, 2, 3, 4...}
  for (int i = 0; i < number_of_music; i++)
  {
    list[i] = i + 1;
  }
  myDFPlayer.play(list[0]);
  TimerResetAll(); // 所有計時器初始化 
  TimerSet(2);     // 計時器 2 開始計時
  Flag1 = true;
}

void loop()
{
  if (myDFPlayer.available())  // 監視MP3有沒有回應
  {                            // 有的話印出詳情
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  if (Timeout(3, 1000)) // 如果計時器 3 計時達 1 秒
  {
    TimerReset(3);      // 關閉計時器 3 
    Flag1 = true;
  }
  // Timeout(2,10) &&
  if ( Timeout(2, 10) && Flag1)
  {
    if (num < 5 * 65536)
    {
      num += 256;
    }
    else
    {
      num = 0;
    }
    for (int i = 0 ; i < 8 ; i++)
    {
      int pixelHue = num + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    TimerSet(2);
    strip.show();
  }
#if(DEBUG)
  Serial.print("state = ");
  Serial.print(state);
#endif
  curr = curr % number_of_music; // 如果 curr = number_of_music， list[curr] 會超界。(int list[number_of_music]; 的 index 是從 0 ~ number_of_music - 1。) 
#if(DEBUG)
  Serial.print("    curr: ");
  Serial.print(curr);                 // 印出現在index
  Serial.print("    current song: ");
  Serial.println(list[curr]);         // 印出現在曲目
#endif

///////// 有限狀態機 FSM ///////////////////////////////
  switch (state)
  {
    case 0: // play & 循環 紅色燈
    case 2: // play & 單曲 綠色燈
    case 4: // play & 隨機 藍色燈
      if (digitalRead(idle) == HIGH) // 如果播完一首歌
      {
        if (curr == number_of_music - 1) curr = -1; // curr == number_of_music - 1 是 curr 的上限(不熟的人可以複習基礎程式的陣列array)，要在 curr 爆掉前先改成 -1
        myDFPlayer.play(list[curr + 1]);            // 播放下一首
        curr++;                                     // curr = curr + 1
        delay(200);
      }
      if (digitalRead(next) == HIGH) // 如果按下按鈕( pin2 )
      {
        playing = state; // 記錄當前 state
        state = 100;     // 跳到 state 100
        TimerSet(0);     // 計時器 0 開始計時
      }
      if (digitalRead(prev) == HIGH) // 如果按下按鈕( pin3 )
      {
        playing = state; // 記錄當前 state
        state = 102;     // 跳到 state 102
        TimerSet(0);     // 計時器 0 開始計時
      }
      if (digitalRead(Play) == HIGH) // 如果按下按鈕( pin4 )
      {
        myDFPlayer.pause(); // 暫停
        state = state + 1;  // 跳到暫停 state (0->1, 2->3, 4->5)
        delay(200);
      }
      if (digitalRead(mode) == HIGH) // 如果按下按鈕( pin5 )
      {
        Flag1 = false; // 暫時關閉彩虹酷炫燈
        TimerSet(3);   // 計時器 3 開始計時
        if (state == 0)       // 循環撥放 => 單曲重複
        {
          single_list(list, curr); // 將播放清單改成單曲重複
          curr = 0;
          state = 2;
          R = 0;
          G = 255; // 燈條亮綠燈
          B = 0;
        }
        else if (state == 2) // 單曲重複 => 隨機撥放
        {
          random_list(list, curr); // 將播放清單改成隨機撥放
          curr = 0;
          state = 4;
          R = 0;
          G = 0;
          B = 255; // 燈條亮藍燈
        }
        else                // 隨機撥放 => 循環撥放
        {
          reset_list(list, curr); // 將播放清單改成循環撥放
          curr = 0;
          state = 0;
          R = 255; // 燈條亮紅燈
          G = 0;
          B = 0;
        }
        for (int i = 0; i < 8 ; i++) {
          strip.setPixelColor(i, strip.Color(R,   G,   B)); //  Set pixel's color (in RAM)
          strip.show();                                     //  Update strip to match
        }
        delay(200);
      }
      break;
    ////////// 100 ~ 103是專門為了長按設計的 state ////////////////////////////////////
    case 100:
      if (digitalRead(next) == LOW)
      {
        vol += 3; // vol = vol + 3
        if (vol > 30) vol = 30; // 最大只能到 30
        myDFPlayer.volume(vol); // 調整音量的函式
        strip.clear();
        Flag1 = false;          // 暫時關掉彩虹燈
        TimerSet(3);            // 計時器 3 開始計時
        for (int i = 0; i < vol * 8 / 30; i++) {
          strip.setPixelColor(i, strip.Color(R,   G,   B)); //  Set pixel's color (in RAM)
          strip.show();                                     //  Update strip to match
        }
        state = playing;        // 跳回原本的 state (0 ~ 5)g
      }
      else if ( Timeout( 0, 200 ) ) // 計時器 0 超過 200ms (按下按鈕時，計時器 0 開始計時)
      {
        TimerSet(1); // 計時器 1 開始計時
        state = 101; // 跳到 state 101
      }
      break;

    case 101:
      if (digitalRead(next) == LOW) // 如果放開按鈕
      {
        state = playing; // 跳回原本的 state (0 ~ 5)
        Flag = 0;        // Flag 初始化
      }
      else if ( Timeout( 1, 200) ) // 計時器 1 超過 200ms (跳到 state 101 時，計時器 1 開始計時)
      {
        if (!Flag) // 如果 Flag == 0，播放下一首
        {
          if (curr == number_of_music - 1) curr = -1; // curr == number_of_music - 1 是 curr 的上限(不熟的人可以複習基礎程式的陣列array)，要在 curr 爆掉前先改成 -1
          myDFPlayer.play(list[curr + 1]);            // 播放下一首
          curr++;                                     // curr = curr + 1
          Flag = 1;                                   // Flag 變成 1 之後， if (!Flag) 內的程式就不會再被執行，直到 Flag 被初始化
        }
      }
      break;

    case 102:
      if (digitalRead(prev) == LOW)
      {
        vol-=3;
        if (vol < 3) vol = 3;
        myDFPlayer.volume(vol);
        state = playing;
        strip.clear();
        Flag1 = false;
        TimerSet(3);
        for (int i = 0; i < vol * 8 / 30; i++) {
          strip.setPixelColor(i, strip.Color(R,   G,   B));   //  Set pixel's color (in RAM)
          strip.show();                                       //  Update strip to match
        }
      }
      else if ( Timeout( 0, 200 ) )
      {
        TimerSet(1);
        state = 103;
      }
      break;

    case 103:
      if (digitalRead(prev) == LOW)
      {
        state = playing;
        Flag = 0;
      }
      else if ( Timeout( 1, 200) )
      {
        if (!Flag)
        {
          if (curr == 0) curr = number_of_music;
          myDFPlayer.play(list[curr - 1]);
          curr--;
          Flag = 1;
        }
        TimerSet(1);
        state = 103;
      }
      break;
    //////////////////////////////////////////////
    case 1: // pause & 循環 紅色燈
    case 3: // pause & 單曲 綠色燈
    case 5: // pause & 隨機 藍色燈
      if (digitalRead(next) == HIGH)
      {
        playing = state;
        state = 100;
        TimerSet(0);
      }
      if (digitalRead(prev) == HIGH)
      {
        playing = state;
        state = 102;
        TimerSet(0);
      }
      if (digitalRead(Play) == HIGH)
      {
        myDFPlayer.start();
        state = state - 1;
        delay(200);
      }
      if (digitalRead(mode) == HIGH)
      {
        Flag1 = false;
        TimerSet(3);
        if (state == 1)
        {
          single_list(list, curr);
          curr = 0;
          state = 3;
          R = 0;
          G = 255;
          B = 0;
        }
        else if (state == 3)
        {
          random_list(list, curr);
          curr = 0;
          state = 5;
          R = 0;
          G = 0;
          B = 255;
        }
        else
        {
          reset_list(list, curr);
          curr = 0;
          state = 1;
          R = 255;
          G = 0;
          B = 0;
        }
        delay(200);
        for (int i = 0; i < 8 ; i++) {
          strip.setPixelColor(i, strip.Color(R,   G,   B));              //  Set pixel's color (in RAM)
        }
      }
      strip.show();                                              //  Update strip to match
      break;
    default: break;
  }
}
