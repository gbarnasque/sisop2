#pragma once

#define RED_TEXT 0
#define GREEN_TEXT 1
#define YELLOW_TEXT 2
#define CYAN_TEXT 3

#include <string>

using namespace std;

class StringUtils {
    private:
        StringUtils() {}
        static string resetText;
        static string redText;
        static string greenText;
        static string yellowText;
        static string cyanText;
        static string boldText;
        
    public:
        static void print(string s, int textColor);
        static void printBold(string s);
};