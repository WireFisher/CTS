#ifndef CTS_EXCEPTION
#define CTS_EXCEPTION

#include <exception>
using namespace std;

class CTS_exception: public exception
{
    const char* msg;
    virtual const char* what() const throw()
    {
        return this->msg;
    }
    public:
        CTS_exception(const char* err_msg): msg(err_msg){}
};

extern void CTS_WARNING(const char *str);
extern void CTS_WARNING(const char *fmt, const char *str);

#endif
