#pragma once
#include <string>
#include <array>
#include <iostream>
#include <filesystem>
#include <fstream> 
#include <sstream>
#include <unordered_map>
#include <variant>
#include "header.h"
#include "error_codes.h"
// things to do:
// implement chunked transfer encoding algo
    //Now need to add in states/switch case
// implement new states for header values so we don't need to repeat loop for whitespace
// Implement a builder pattern for Flags so that its easier to explicitly initialize certain flags (this is more so for writing tests)
// NEED! to have EOF checks and substr checks, make sure THERE IS NO THROWS!

// Nices to have:
// query params in string? 
// implement strict mode
// look into going through assembly of optimized, unoptimized versions <- USE A GDB FRONTEND

class HTTPRequest {
    private:
        static const uint8_t MAX_HEADER_LENGTH = 50;
        static const uint32_t MAX_HEADER_SIZE = MAX_HEADER_LENGTH*8192; //if headers total size greater, return 4xx error
        static const uint32_t MAX_SIZE = 1000000; //if total size of request/response greater, return 4xx
        static const uint8_t MAX_HEADER_VALUE_COUNT = 50;

        uint8_t headerCnt = 0;
        //having strict? -> possibly implement a strict/relaxed version of parser
        enum ParsingState {
            METHOD,
            REQUEST_TARGET,
            HTTP_VERSION,
            HEADER_FIELD,
            START_OWS_HEADER_VALUE,
            HEADER_VALUE,
            END_OWS_HEADER_VALUE,
            TRANSFER_CODING_VALUE, 
            CONTENT_LENGTH_VALUE,
            HEADER_VALUE_END,
            BODY,
            CHUNKED_BODY_CHUNK, //include parsing last chunk
            CHUNKED_HEADER_FIELD, //actually might be better to separate parsing headers into separate loop, 
            END_PARSE,
        };

        enum ParsingChunkedState {
            CHUNK_SIZE,
            CHUNK_EXTENSIONS,
            CHUNK_DATA,
            CHUNK_DATA_CRLF,
            TRAILER_HEADER_FIELD,
            TRAILER_START_OWS_HEADER_VALUE,
            TRAILER_HEADER_VALUE,
            TRAILER_END_OWS_HEADER_VALUE,
            TRAILER_HEADER_VALUE_END,
        };
        // note this is 8 bytes https://stackoverflow.com/questions/45575892/why-is-sizeofstdvariant-the-same-size-as-a-struct-with-the-same-members
        // does it matter? should we just use int or combine two enums into one?
        // std::variant<ParsingState, ParsingChunkedState> currState = METHOD;
        uint8_t currState = METHOD;

        //basically is VCHAR also (visible char)
        bool isOBSText(const char c) { unsigned char newC =  static_cast<unsigned char>(c); return newC > 127; } //Gets all other non US ASCII chars (valid for header value)
        bool isVChar(const char c) { unsigned char newC =  static_cast<unsigned char>(c); return newC < 127 && newC > 32; } //between Space and DEL in ASCII chart 
        bool isTokenChar(const char c ) { return c == '!' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\'' || c == '*' || c ==  '+' || c == '-' || c == '.' || 
            c == '^' || c == '_' || c == '`' || c == '|' || c == '~' || std::isdigit(c) || std::isalpha(c); }                                                                                                                            
        bool isHexDig(const char c) { return std::isdigit(c) || std::isalpha(c); }
        bool isTabOrSpace(const char c) { return c == ' ' || c == '\t'; }                                                                                                            
    public:
        struct Flags {
            bool has_content_length;
            bool has_chunked_te;
            bool has_gzip_te;
            bool has_compress_te;
            bool has_deflate_te;
            bool is_keep_alive;
            bool has_upgrade_header;
            bool skip_body; // see (3.3 rfc 7230 for conditions for this, mostly for response body but for request signaled by Content-Length or Transfer-Encoding (strict?))

