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

namespace my_huffman
{
    /** huffman Tree Node */
    struct huffman_node
    {
        /** Data for each node. positive for leaf node, negative for intermediate node */
        int32_t data;
        /** Node weight */
        int weight;
        /** Left child pointer */
        std::shared_ptr<huffman_node> left;
        /** Right child pointer */
        std::shared_ptr<huffman_node> right;

        /** Constructor */
        huffman_node(int data, int weight, huffman_node *left = NULL, huffman_node *right = NULL)
        : data(data), weight(weight), left(left), right(right)
        {

        }

        /** Copy constructor */
        // huffman_node(const huffman_node &rhs)
        // {
        //     if (&rhs == this) {
        //         return;
        //     }

        //     this->data = rhs.data;
        //     this->weight = rhs.weight;
        //     this->left = rhs.left;
        //     this->right = rhs.right;
        // }

        /** Comparer (for node weight) */
        bool operator<(const huffman_node &rhs) const
        {
            if (this->weight < rhs.weight) {
                return true;
            }
            else if (this->weight == rhs.weight) {
                return !is_data_bigger_then(rhs);
            }
            else {
                return false;
            }
        }

        /** Comparer (for node weight) */
        bool operator>(const huffman_node &rhs) const
        {
            if (this->weight > rhs.weight) {
                return true;
            }
            else if (this->weight == rhs.weight) {
                return is_data_bigger_then(rhs);
            }
            else {
                return false;
            }
        }

        /** if two node have equal weight, compare data value */
        inline bool is_data_bigger_then(const huffman_node &rhs) const
        {
            int l_data = this->data < 0 ? -(this->data) : this->data;
            int r_data = rhs.data < 0 ? -(rhs.data) : rhs.data;

            return (l_data > r_data);
        }
    };

    /** Base class for huffman Encode and Decode */
    class huffman
    {
    protected:
        /** Char to huffman code */
        std::vector< std::vector<uint8_t> > _char_table;
        /** huffman tree root */
        std::shared_ptr<huffman_node> _root;
        /** Input stream */
        const void *_input;
        /** Input stream size */
        uint32_t _file_size;

        /** Turn huffman tree to char table */
        void _build_char_table()
        {
            using namespace std;

            // If root is leaf node (i.e. only one kind of char in input file)
            if (_root->data > 0) {
                _char_table.at(_root->data).push_back(0);

                return;
            }
            /** Left child huffman code */
            vector<uint8_t> l_code;
            /** Right child huffman code */
            vector<uint8_t> r_code;

            l_code.push_back(0);
            r_code.push_back(1);

            _build_char_table(_root->left, l_code);
            _build_char_table(_root->right, r_code);
        }

        /** Turn huffman tree to char table, with recursion */
        void _build_char_table(std::shared_ptr<huffman_node> &current, std::vector<uint8_t> &code)
        {
            using namespace std;

            if (current->data >= 0) {
                _char_table.at(current->data) = code;

                return;
            }

            /** Left child huffman code */
            auto l_code = code;
            /** Right child huffman code */
            auto r_code = code;

            l_code.push_back(0);
            r_code.push_back(1);

            _build_char_table(current->left, l_code);
            _build_char_table(current->right, r_code);
        }

    public:
        /** Constructor */
        huffman(const void *input, size_t size)
        : _input(input), _file_size(size)
        {
            // for each char (0~255)
            _char_table.resize(256);
        }

        /** Destructor */
        // ~huffman()
        // {
        //     using namespace std;

        //     // Delete every huffman node
        //     if (_root != NULL) {
        //         /** Stack for next one to remove */
        //         stack<huffmaconst void **buf, int *buflenn_node *> to_remove;
        //         to_remove.push(_root);

        //         while (!to_remove.empty()) {
        //             /** Current node to remove */
        //             huffman_node *temp = to_remove.top();
        //             to_remove.pop();

        //             // If has child, push into stack for later deletion
        //             if (temp->left != NULL) {
        //                 to_remove.push(temp->left);
        //             }
        //             if (temp->right != NULL) {
        //                 to_remove.push(temp->right);
        //             }

        //             delete temp;
        //         }
        //     }
        // }

        /** Dummy function for derived classes */
        void virtual build_huffman_tree()
        {

        }

        int virtual write(const void **buf, int *buflen) = 0;

        std::vector<uint32_t> write_huffman_code()
        {
            using namespace std;
            
            stack< shared_ptr<huffman_node> > huff_tree;
            huff_tree.push(_root);
            vector<uint32_t> tree;

            while (!huff_tree.empty()) {
                /** next node in post order to write to output stream */
                shared_ptr<huffman_node> current = huff_tree.top();
                huff_tree.pop();

                tree.push_back( htonl( *(reinterpret_cast<uint32_t *>(&current->data)) ) );

                if (current->right != NULL) {
                    huff_tree.push(current->right);
                }
                if (current->left != NULL) {
                    huff_tree.push(current->left);
                }
            }

            return tree;
        }
    };

