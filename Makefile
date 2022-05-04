####################################################################################
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
####################################################################################

CC := gcc
CFLAGS := --std=gnu99 -pedantic -Wall -Wextra -Wformat -O2 -g

all: brainfuck-compile brainfuck

run: all
	./brainfuck

brainfuck: src/brainfuck.c
	$(CC) $(CFLAGS) -o brainfuck src/brainfuck.c

brainfuck-compile: src/brainfuck-compile.c
	$(CC) $(CFLAGS) -o brainfuck-compile src/brainfuck-compile.c

fmt: $(wildcard *.c)
	clang-format --style=WebKit -i src/*
