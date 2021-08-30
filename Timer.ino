static long Time[10];

void TimerResetAll()
{
  for(int i=0;i<10;i++)
  {
    Time[i]=-1;
  }
}

void TimerReset(int n)
{
  Time[n]=-1;
}

void TimerSet(int n)
{
  Time[n]=millis();
}

bool Timeout(int n,int milli)
{
  long C_Time=millis();
  if((C_Time - Time[n]) < milli || Time[n]==-1)
  {
    return false;
  }
  else
  {
    return true;
  }
}