    /** huffman encode */
    class huffman_encode : public huffman
    {
    private:
        uint8_t *_result;
        size_t _result_size;

    public:
        /** Constructor */
        huffman_encode(const void *input, size_t size)
        : huffman(input, size), _result(NULL), _result_size(0)
        {
            build_huffman_tree();
            _build_char_table();
        }

        ~huffman_encode()
        {
            free(_result);
        }

        /** Build huffman tree from input stream */
        void virtual build_huffman_tree()
        {
            using namespace std;

            /** Count occurrence of every char in input stream */
            int freq[256] = { 0 };

            /** Get char from input stream */
            for (size_t i = 0; i < _file_size; ++i) {
                uint8_t c = *(reinterpret_cast<const uint8_t *>(_input) + i);
                freq[c] += 1;
            }

            /** A min heap queue */
            priority_queue< huffman_node, vector<huffman_node>, greater<huffman_node> > table;

            for (int i = 0; i < 256; ++i) {
                if (freq[i] != 0) {
                    table.push(huffman_node(i, freq[i]));
                }
            }

            // Priority queue guarantees that the top is the least
            // Pop the least two and merge them
            while (table.size() > 1) {
                huffman_node *n1 = new huffman_node(table.top());
                table.pop();

                huffman_node *n2 = new huffman_node(table.top());
                table.pop();

                // Only leaf node have positive data value
                table.push(
                    huffman_node(
                        n1->data < 0 ? n1->data : -(n1->data) - 1,
                        n1->weight + n2->weight,
                        n1,
                        n2
                    )
                );
            }

            _root.reset(new huffman_node(table.top()));
            table.pop();
        }

        /** Write encoded huffman code and header to output stream */
        int virtual write(const void **buf, int *buflen)
        {
            using namespace std;

            if (_result != NULL) {
                *buf = _result;
                *buflen = static_cast<int>(_result_size);
                return 0;
            }

            vector<uint32_t> header = write_huffman_code();
            header.insert(header.begin(), htonl(_file_size));
            
            size_t header_size = header.size() * sizeof (uint32_t);

            _result = (uint8_t *) malloc(_file_size / 2 + header_size);
            if (_result == NULL) {
                *buf = NULL;
                *buflen = 0;
                return -1;
            }
            _result_size = _file_size / 2 + header_size;
            memcpy(_result, &header.front(), header_size);
            memset(_result + header_size, 0, _result_size - header_size);

            unsigned int result_bytes = header_size;
            unsigned int result_bit_offset = 0;

            for (size_t pos = 0; pos < _file_size; ++pos) {
                auto code = _char_table.at(reinterpret_cast<const uint8_t *>(_input)[pos]);
                for (auto bit : code) {
                    _result[result_bytes] |= ( bit << (result_bit_offset) );
                    result_bit_offset += 1;
                    if (result_bit_offset >= 8) {
                        result_bytes += 1;
                        result_bit_offset = 0;
                    }
                }

                if (result_bytes + 256 >= _result_size) {
                    uint8_t *new_result = (uint8_t *) realloc(_result, _result_size + 1024);
                    if (new_result == NULL) {
                        free(_result);
                        _result_size = 0;
                        _result = NULL;
                        *buf = NULL;
                        *buflen = 0;
                        return -1;
                    }
                    memset(new_result + _result_size , 0, 1024);
                    _result = new_result;
                    _result_size += 1024;
                }
            }

            _result_size = (result_bit_offset > 0) ? result_bytes + 1 : result_bytes;
            uint8_t *new_result = (uint8_t *) realloc(_result, _result_size);
            if (new_result == NULL) {
                free(_result);
                _result_size = 0;
                _result = NULL;
                *buf = NULL;
                *buflen = 0;
                return -1;
            }
            _result = new_result;

            *buf = _result;
            *buflen = static_cast<int>(_result_size);

            return 0;
        }
    };

    /** huffman decode */
    class huffman_decode : public huffman
    {
    private:
        uint8_t *_result;
        uint32_t _original_size;
        size_t _pos;

    public:
        /** Constructor */
        huffman_decode(const void *input, size_t size)
        : huffman(input, size), _result(NULL), _original_size(0), _pos(0)
        {
            build_huffman_tree();
        }

        ~huffman_decode()
        {
            free(_result);
        }

