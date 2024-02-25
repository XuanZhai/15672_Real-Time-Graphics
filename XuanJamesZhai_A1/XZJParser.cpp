//
// Created by Xuan Zhai on 2024/1/30.
//

#include "XZJParser.h"

/* ====================================== ParserNode ================================================================ */


/**
 * @brief If the data is a hash map, return the value for the given key. Else return nullptr.
 * @param key The key as a string.
 * @return A ParserNode value mapped on that key.
 */
std::shared_ptr<ParserNode> ParserNode::GetObjectValue(const std::string& key){
    ParserNode::PNMap* pnMap = std::get_if<ParserNode::PNMap>(&data);

    /* If it's not a hashmap or if it does not contain the key. */
    if(pnMap == nullptr || pnMap->find(key) == pnMap->end()) {
        return nullptr;
    }
    return (*pnMap)[key];
}


/* ====================================== XZJParser ================================================================= */


/**
 * @brief Parse a s72 file with a given file path and file name.
 * Store the result in the root node in XZJParser.
 * @param filename The name of the s72 file, also could add path to it.
 */
std::shared_ptr<ParserNode> XZJParser::Parse(const std::string &fileName) {

    std::ifstream input = std::ifstream(fileName, std::ios::binary);

    if(!input){
        throw std::runtime_error("Parse Error: Unable to open the input file.");
    }

    /* Read the whole file into a string stream */
    std::stringstream buffer;
    buffer << input.rdbuf();

    /* Remove all the empty space and unnecessary characters */
    RemoveSpace(buffer.str());

    /* Recursively parse s72Data into ParseNodes */
    std::shared_ptr<ParserNode> root = ParseInput(0,s72Data.length()-1);
    s72Data = "";

    input.close();

    return root;
}


/**
 * @brief Parse the s72Data between l and r.
 * @param l The left bound of the string that parsed in s72Data.
 * @param r The right bound of the string that parsed in s72Data.
 * @return The parsed result stored in a ParserNode.
 */
std::shared_ptr<ParserNode> XZJParser::ParseInput(size_t l,size_t r) {

    /* Create a new Node object. */
    std::shared_ptr<ParserNode> obj(new ParserNode());

    /* If the data is a string. */
    if(s72Data[l] == '"'){
        obj->data = s72Data.substr(l+1,r-l-1);  // Need to remove the quotation marks.
    }
    /* If the data is a float/number. */
    else if((s72Data[l] >= '0' && s72Data[l] <= '9') || s72Data[l] == '-'){
        /* Convert the string to float using string stream. */
        std::stringstream dss(s72Data.substr(l,r-l+1));
        float d = 0;
        dss >> d;

        obj->data = d;
    }
    /* If the data is an array. */
    else if(s72Data[l] == '['){

        if(l+1 < s72Data.length() && s72Data[l+1] == ']'){
            return nullptr;
        }

        /* Construct data as PNVector */
        obj->data = ParserNode::PNVector();
        size_t sl = l+1;
        size_t sr = l+1;

        /* We don't care about comma within the bracket, so only work on the outer level. */
        size_t level = 0;

        while(sr != r-1){
            if(s72Data[sr] == '[' || s72Data[sr] == '{'){
                level++;
            }
            else if(s72Data[sr] == ']' || s72Data[sr] == '}'){
                level--;
            }
            /* Process only when level is 0 which is the outer level. */
            else if(s72Data[sr] == ',' && level == 0){
                /* Recursively add nodes into the array. */
                std::get<ParserNode::PNVector>(obj->data).emplace_back(ParseInput(sl,sr-1));
                sl = sr+1;
            }
            sr++;
        }
        /* Add the last node */
        std::get<ParserNode::PNVector>(obj->data).emplace_back(ParseInput(sl,sr));
    }
    /* If the data is an object */
    else if(s72Data[l] == '{'){

        if(l+1 < s72Data.length() && s72Data[l+1] == '}'){
            return nullptr;
        }

        /* Construct data as PNMap. */
        obj->data = ParserNode::PNMap();

        size_t sl = l+1;
        size_t sr = l+1;

        size_t level = 0;

        while(sr != r-1){

            if(s72Data[sr] == '[' || s72Data[sr] == '{') {
                level++;
            }
            else if(s72Data[sr] == ']' || s72Data[sr] == '}'){
                level--;
            }
            else if(s72Data[sr] == ',' && level == 0){
                /* Find the colon that splits the key and the value */
                size_t colon = FindColon(sl,sr);
                std::get<ParserNode::PNMap>(obj->data)[s72Data.substr(sl+1,colon-sl-2)] = ParseInput(colon+1,sr-1);
                sl = sr+1;
            }
            sr++;
        }
        /* Add the last node */
        size_t colon = FindColon(sl,sr);
        std::get<ParserNode::PNMap>(obj->data)[s72Data.substr(sl+1,colon-sl-2)] = ParseInput(colon+1,sr);
    }
    else{
        throw std::runtime_error("Parse Error: Invalid character read from the string.");
    }

    return obj;
}


/**
 * @brief This function will remove the 'space', 'tab', 'newline', and 'return' from the s72 file,
 * and store the new string in s72Data in the XZJParser instance.
 * It will not deal with those characters that are within a string e.g. filename that has a space.
 * @param input The original s72 data stored as a string.
 */
void XZJParser::RemoveSpace(const std::string& input) {

    /* Check if we are currently within a string */
    bool isString = false;
    bool isComment = false;
    /* Allocate enough space so that we don't need to do allocation again */
    s72Data.reserve(input.length());

    for(size_t index = 0; index < input.length(); index++){
        /* If there is a '"', it means we are entering/exiting a string */
        const char c = input[index];

        if(c == '"'){
            isString = !isString;
        }
        if(index < input.length()-1 && c == '/' && input[index+1] == '*' ){
            isComment = true;
            continue;
        }
        else if(index < input.length()-1 && c == '*' && input[index+1] == '/' ){
            isComment = false;
            index += 1;
            continue;
        }


        /* Append the valid character */
        if((c != ' ' && c != '\t' && c != '\n' && c != '\r' && !isComment) || isString){
            s72Data.push_back(c);
        }
    }

    /* Remove the reserved space that are unused */
    s72Data.shrink_to_fit();
}


/**
 * @brief For a line within a JSON object, find the position of the colon that separates the key and the value.
 * @param l The left bound of the line within s72Data.
 * @param r The right bound of the line within s72Data.
 * @return The position of the colon.
 */
size_t XZJParser::FindColon(size_t l, size_t r) {
    /* Use isString because we don't care about the colon within a string. */
    bool isString = false;

    for(size_t i = l; i < r; i++){
        if(s72Data[i] == '"'){
            isString = !isString;
        }
        else if(s72Data[i] == ':' && !isString){
            return i;
        }
    }

    throw std::runtime_error("Parse Error: Error Finding Colon.");
}



