/**
* @file md5.cpp
* @The implement of md5.
* @author Jiewei Wei
* @mail weijieweijerry@163.com
* @github https://github.com/JieweiWei
* @data Oct 19 2014
*
*/

#include "md5.h"

/* Define the static member of MD5. */
const byte MD5::PADDING[64] = {0x80};
const char MD5::HEX_NUMBERS[16] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'
};

/**
* @Construct a MD5 object with a string.
*
* @param {message} the message will be transformed.
*
*/
MD5::MD5(const string& message)
{
    finished = false;
    /* Reset number of bits. */
    count[0] = count[1] = 0;
    /* Initialization constants. */
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;

    /* Initialization the object according to message. */
    init((const byte*)message.c_str(), message.length());
}

/**
* @Generate md5 digest.
*
* @return the message-digest.
*
*/
const byte* MD5::getDigest()
{
    if (!finished)
    {
        finished = true;

        byte bits[8];
        bit32 oldState[4];
        bit32 oldCount[2];
        bit32 index, padLen;

        /* Save current state and count. */
        memcpy(oldState, state, 16);
        memcpy(oldCount, count, 8);

        /* Save number of bits */
        encode(count, bits, 8);

        /* Pad out to 56 mod 64. */
        index = (bit32)((count[0] >> 3) & 0x3f);
        padLen = (index < 56) ? (56 - index) : (120 - index);
        init(PADDING, padLen);

        /* Append length (before padding) */
        init(bits, 8);

        /* Store state in digest */
        encode(state, digest, 16);

        /* Restore current state and count. */
        memcpy(state, oldState, 16);
        memcpy(count, oldCount, 8);
    }
    return digest;
}

/**
* @Initialization the md5 object, processing another message block,
* and updating the context.
*
* @param {input} the input message.
*
* @param {len} the number btye of message.
*
*/
void MD5::init(const byte* input, size_t len)
{
    bit32 i, index, partLen;

    finished = false;

    /* Compute number of bytes mod 64 */
    index = (bit32)((count[0] >> 3) & 0x3f);

    /* update number of bits */
    if ((count[0] += ((bit32)len << 3)) < ((bit32)len << 3))
    {
        ++count[1];
    }
    count[1] += ((bit32)len >> 29);

    partLen = 64 - index;

    /* transform as many times as possible. */
    if (len >= partLen)
    {
        memcpy(&buffer[index], input, partLen);
        transform(buffer);

        for (i = partLen; i + 63 < len; i += 64)
        {
            transform(&input[i]);
        }
        index = 0;
    }
    else
    {
        i = 0;
    }

    /* Buffer remaining input */
    memcpy(&buffer[index], &input[i], len - i);
}

