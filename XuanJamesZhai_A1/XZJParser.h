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


/**
 * @brief Node That will store the parsed data and later form a tree structure
 * The Node has four options:
 *      string: String in JSON
 *      double: Number in JSON
 *      PNVector: Array in JSON
 *      PNMap: Object in JSON
 */
class ParserNode{
public:
    /* A type-define for a vector of nodes. */
    typedef std::vector<std::shared_ptr<ParserNode>> PNVector;
    /* A type-define for a map of string and node. */
    typedef std::map<std::string,std::shared_ptr<ParserNode>> PNMap;

    /* Each node has a data, it can represent the four options. */
    std::variant<std::string, double, PNVector, PNMap> data;
};


/**
 * @brief A parser that will parse the .s72 file and store them into a tree-like data structure.
 * Each node on the tree is a ParseNode, and there is a root which represents the object in s72.
 */
class XZJParser {

private:
    /* The whole s72 file as a string */
    std::string s72Data;

    /* The scene object as the root of the data structure */
    std::shared_ptr<ParserNode> root;

public:
    /* Parse The s72 file with a given name */
    void Parse(const std::string&);

    /* Parse the substring of s72Data between left and right */
    std::shared_ptr<ParserNode> ParseInput(size_t,size_t);

    /* Remove all the space and unnecessary character from input and store result into s72Data */
    void RemoveSpace(const std::string&);

    /* Find the position of the ':' colon in s72Data between left and right. */
    size_t FindColon(size_t,size_t);

    /* Reconstruct all the nodes to form a tree structure and let the scene object to be the root */
    void ReconstructRoot();

    /* Reconstruct a node and reset all its children */
    void ReconstructNode(std::shared_ptr<ParserNode>&);
};


#endif //XUANJAMESZHAI_A1_XZJPARSER_H
