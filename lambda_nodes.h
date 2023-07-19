#ifndef LAMBDA_NODES
#define LAMBDA_NODES

enum GateType {TYPE_A, TYPE_B, TYPE_X};
enum NodeType {HEAD, JOIN, SPLIT};

// Forward declaration so that Gates can use Nodes
class Node;

/* Each Node will have Gates that connect it to other Nodes. When two Gates
   are connected, they will contain pointers to each other, and to the Nodes
   they belong to. Gates also record what type they are.
*/
class Gate
{
public:
    GateType type;
    Node& node;
    Gate *connection;
    Gate(Node& n, GateType t);
};

/* A Node will have three Gates called A, B, and X. A Node will also store
   it's type (NodeType).
*/
class Node
{
public:
    NodeType type;
    Gate A;
    Gate B;
    Gate X;
    Node(NodeType t);
    // Tools for constructing and editing graphs
    static void connect(Gate& g1, Gate& g2);
    static void join(Gate& gate);
};

#endif
