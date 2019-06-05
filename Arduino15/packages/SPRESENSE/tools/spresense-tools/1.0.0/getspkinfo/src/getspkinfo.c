/****************************************************************************
 * getspkinfo.c - Get SPK file information
 *
 *   Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define SPK_MAGIC_VALUE 0x444F4DEF

int main(int argc, char *argv[])
{
  FILE *fp;
  uint32_t magic, stack;

  if (argc < 2) {
    printf("Usage: %s <spk file name>\n", argv[0]);
    return -1;
  }

  fp = fopen(argv[1], "rb");
  if (!fp) {
    printf("%s: cannot access '%s': No such file or directory\n", argv[0], argv[1]);
    return -1;
  }

  fread(&magic, sizeof(magic), 1, fp);
  if (magic !=  SPK_MAGIC_VALUE) {
    printf("%s: cannot get SPK information from %s: Invalid SPK file\n", argv[0], argv[1]);
    return -1;
  }

  /* Get memory size */
  fseek(fp, 20, SEEK_SET);
  fread(&stack, sizeof(stack), 1, fp);

#ifdef _WIN32
  _setmode(_fileno(stderr), _O_BINARY);
#endif

  fprintf(stderr, "####################################\n");
  fprintf(stderr, "## Used memory size: %4d [KByte] ##\n", (stack & 0x00ffffff) / 1024);
  fprintf(stderr, "####################################\n");

  fclose(fp);
  return 0;
}