            bool operator==(const Flags& other) const
            {
                return has_content_length == other.has_content_length &&
                    has_chunked_te == other.has_chunked_te &&
                    has_gzip_te == other.has_gzip_te &&
                    has_compress_te == other.has_compress_te &&
                    has_deflate_te == other.has_deflate_te &&
                    is_keep_alive == other.is_keep_alive &&
                    has_upgrade_header == other.has_upgrade_header &&
                    skip_body == other.skip_body;
            }
            friend std::ostream& operator<<(std::ostream& os, const Flags& obj) {
                os << "{ has_content_length: " << obj.has_content_length
                            << ", has_chunked_transfer_encoding: " << obj.has_chunked_te
                            << ", has_gzip_transfer_encoding: " << obj.has_gzip_te
                            << ", has_compress_transfer_encoding: " << obj.has_compress_te
                            << ", has_deflate_transfer_encoding: " << obj.has_deflate_te
                            << ", is_keep_alive: " << obj.is_keep_alive
                            << ", has_upgrade_header: " << obj.has_upgrade_header
                            << ", skip_body: " << obj.skip_body 
                            << " }" << std::endl;
                return os;
            }   
        };

        std::string_view method;
        std::string_view URI;
        uint8_t maj_version; //waste of space, maybe just use bool if we are only allowing 1.0 or 1.1
        uint8_t min_version;
        std::array<header, MAX_HEADER_LENGTH> headers{};
        std::string_view body;
        uint32_t content_length{};
        Flags flags{};
        ErrorCode return_code;

    HTTPRequest() {}

    HTTPRequest(std::string_view buf) {
        parse(buf);
    }

    //mostly used for testing/creating mock object
    HTTPRequest(std::string_view _method, 
        std::string_view _URI,
        uint8_t _maj_version, //waste of structure space, maybe just use bool if we are only allowing 1.0 or _1.,
        uint8_t _min_version,
        std::array<header, MAX_HEADER_LENGTH> _headers,
        std::string_view _body,
        uint32_t _content_length,
        Flags _flags,
        ErrorCode _return_code)
            : method { _method },
            URI { _URI },
            maj_version { _maj_version },
            min_version { _min_version },
            headers { _headers },
            body { _body },
            content_length { _content_length },
            flags { _flags },
            return_code { _return_code }
        {
        }

    bool equals(const HTTPRequest& obj) {
        return this->method == obj.method &&
            this->URI == obj.URI &&
            this->maj_version == obj.maj_version &&
            this->min_version == obj.min_version &&
            this->headers == obj.headers &&
            this->body == obj.body &&
            this->content_length == obj.content_length &&
            this->flags == obj.flags &&
            this->return_code == obj.return_code;
    }

    friend std::ostream& operator<<(std::ostream& os, const HTTPRequest& obj) {
        os << "Method: " << obj.method << std::endl;
        os << "URI: " << obj.URI << std::endl;
        os << "HTTP Version: " << unsigned(obj.maj_version) << "." << unsigned(obj.min_version) << std::endl;
        for (auto header : obj.headers) {
            if (header.field != "" && header.value != "")
                os << "Header Field/Value: " << "\"" << header.field << "\"" << " / " << "\"" << header.value << "\"" << std::endl;
        }
        os << "Content-Length: " << obj.content_length << std::endl;
        os << "Body: "  << "\"" << obj.body << "\"" << std::endl;
        os << "Flags: " << obj.flags << std::endl;
        os << "Return Code: " << obj.return_code << std::endl;
        return os;
    }      
    
