/*
 *  rendering_objif.ino - High speed rendering sample application
 *  Copyright 2020 Sony Semiconductor Solutions Corporation
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

#include <SDHCI.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>
#include <stdio.h>
#include <stdlib.h>

#include <arch/board/board.h>

#define READSAMPLE (240)
#define BYTEWIDTH (2)
#define CHNUM (2)
#define READSIZE (READSAMPLE * BYTEWIDTH * CHNUM)
#define RREREQUEST (3)

SDClass theSD;
OutputMixer *theMixer;

File myFile;

class BridgeBuffer {
private:

  const uint32_t mc_readsize = READSIZE;
  
  uint8_t  m_buf[30 * READSIZE]; /* size for 30 frame */
  uint32_t m_wp;
  uint32_t m_rp;
  
public:

  BridgeBuffer()
    : m_wp(0)
    , m_rp(0)
    { printf("bufsize %d\n", sizeof(m_buf)); }
  ~BridgeBuffer() {}

  int32_t writebuf(File& file)
  {
    return writebuf(file, mc_readsize);    
  }

  /* Read from source and write to buffer. */
  int32_t writebuf(File& file, uint32_t reqsize)
  {
    /* Get vacant space. */
    uint32_t space = (m_wp < m_rp) ? (m_rp - m_wp) - 1 : (sizeof(m_buf) - m_wp + m_rp) - 1;
    uint32_t writesize = 0;

    /* If vacant space is smaller than request size, don't write buffer. */
    if (space < reqsize) {
      return -1;
    }

    /* Read file and write to buffer. */
    if (m_wp + reqsize > sizeof(m_buf)) {
      writesize = file.read(&m_buf[m_wp], sizeof(m_buf) - m_wp);
      writesize += file.read(&m_buf[0], reqsize - (sizeof(m_buf) - m_wp));
      m_wp = (m_wp + writesize) % sizeof(m_buf);
    }
    else {
      writesize = file.read(&m_buf[m_wp], reqsize);
      m_wp += writesize;
    }

    /* Return wrote size. */
    return writesize;
  }

  /* Read from buffer and move to destination. */
  int32_t readbuf(uint8_t *dst)
  {
    /* Get remain size of data in the buffer. */
    uint32_t rem = (m_rp <= m_wp) ? (m_wp - m_rp) : (sizeof(m_buf) - m_rp + m_wp);
    uint32_t readsize = (rem > mc_readsize) ? mc_readsize : rem;
    //printf("wp %d rp %d rem %d read %d\n", m_wp, m_rp, rem, readsize);

    /* No data remains, then return. */
    if (readsize <= 0) {
      return readsize;
    }

    /* Read buffer and move to destination. */
    if (m_rp + readsize > sizeof(m_buf)) {
      memcpy(dst, &m_buf[m_rp], sizeof(m_buf) - m_rp);
      memcpy(dst + (sizeof(m_buf) - m_rp), &m_buf[0], readsize - (sizeof(m_buf) - m_rp));
      m_rp = (m_rp + readsize) % sizeof(m_buf);
    }
    else {
      memcpy(dst, &m_buf[m_rp], readsize);
      m_rp += readsize;
    }

    /* Return read size. */
    return readsize;
  }

  /* Clear buffer. */
  void clearbuf(void)
  {
    m_rp = 0;
    m_wp = 0;
  }
};

BridgeBuffer myBuffer;

bool ErrEnd = false;

static enum State {
  Ready = 0,
  Active,
  Stopping,
} s_state = Ready;

static bool getFrame(AsPcmDataParam *pcm, bool direct_read)
{
  /* Alloc MemHandle */
  if (pcm->mh.allocSeg(S0_REND_PCM_BUF_POOL, READSIZE) != ERR_OK) {
    return false;
  }

  /* Set PCM parameters */
  pcm->identifier = 0;
  pcm->callback = 0;
  pcm->bit_length = 16;
  pcm->size = (direct_read) ? myFile.read((uint8_t *)pcm->mh.getPa(), READSIZE) : myBuffer.readbuf((uint8_t *)pcm->mh.getPa());
  pcm->sample = pcm->size / BYTEWIDTH / CHNUM;  
  pcm->is_end = (pcm->size < READSIZE);
  pcm->is_valid = (pcm->size > 0);

  if (pcm->size < 0) {
    puts("Cannot read SD card!");
    return false;
  }

  return true;
}

static bool getFrame(AsPcmDataParam *pcm)
{
  return getFrame(pcm, false);
}

static bool start(uint8_t no)
{
  printf("start(%d) start\n", no);

  /* Open file placed on SD card */

  const char *raw_files[] =
  {
    "sound0.raw",
    "sound1.raw",
    "sound2.raw",
  };

  char fullpath[64] = { 0 };

  assert(no < sizeof(raw_files) / sizeof(char *));

  snprintf(fullpath, sizeof(fullpath), "AUDIO/%s", raw_files[no]);

  myFile = theSD.open(fullpath);

  if (!myFile)
    {
      printf("File open error\n");
      return false;
    }

  /* Start rendering. */
  for (int i = 0; i < RREREQUEST; i++) {
    AsPcmDataParam pcm_param;
    if (!getFrame(&pcm_param, true)) {
      break;
    }

    /* Send PCM */
    int err = theMixer->sendData(OutputMixer0,
                                 outmixer_send_callback,
                                 pcm_param);

    if (err != OUTPUTMIXER_ECODE_OK) {
      printf("OutputMixer send error: %d\n", err);
      return false;
    }
  }

  /* Seek set to top of file, and clear buffer. */
  myBuffer.clearbuf();

  /* Buffer pre store. */
  myBuffer.writebuf(myFile, READSIZE * RREREQUEST);
  
  printf("start() complete\n");

  return true;
}

