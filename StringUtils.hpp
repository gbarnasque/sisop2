#pragma once

#define RED 0
#define GREEN 1
#define YELLOW 2
#define CYAN 3
#define BLUE 4
#define MAGENTA 5
#define WHITE 6
#define GRAY 7
#define BRIGHT_RED 8
#define BRIGHT_GREEN 9
#define BRIGHT_YELLOW 10
#define BRIGHT_BLUE 11
#define BRIGHT_MAGENTA 12
#define BRIGHT_CYAN 13
#define BRIGHT_GRAY 14

#include <string>

using namespace std;

class StringUtils {
    private:
        StringUtils() {}
        static string _resetText;
        static string _bold;
        static string _red;
        static string _green;
        static string _yellow;
        static string _cyan;
        static string _blue;
        static string _magenta;
        static string _white;
        static string _gray;
        static string _bright_red;
        static string _bright_green;
        static string _bright_yellow;
        static string _bright_blue;
        static string _bright_magenta;
        static string _bright_cyan;
        static string _bright_gray;

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

        static void testColors();
};