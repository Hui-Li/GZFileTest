#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <zlib.h>

// write line by line
void compress_string() {

    // 9 is the level
    // http://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/zlib-gzopen-1.html
    gzFile fi = gzopen("file.gz", "wb9");
    gzprintf(fi, "This is a string\n");
    gzprintf(fi, "This is another string\n");
    gzclose(fi);

}

/* Reads a file into memory. */
bool loadBinaryFile(const std::string &filename, std::string &contents) {
    // Open the gzip file in binary mode
    FILE *f = fopen(filename.c_str(), "rb");
    if (f == NULL)
        return false;

    // Clear existing bytes in output vector
    contents.clear();

    // Read all the bytes in the file
    int c = fgetc(f);
    while (c != EOF) {
        contents += (char) c;
        c = fgetc(f);
    }
    fclose(f);

    return true;
}

bool gzipInflate(const std::string &compressedBytes, std::string &uncompressedBytes) {
    if (compressedBytes.size() == 0) {
        uncompressedBytes = compressedBytes;
        return true;
    }

    uncompressedBytes.clear();

    unsigned full_length = compressedBytes.size();
    unsigned half_length = compressedBytes.size() / 2;

    unsigned uncompLength = full_length;
    char *uncomp = (char *) calloc(sizeof(char), uncompLength);

    z_stream strm;
    strm.next_in = (Bytef *) compressedBytes.c_str();
    strm.avail_in = compressedBytes.size();
    strm.total_out = 0;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;

    bool done = false;

    if (inflateInit2(&strm, (16 + MAX_WBITS)) != Z_OK) {
        free(uncomp);
        return false;
    }

    while (!done) {
        // If our output buffer is too small
        if (strm.total_out >= uncompLength) {
            // Increase size of output buffer
            char *uncomp2 = (char *) calloc(sizeof(char), uncompLength + half_length);
            memcpy(uncomp2, uncomp, uncompLength);
            uncompLength += half_length;
            free(uncomp);
            uncomp = uncomp2;
        }

        strm.next_out = (Bytef *) (uncomp + strm.total_out);
        strm.avail_out = uncompLength - strm.total_out;

        // Inflate another chunk.
        int err = inflate(&strm, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) done = true;
        else if (err != Z_OK) {
            break;
        }
    }

    if (inflateEnd(&strm) != Z_OK) {
        free(uncomp);
        return false;
    }

    for (size_t i = 0; i < strm.total_out; ++i) {
        uncompressedBytes += uncomp[i];
    }
    free(uncomp);
    return true;
}

void read_string_test() {

    // Read using Boost
    std::cout << "Read by Boost: " << std::endl;
    std::ifstream file("file.gz", std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    for (std::string str; std::getline(in, str);) {
        std::cout << "Processed line: " << str << std::endl;
    }

    // Read using zlib
    std::cout << "Read by zlib: " << std::endl;
    std::string fileData;
    if (!loadBinaryFile("file.gz", fileData)) {
        printf("Error loading input file.");
        return;
    }

    // Uncompress the file data
    std::string data;
    if (!gzipInflate(fileData, data)) {
        printf("Error decompressing file.");
        return;
    }

    std::stringstream ss(data.c_str());
    std::string line;

    while (std::getline(ss, line, '\n')) {
        std::cout << "Processed line: " << line << std::endl;
    }
}

void compress_binary() {
    // write binary file
    double *d = new double[5];
    int *i = new int[5];

    for (int j = 0; j < 5; j++) {
        d[j] = j + 0.1;
        i[j] = j;
    }

    gzFile fb = gzopen("file2.gz", "wb9");

    gzwrite(fb, d, sizeof(double) * 5);
    gzwrite(fb, i, sizeof(int) * 5);

    gzclose(fb);

    delete[] d;
    delete[] i;
}

void read_binary_test() {
    // manually uncompress file2.gz and then test the following code...
    std::ifstream data_file("file2");

    int *int_for_double = new int[5 * sizeof(double)];

    data_file.read(reinterpret_cast<char *>(int_for_double), sizeof(double) * 5);
    double *double_val = reinterpret_cast<double *>(int_for_double);

    for (int j = 0; j < 5; j++) {
        std::cout << double_val[j] << std::endl;
    }

    int *int_val = new int[5];
    data_file.read(reinterpret_cast<char *>(int_val), sizeof(int) * 5);

    for (int j = 0; j < 5; j++) {
        std::cout << int_val[j] << std::endl;
    }

    delete[] int_for_double;
    delete[] int_val;

    data_file.close();

}

int main() {

    compress_string();
    read_string_test();

    compress_binary();

    // manually uncompress file2.gz and then test the following code...
//    read_binary_test();
}