    //another thing to consider -> ASCII (token) vs non ASCII characters
    ErrorCode parse(std::string_view req, size_t startPos = 0) { //return status as int
        size_t pos;
        // std::string delimiter = "\r\n";
        uint32_t reqLength = req.length();
        for (size_t i = startPos; i < reqLength; i++ ) {
            pos = i;
            switch (currState) {
                //NO PREDEFINED LIMIT ON LENGTH OF REQUEST LINE, min 8000 octets (otherwise error if above max)
                
                //Robustness for Parsing 3.5 in 7230
                //SHOULD IGNORE AT LEAST ONE EMPTY LINE (CRLF) before REQUEST LINE
                //KEEP IN MIND: Our CRLF checks could also just recgonize ONE LF and ignore any preceding CR strict?
                //take OWS instead of single SP in requst and status line (strict?)
                case METHOD:
                    while (isTokenChar(req[i])) {
                        ++i;
                    }    
                    if (req[i] != ' ' || pos == i) {
                        return_code = BAD_START_LINE;
                        return BAD_START_LINE;
                    }
                    this->method =  std::string_view(req).substr(pos, i-pos);
                    currState = REQUEST_TARGET;
                break;
                case REQUEST_TARGET:
                    //There is likely a LOT more in what goes in request-target/URI (what charset is actually allowed?)
                    while (req[i] != ' ' && req[i] != '\t') {
                        ++i;
                    }    
                    if (req[i] != ' ' || pos == i) {
                        return_code = BAD_START_LINE;
                        return BAD_START_LINE;
                    }
                    this->URI =  std::string_view(req).substr(pos, i-pos);
                    currState = HTTP_VERSION;
                break;
                case HTTP_VERSION:
                    if (req[i] != 'H') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != 'T') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != 'T') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != 'P') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != '/') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != '0' && req[i] != '1' && req[i] != '2') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    this->maj_version = req[i] - '0';
                    ++i;
                    if (req[i] != '.') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    ++i;
                    if (req[i] != '0' && req[i] != '1' && req[i] != '2') {
                        return_code = BAD_HTTP_VERSION;
                        return BAD_HTTP_VERSION;
                    }
                    this->min_version = req[i] - '0';
                    ++i;
                    if (req[i++] != '\r' || req[i] != '\n') {
                        return_code = BAD_START_LINE;
                        return BAD_START_LINE;
                    }
                    currState = HEADER_FIELD;
                break;
                //AGAIN IGNORE LARGER THAN MAX SIZE FOR FIELD AND VALUE
                case HEADER_FIELD:
                    return parseHeaders(req, i);
                break;
                case BODY:
                    //skip through CRLF
                    //initial carriage is skipped by for loop
                    //Request check if Content-Length or Transfer-Encoding
                    //Reponse see (RFC 7230 3.3), depends on request method and status code
                    // Incomplete message (through some error?) If 1. chunked encoding and 0-sized chunk NOT received or 2. content length and body size in octets is less than val

                    // 3.3.3 in 7230 to determine length of body
                        //for request: 1. Need Transfer Encoding header or Content Length (both present, TE overrides)
                        //                1a. Content Length represents message body length in octets
                        //             2. If 1. not true, we assume body length is 0 (strict? we can reject if there is body with no Content Length)
                    this->body = std::string_view(req).substr(i, req.length()); 
                    return_code = OK;
                    return OK;
                break;
                case CHUNKED_BODY_CHUNK:
                    // AT THIS POINT, CHUNK COULD BE CUT OFF AT ANY TIME, INSTEAD OF DOING PARSING/HAVING A STATE HERE, SHOULD LIKELY RETURN A STATUS CODE NOTING TO PARSE THE CHUNKS
                    currState = CHUNK_SIZE;
                    content_length = -1;
                    return parseChunked(req, i);
                break;
            }
        }
        // std::cout << httpRequest;
        return_code = OK;
        return OK;
    }
    //Create a separate method just for parsing the chunked
    ErrorCode parseHeaders(std::string_view req, size_t startPos = 0) {
        size_t pos;
        // std::string delimiter = "\r\n";
        uint32_t reqLength = req.length();
        for (size_t i = startPos; i < reqLength; i++ ) {
            pos = i;
            switch (currState) {
            case HEADER_FIELD:
                // HEADER FIELD STRICTLY TOKEN AND NO WHITESPACE
                while (req[i] != ':')
                {
                    if (!isTokenChar(req[i]))
                    {
                        return_code = BAD_HEADER_FIELD;
                        return BAD_HEADER_FIELD;
                    }
                    ++i;
                }
                // NO SPACES IN HEADER FIELD
                this->headers[headerCnt].field = std::string_view(req).substr(pos, i - pos);
                if (this->headers[headerCnt].field == "Transfer-Encoding")
                { // generally only 1.1 feature (strict?)
                    currState = TRANSFER_CODING_VALUE;
                }
                else if (this->headers[headerCnt].field == "Content-Length")
                {
                    currState = CONTENT_LENGTH_VALUE;
                }
                else
                {
                    currState = START_OWS_HEADER_VALUE;
                }
            break;
            // according to 3.2.4 RFC 7230 (parser ought to trim traiing/leading OWS from value)
            case START_OWS_HEADER_VALUE:
                while (isTabOrSpace(req[i]))
                {
                    ++i;
                }
                pos = i;
                i -= 1;
                currState = HEADER_VALUE;
            break;
            // NOTE: according to ABNF in 3.2 rfc 7230, empty field-value allowed... maybe add flag to disallow?
            case HEADER_VALUE:
                // //check that we actually had a value (some VCHAR/non delimiter, visible char), err if we dont
                // if (isspace(req[i])) {
                //     return -6;
                // }
                // while (!isspace(req[i])) {
                //     ++i;
                // }
                // //at this point should check for space/htab for additional values
                // if (req[i] == ' ' || req[i] == '\t') {
                //     ++i; //go back to beginning while loop
                // }
                while (isVChar(req[i]) || isOBSText(req[i]) || isTabOrSpace(req[i]))
                {
                    ++i;
                }
                // this logic could get more complicated with delimiting, support for quoted strings, comments, obs-text (3.2.6)
                // but this is extremely naive
                // For header value, if octet is not US-ASCII, can just treat as opaque data
                if (req[i++] != '\r' || req[i] != '\n')
                {
                    return_code = BAD_HEADER_VALUE;
                    return BAD_HEADER_VALUE;
                }

                this->headers[headerCnt].value = std::string_view(req).substr(pos, (i - 1) - pos); // i-1 beccause we included the \n
                ++headerCnt;                                                                       // increment header length/counter
                currState = HEADER_VALUE_END;
            break;
            case TRANSFER_CODING_VALUE: // this and content length is mutually exclusive strict?
                // 1. should not apply chunked more than once to body2. chunked should be final transfer coding strict? MEANING IT SHOULD ALWAYS BE PRESENT ON A REQUEST IF TE IS USED
                if (flags.has_content_length)
                {
                    return_code = IMPROPER_TRANSFER_CODING;
                    return IMPROPER_TRANSFER_CODING;
                }
                while (req[i] == ' ' || req[i] == '\t')
                {
                    ++i;
                    ++pos;
                }
                while (req[i] != '\r' && req[i] != '\n')
                {
                    while (req[i] == ' ' || req[i] == '\t')
                    { // notice difference from initial loop is the ++pos, the initial loop is to eat whitespace AND adjust pos
                        ++i;
                    }
                    // opportunity to use findchar_fast delimit on the ',
                    if (req[i] == 'c')
                    {
                        i++;
                        if (req[i] == 'h')
                        {
                            // this sohuld be last encoding in requests otherwise error (? 3.3.3)
                            if (req.substr(i, 6) == "hunked")
                            {
                                i += 6;
                                flags.has_chunked_te = true;
                            }
                            else
                            {
                                return_code = IMPROPER_TRANSFER_CODING;
                                return IMPROPER_TRANSFER_CODING;
                            }
                        }
                        else if (req[i] == 'o')
                        {
                            if (req.substr(i, 7) == "ompress")
                            {
                                flags.has_compress_te = true;
                                i += 7;
                            }
                            else
                            {
                                return_code = IMPROPER_TRANSFER_CODING;
                                return IMPROPER_TRANSFER_CODING;
                            }
                        }
                        else
                        {
                            return_code = IMPROPER_TRANSFER_CODING;
                            return IMPROPER_TRANSFER_CODING;
                        }
                        // chunked or compress
                    }
                    else if (req[i] == 'd')
                    {
                        if (req.substr(i, 7) == "deflate")
                        {
                            flags.has_deflate_te = true;
                            i += 7;
                        }
                        else
                        {
                            return_code = IMPROPER_TRANSFER_CODING;
                            return IMPROPER_TRANSFER_CODING;
                        }
                    }
                    else if (req[i] == 'g')
                    {
                        if (req.substr(i, 4) == "gzip")
                        {
                            flags.has_gzip_te = true;
                            i += 4;
                        }
                        else
                        {
                            return_code = IMPROPER_TRANSFER_CODING;
                            return IMPROPER_TRANSFER_CODING;
                        }
                    }
                    else
                    {
                        return_code = IMPROPER_TRANSFER_CODING;
                        return IMPROPER_TRANSFER_CODING;
                    }

                    while (req[i] == ' ' || req[i] == '\t')
                    {
                        ++i;
                    }
                    if (req[i] == ',')
                    {
                        i++;
                        continue;
                    }
                    else if (req[i] == '\r')
                    {
                        break;
                    }
                    else
                    {
                        return_code = IMPROPER_TRANSFER_CODING;
                        return IMPROPER_TRANSFER_CODING;
                    }
                }
                if (req[i++] != '\r' || req[i] != '\n')
                {
                    return_code = BAD_HEADER_VALUE;
                    return BAD_HEADER_VALUE;
                }
                // for now only taking chunked, compress, deflate, gzip (only ones registered and valid in HTTP Transfer Coding registry)
                headers[headerCnt].value = std::string_view(req).substr(pos, (i - 1) - pos); // i-1 beccause we included the \n
                ++headerCnt;                                                                 // increment header length/counter
                currState = HEADER_VALUE_END;
            break;
            case CONTENT_LENGTH_VALUE:
                while (req[i] == ' ' || req[i] == '\t')
                { // get rid of OWS
                    ++i;
                    ++pos;
                }
                // case where multiple content-length header fields strict?
                if (flags.has_content_length || flags.has_chunked_te || flags.has_gzip_te || flags.has_compress_te || flags.has_deflate_te)
                {
                    return_code = IMPROPER_CONTENT_LENGTH;
                    return IMPROPER_CONTENT_LENGTH;
                }
                // unlike normal header value need to check there was actually value (digit), also handles cases where multiple values (42, 42)
                while (isdigit(req[i]))
                {
                    ++i;
                }
                if (pos == i)
                {
                    return IMPROPER_CONTENT_LENGTH;
                }

                flags.has_content_length = true;
                {
                    auto contentLenStr = std::string_view(req).substr(pos, (i)-pos);
                    headers[headerCnt].value = contentLenStr;
                    int placeValue = 1;
                    for (auto it = contentLenStr.rbegin(); it != contentLenStr.rend(); ++it)
                    {
                        content_length += (*it - '0') * placeValue;
                        placeValue *= 10;
                    }
                }

                while (req[i] == ' ' || req[i] == '\t')
                { // get rid of OWS
                    ++i;
                }
                // note \r\n check, value setter, etc. is repeated, probably should separate into another state
                if (req[i++] != '\r' || req[i] != '\n')
                {
                    return_code = BAD_HEADER_VALUE;
                    return BAD_HEADER_VALUE;
                }

                ++headerCnt; // increment header length/counter
                // If chunked transfer encoding,
                currState = HEADER_VALUE_END;
            break;
            case HEADER_VALUE_END:
                if (req[i] == '\r')
                { // read ahead [i+1] here maybe unnecessary
                    if (req[++i] != '\n')
                    {
                        return_code = BAD_BODY;
                        return BAD_BODY;
                    }
                    currState = flags.has_chunked_te ? CHUNKED_BODY_CHUNK : BODY;
                }
                else if (isTokenChar(req[i]))
                {
                    i -= 1; // Since we are already on first character of header field, must revert for loop increment
                    currState = HEADER_FIELD;
                }
                else
                {
                    return_code = BAD_BODY;
                    return BAD_BODY;
                }
            break;
            case CHUNKED_BODY_CHUNK:
            case BODY:
                return parse(req, i);
            break;
            }
        }
        return_code = OK;
        return OK;
    }
    //Create a sepraate method just for parsing the headers
    ErrorCode parseChunked(std::string_view req, size_t startPos = 0) {
        size_t pos = 0;
        // std::string delimiter = "\r\n";
        uint32_t reqLength = req.length();
        for (size_t i = startPos; i < reqLength; i++ ) {
            switch (currState) {
                // read size first'
                case CHUNK_SIZE: 
                    if (isHexDig(req[i]))
                    {
                        if (content_length == -1) { //is this bad? we do this in order to check if there was a valid digit here
                            content_length = 0;
                        }
                        if (std::isdigit(req[i]))
                            content_length += (req[i] - '0') + content_length * 16;
                        else
                            content_length += (std::isupper(req[i]) ? (req[i] - 'A') : (req[i] - 'a')) + content_length * 16;
                    }
                    if (content_length == -1) 
                    {
                        return_code = BAD_CHUNK;
                        return BAD_CHUNK;
                    }
                    else if (req[i] != ';' && req[i] != '\r')
                        currState = req[i] == '\r' ? CHUNK_DATA : CHUNK_EXTENSIONS;
                    break;
                break;
                case CHUNK_EXTENSIONS:
                    // then read any extensions ([;token=token]) GENERALLY THIS AND SIZE OF CHUNK SHOULD HAVE LIMIT ALSO
                    //  And to be honest, this is likely going to be not used at all so... skipping over for now
                    if (req[i] == '\r')
                    {
                        currState = CHUNK_DATA;
                    }
                break;
                case CHUNK_DATA:
                    if (req[i] != '\n') {
                        return_code = BAD_CHUNK;
                        return BAD_CHUNK;
                    }
                    // Must have been last chunk
                    if (content_length == 0)
                    {
                        currState = req[i] == '\r' ? TRAILER_HEADER_VALUE_END : TRAILER_HEADER_FIELD;
                    }
                    int minOfLengths = reqLength - i < content_length ? reqLength - i : content_length;
                    // then data (just skip over)
                    body = std::string_view(req).substr(i, minOfLengths);
                    content_length -= minOfLengths;

                    if (content_length == 0)
                    {
                        currState = CHUNK_DATA_CRLF;
                    }
                break;
                case CHUNK_DATA_CRLF:
                    if (req[i] == '\r' || req[i] == '\n') {
                        if (req[i] == '\r' && i+1 < reqLength && req[i+1] != '\n')
                        {
                            return_code = BAD_CHUNK;
                            return BAD_CHUNK;
                        }
                        if (req[i] != '\r' && req[i] != '\n') {
                            content_length = -1;
                            currState = CHUNK_SIZE;
                        }
                    }
                break;
                case CHUNKED_HEADER_FIELD:
                    //We parse like normal, but there are still certain forbidden headers (can handle in server)
                    return parseTrailers(req, i);    
                break;
            }
        }
        return_code = OK;
        return OK;
    }
    ErrorCode parseTrailers(std::string_view req, size_t startPos = 0) {
        size_t pos;
        uint32_t reqLength = req.length();
        for (size_t i = startPos; i < reqLength; i++ ) {
            pos = i;
            switch (currState) {
            case TRAILER_HEADER_FIELD:
                while (req[i] != ':')
                {
                    if (!isTokenChar(req[i]))
                    {
                        return_code = BAD_HEADER_FIELD;
                        return BAD_HEADER_FIELD;
                    }
                    ++i;
                }
                this->headers[headerCnt].field = std::string_view(req).substr(pos, i - pos);
                currState = TRAILER_START_OWS_HEADER_VALUE;
            break;
            case TRAILER_START_OWS_HEADER_VALUE:
                while (isTabOrSpace(req[i]))
                {
                    ++i;
                }
                pos = i;
                i -= 1;
                currState = TRAILER_HEADER_VALUE;
            break;
            case TRAILER_HEADER_VALUE:
                while (isVChar(req[i]) || isOBSText(req[i]) || isTabOrSpace(req[i]))
                {
                    ++i;
                }
                if (req[i++] != '\r' || req[i] != '\n')
                {
                    return_code = BAD_HEADER_VALUE;
                    return BAD_HEADER_VALUE;
                }

                this->headers[headerCnt].value = std::string_view(req).substr(pos, (i - 1) - pos); // i-1 beccause we included the \n
                ++headerCnt;                                                                       // increment header length/counter
                currState = TRAILER_HEADER_VALUE_END;
            break;
            case TRAILER_HEADER_VALUE_END:
                if (req[i] == '\r')
                {                     if (req[++i] != '\n')
                    {
                        return_code = BAD_TRAILER_FIELD;
                        return BAD_TRAILER_FIELD;
                    }
                    break;
                }
                else if (isTokenChar(req[i]))
                {
                    i -= 1; // Since we are already on first character of header field, must revert for loop increment
                    currState = TRAILER_HEADER_FIELD;
                }
                else
                {
                    return_code = BAD_TRAILER_FIELD;
                    return BAD_TRAILER_FIELD;
                }
            break;
            }
        }
        return_code = OK;
        return OK;
    }
    //Creating separate methods for parsing different parts of message is like picohttp, 
        //but for chunked I think we can keep reuse same parser object like http-parser does it
            //Specifically use content length to store chunk sizes and use it as the offset/how many more bytes expected to read 
            //Won't need content length since we parsing only parts of body (usually) and technically exclusive with TE regardless 

};