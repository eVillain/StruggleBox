//
//  StringUtil.h
//  DungeonSmith
//
//  Created by The Drudgerist on 20/04/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#ifndef __DungeonSmith__StringUtil__
#define __DungeonSmith__StringUtil__

#include <string>
#include <deque>

class StringUtil {
public:
    static std::string Format(const char *fmt, va_list ap);
    static void SplitIntoLines( const std::string &input, std::deque <const std::string> &lines );
    static void Tokenize( const std::string &input, std::deque<std::string> &tokens );
    static bool IsNumber( const std::string& string );
    static std::string BoolToString(bool number);
    static std::string IntToString( int number );
    static std::string LongToString( long number );
    static std::string FloatToString( float number, int precision = 3 );
    static std::string DoubleToString( double number, int precision = 3 );
#if defined(_WIN32)
    // Convert a wide Unicode string to an UTF8 string
    static std::string utf8_encode(const std::wstring &wstr);
    // Convert an UTF8 string to a wide Unicode String
    static std::wstring utf8_decode(const std::string &str);
    // Convert widechars to string
    static std::string wchar_t2string(const wchar_t *wchar);
    // Convert string to widechars
    static void string2wchar_t(const std::string &str,  wchar_t* wchar);
#endif
};

#endif /* defined(__DungeonSmith__StringUtil__) */