/**
* @MD5 basic transformation. Transforms state based on block.
*
* @param {block} the message block.
*/
void MD5::transform(const byte block[64])
{
    bit32 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    decode(block, x, 64);

    /* Round 1 */
    FF(a, b, c, d, x[0], s11, 0xd76aa478);
    FF(d, a, b, c, x[1], s12, 0xe8c7b756);
    FF(c, d, a, b, x[2], s13, 0x242070db);
    FF(b, c, d, a, x[3], s14, 0xc1bdceee);
    FF(a, b, c, d, x[4], s11, 0xf57c0faf);
    FF(d, a, b, c, x[5], s12, 0x4787c62a);
    FF(c, d, a, b, x[6], s13, 0xa8304613);
    FF(b, c, d, a, x[7], s14, 0xfd469501);
    FF(a, b, c, d, x[8], s11, 0x698098d8);
    FF(d, a, b, c, x[9], s12, 0x8b44f7af);
    FF(c, d, a, b, x[10], s13, 0xffff5bb1);
    FF(b, c, d, a, x[11], s14, 0x895cd7be);
    FF(a, b, c, d, x[12], s11, 0x6b901122);
    FF(d, a, b, c, x[13], s12, 0xfd987193);
    FF(c, d, a, b, x[14], s13, 0xa679438e);
    FF(b, c, d, a, x[15], s14, 0x49b40821);

    /* Round 2 */
    GG(a, b, c, d, x[1], s21, 0xf61e2562);
    GG(d, a, b, c, x[6], s22, 0xc040b340);
    GG(c, d, a, b, x[11], s23, 0x265e5a51);
    GG(b, c, d, a, x[0], s24, 0xe9b6c7aa);
    GG(a, b, c, d, x[5], s21, 0xd62f105d);
    GG(d, a, b, c, x[10], s22, 0x2441453);
    GG(c, d, a, b, x[15], s23, 0xd8a1e681);
    GG(b, c, d, a, x[4], s24, 0xe7d3fbc8);
    GG(a, b, c, d, x[9], s21, 0x21e1cde6);
    GG(d, a, b, c, x[14], s22, 0xc33707d6);
    GG(c, d, a, b, x[3], s23, 0xf4d50d87);
    GG(b, c, d, a, x[8], s24, 0x455a14ed);
    GG(a, b, c, d, x[13], s21, 0xa9e3e905);
    GG(d, a, b, c, x[2], s22, 0xfcefa3f8);
    GG(c, d, a, b, x[7], s23, 0x676f02d9);
    GG(b, c, d, a, x[12], s24, 0x8d2a4c8a);

    /* Round 3 */
    HH(a, b, c, d, x[5], s31, 0xfffa3942);
    HH(d, a, b, c, x[8], s32, 0x8771f681);
    HH(c, d, a, b, x[11], s33, 0x6d9d6122);
    HH(b, c, d, a, x[14], s34, 0xfde5380c);
    HH(a, b, c, d, x[1], s31, 0xa4beea44);
    HH(d, a, b, c, x[4], s32, 0x4bdecfa9);
    HH(c, d, a, b, x[7], s33, 0xf6bb4b60);
    HH(b, c, d, a, x[10], s34, 0xbebfbc70);
    HH(a, b, c, d, x[13], s31, 0x289b7ec6);
    HH(d, a, b, c, x[0], s32, 0xeaa127fa);
    HH(c, d, a, b, x[3], s33, 0xd4ef3085);
    HH(b, c, d, a, x[6], s34, 0x4881d05);
    HH(a, b, c, d, x[9], s31, 0xd9d4d039);
    HH(d, a, b, c, x[12], s32, 0xe6db99e5);
    HH(c, d, a, b, x[15], s33, 0x1fa27cf8);
    HH(b, c, d, a, x[2], s34, 0xc4ac5665);

    /* Round 4 */
    II(a, b, c, d, x[0], s41, 0xf4292244);
    II(d, a, b, c, x[7], s42, 0x432aff97);
    II(c, d, a, b, x[14], s43, 0xab9423a7);
    II(b, c, d, a, x[5], s44, 0xfc93a039);
    II(a, b, c, d, x[12], s41, 0x655b59c3);
    II(d, a, b, c, x[3], s42, 0x8f0ccc92);
    II(c, d, a, b, x[10], s43, 0xffeff47d);
    II(b, c, d, a, x[1], s44, 0x85845dd1);
    II(a, b, c, d, x[8], s41, 0x6fa87e4f);
    II(d, a, b, c, x[15], s42, 0xfe2ce6e0);
    II(c, d, a, b, x[6], s43, 0xa3014314);
    II(b, c, d, a, x[13], s44, 0x4e0811a1);
    II(a, b, c, d, x[4], s41, 0xf7537e82);
    II(d, a, b, c, x[11], s42, 0xbd3af235);
    II(c, d, a, b, x[2], s43, 0x2ad7d2bb);
    II(b, c, d, a, x[9], s44, 0xeb86d391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

/**
* @Encodes input (unsigned long) into output (byte).
*
* @param {input} usigned long.
*
* @param {output} byte.
*
* @param {length} the length of input.
*
*/
void MD5::encode(const bit32* input, byte* output, size_t length)
{
    for (size_t i = 0, j = 0; j < length; ++i , j += 4)
    {
        output[j] = (byte)(input[i] & 0xff);
        output[j + 1] = (byte)((input[i] >> 8) & 0xff);
        output[j + 2] = (byte)((input[i] >> 16) & 0xff);
        output[j + 3] = (byte)((input[i] >> 24) & 0xff);
    }
}

/**
* @Decodes input (byte) into output (usigned long).
*
* @param {input} bytes.
*
* @param {output} unsigned long.
*
* @param {length} the length of input.
*
*/
void MD5::decode(const byte* input, bit32* output, size_t length)
{
    for (size_t i = 0, j = 0; j < length; ++i , j += 4)
    {
        output[i] = ((bit32)input[j]) | (((bit32)input[j + 1]) << 8) |
            (((bit32)input[j + 2]) << 16) | (((bit32)input[j + 3]) << 24);
    }
}


/**
* @Convert digest to string value.
*
* @return the hex string of digest.
*
*/
string MD5::toStr()
{
    const byte* digest_ = getDigest();
    string str;
    str.reserve(16 << 1);
    for (size_t i = 0; i < 16; ++i)
    {
        int t = digest_[i];
        int a = t / 16;
        int b = t % 16;
        str.append(1, HEX_NUMBERS[a]);
        str.append(1, HEX_NUMBERS[b]);
    }
    return str;
}
