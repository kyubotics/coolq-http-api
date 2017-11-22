#include "./crypt.h"

#include "utils/md5.h"

using namespace std;

string hmac_sha1_hex(const string &key, const string &msg) {
    stringstream hmac_hex_ss;
    return hmac_hex_ss.str();
}

string md5_hash_hex(const string &input) {
    return MD5(input).toStr();
}
