#include "./crypt.h"

#include <openssl/hmac.h>

using namespace std;

namespace cqhttp::utils::crypt {
    string hmac_sha1_hex(const string &key, const string &msg) {
        unsigned digest_len = 20;
        const auto digest = new unsigned char[digest_len];
        HMAC_CTX ctx;
        HMAC_CTX_init(&ctx);
        HMAC_Init_ex(&ctx, key.c_str(), key.size(), EVP_sha1(), nullptr);
        HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.size());
        HMAC_Final(&ctx, digest, &digest_len);
        HMAC_CTX_cleanup(&ctx);

        stringstream ss;
        for (unsigned i = 0; i < digest_len; ++i) {
            ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(digest[i]);
        }
        delete[] digest;
        return ss.str();
    }
} // namespace cqhttp::utils::crypt
