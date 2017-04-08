/**
* purpose: Huffman Encoding compression/uncompression
* name: 吳宗哲 B023040011
* Date: 2015/01/06
*/

#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <functional>

namespace MyHuffman
{
    /** Huffman Tree Node */
    struct HuffmanNode
    {
        /** Data for each node. positive for leaf node, negative for intermediate node */
        int data;
        /** Node weight */
        int weight;
        /** Left child pointer */
        HuffmanNode *left;
        /** Right child pointer */
        HuffmanNode *right;

        /** Constructor */
        HuffmanNode(int data, int weight, HuffmanNode *left = NULL, HuffmanNode *right = NULL)
        {
            this->data = data;
            this->weight = weight;
            this->left = left;
            this->right = right;
        }

        /** Copy constructor */
        HuffmanNode(const HuffmanNode &rhs)
        {
            if (&rhs == this) {
                return;
            }

            this->data = rhs.data;
            this->weight = rhs.weight;
            this->left = rhs.left;
            this->right = rhs.right;
        }

        /** Comparer (for node weight) */
        bool operator<(const HuffmanNode &rhs) const
        {
            if (this->weight == rhs.weight) {
                return !isDataBiggerThan(rhs);
            }
            else {
                return (this->weight < rhs.weight);
            }
        }

        /** Comparer (for node weight) */
        bool operator>(const HuffmanNode &rhs) const
        {
            if (this->weight == rhs.weight) {
                return isDataBiggerThan(rhs);
            }
            else {
                return (this->weight > rhs.weight);
            }
        }

        /** if two node have equal weight, compare data value */
        inline bool isDataBiggerThan(const HuffmanNode &rhs) const
        {
            int l_data = this->data < 0 ? -(this->data) : this->data;
            int r_data = rhs.data < 0 ? -(rhs.data) : rhs.data;

            return (l_data > r_data);
        }
    };

    /** Base class for Huffman Encode and Decode */
    class Huffman
    {
    protected:
        /** Char to huffman code */
        std::vector< std::vector<bool> > _char_table;
        /** Huffman tree root */
        HuffmanNode *_root;
        /** Input stream */
        std::istream *_input;
        /** Input stream size */
        unsigned int _file_size;

        /** Turn Huffman tree to char table */
        void _buildCharTable(const HuffmanNode *root)
        {
            using namespace std;

            // If root is leaf node (i.e. only one kind of char in input file)
            if (root->data > 0) {
                _char_table.at(root->data).push_back(0);

                return;
            }
            /** Left child huffman code */
            vector<bool> l_code;
            /** Right child huffman code */
            vector<bool> r_code;

            l_code.push_back(0);
            r_code.push_back(1);

            _buildCharTable(root->left, l_code);
            _buildCharTable(root->right, r_code);
        }

        /** Turn Huffman tree to char table, with recursion */
        void _buildCharTable(const HuffmanNode *root, std::vector<bool> code)
        {
            using namespace std;

            if (root->data >= 0) {
                _char_table.at(root->data) = code;

                return;
            }

            /** Left child huffman code */
            vector<bool> l_code = code;
            /** Right child huffman code */
            vector<bool> r_code = code;

            l_code.push_back(0);
            r_code.push_back(1);

            _buildCharTable(root->left, l_code);
            _buildCharTable(root->right, r_code);
        }

    public:
        /** Constructor */
        Huffman(std::istream &input)
        {
            // for each char (0~255)
            _char_table.resize(256);
            _input = &input;
        }

        /** Destructor */
        ~Huffman()
        {
            using namespace std;

            // Delete every Huffman node
            if (_root != NULL) {
                /** Stack for next one to remove */
                stack<HuffmanNode *> to_remove;
                to_remove.push(_root);

                while (!to_remove.empty()) {
                    /** Current node to remove */
                    HuffmanNode *temp = to_remove.top();
                    to_remove.pop();

                    // If has child, push into stack for later deletion
                    if (temp->left != NULL) {
                        to_remove.push(temp->left);
                    }
                    if (temp->right != NULL) {
                        to_remove.push(temp->right);
                    }

                    delete temp;
                }
            }
        }

