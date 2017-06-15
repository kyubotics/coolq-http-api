#pragma once

#include <string>

#include "str.h"
#include "win32_encoding.h"

namespace rc {

    namespace win32 {

        typedef std::string bytes;

        /**
         * Predefined encoding constants.
         */
        class Encoding {
        public:
            // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx
            const static int ANSI = CP_ACP;
            const static int UTF7 = CP_UTF7;
            const static int UTF8 = CP_UTF8;
            const static int SHIFT_JIS = 932;
            const static int GB2312 = 936;
            const static int KS_C_5601_1987 = 949;
            const static int BIG5 = 950;
            const static int JOHAB = 1361;
            const static int EUC_JP = 51932;
            const static int EUC_CN = 51936;
            const static int EUC_KR = 51949;
            const static int EUC_TW = 51950;
            const static int GB18030 = 54936;
            const static int GBK = GB18030;
        };

        /**
         * Encode a str object into bytes, using the encoding specified.
         */
        static bytes encode(const str &s, int encoding = Encoding::UTF8) {
            auto ws = s.to_wstring();
            return bytes(widechar_to_multibyte(encoding, ws.c_str()).get());
        }

        /**
         * Decode a bytes object into str, using the encoding specified.
         */
        static str decode(const bytes &b, int encoding = Encoding::UTF8) {
            // check if in WinNT (not Wine)
            static auto in_nt = false;

            if (!in_nt) {
                auto hntdll = GetModuleHandle(L"ntdll.dll");
                if (hntdll) {
                    auto pwine_get_version = GetProcAddress(hntdll, "wine_get_version");
                    if (!pwine_get_version) {
                        // has ntdll.dll but not Wine, we assume it is NT
                        in_nt = true;
                    }
                }
            }

            // special case for Windows NT
            if (in_nt && encoding == Encoding::ANSI && GetACP() == Encoding::GB2312) {
                // do encoding rise
                encoding = Encoding::GB18030;
            }

            auto ws = multibyte_to_widechar(encoding, b.c_str());
            return str(ws.get());
        }
    }
}
