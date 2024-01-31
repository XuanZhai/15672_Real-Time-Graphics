//
// Created by Xuan Zhai on 2024/1/30.
//

#include "XZJParser.h"

void XZJParser::Parse(const std::string &filename) {

    std::ifstream input = std::ifstream(filename, std::ios::binary);

    if(!input){
        throw std::runtime_error("Parse Error: Unable to open the input file.");
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    str = buffer.str();

    RemoveSpace();

    root = ParseInput(0,str.length()-1);
    std::cout << std::get<ParserNode::PNVector>(root->data).size() << std::endl;
}

std::shared_ptr<ParserNode> XZJParser::ParseInput(size_t l,size_t r) {

    std::shared_ptr<ParserNode> obj(new ParserNode());

    if(str[l] == '['){
        std::vector<std::shared_ptr<ParserNode>> newData;
        size_t sl = l+1;
        size_t sr = l+1;

        std::stack<size_t> sqBracket;

        while(sr != r-1){
            if(str[sr] == '[' || str[sr] == '{'){
                sqBracket.push(sr);
            }
            else if(str[sr] == ']' || str[sr] == '}'){
                if(sqBracket.empty()){
                    throw std::runtime_error("Parse Error: Square Bracket Mismatch.");
                }
                else{
                    sqBracket.pop();
                }
            }
            else if(str[sr] == ',' && sqBracket.empty()){
                newData.emplace_back(ParseInput(sl,sr-1));
                sl = sr+1;
            }

            sr++;
        }
        newData.emplace_back(ParseInput(sl,sr));

        obj->data = newData;
    }
    else{
        obj->data = str.substr(l,r-l+1);
    }

    return obj;
}

void XZJParser::RemoveSpace() {

    size_t index = 0;
    bool isString = false;

    for(const char& c : str){
        if(c == '"'){
            isString = !isString;
        }
        if((c != ' ' && c != '\t' && c != '\n' && c != '\r') || isString){
            str[index] = c;
            index++;
        }
    }

    str.erase(index,str.length()-index);
    std::cout << str << std::endl;
}

//void XZJParser::SkipEmpty() {
//    for(;;) {
//        std::istream::int_type c = input.peek();
//        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
//            input.get();
//        } else {
//            break;
//        }
//    }
//}
