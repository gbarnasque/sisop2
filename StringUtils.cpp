#include "StringUtils.hpp"
#include <string>
#include <iostream>

using namespace std;

string StringUtils::resetText = "\033[0m";

string StringUtils::redText = "\033[31m[-] " + StringUtils::resetText;
string StringUtils::greenText = "\033[32m[+] " + StringUtils::resetText;
string StringUtils::yellowText = "\033[33m[!] " + StringUtils::resetText;
string StringUtils::cyanText = "\033[96m[~] " + StringUtils::resetText;

string StringUtils::boldText = "\033[1m";

void StringUtils::print(string s, int textColor) {
    string outString = StringUtils::boldText;
    switch (textColor)
    {
        case RED_TEXT:
            outString += StringUtils::redText;
            break;
        case GREEN_TEXT:
            outString += StringUtils::greenText;
            break;
        case YELLOW_TEXT:
            outString += StringUtils::yellowText;
            break;
        case CYAN_TEXT:
            outString += StringUtils::cyanText;
            break;

        default:
            break;
    }
    outString += s + StringUtils::resetText;
    cout << outString << endl;
}

void StringUtils::printBold(string s) {
    cout << StringUtils::boldText << s << StringUtils::resetText << std::endl;
}