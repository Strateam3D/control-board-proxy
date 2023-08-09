#pragma once

namespace strateam{
    namespace string_utils{
        inline std::string formatStringImpl(const char* s, std::stringstream& ss){
                ss << s;
                return ss.str();
        }

        template<typename T, typename... Args>
        inline std::string formatStringImpl( char const* s, std::stringstream& ss, T arg, const Args&... args ){
            std::string result;

            while( s != 0 ){
                if( *s == '{' && *( s + 1 ) == '}' ){
                    ss << arg;
                    result = formatStringImpl( s + 2 * sizeof( char ), ss, args... );
                    break;
                }else{
                    ss << *s;
                }

                s++;
            }

            return result;
        }



        template<typename... Args>
        inline std::string formatString( std::string const& fmt, const Args&... args ){
            std::stringstream ss;
            return formatStringImpl( fmt.c_str(), ss, args... );
        }
    }

}