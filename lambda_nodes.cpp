#include <cstddef>
#include <iostream>

#include "lambda_nodes.h"

/* The one constructor for Gates
*/
Gate::Gate(Node& n, GateType t)
    : node(n)
    , type(t)
    , connection(NULL)
{}

/* The Node constructor. It only takes the type.
*/
Node::Node(NodeType t)
    : A{*this, TYPE_A}
    , B{*this, TYPE_B}
    , X{*this, TYPE_X}
    , type(t)
{}

/* Takes two gates, and will connect the pointers between them.
*/
void Node::connect(Gate& g1, Gate& g2)
{
    g1.connection = &g2;
    g2.connection = &g1;
}

/* Takes either gate in a connection between two nodes, and will join them.
   This function assumes that the gates are both TYPE_X. Horrible bugs will
   result otherwise. Beware! This function also assumes that the node that
   owns the provided gate is the first of the two gates that would be
   encountered according to the standard graph traversal rules.
*/
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
    // Check for a number of edge cases
    if(&node2->A == &b2)
    {
        Node::connect(a1, b1);
    }
    else if(&node1->A == &b2)
    {
        Node::connect(b1, a2);
    }
    else if(&node1->A == &a2)
    {
        Node::connect(b1, b2);
    }
    else if(&node1->B == &b2)
    {
        Node::connect(a1, a2);
    }
    else if(&node1->B == &a2)
    {
        Node::connect(a1, b2);
    }
    else
    {
        Node::connect(a1, a2);
        Node::connect(b1, b2);
    }
    // Delete the nodes
    delete node1;
    delete node2;
}
