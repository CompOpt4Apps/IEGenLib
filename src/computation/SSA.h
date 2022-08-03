//
// Created by ANT on 8/2/22.
//

#ifndef SSA_H_
#define SSA_H_

#include <vector>
#include <tuple>

class SSA {
public:
    SSA();
    virtual ~SSA();
    struct dominanceTree{
        dominanceTree * parent; // parent to the node
        std::vector<dominanceTree*>child;  // list of the Children's
        std::pair<int, std::vector<std::string>> data; // iteration domain with stmtIdx as key
    };

    dominanceTree** getDominanceTree(std::vector<std::pair<int, std::vector<std::string>>>&executionS);
};

#endif
