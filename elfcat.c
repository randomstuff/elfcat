/* The MIT License (MIT)

Copyright (c) 2015 Gabriel Corona

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define _FILE_OFFSET_BITS 64

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <libelf.h>
#include <gelf.h>

// Genetic dump:

static int dump_zero(off_t size)
{
  size_t buffer_size = 64*1024;
  void* buffer = calloc(1, buffer_size);
  while(size) {
    size_t count = buffer_size < size ? buffer_size : size;
    size_t wcount = write(STDOUT_FILENO, buffer, count);
    if (wcount == -1) {
      fprintf(stderr, "Could not write\n");
      free(buffer);
      return EXIT_FAILURE;
    }
    size -= wcount;
  }
  free(buffer);
  return EXIT_SUCCESS;
}

static int dump_data(int fd, off_t offset, off_t size)
{
  size_t buffer_size = 64*1024;
  void* buffer = malloc(buffer_size);
  off_t start = lseek(fd, 0, SEEK_CUR);
  if (start == -1) {
    fprintf(stderr, "Could not seek\n");
    goto err_free;
  }
  if (lseek(fd, offset, SEEK_SET) == -1) {
    fprintf(stderr, "Could not seek\n");
    goto err_lseek;
  }

  while(size) {
    size_t available_size = buffer_size < size ? buffer_size : size;
    size_t rcount = read(fd, buffer, available_size);
    if (rcount == -1) {
      fprintf(stderr, "Could not read data\n");
      goto err_lseek;
    }
    size -= rcount;

    char* data = buffer;
    while(rcount) {
      size_t wcount = write(STDOUT_FILENO, data, rcount);
      if (wcount == -1) {
        fprintf(stderr, "Could not write data\n");
        goto err_lseek;
      }
      rcount -= wcount;
      data += wcount;
    }
  }

  if (lseek(fd, start, SEEK_SET) == -1) {
    fprintf(stderr, "Could not seek\n");
    goto err_free;
  }
  free(buffer);
  return EXIT_SUCCESS;

err_lseek:
  lseek(fd, start, SEEK_SET);
err_free:
  free(buffer);
  return EXIT_FAILURE;
}

// Program entries:

static int dump_program_by_index(Elf* elf, int fd, size_t index)
{
  GElf_Phdr phdr;
  if (gelf_getphdr(elf, index, &phdr) != &phdr) {
    fprintf(stderr, "Program entry not found\n");
    return EXIT_FAILURE;
  }
  int res = dump_data(fd, phdr.p_offset, phdr.p_filesz);
  if (res) {
    fprintf(stderr, "Could not dump data from file\n");
    return EXIT_FAILURE;
  }
  return dump_zero(phdr.p_memsz - phdr.p_filesz);
}

// Sections:

static int dump_section(int fd, Elf_Scn* scn)
{
  GElf_Shdr shdr;
  if (gelf_getshdr(scn , &shdr) != &shdr) {
    fprintf(stderr, "Could not get section data\n");
    return EXIT_FAILURE;
  }
  if (shdr.sh_type == SHT_NOBITS)
    return dump_zero(shdr.sh_size);
  return dump_data(fd, shdr.sh_offset, shdr.sh_size);
}

static int dump_section_by_index(Elf* elf, int fd, size_t index)
{
  Elf_Scn* scn = elf_getscn(elf, index);
  if (scn == NULL) {
    fprintf(stderr, "Section not found\n");
    return EXIT_FAILURE;
  }
  return dump_section(fd, scn);
}

static int dump_section_by_name(Elf* elf, int fd, char* section_name)
{

  Elf_Scn* scn = NULL;
  GElf_Shdr shdr;

  size_t shstrndx;
  if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
    fprintf(stderr, "Could not find section name section\n");
    return EXIT_FAILURE;
  }

  while ((scn = elf_nextscn(elf, scn)) != NULL) {
    if (gelf_getshdr(scn , &shdr) != &shdr) {
      fprintf(stderr, "Could not get section data\n");
      return EXIT_FAILURE;
    }
    const char* scn_name = elf_strptr(elf, shstrndx , shdr.sh_name);
    if (scn_name == NULL) {
      fprintf(stderr, "Could not get section name\n");
      return EXIT_FAILURE;
    }
    if (strcmp(scn_name, section_name) == 0)
      return dump_section(fd, scn);
  }
  fprintf(stderr, "Section not found\n");
  return EXIT_FAILURE;
}

// CLI:

static int show_help()
{
  fprintf(stderr, "elfcat ./foo --section-name .text\n");
  fprintf(stderr, "elfcat ./foo --section-index 3\n");
  fprintf(stderr, "elfcat ./foo --program-index 3\n");
  return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
  const char* elf_file_name = NULL;
  int elf_fd = -1;
  Elf* elf = NULL;

  if (elf_version(EV_CURRENT) == EV_NONE) {
    fprintf(stderr, "Could not initialize libelf\n");
    return EXIT_FAILURE;
  }

  for (int i = 1; i != argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == '-') {
      if (strcmp(argv[i], "--section-name") == 0) {
        if (argc == i + 1) {
          fprintf(stderr, "Missing section name\n");
          return EXIT_FAILURE;
        }
        if (dump_section_by_name(elf, elf_fd, argv[i+1]))
          return EXIT_FAILURE;
          ++i;
      } else if (strcmp(argv[i], "--section-index") == 0) {
        if (argc == i + 1) {
          fprintf(stderr, "Missing section index\n");
          return EXIT_FAILURE;
        }
        long long index = atoll(argv[i+1]);
        if (dump_section_by_index(elf, elf_fd, index))
          return EXIT_FAILURE;
        ++i;
      } else if (strcmp(argv[i], "--program-index") == 0) {
        if (argc == i + 1) {
          fprintf(stderr, "Missing program index\n");
          return EXIT_FAILURE;
        }
        long long index = atoll(argv[i+1]);
        if (dump_program_by_index(elf, elf_fd, index))
          return EXIT_FAILURE;
        ++i;
      } else if (strcmp(argv[i], "--help") == 0) {
        return show_help();
      } else {
        fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
        return EXIT_FAILURE;
      }
    }

    else if (argv[i][0] == '-') {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      return EXIT_FAILURE;
    }

    else {
      if (elf)
        elf_end(elf);
      if (elf_fd != -1)
        close(elf_fd);

      elf_file_name = argv[i];

      elf_fd = open(elf_file_name, O_RDONLY);
      if (elf_fd == -1) {
        fprintf(stderr, "Could not open file %s\n", argv[i]);
          return EXIT_FAILURE;
      }

      elf = elf_begin(elf_fd, ELF_C_READ, NULL);
      if (elf == NULL) {
        fprintf(stderr, "Could read file as ELF: %s\n", argv[i]);
        fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
        return EXIT_FAILURE;
      }
    }
  }
  return EXIT_SUCCESS;
}