        /** Print all huffman code */
        void printHuffmanTable()
        {
            using namespace std;

            /** Iterator for char table */
            vector< vector<bool> >::iterator it;
            for (it = _char_table.begin(); it != _char_table.end(); ++it) {
                if (it->size() == 0) {
                    continue;
                }

                // Print char
                cout << static_cast<char>(it - _char_table.begin()) << ": ";
                // Print huffman code
                for (unsigned int i = 0; i < it->size(); ++i) {
                    cout << static_cast<int>(it->at(i));
                }
                cout << endl;
            }
        }

        /** Dummy function for derived classes */
        void virtual buildHuffmanTable(std::istream &input)
        {

        }

        /** Dummy function for derived classes */
        void virtual write(std::ostream &output)
        {

        }
    };

    /** Huffman encode */
    class HuffmanEncode : public Huffman
    {
    public:
        /** Constructor */
        HuffmanEncode(std::istream &input) : Huffman(input)
        {
            buildHuffmanTable(input);
        }

        /** Build Huffman tree from input stream */
        void virtual buildHuffmanTable(std::istream &input)
        {
            using namespace std;

            /** Count occurrence of every char in input stream */
            int freq[256] = { 0 };

            /** Get char from input stream */
            int c = input.get();
            while (!input.eof()) {
                ++freq[c];
                c = input.get();
            }

            // Clean flags (such as 'eofbit') for 'tellg()' to function
            input.clear();
            // Since the stream has reached the end, the position is file size
            _file_size = input.tellg();

            /** A min heap queue */
            priority_queue<HuffmanNode, vector<HuffmanNode>, greater<HuffmanNode> > table;

            for (int i = 0; i < 256; ++i) {
                if (freq[i] == 0) {
                    continue;
                }

                table.push(HuffmanNode(i, freq[i]));
            }

            // Priority queue guarantees that the top is the least
            // Pop the least two and merge them
            while (table.size() > 1) {
                HuffmanNode *n1 = new HuffmanNode(table.top());
                table.pop();

                HuffmanNode *n2 = new HuffmanNode(table.top());
                table.pop();

                // Only leaf node have positive data value
                table.push(HuffmanNode(n1->data < 0 ? n1->data : -(n1->data) - 1, n1->weight + n2->weight, n1, n2));
            }

            _root = new HuffmanNode(table.top());
            table.pop();

            _buildCharTable(_root);
        }

        /** Write encoded huffman code and header to output stream */
        void virtual write(std::ostream &output)
        {
            using namespace std;

            // Rewind to the beginning of file
            _input->clear();
            _input->seekg(0, ios::beg);

            output.write(reinterpret_cast<char *>(&_file_size), sizeof(int));

            // Write huffman codebook using post order
            /** No recursion, just loop! */
            stack<HuffmanNode *> huff_tree;
            huff_tree.push(_root);

            while (!huff_tree.empty()) {
                /** next node in post order to write to output stream */
                HuffmanNode *current = huff_tree.top();
                huff_tree.pop();

                output.write(reinterpret_cast<char *>(&(current->data)), sizeof(int));

                if (current->right != NULL) {
                    huff_tree.push(current->right);
                }
                if (current->left != NULL) {
                    huff_tree.push(current->left);
                }
            }

            /** Buffer for writing bits */
            char buf[64] = { 0 };
            /** Current write position */
            int pos = 0;

            /** Char to encode */
            int ch = _input->get();
            while (!_input->eof()) {
                /** Iterator for vector<bool> */
                vector<bool>::iterator it;

                for (it = _char_table.at(ch).begin(); it != _char_table.at(ch).end(); ++it) {
                    // Magic, don't touch
                    buf[pos / 8] |= ((*it) << (pos % 8));
                    ++pos;
                }

                // The tree height is less than 256 (i.e. the code is less than 256 bits)
                // It is safe to write 256 bits for each time
                if (pos >= 256) {
                    output.write(buf, 32);

                    memmove(buf, buf + 32, pos / 8 + (pos % 8 == 0 ? 0 : 1));
                    memset(buf + 32, 0, 32);

                    pos -= 256;
                }

                ch = _input->get();
            }

            // In case there are still some trailing bits
            output.write(buf, pos / 8 + (pos % 8 == 0 ? 0 : 1));
        }
    };

    /** Huffman decode */
    class HuffmanDecode : public Huffman
    {
    private:
        int _header_offset; /** Where data starts */

    public:
        /** Constructor */
        HuffmanDecode(std::istream &input) : Huffman(input)
        {
            buildHuffmanTree(input);
        }

