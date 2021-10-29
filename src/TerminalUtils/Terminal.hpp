#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <semaphore.h>

#include <string>
#include <iostream>
#include "../StringUtils/StringUtils.hpp"

#define gotoxy(row,col) printf("\033[%d;%dH", (row), (col))

using namespace std;

class Terminal {
    private: 
        

    public:
        sem_t _terminalSempahore;
        int _lastRow;
        int _maxRow;
        int _rows;
        int _cols;
        
        Terminal();
        void setMaxRow(int maxRow);

        void clearScreen(); 
        void gotoRow(int row);
        void gotoRowAndUpdate(int* line);
        void gotoXY(int row, int col);
        void gotoEnd();

        void clearCommandFromTerminal();


        void printSeparator();

        void print(string s, PrintType printType, int* line, string prefix = "", Color colorCode = Color::NONE, bool bold = false);

        void printInfo(string s, int* line);
        void printWarning(string s, int* line);
        void printDanger(string s, int* line);
        void printSuccess(string s, int* line);
        void printLine(string s, int* line);
        void printLine(const char* s, int* line);

        void print(string s, Color colorCode, bool bold, int* line);
        void printBold(string s, int* line);
        void printWithPrefix(string s, string prefix, Color colorCode, int* line);
        void printWithRandomPrefixColor(string s, string prefix, int* line);
};