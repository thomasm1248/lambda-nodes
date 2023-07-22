#include <vector>

#ifndef LAMBDA_NODES
#define LAMBDA_NODES

class LambdaNodes
{
public:
    // Class-specific types to make things clearer
    enum GateType {
        N,

        H, A, B, X, S,
        AX, BX, A2X, B2X, I,

        R, JJ, OO
    };
    enum NodeType {NONE, HEAD, JOIN, SPLIT};
    typedef int Node;
    struct Gate;
    typedef std::vector<Node> Cluster;
    struct GatePair;

private:
    // This 2D vector will contain the entire node graph
    std::vector<std::vector<GateType>> table;
    // A vector for keeping track of the type of each node
    std::vector<NodeType> types;

public:
    // Constructor
    LambdaNodes();
    // Some functions for interacting with the graph
    Node getHead();
    void printTable();
    Gate followGate(Node node, GateType type);
    Gate followGate(Gate gate);
    std::vector<Gate> getGatesTo(Node node);
    std::vector<Node> getConnectedNodes(Node node);
    // Some functions for building the graph
    Node createNode(NodeType type);
    void disconnectGate(Node node, GateType gateType);
    void disconnectGate(Gate gate);
    void connect(Node node1, GateType type1, GateType type2, Node node2);
    void connect(Node node1, GateType type1, Gate gate2);
    void connect(Gate gate1, GateType type2, Node node2);
    void connect(Gate gate1, Gate gate2);
    // Graph transformations
    GatePair prepareNeighborsForJoin(Node node, std::vector<Node> neighbors);
    void join(Node node1, Node node2);
    Cluster selectCluster(Gate gate);
    void copy(Gate sourceGate, Gate destinationGate);
    // This is the main part: The code that actually simulates everything
    bool propagatePulse(int limit);
    void run();
    void run(int limit);
};

struct LambdaNodes::Gate {
    Node node;
    GateType type;
    Gate(Node node, GateType type)
        : node(node)
        , type(type)
    {}
};

#endif