        /** Build Huffman tree from input stream */
        void virtual buildHuffmanTree(std::istream &input)
        {
            using namespace std;

            _input->read(reinterpret_cast<char *>(&_file_size), sizeof(int));

            /** Read the tree saved in input stream */
            int tree_data;
            _input->read(reinterpret_cast<char *>(&tree_data), sizeof(int));

            _root = new HuffmanNode(tree_data, 0);

            // In case the root might not have children (i.e. root is leaf node)
            if (tree_data < 0) {
                /** For restore tree from post order, without recursion */
                stack<HuffmanNode *> huff_tree;
                huff_tree.push(_root);

                while (!huff_tree.empty()) {
                    /** Current tree node */
                    HuffmanNode *current = huff_tree.top();

                    _input->read(reinterpret_cast<char *>(&tree_data), sizeof(int));

                    HuffmanNode *new_node = new HuffmanNode(tree_data, 0);

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
                    if (tree_data < 0) {
                        huff_tree.push(new_node);
                    }
                }
            }

            _header_offset = _input->tellg();

            _buildCharTable(_root);
        }

        /** Write decoded data to output stream */
        void virtual write(std::ostream &output)
        {
            using namespace std;

            _input->clear();
            _input->seekg(_header_offset);

            /** Buffer for decoded char */
            char buf[32];
            /** Tree tranversal */
            HuffmanNode *current = _root;
            /** Current position (in bits) */
            int pos = 0;
            /** Number of char wrote */
            int char_count = 0;

            while (char_count < _file_size) {
                if ((pos % 256) == 0) {
                    _input->read(buf, 32);
                }

                // If current bit is 1, go right
                // Magic, don't touch
                if (buf[(pos % 256) / 8] & (1 << (pos % 8))) {
                    if (current->right != NULL) {
                        current = current->right;
                    }
                }
                else {
                    if (current->left != NULL) {
                        current = current->left;
                    }
                }

                // Write char if reached leaf node
                if (current->data >= 0) {
                    output.put(static_cast<char>(current->data));

                    current = _root;
                    ++char_count;
                }

                ++pos;
            }
        }
    };
};

int main(int argc, char *argv[])
{
    using namespace MyHuffman;
    using namespace std;

    /** Do compress or uncompress */
    bool compress = false;
    /** input file name */
    const char *input_file = NULL;
    /** output file name */
    const char *output_file = NULL;

    if (argc <= 5) {
        cout << "Usage: " << argv[0] << " (-c|-u) -i inputfile -o outputfile" << endl;

        return 0;
    }
    else {

        for (int i = 1; i < argc; ++i) {
            switch (argv[i][1]) {
            case 'c':
                compress = true;
                break;

            case 'u':
                compress = false;
                break;

            case 'i':
                ++i;
                input_file = argv[i];
                break;

            case 'o':
                ++i;
                output_file = argv[i];
                break;

            default:
                break;
            }
        }

        if (input_file == NULL || output_file == NULL) {
            cout << "Usage: " << argv[0] << " (-c|-u) -i inputfile -o outputfile" << endl;

            return 0;
        }
    }

    ifstream f_in(input_file, ios::in | ios::binary);
    ofstream f_out(output_file, ios::out | ios::binary);

    if (f_in.fail() | f_out.fail()) {
        cout << "Fail to open file." << endl;

        return 0;
    }

    /** Don't know whether compress or uncompress, use base class */
    Huffman *huffman;

    // Polymorphism
    if (compress) {
        huffman = new HuffmanEncode(f_in);
    }
    else {
        huffman = new HuffmanDecode(f_in);
    }

    huffman->write(f_out);

    // get file size
    f_in.clear();
    f_in.seekg(0, ios::end);

    f_out.clear();
    f_out.seekp(0, ios::end);

    /** compressed file size */
    int compressed_size = (compress ? f_out.tellp() : f_in.tellg());
    /** original uncompressed file size */
    int original_size = (compress ? f_in.tellg() : f_out.tellp());

    cout << "original: ";
    cout << original_size << endl;

    cout << "compressed: ";
    cout << compressed_size << endl;

    cout << "compression ratio: ";
    cout << static_cast<double>(compressed_size) / static_cast<double>(original_size) << endl;

    cout << "Huffman Table:" << endl;
    huffman->printHuffmanTable();

    delete huffman;

    return 0;
}
