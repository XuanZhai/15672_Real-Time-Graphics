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

    s72Data = "";
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
    /* If the data is a double/number. */
    else if((s72Data[l] >= '0' && s72Data[l] <= '9') || s72Data[l] == '-'){
        /* Convert the string to double using string stream. */
        std::stringstream dss(s72Data.substr(l,r-l+1));
        double d = 0;
        dss >> d;

        obj->data = d;
    }
    /* If the data is an array. */
    else if(s72Data[l] == '['){
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
                /* Find the colon that splits the key and the valu.e */
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


/**
 * @brief Reconstruct the data structure to let the scene object to be the root.
 * Also reconstruct all the children and mesh relations.
 */
void XZJParser::ReconstructRoot() {

    /* Loop through the all the nodes to find the scene node. */
    for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(root->data) ){

        /* Skip the first node which is the "s72-v1" */
        if(std::get_if<std::string>(&node->data) != nullptr){
            continue;
        }

        /* If the object has a key which is the roots, we found the scene node */
        if(std::get<ParserNode::PNMap>(node->data).count("roots")){
            /* Recursively reconstruct its children and reset the root node */
            ReconstructNode(node);
            root = node;
            return;
        }
    }
}


/**
 * @brief Reconstruct the child relation for a given node.
 * @param newNode The node we need to reconstruct.
 */
void XZJParser::ReconstructNode(std::shared_ptr<ParserNode> & newNode) {

    /* If it is not an object, no need to reconstruct it. */
    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);

    /* If it has a mesh key. */
    if(newMap.count("mesh")){
        size_t idx = (size_t)std::get<double>(newMap["mesh"]->data);
        newMap["mesh"] = std::get<ParserNode::PNVector>(root->data)[idx];
    }
    /* If it has a roots key. */
    if(newMap.count("roots")){
        ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["roots"]->data);

        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : newVec ){
            auto idx = (size_t)std::get<double>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node);
        }
        newMap["roots"]->data = newVec;
    }
    /* If it has a children key. */
    if(newMap.count("children")){
        ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["children"]->data);

        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : newVec ){
            auto idx = (size_t)std::get<double>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node);
        }
        newMap["children"]->data = newVec;
    }

    newNode->data = newMap;
}


