/*
 * Author(s)		: [Collins Nji] <collins.geek@gmail.com> 
 * Organisation		: Nerd Inc.
 * App Name			: Cassie
 * Version			: 1.0 [beta]
 * Detail			: Implementation of LZW algorithm and
 * Compile: gcc -Wall -Wextra -c -std=c++11 
 * 
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <limits>
#include <exception>

using namespace std;
using cassie = uint16_t;

namespace grimm {
	const cassie dictionary_max_size {
		numeric_limits <cassie>::max()
	};
}
// Print usage information
void print_usage(const string &custom_error = "", bool show_usage_info = true)
{
    if (!custom_error.empty())
        cout << "\n ERROR: " << custom_error << endl;

    if (show_usage_info)
    {
        cout << "\n Usage:" <<endl;
        cout << "\t program -flag input output" <<endl;
        cout << "Where `flag` is either `-c` for compression, or `-d` for decompression, and" <<endl;
        cout << "`input` and `output` are distinct files." <<endl;
        cout << "Examples: " <<endl;
        cout << "\t ./cassie -c sample_file.txt output_file.grm" <<endl;
        cout << "\t ./cassie -d output_file.grm sample_file.txt" <<endl;
    }

    cout << endl;
}
void help_menu (){
	cout << "cassie \t\t Nerd \t\t cassie" <<endl;
	cout << "Usage:" << endl;
	cout << "\t ./cassie -flag in_file out_file" <<endl;
	cout << "\t ./cassie [-c] [-d] [-h | --help] `in_file.[extension]` out_file.grm " <<endl;
	cout << "\t\t  [-c] flag for compressing the \'in_file\' to \'out_file\'" <<endl;
	cout << "\t\t  [-d] flag for decompressing the \'out_file\' to \'in_file\'" <<endl;
	cout << "\t\t  [-h | --help] flag to show this message and exit" <<endl;

	}
	
vector<char> operator + (vector<char> vc, char c)
{
    vc.push_back(c);
    return vc;
}

// Compression Function.
void compress(istream &is, ostream &os)
{
    map<vector<char>, cassie> dictionary;

    // lambda function, used to reset the dictionary to its initial contents
    const auto dict_reset = [&dictionary] {
        dictionary.clear();

        const long int minc = numeric_limits<char>::min();
        const long int maxc = numeric_limits<char>::max();

        for (long int c = minc; c <= maxc; ++c)
        {
            const cassie dictionary_size = dictionary.size();

            dictionary[{static_cast<char> (c)}] = dictionary_size;
        }
    };

		dict_reset();

    vector<char> s; 
    char c;

    while (is.get(c))
    {
        if (dictionary.size() == grimm::dictionary_max_size)
            dict_reset();

        s.push_back(c);

        if (dictionary.count(s) == 0)
        {
            const cassie dictionary_size = dictionary.size();

            dictionary[s] = dictionary_size;
            s.pop_back();
            os.write(reinterpret_cast<const char *> (&dictionary.at(s)), sizeof (cassie));
            s = {c};
        }
    }

    if (!s.empty())
        os.write(reinterpret_cast<const char *> (&dictionary.at(s)), sizeof (cassie));
}

// Decompression Function
void decompress(istream &is, ostream &os)
{
    vector<vector<char> > dictionary;
    const auto dict_reset = [&dictionary] {
        dictionary.clear();
        dictionary.reserve(grimm::dictionary_max_size);

        const long int minc = numeric_limits<char>::min();
        const long int maxc = numeric_limits<char>::max();

        for (long int c = minc; c <= maxc; ++c)
            dictionary.push_back({static_cast<char> (c)});
    };

		dict_reset();

    vector<char> s; // empty string
    cassie k; // code key

    while (is.read(reinterpret_cast<char *> (&k), sizeof (cassie)))
    {
        if (dictionary.size() == grimm::dictionary_max_size)
            dict_reset();

        if (k > dictionary.size())
            throw runtime_error("invalid compressed code");

        if (k == dictionary.size())
            dictionary.push_back(s + s.front());
        else
        if (!s.empty())
            dictionary.push_back(s + dictionary.at(k).front());

        os.write(&dictionary.at(k).front(), dictionary.at(k).size());
        s = dictionary.at(k);
    }

    if (!is.eof() || is.gcount() != 0)
        throw runtime_error("corrupted compressed file");
}

// return EXIT_FAILURE    failed
// return EXIT_SUCCESS    success
int main(int argc, char *argv[])
{	
	if (argc != 4)
    {
        print_usage("Wrong number of arguments.");
        return EXIT_FAILURE;
    }

    enum class Flag {
        Compress,
        Decompress
    };

    Flag flag_state;
    
	 if (string(argv[1]) == "-h" || string(argv[1]) == "--help")
		help_menu();
		
    if (string(argv[1]) == "-c")
        flag_state = Flag::Compress;
        
    else if (string(argv[1]) == "-d")
        flag_state = Flag::Decompress;
    else
    {
        print_usage(string("flag `") + argv[1] + "` is not recognized.");
        return EXIT_FAILURE;
    }

    ifstream input_file(argv[2], ios_base::binary);

    if (!input_file)
    {
        print_usage(string("the file `") + argv[2] + "` could not be opened for input.");
        return EXIT_FAILURE;
    }

    ofstream output_file(argv[3], ios_base::binary);

    if (!output_file.is_open())
    {
        print_usage(string("the file `") + argv[3] + "` could not be opened for output.");
        return EXIT_FAILURE;
    }

    try
    {
        input_file.exceptions(ios_base::badbit);
        output_file.exceptions(ios_base::badbit | ios_base::failbit);

        if (flag_state == Flag::Compress)
            compress(input_file, output_file);
        else
        if (flag_state == Flag::Decompress)
            decompress(input_file, output_file);
    }
    catch (const ios_base::failure &f)
    {
        print_usage(string("File input/output failure: ") + f.what() + '.', false);
        return EXIT_FAILURE;
    }
    catch (const exception &e)
    {
        print_usage(string("Caught exception: ") + e.what() + '.', false);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
