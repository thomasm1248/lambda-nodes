#include <cstddef>

#include "lambda_nodes.h"

Gate::Gate(Node& n, GateType t)
    :node(n)
{
    type = t;
    connection = NULL;
};
void Node::connect(Gate& g1, Gate& g2)
{
    g1.connection = &g2;
    g2.connection = &g1;
}
Node::Node(NodeType t)
    : A{*this, TYPE_A}
    , B{*this, TYPE_B}
    , X{*this, TYPE_X}
{
    type = t;
}
void Node::join(Gate& gate)
{
    // Get the nodes on either end of the connection
    Node* node1 = &gate.node;
    Node* node2 = &gate.connection->node;
    // Get the gates connected to those nodes
    Gate& a1 = *node1->A.connection;
    Gate& b1 = *node1->B.connection;
    Gate& a2 = *node2->A.connection;
    Gate& b2 = *node2->B.connection;
    // Delete the nodes
    delete node1;
    delete node2;
    // Reconnect the gates without the nodes
    Node::connect(a1, a2);
    Node::connect(b1, b2);
}
