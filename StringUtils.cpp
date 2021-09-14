#include "StringUtils.hpp"
#include <string>
#include <iostream>
#include <string.h>

using namespace std;

string StringUtils::_resetText = "\033[0m ";

string StringUtils::_red = "\033[31m";
string StringUtils::_green = "\033[32m";
string StringUtils::_yellow = "\033[33m";
string StringUtils::_cyan = "\033[96m";

string StringUtils::_bold = "\033[1m";

string StringUtils::getColorString(int colorCode) {
    std::string color;
    switch (colorCode)
    {
        case RED:
            color = _red;
            break;
        case GREEN:
            color = _green;
            break;
        case YELLOW:
            color = _yellow;
            break;
        case CYAN:
            color = _cyan;
            break;
        default:
            color = "";
            break;
    }
    return color;
}

void StringUtils::print(string s, int colorCode) {
    string outString = _bold;

    outString += getColorString(colorCode);

    outString += s + _resetText;
    cout << outString << endl;
}

void StringUtils::printBold(string s) {
    cout << _bold << s << _resetText << endl;
}

void StringUtils::printInfo(string s) {
    printWithPrefix(s, "[~]", CYAN);
}

void StringUtils::printWarning(string s) {
    printWithPrefix(s, "[!]", YELLOW);
}

void StringUtils::printDanger(string s) {
    printWithPrefix(s, "[-]", RED);
}

void StringUtils::printSuccess(string s) {
    printWithPrefix(s, "[+]", GREEN);
}

void StringUtils::printWithPrefix(string s, string prefix, int colorCode) {
    string outString = StringUtils::_bold;
    outString += getColorString(colorCode);
    outString += prefix;
    outString += _resetText;
    outString += s;
    cout << outString << endl;
}

string StringUtils::removeNewLineAtEnd(string s) {
    s[s.length() - 1] = '\0';
    return s;
}

char* StringUtils::removeNewLineAtEnd(char* s) {
    s[strlen(s) - 1] = '\0';
    return s;
}

