/***********************************************************************************
#
# Prokop Schield's BrainFuck implementation for GNU/Linux
# (c) 2022 Prokop Schield.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
***********************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define I_INCV ('+')
#define I_DECV ('-')
#define I_MVPR ('>')
#define I_MVPL ('<')
#define I_PUTV ('.')
#define I_GETV (',')
#define I_LOOP ('[')
#define I_ENDL (']')

#define I_NOOP (0x20)
#define I_EXIT (-1)

#define BUFFER_SIZE (0x100000000)
#define PROGRAM_OFFSET (0x10000000)
#define REG_OFFSET (0x1000000)
#define LJB_OFFSET (0x2000000)

#define ERR_READING_FILE "Error %sing file %s: %s\n"
#define ERR_EOF_WITHIN_LOOP "Program was trying to skip over %d loop(s), but EOF was encountered at IP=0x%x\n"

typedef struct registers {
    uint32_t IP;
    uint32_t VP;
} registers_t;

int main(int argc, const char* argv[])
{
    char* memory = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    registers_t* registers = (struct registers*)(memory + REG_OFFSET);
    uint32_t* loop_jump_pos = (uint32_t*)(memory + LJB_OFFSET);
    char* program = memory + PROGRAM_OFFSET;
    int c = '\0';

    if (memory == NULL || memory == MAP_FAILED) {
        perror("Memory allocation error");
        return 1;
    }

    registers->IP = PROGRAM_OFFSET;

    for (int i = 1; i < argc; ++i) {
        const char* restrict filename = argv[i];
        struct stat s;
        if (stat(filename, &s)) {
            fprintf(stderr, ERR_READING_FILE, "stat", filename, strerror(errno));
            continue;
        }
        int fd = open(filename, 'r');
        if (fd <= 0) {
            fprintf(stderr, ERR_READING_FILE, "open", filename, strerror(errno));
            continue;
        }
        char* rmap = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (rmap == NULL || rmap == MAP_FAILED) {
            close(fd);
            fprintf(stderr, ERR_READING_FILE, "mmap", filename, strerror(errno));
            continue;
        }
        close(fd);
        memcpy(program, rmap, s.st_size);
        program += s.st_size;
        munmap(rmap, s.st_size);
    }

    while (memory[registers->IP] != I_EXIT) {
        switch (memory[registers->IP++]) {
        case I_INCV:
            ++memory[registers->VP];
            break;
        case I_DECV:
            --memory[registers->VP];
            break;
        case I_MVPR:
            ++registers->VP;
            break;
        case I_MVPL:
            --registers->VP;
            break;
        case I_PUTV:
            putchar(memory[registers->VP]);
            break;
        case I_GETV:
            c = getchar();
            if (c != EOF) {
                memory[registers->VP] = c;
            }
            break;
        case I_LOOP:
            if (memory[registers->VP]) {
                *(++loop_jump_pos) = registers->IP;
            } else {
                for (int i = 1; i; ++registers->IP) {
                    if (memory[registers->IP] == I_LOOP) {
                        ++i;
                    } else if (memory[registers->IP] == I_ENDL) {
                        --i;
                    } else if (memory[registers->IP] == '\0') {
                        c = getchar();
                        if (c != EOF) {
                            memory[registers->IP--] = c;
                        } else {
                            fprintf(stderr, ERR_EOF_WITHIN_LOOP, i, (unsigned)registers->IP - PROGRAM_OFFSET);
                            return 1;
                        }
                    }
                }
            }
            break;
        case I_ENDL:
            if (memory[registers->VP]) {
                registers->IP = *(loop_jump_pos);
            } else {
                --loop_jump_pos;
            }
            break;
        case I_NOOP:
            break;
        case I_EXIT:
            munmap(memory, BUFFER_SIZE);
            return 0;
        case '\0':
            memory[--registers->IP] = getchar();
            break;
        default:
            memory[registers->IP - 1] = I_NOOP;
            break;
        }
    }
}
