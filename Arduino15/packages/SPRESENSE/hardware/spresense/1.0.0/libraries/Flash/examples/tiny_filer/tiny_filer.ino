/**
 * @file tiny_filer.ino
 * @author Tomonobu.Saito@gmail.com
 * @brief Tiny Filer to explore in Flash device.
 */

#include <Arduino.h>
#include <File.h>
#include <Flash.h>

// const
#define PROMPT "cmd('0':goto root, num:show, 'd'+num:delete)>"
#define MOUNT_POINT "/mnt/spif/"
#define MAX_ENTRY_POINT 64
#define MAX_PATH_LENGTH 64

#define COMMAND_FLAG_SHOW ((uint32_t)(0x0000 << 16))
#define COMMAND_FLAG_DEL  ((uint32_t)(0x0001 << 16))
#define COMMAND_FLAG_ERR  ((uint32_t)(0x1000 << 16))

// structure
typedef struct {
  bool isvalid;  // availability of this EntryPoint
  char path[MAX_PATH_LENGTH]; // path
  bool isdir;
  int  size;
} EntryPoint;

// global
EntryPoint entryPoints[MAX_ENTRY_POINT]; /* entry point list */
File current; /* current directory */

// Print all file and directory names.
// like 'ls' command
void printDirectory(File dir) {
  memset(entryPoints, 0, sizeof(entryPoints)); // all reset
  int idx = 1;
  while (idx < MAX_ENTRY_POINT) {
    File entry = dir.openNextFile();
    // end check (and break)
    if (!entry) {
      dir.rewindDirectory();
      break;
    }
    // copy to entry point
    // Serial.println(entry.name());
    strncpy(entryPoints[idx].path, entry.name() + strlen(MOUNT_POINT), MAX_PATH_LENGTH);
    entryPoints[idx].isdir = entry.isDirectory();
    entryPoints[idx].size  = entry.size();
    entryPoints[idx].isvalid = true;
    entry.close();
    
    // print path
    Serial.print("  ");
    Serial.print(idx);
    Serial.print(" : ");
    Serial.print(entryPoints[idx].path);

    // print attribute
    if (entryPoints[idx].isdir) {
      Serial.println(" <DIR>");
    } else {
      Serial.print(" ");
      Serial.print(entryPoints[idx].size, DEC);
      Serial.println(" bytes");
    }
    
    idx++;
  } // end of while
}

// Print contents of text file.
// like 'cat' command
void printContents(File file) {
  Serial.println("-----");
  /* read file. */
  int len = 1; // dummy
  char buf[128 + 1]; // +1 for centinel(\0)
  while (0 < len) {
    memset(buf, 0, sizeof(buf));
    len = file.read(buf, sizeof(buf) - 1);
    Serial.print(buf);
  }
  Serial.println("[EOF]");
  Serial.println("-----");
}

// get command No from serial
// upper 16 bit : flags
//   0x 0000 xxxx : show   (COMMAND_FLAG_SHOW)
//   0x 0001 xxxx : delete (COMMAND_FLAG_DEL)
///  0x 1000 xxxx : error  (COMMAND_FLAG_ERR)
// lower 16 bit : command No
uint32_t GetCommand()
{
  static char buf[64];
  memset(buf, 0, sizeof(buf));
  int idx = 0;
  uint32_t retval = COMMAND_FLAG_ERR;

  // get 1st character of command string
  if (0 < Serial.available()) {
    char c = (char)Serial.read();
    Serial.println(c);
    if ('0' <= c && c <= '9') {
      retval = COMMAND_FLAG_SHOW;
      buf[idx++] = c;  
    }
    else if ('d' == c || 'D' == c) {
      retval = COMMAND_FLAG_DEL;
    }
  }
  
  // get remaining command string
  while (0 < Serial.available()) {
    buf[idx++] = (char)Serial.read();
  }
  buf[idx] = 0;

  // string to number
  return strtol(buf, NULL, 10) | retval;
}

// setup
void setup() {
  /* Open serial communications and wait for port to open */
  Serial.begin(115200);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

  /* Show root directory of Flash */
  current = Flash.open("/");
  printDirectory(current);
  Serial.print(PROMPT);
}

// loop
void loop() {
  if (0 < Serial.available()) {
    // get commad number
    uint32_t cmd_no = GetCommand();

    // execute command
    if (0 == cmd_no) {
      // goto root again.
      Serial.println("Back to root directory.");
      current = Flash.open("/");
      printDirectory(current);
    }
    else {
      // show file or enter directory
      if (cmd_no < MAX_ENTRY_POINT && entryPoints[cmd_no].isvalid) {
        if (entryPoints[cmd_no].isdir) {
          current.close();
          Serial.print("Move to ");
          Serial.println(entryPoints[cmd_no].path);
          current = Flash.open(entryPoints[cmd_no].path);
          printDirectory(current);
        } else {
          Serial.print("Show : ");
          Serial.println(entryPoints[cmd_no].path);
          File tmp = Flash.open(entryPoints[cmd_no].path);
          printContents(tmp);
          tmp.close();
          printDirectory(current);        
        }
      }
      // delete file
      else if (0 < (COMMAND_FLAG_DEL & cmd_no)) {
        uint32_t target = cmd_no & 0xffff;
        Serial.print("delete : ");
        Serial.println(target);
        if (target < MAX_ENTRY_POINT && entryPoints[target].isvalid){
          if (entryPoints[target].isdir) {
            // delete directory
            Flash.rmdir(entryPoints[target].path);
          }
          else {
            // delete file
            Flash.remove(entryPoints[target].path);
          }
          Serial.print("Remove : ");
          Serial.println(entryPoints[target].path);
          printDirectory(current);
        }
      }
      // 
      else {
        Serial.print("invalid cmd number : ");
        Serial.println(cmd_no);
      }
    }
    
    // show prompt for next input
    Serial.print(PROMPT);
  }
}
