
#pragma once

struct header {
    std::string_view field;
    std::string_view value; 
    //we also treat quoted strings as one string (including the string)
    //keep in mind this can be CSV, even ';' separated, simply have multiple header fields with same name
    //But for simplicity keeping it as one string
    //Comma or ';' separated? https://stackoverflow.com/questions/78183179/when-are-http-header-values-comma-or-semi-colon-separated
        //Usually its CSV but ultimately based on spec of specific header field
        //Regardless the parsing doesn't care about what header field it is until later, 3.2.4 Field Parsing

    bool operator==(const header& other) const
    {
        return field == other.field && value == other.value;
    }
};
