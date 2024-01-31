//
// Created by Xuan Zhai on 2024/1/30.
//

#ifndef XUANJAMESZHAI_A1_XZJPARSER_H
#define XUANJAMESZHAI_A1_XZJPARSER_H

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <stack>
#include <variant>
#include <vector>
#include <map>


class ParserNode{
public:

    typedef std::vector<std::shared_ptr<ParserNode>> PNVector;
    typedef std::map<std::string,std::shared_ptr<ParserNode>> PNMap;

    size_t index = 0;
    std::variant<std::string, double, PNVector, PNMap> data;
};

class XZJParser {

private:
    //std::ifstream input;
    std::string str;

    std::shared_ptr<ParserNode> root;

public:

    std::shared_ptr<ParserNode> ParseInput(size_t,size_t);

    void RemoveSpace();

    void Parse(const std::string&);
};


#endif //XUANJAMESZHAI_A1_XZJPARSER_H
