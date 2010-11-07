#include <string>
#include <cerrno>
#include <cstring>
#include <iconv.h>

#include "utils.h"
#include "common.h"

//-----------------------------------------------------------------------------
std::string IconvString(const std::string & src,
                        const std::string & from,
                        const std::string & to)
{
if (src.empty())
    return std::string();

size_t inBytesLeft = src.length() + 1;
size_t outBytesLeft = src.length() * 2 + 1;

char * inBuf = new char[inBytesLeft];
char * outBuf = new char[outBytesLeft];

strncpy(inBuf, src.c_str(), src.length());

inBuf[src.length()] = 0;

#if defined(FREE_BSD) || defined(FREE_BSD5)
const char * srcPos = inBuf;
#else
char * srcPos = inBuf;
#endif
char * dstPos = outBuf;

iconv_t handle = iconv_open(to.c_str(),
                            from.c_str());

if (handle == iconv_t(-1))
    {
    if (errno == EINVAL)
        {
        printfd(__FILE__, "IconvString(): iconv from %s to %s failed\n", from.c_str(), to.c_str());
        delete[] outBuf;
        delete[] inBuf;
        return src;
        }
    else
        printfd(__FILE__, "IconvString(): iconv_open error\n");

    delete[] outBuf;
    delete[] inBuf;
    return src;
    }

size_t res = iconv(handle,
                   &srcPos, &inBytesLeft,
                   &dstPos, &outBytesLeft);

if (res == size_t(-1))
    {
    printfd(__FILE__, "IconvString(): '%s'\n", strerror(errno));

    iconv_close(handle);
    delete[] outBuf;
    delete[] inBuf;
    return src;
    }

dstPos = 0;

std::string dst(outBuf);

iconv_close(handle);

delete[] outBuf;
delete[] inBuf;

return dst;
}
