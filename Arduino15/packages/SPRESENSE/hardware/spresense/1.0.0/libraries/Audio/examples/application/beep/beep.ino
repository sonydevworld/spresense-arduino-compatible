/*
 *  beep.ino - beep example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <Audio.h>

AudioClass *theAudio;

class Score
{
public:

  typedef struct {
    int fs;  
    int time;
  } Note;

  void init(){
    pos = 0;
  }
  
  Note get(){
    return data[pos++];
  }
  
private:

  int pos;

  Note data[17] =
  {
    {262,500},
    {294,500},
    {330,500},
    {349,500},
    {392,500},
    {440,500},
    {494,500},
    {523,1000},

    {523,500},
    {494,500},
    {440,500},
    {392,500},
    {349,500},
    {330,500},
    {294,500},
    {262,1000},

    {0,0}
  };
 
/*  Note data[17] =
  {
    {440,1000},
    {440,1000},
    {494,2000},

    {440,1000},
    {440,1000},
    {494,2000},
 
    {440,1000},
    {494,1000},
    {523,1000},
    {494,1000},
 
    {440,1000},
    {494,500},
    {440,500},
    {349,2000},

    {0,0}
  };*/
  
};

Score theScore;

void setup()
{

  theAudio = AudioClass::getInstance();
  theAudio->begin();
  puts("initialization Audio Library");

  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, 0, 0);

  theScore.init();
  
}

void loop()
{
  puts("loop!!");

  Score::Note theNote = theScore.get();
  if (theNote.fs == 0) {
    puts("End,");
    theAudio->setReadyMode();
    theAudio->end();
    exit(1);
  }

  theAudio->setBeep(1,-40,theNote.fs);

  /* The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */
 
  usleep(theNote.time * 1000);

  theAudio->setBeep(0,0,0);

  /* The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */
 
  usleep(100000);

}
