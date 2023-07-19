#include <cstddef>

#include "lambda_nodes.h"

Gate::Gate(Node* n, GateType t)
{
    type = t;
    node = n;
    connection = NULL;
};
void Node::connect(Gate& g1, Gate& g2)
{
    g1.connection = &g2;
    g2.connection = &g1;
}
Node::Node(NodeType t)
    : A{this, TYPE_A}
    , B{this, TYPE_B}
    , X{this, TYPE_X}
{
    type = t;
}
