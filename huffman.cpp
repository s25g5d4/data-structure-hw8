#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <functional>
#include <memory>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "my_huffman.hpp"

int main(int argc, char *argv[])
{
    using namespace my_huffman;
    using namespace std;

    if (argc < 4 || (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-u") != 0)) {
        cout << "Usage: " << argv[0] << " <-c|-u> <input> <output>" << endl;
        return 1;
    }

    ifstream input(argv[2], fstream::in | fstream::binary);
    if (!input.is_open()) {
        cout << "Failed to open file." << endl;
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        huffman_encode encode(input);
        uint8_t *encoded_content;
        int encoded_len;
        input.seekg(0);
        if (encode.write(input, &encoded_content, &encoded_len) < 0) {
            cout << "Error" << endl;
            return 1;
        }

        ofstream output(argv[3], fstream::out | fstream::binary);
        if (!output.is_open()) {
            cout << "Failed to open outupt file." << endl;
            return 1;
        }
        output.write(reinterpret_cast<const char *>(encoded_content), static_cast<streamsize>(encoded_len));
    }
    else {
        ofstream output(argv[3], fstream::out | fstream::binary);
        if (!output.is_open()) {
            cout << "Failed to open outupt file." << endl;
            return 1;
        }

        huffman_decode decode(input);
        decode.write(output);
    }

    return 0;
}
