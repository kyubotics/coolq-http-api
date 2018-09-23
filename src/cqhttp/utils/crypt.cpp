#include "./crypt.h"

#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>

using namespace std;

namespace cqhttp::utils::crypt {
    string hmac_sha1_hex(const string &key, const string &msg) {
        unsigned char digest[20];
        HMAC_CTX ctx;
        HMAC_CTX_init(&ctx);
        HMAC_Init_ex(&ctx, key.c_str(), key.size(), EVP_sha1(), nullptr);
        HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.size());
        unsigned digest_len;
        HMAC_Final(&ctx, digest, &digest_len);
        HMAC_CTX_cleanup(&ctx);

        stringstream ss;
        for (unsigned i = 0; i < digest_len; ++i) {
            ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(digest[i]);
        }
        return ss.str();
    }

    std::string md5_hash_hex(const std::string &str) {
        constexpr size_t digest_len = 16;
        unsigned char digest[digest_len];
        MD5(reinterpret_cast<const unsigned char *>(str.c_str()), str.size(), digest);
        stringstream ss;
        for (unsigned i = 0; i < digest_len; ++i) {
            ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(digest[i]);
        }
        return ss.str();
    }
} // namespace cqhttp::utils::crypt
