#pragma once

#define RED 0
#define GREEN 1
#define YELLOW 2
#define CYAN 3

#include <string>

using namespace std;

class StringUtils {
    private:
        StringUtils() {}
        static string _resetText;
        static string _red;
        static string _green;
        static string _yellow;
        static string _cyan;
        static string _bold;

        static string getColorString(int colorCode);
        
    public:
        static void print(string s, int colorCode);
        static void printBold(string s);
        static void printWithPrefix(string s, string prefix, int colorCode);

        static void printInfo(string s);
        static void printWarning(string s);
        static void printDanger(string s);
        static void printSuccess(string s);
        static void printLine(string s);
        static void printLine(char* s);

        static string removeNewLineAtEnd(string s);
        static char* removeNewLineAtEnd(char* s);
};