#include "StringUtil.h"
#include <sstream>
#include <cctype>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

std::string StringUtil::Format(
	const char *fmt,
	va_list ap)
{
    // Allocate a buffer on the stack that's big enough for us almost
    // all the time.  Be prepared to allocate dynamically if it doesn't fit.
    size_t size = 1024;
    char* stackbuf = new char[size];
    std::vector<char> dynamicbuf;
    char *buf = &stackbuf[0];
    
    while (1) {
        // Try to vsnprintf into our buffer.
        int needed = vsnprintf (buf, size, fmt, ap);
        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.
        
        if (needed <= (int)size && needed >= 0) {
            // It fit fine so we're done.
            return std::string (buf, (size_t) needed);
        }
        
        // vsnprintf reported that it wanted to write more characters
        // than we allotted.  So try again using a dynamic buffer.  This
        // doesn't happen very often if we chose our initial size well.
        size = (needed > 0) ? (needed+1) : (size*2);
        dynamicbuf.resize (size);
        buf = &dynamicbuf[0];
    }
}

// Split up an input string into separate lines
void StringUtil::SplitIntoLines(
	const std::string& input,
	std::deque <std::string>& result )
{
    std::istringstream ss( input );
    std::string line;
    while (std::getline( ss, line )) {
        result.push_back( line );
    }
}

// Split up an input string into separate tokens
void StringUtil::Tokenize(
	const std::string& input,
	std::deque<std::string>& tokens )
{
    std::string::size_type last_pos = 0;
    std::string::size_type pos = 0;
    
    while(true)
    {
        pos = input.find_first_of(' ', last_pos);
        if( pos == std::string::npos )
        {
            tokens.push_back(input.substr(last_pos));
            break;
        } else {
            tokens.push_back(input.substr(last_pos, pos - last_pos));
            last_pos = pos + 1;
        }
    }
}

bool StringUtil::IsNumber( const std::string& string)
{
    std::string::const_iterator it = string.begin();
    while ( it != string.end() &&
           (std::isdigit(*it) || *it == '.') )
    {
        ++it;
    }
    return !string.empty() && it == string.end();
}

std::string StringUtil::BoolToString( bool number ) {
    return number == true ? "TRUE" : "FALSE";   //return a string with the contents of the stream
}

std::string StringUtil::IntToString( int number ) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
std::string StringUtil::LongToString( long number ) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
std::string StringUtil::FloatToString( float number, int precision ) {
    std::ostringstream buff;
    buff.precision(precision);
    buff<<number;
    return buff.str();
}

std::string StringUtil::DoubleToString( double number, int precision ) {
    std::ostringstream buff;
    buff.precision(precision);
    buff<<number;
    return buff.str();
}

// --- WINDOWS SPECIFIC SHIZZLES --- //
#if defined(_WIN32)
// Convert a wide Unicode string to an UTF8 string
std::string StringUtil::utf8_encode(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
// Convert an UTF8 string to a wide Unicode String
std::wstring StringUtil::utf8_decode(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
// Convert widechars to string
std::string StringUtil::wchar_t2string(const wchar_t *wchar)
{
    std::string str = "";
    int index = 0;
    while(wchar[index] != 0)
    {
        str += (char)wchar[index];
        ++index;
    }
    return str;
}
// Convert string to widechars
void StringUtil::string2wchar_t(const std::string &str,  wchar_t* wchar)
{
    unsigned int index = 0;
    while(index < str.size())
    {
        wchar[index] = (wchar_t)str[index];
        ++index;
    }
    wchar[index] = 0;
}
#endif