static bool restart()
{
  printf("restart()\n");

  /* Seek set to top of file. */
  myFile.seek(0);

  for (int i = 0; i < RREREQUEST; i++) {
    AsPcmDataParam pcm_param;
    if (getFrame(&pcm_param, true)) {
      /* Send PCM */
      int err = theMixer->sendData(OutputMixer0,
                                   outmixer_send_callback,
                                   pcm_param);

      if (err != OUTPUTMIXER_ECODE_OK) {
        printf("OutputMixer send error: %d\n", err);
        return false;
      }
    }
  }

  /* Clear buffer. */
  myBuffer.clearbuf();

  /* Store data. (Don't write a lot of frame at once.) */
  for (int i = 0; i < RREREQUEST; i++) {
    if (myBuffer.writebuf(myFile) < 0) {
      break; 
    }
  }

  return true;
}

static void stop()
{
  printf("stop\n");

  AsPcmDataParam pcm_param;
  getFrame(&pcm_param);

  pcm_param.is_end = true;

  int err = theMixer->sendData(OutputMixer0,
                               outmixer_send_callback,
                               pcm_param);

  if (err != OUTPUTMIXER_ECODE_OK) {
    printf("OutputMixer send error: %d\n", err);
  }

  /* Clear buffer. */
  myBuffer.clearbuf();
  myFile.close();
}

/**
 * @brief Mixer done callback procedure
 *
 * @param [in] requester_dtq    MsgQueId type
 * @param [in] reply_of         MsgType type
 * @param [in,out] done_param   AsOutputMixDoneParam type pointer
 */
static void outputmixer_done_callback(MsgQueId requester_dtq,
                                      MsgType reply_of,
                                      AsOutputMixDoneParam *done_param)
{
  printf(">> %x done\n", reply_of);
  return;
}

/**
 * @brief Mixer data send callback procedure
 *
 * @param [in] identifier   Device identifier
 * @param [in] is_end       For normal request give false, for stop request give true
 */
static void outmixer_send_callback(int32_t identifier, bool is_end)
{
  //printf("send done %d %d\n", identifier, is_end);

  AsPcmDataParam pcm_param;

  while (true) {
    if (s_state == Stopping) {
      break;
    }

    if (!getFrame(&pcm_param)) {
      break;
    }
    
    /* Send PCM */
    pcm_param.is_end = false;
    pcm_param.is_valid = true;
    if (pcm_param.size == 0) {
      pcm_param.size = READSIZE;
      pcm_param.sample = pcm_param.size / BYTEWIDTH / CHNUM;
      memset(pcm_param.mh.getPa(), 0, pcm_param.size);
    }
    int err = theMixer->sendData(OutputMixer0,
                                 outmixer_send_callback,
                                 pcm_param);

    if (err != OUTPUTMIXER_ECODE_OK) {
      printf("OutputMixer send error: %d\n", err);
      break;
    }
  }

  if (is_end) {
    s_state = Ready;
  }

  return;
}


static void attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
    ErrEnd = true;
  }
}

void setup()
{
  printf("setup() start\n");
  
  /* Display menu */

  Serial.begin(115200);
  
  /* Initialize memory pools and message libs */  
  initMemoryPools();
  createStaticPools(MEM_LAYOUT_PLAYER);

  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Start audio system */
  theMixer  = OutputMixer::getInstance();
  theMixer->activateBaseband();

  /* Create Objects */
  theMixer->create(attention_cb);

  /* Set rendering clock */
  theMixer->setRenderingClkMode(OUTPUTMIXER_RNDCLK_NORMAL);

  /* Activate Mixer Object.
   * Set output device to speaker with 2nd argument.
   * If you want to change the output device to I2S,
   * specify "I2SOutputDevice" as an argument.
   */
  theMixer->activate(OutputMixer0, HPOutputDevice, outputmixer_done_callback);

  usleep(100 * 1000);

  /* Set main volume */
  theMixer->setVolume(0, 0, 0);

  /* Unmute */
  board_external_amp_mute_control(false);

  printf("setup() complete\n");
}

uint8_t start_event(uint8_t playno, uint8_t eventno)
{
  if (s_state == Ready) {
    if (start(eventno)) {
      s_state = Active;      
    }
  } else {
    if(playno == eventno){
      printf("Restart\n");
      restart();
    }else{
      myFile.close();
      start(eventno);
    }
  }
  return eventno;
}


void loop()
{
  static uint8_t playno = 0xff;

  /* Fatal error */

  if (ErrEnd) {
    printf("Error End\n");
    goto stop_rendering;
  }

  /* Menu operation */

  if (Serial.available() > 0)
    {
      switch (Serial.read()) {
        case 'p': // play
          playno = start_event(playno,0);
          break;
        case 'o': // play
          playno = start_event(playno,1);
          break;
        case 'i': // play
          playno = start_event(playno,2);
          break;
        case 's': // stop
          if (s_state == Active)
            {
              stop();
            }
          s_state = Stopping;
          break;
          
        default:
          break;
      }
    }

  /* Processing in accordance with the state */

  switch (s_state) {
    case Stopping:
      break;

    case Ready:
      break;

    case Active:
      /* Send new frames to be decoded until end of file */
      for(int i = 0; i < 10; i++) {
        /* get PCM */
        int32_t wsize = myBuffer.writebuf(myFile);
        if (wsize < 0) {
          break;
        }
        else if (wsize < READSIZE) {
          break;        
        }
        else {
        }
      }
      break;

    default:
      break;
  }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
   */

  return;

stop_rendering:
  board_external_amp_mute_control(true); 
  myFile.close();
  theMixer->deactivate(OutputMixer0);
  theMixer->end();
  puts("Exit Rendering");
  exit(1);
}
