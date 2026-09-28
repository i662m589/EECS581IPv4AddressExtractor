/*
Author: Isaac Miller
Date Created: 9/24/2026
Last Modified: 9/27/2026
Summary: Program that takes in user's string, looks for IP address and port number, and outputs it back to the user
Tools used: Claude (Sonnet 5)
*/
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h> //These imports will be used throughout the remainder of the program

int extractIPv4(const char* str, unsigned long* outAddress, int* outPort) { //This method will parse for and look for a valid IP address and port number using the string that's provided
    const char* p = str;

    while (*p != '\0') { //This while loop checks if the pointer has hit the end of the string or not.
        const char* start = p;

        if (start != str) {
            char prevChar = *(start - 1);
            if (isdigit((unsigned char)prevChar) || prevChar == '.') {
                p++;
                continue; //This block makes sure that the current match doesn't start with a '.' or a digit by checking if the previous character before the match has a '.' or a digit.
            }
        }

        int octets[4]; //This will store the IPv4 address each octet
        int i;
        const char* cursor = p; //Cursor will store the current character that's being parsed.

        for (i = 0; i < 4; i++) { //This for loop goes through and forms the IP address and stores it in the octet[] variable
            if (i > 0) {
                if (*cursor != '.') break;
                cursor++; //This acts as a seperator check between each octet. If the current character isn't a dot, then the match is malformed and abandoned.
            }

            if (!isdigit((unsigned char)*cursor)) break; //If the current character isn't a digit, the match is abandoned

            bool leadingZero = (*cursor == '0'); //This will be used later to check if there's a leading zero before a digit
            int value = 0; //This will eventually contain the value that will be stored in each octet
            int digits = 0; //This stores how many digits are currently in a given octet
            while (isdigit((unsigned char)*cursor) && digits < 3) {
                value = value * 10 + (*cursor - '0');
                cursor++;
                digits++; //If there is a digit and there aren't more than 3 digits, the value of the character will be stored in value and multiplied by 10 for future addition of digits.
                          //Meanwhile, cursor will be incremented to go to the next character and digits will be incremented to see how many digits are in the current octet.
            }
            if (value > 255) break; //Since an octet can't be higher than 255, this will check if value exceeds that. It will abandon the match if it does exceed 255
            if (leadingZero && digits > 1) break; //If there's a leading zero and the zero isn't the singular value, the match will be abandoned. 
            octets[i] = value; //If all of the previous checks have been passed, the octet value will be inputted into its respective octet space.  
        }

        if (i == 4) { //Once a valid octet has been found, the following block will be run.
            /* Found a valid dotted-quad; make sure it isn't glued to
               more digits (e.g. "1.2.3.4567" should not match as
               "1.2.3.456") or followed by another dot (e.g.
               "192.168.1.1." or "1.2.3.4.5" should not match). */
            if (!isdigit((unsigned char)*cursor) && *cursor != '.') { //This checks to see if there are any glued digits or '.'s and will abandon the match if there is either. 
                unsigned long address = ((unsigned long)octets[0] << 24) |
                                         ((unsigned long)octets[1] << 16) |
                                         ((unsigned long)octets[2] << 8)  |
                                          (unsigned long)octets[3]; //Using bitwise math, this will find store the octet/IP address as a decimal value in address

                int port = -1;
                bool portAttached = (*cursor == ':'); 
                bool portValid = true; //If there is a ':' after the IP address, the program will start checking the values after the colon to check for a port number

                if (portAttached) {
                    const char* portStart = cursor + 1;
                    bool portLeadingZero = (*portStart == '0');
                    int value = 0;
                    int digits = 0; //Similar to the octets, this will check for a leading zero, the value of the number, and how many digits the port number has currently
                    while (isdigit((unsigned char)*portStart) && digits < 6) {
                        value = value * 10 + (*portStart - '0');
                        portStart++;
                        digits++; //If there is a digit and there aren't more than 3 digits, the value of the character will be stored in value and multiplied by 10 for future addition of digits.
                                  //Meanwhile, cursor will be incremented to go to the next character and digits will be incremented to see how many digits are in the current port number.
                    }

                    if (digits == 0 || digits > 5 || value > 65535 ||
                        (portLeadingZero && digits > 1) ||
                        isdigit((unsigned char)*portStart)) { //This checks if there are no digits, if there's too many digits, if the port number's value has exceeded 65536,
                        portValid = false;                    //if there is a leading zero in the port number, and double checks if the port number is glued to any digits. If so, the match is abandoned.
                    } else {
                        port = value; //If the previous checks were met, then the port number is given its value.
                    }
                }

                if (!portAttached || portValid) {
                    *outAddress = address;
                    *outPort = port;
                    return 1; //If there is no port number attached or its a valid number, outAddress and outPort are given their values and the function ends with a found IP address and port number. Otherwise, the match is abandoned.
                }
            }
        }

        p = start + 1; //If a match has been abandoned, this will iterate the pointer and start looking for another match
    }

    return 0; //This indicates that there was no matches throughout the string, and that nothing was found. 
}

int main() {
    char input[100];
    bool running = true; //Input continuously stores user input throughout runtime, and running keeps the program running until "END" is inputted

    while (running) {
        printf("Enter a string (or 'END' to quit): ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input: no valid IPv4 address found\n"); //This will ask the user to input the string, take the user input, and store it in input. If the input is blank, this edge case will print.
            continue;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') { //This strips any trailing newline that fgets() leaves if it does leave a new line. 
            input[len - 1] = '\0';
        }

        if (strcmp(input, "END") == 0) {
            running = false; //If END is inputted, the while loop is ended, therefore ending the program
        } else {
            unsigned long outAddress;
            int outPort; //These will respectively store the IP address and port numbers in extractIPv4() if they are present

            if (extractIPv4(input, &outAddress, &outPort) == 1) {
                /* Reconstruct the dotted-quad text from the decimal
                   value, rather than echoing the whole input line. */
                char ipText[16];
                snprintf(ipText, sizeof(ipText), "%lu.%lu.%lu.%lu",
                          (outAddress >> 24) & 0xFF,
                          (outAddress >> 16) & 0xFF,
                          (outAddress >> 8)  & 0xFF,
                           outAddress        & 0xFF); //Using decimal to octet bit-wise functions, this will take the IP address (which is in decimal form) and turn it into it's octet form.

                if (outPort != -1) {
                    printf("Extracted IPv4 address: %s (decimal value: %lu, port: %d)\n",
                           ipText, outAddress, outPort); //If there is a port number, this will print the IP address, decimal value, and Port number
                } else {
                    printf("Extracted IPv4 address: %s (decimal value: %lu, port: none)\n",
                           ipText, outAddress); //If there is not a port number, this will print the IP address and its decimal value
                }
            } else {
                printf("Invalid input: no valid IPv4 address found\n"); //If there isn't a valid address in the string, this will state that one wasn't found. 
            }
        }
    }

    printf("Program terminated.\n"); //This indicates that "END" was inputted, and that the program has ended.
    return 0;
}