        /** Build huffman tree from input stream */
        void virtual build_huffman_tree()
        {
            using namespace std;

            memcpy(&_original_size, _input, sizeof (_original_size));
            _original_size = ntohl(_original_size);
            _pos += sizeof (_original_size);

            /** Read the tree saved in input stream */
            uint32_t u_node_data;
            memcpy(&u_node_data, reinterpret_cast<const uint8_t *>(_input) + _pos, sizeof (u_node_data));
            u_node_data = ntohl(u_node_data);
            _pos += sizeof (u_node_data);

            int32_t node_data = *(reinterpret_cast<int32_t *>(&u_node_data));

            _root.reset(new huffman_node(node_data, 0));

            // In case the root might not have children (i.e. root is leaf node)
            if (node_data < 0) {
                /** For restore tree from post order, without recursion */
                stack< shared_ptr<huffman_node> > huff_tree;
                huff_tree.push(_root);

                while (!huff_tree.empty()) {
                    /** Current tree node */
                    shared_ptr<huffman_node> current = huff_tree.top();

                    memcpy(&u_node_data, reinterpret_cast<const uint8_t *>(_input) + _pos, sizeof (u_node_data));
                    u_node_data = ntohl(u_node_data);
                    _pos += sizeof (u_node_data);

                    node_data = *(reinterpret_cast<int32_t *>(&u_node_data));

                    shared_ptr<huffman_node> new_node(new huffman_node(node_data, 0));

                    // Left child first (post order)
                    if (current->left == NULL) {
                        current->left = new_node;
                    }
                    else {
                        current->right = new_node;
                        // Both children are fulfilled, remove this node
                        huff_tree.pop();
                    }

                    // Not leaf node
                    if (node_data < 0) {
                        huff_tree.push(new_node);
                    }
                }
            }

            _build_char_table();
        }

        int virtual write(const void **buf, int *buflen)
        {
            using namespace std;

            if (_result != NULL) {
                *buf = _result;
                *buflen = static_cast<int>(_original_size);
                return 0;
            }

            size_t pos = _pos;

            _result = (uint8_t *) malloc(_original_size);
            if (_result == NULL) {
                *buf = NULL;
                *buflen = 0;
                return -1;
            }

            shared_ptr<huffman_node> current = _root;
            unsigned int result_bytes = 0;
            unsigned int input_bit_offset = 0;

            while (result_bytes < _original_size) {
                // If current bit is 1, go right
                // Magic, don't touch
                if ( reinterpret_cast<const uint8_t *>(_input)[pos] & (1 << input_bit_offset) ) {
                    current = current->right;
                }
                else {
                    current = current->left;
                }

                if (current == NULL) {
                    free(_result);
                    *buf = NULL;
                    *buflen = 0;
                    return -1;
                }

                // Write char if reached leaf node
                if (current->data >= 0) {
                    _result[result_bytes] = current->data;
                    current = _root;
                    result_bytes += 1;
                }

                input_bit_offset += 1;
                if (input_bit_offset >= 8) {
                    input_bit_offset = 0;
                    pos += 1;
                }
            }

            *buf = _result;
            *buflen = static_cast<int>(_original_size);

            return 0;
        }
    };
};

int main(int argc, char *argv[])
{
    using namespace my_huffman;
    using namespace std;

    if (argc < 4 || (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-u") != 0)) {
        cout << "Usage: " << argv[0] << " <-c|-u> <input> <output>" << endl;
        return 1;
    }

    string pathname_in = argv[2];

    struct stat filestat;
    int status = stat(pathname_in.c_str(), &filestat);
    if (status < 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISREG(filestat.st_mode)) {
        cout << "Not a regular file." << endl;
        return 1;
    }

    off_t filesize = filestat.st_size;
    uint8_t *file_content = new uint8_t[filesize];

    ifstream input(pathname_in, fstream::in | fstream::binary);
    if (!input.is_open()) {
        cout << "Failed to open file." << endl;
        return 1;
    }

    input.read(reinterpret_cast<char *>(file_content), filesize);

    if (strcmp(argv[1], "-c") == 0) {
        huffman_encode encoded(file_content, filesize);
        const void *encoded_content;
        int encoded_len;
        if (encoded.write(&encoded_content, &encoded_len) < 0) {
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
        huffman_decode decoded(file_content, filesize);
        const void *decoded_content;
        int decoded_len;
        if (decoded.write(&decoded_content, &decoded_len) < 0) {
            cout << "Error" << endl;
            return 1;
        }

        ofstream output(argv[3], fstream::out | fstream::binary);
        if (!output.is_open()) {
            cout << "Failed to open outupt file." << endl;
            return 1;
        }
        output.write(reinterpret_cast<const char *>(decoded_content), static_cast<streamsize>(decoded_len));
    }

    delete [] file_content;

    return 0;
}
