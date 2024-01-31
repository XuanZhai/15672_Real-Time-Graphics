//
// Created by Xuan Zhai on 2024/1/30.
//

#include "XZJParser.h"

/**
 * @brief Parse a s72 file with a given file path and file name.
 * Store the result in the root node in XZJParser.
 * @param filename The name of the s72 file, also could add path to it.
 */
void XZJParser::Parse(const std::string &filename) {

    std::ifstream input = std::ifstream(filename, std::ios::binary);

    if(!input){
        throw std::runtime_error("Parse Error: Unable to open the input file.");
    }

    /* Read the whole file into a string stream */
    std::stringstream buffer;
    buffer << input.rdbuf();

    /* Remove all the empty space and unnecessary characters */
    RemoveSpace(buffer.str());

    /* Recursively parse s72Data into ParseNodes */
    root = ParseInput(0,s72Data.length()-1);
    ReconstructRoot();

}

std::shared_ptr<ParserNode> XZJParser::ParseInput(size_t l,size_t r) {

    std::shared_ptr<ParserNode> obj(new ParserNode());

    if(s72Data[l] == '['){
        ParserNode::PNVector newData;
        size_t sl = l+1;
        size_t sr = l+1;

        std::stack<size_t> sqBracket;

        while(sr != r-1){
            if(s72Data[sr] == '[' || s72Data[sr] == '{'){
                sqBracket.push(sr);
            }
            else if(s72Data[sr] == ']' || s72Data[sr] == '}'){
                if(sqBracket.empty()){
                    throw std::runtime_error("Parse Error: Square Bracket Mismatch.");
                }
                else{
                    sqBracket.pop();
                }
            }
            else if(s72Data[sr] == ',' && sqBracket.empty()){
                newData.emplace_back(ParseInput(sl,sr-1));
                sl = sr+1;
            }
            sr++;
        }
        newData.emplace_back(ParseInput(sl,sr));

        obj->data = newData;
    }
    else if(s72Data[l] == '{'){
        ParserNode::PNMap newdata;

        size_t sl = l+1;
        size_t sr = l+1;

        std::stack<size_t> brackets;

        while(sr != r-1){

            if(s72Data[sr] == '[' || s72Data[sr] == '{') {
                brackets.push(sr);
            }
            else if(s72Data[sr] == ']' || s72Data[sr] == '}'){
                if(brackets.empty()){
                    throw std::runtime_error("Parse Error: Square Bracket Mismatch.");
                }
                else{
                    brackets.pop();
                }
            }
            else if(s72Data[sr] == ',' && brackets.empty()){

                size_t colon = FindColon(sl,sr);
                newdata[s72Data.substr(sl+1,colon-sl-2)] = ParseInput(colon+1,sr-1);
                sl = sr+1;
            }
            sr++;
        }

        size_t colon = FindColon(sl,sr);
        newdata[s72Data.substr(sl+1,colon-sl-2)] = ParseInput(colon+1,sr);

        obj->data = newdata;
    }
    else if(s72Data[l] != '"'){
        std::stringstream dss(s72Data.substr(l,r-l+1));
        double d = 0;
        dss >> d;

        obj->data = d;
    }
    else{
        obj->data = s72Data.substr(l+1,r-l-1);
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

    /* Check if we are currenting within a string */
    bool isString = false;
    /* Allocate enough space so that we don't need to do allocation again */
    s72Data.reserve(input.length());

    for(const char& c : input){
        /* If there is a '"', it means we are entering/exiting a string */
        if(c == '"'){
            isString = !isString;
        }
        /* Append the valid character */
        if((c != ' ' && c != '\t' && c != '\n' && c != '\r') || isString){
            s72Data.push_back(c);
        }
    }

    /* Remove the reserved space that are unused */
    s72Data.shrink_to_fit();
}

size_t XZJParser::FindColon(size_t l, size_t r) {
    if(l >= r) {
        throw std::runtime_error("Parse Error: Error Finding Colon.");
    }

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




void XZJParser::ReconstructRoot() {
    std::shared_ptr<ParserNode> newRoot;

    for(const std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(root->data) ){

        if(std::get_if<std::string>(&node->data) != nullptr){
            continue;
        }

        ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(node->data);

        if(newMap.count("roots")){
            newRoot = node;
            break;
        }
    }

    ReconstructNode(newRoot);
    root = newRoot;
    std::cout << "Hello" << std::endl;
}

void XZJParser::ReconstructNode(std::shared_ptr<ParserNode> & newNode) {

    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);

    if(newMap.count("mesh")){
        size_t idx = (size_t)std::get<double>(newMap["mesh"]->data);
        newMap["mesh"] = std::get<ParserNode::PNVector>(root->data)[idx];
    }
    if(newMap.count("roots")){
        ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["roots"]->data);

        for(std::shared_ptr<ParserNode>& node : newVec ){
            size_t idx = (size_t)std::get<double>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node);
        }
        newMap["roots"]->data = newVec;
    }
    if(newMap.count("children")){
        ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["children"]->data);

        for(std::shared_ptr<ParserNode>& node : newVec ){
            size_t idx = (size_t)std::get<double>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node);
        }
        newMap["children"]->data = newVec;
    }

    newNode->data = newMap;
}


