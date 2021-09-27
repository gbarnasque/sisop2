#include "StringUtils.hpp"
#include <string>
#include <iostream>
#include <string.h>

using namespace std;

string StringUtils::_resetText = "\033[0m ";
string StringUtils::_bold = "\033[1m";

string StringUtils::_red = "\033[31m";
string StringUtils::_green = "\033[32m";
string StringUtils::_yellow = "\033[33m";
string StringUtils::_blue = "\033[34m";
string StringUtils::_magenta = "\033[35m";
string StringUtils::_cyan = "\033[36m";
string StringUtils::_bright_gray = "\033[37m";
string StringUtils::_gray = "\033[90m";
string StringUtils::_bright_red = "\033[91m";
string StringUtils::_bright_green = "\033[92m";
string StringUtils::_bright_yellow = "\033[93m";
string StringUtils::_bright_blue = "\033[94m";
string StringUtils::_bright_magenta = "\033[95m";
string StringUtils::_bright_cyan = "\033[96m";
string StringUtils::_white = "\033[97m";

string StringUtils::getColorString(Color colorCode) {
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
    case BLUE:
        color = _blue;
        break;
    case MAGENTA:
        color = _magenta;
        break;
    case WHITE:
        color = _white;
        break;
    case GRAY:
        color = _gray;
        break;
    case BRIGHT_RED:
        color = _bright_red;
        break;
    case BRIGHT_GREEN:
        color = _bright_green;
        break;
    case BRIGHT_YELLOW:
        color = _bright_yellow;
        break;
    case BRIGHT_BLUE:
        color = _bright_blue;
        break;
    case BRIGHT_MAGENTA:
        color = _bright_magenta;
        break;
    case BRIGHT_CYAN:
        color = _bright_cyan;
        break;
    default:
        color = "";
        break;
    }
    return color;
}

void StringUtils::print(string s, Color colorCode, bool bold) {
    string outString = (bold) ? _bold : "";

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

void StringUtils::printWithPrefix(string s, string prefix, Color colorCode) {
    string outString = StringUtils::_bold;
    outString += getColorString(colorCode);
    outString += prefix;
    outString += _resetText;
    outString += s;
    cout << outString << endl;
}

void StringUtils::printLine(string s) {
    cout << s << endl;
}

void StringUtils::printLine(char* s) {
    cout << s << endl;
}

string StringUtils::removeNewLineAtEnd(string s) {
    s[s.length() - 1] = '\0';
    return s;
}

char* StringUtils::removeNewLineAtEnd(char* s) {
    s[strlen(s) - 1] = '\0';
    return s;
}

void StringUtils::testColors() {
    for(int i=Color::RED; i != Color::NONE; i++) {
        printWithPrefix("teste", "[!]", static_cast<Color>(i));
    }
}