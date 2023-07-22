#include <cstddef>
#include <iostream>
#include <queue>

#include "lambda_nodes.h"

const int NOT_FOUND = -1;

// Define the structs that need to be defined
struct LambdaNodes::GatePair {
    Gate a;
    Gate b;
    GatePair(Gate a, Gate b): a(a), b(b) {}
};

/* LambdaNodes graph constructor: It initializes a graph with a single head node.
*/
LambdaNodes::LambdaNodes()
{
    // Create the first row and column
    table.push_back(std::vector<GateType>());
    table[0].push_back(N);

    // Add a HEAD node to the node list
    types.push_back(HEAD);
}

/* Return the head node in the graph
*/
LambdaNodes::Node LambdaNodes::getHead() { return 0; }

/* Prints the table for debugging purposes
*/
void LambdaNodes::printTable()
{
    for(int i = 0; i < table.size(); i++)
    {
        std::cout << i << " : ";
        for(GateType type : table[i])
        {
            if(type)
                std::cout << type << ' ';
            else
                std::cout << "  ";
        }
        std::cout << '\n';
    }
}

/* Follow a gate and return the next node, or return NOT_FOUND
*/
LambdaNodes::Gate LambdaNodes::followGate(Node node, GateType type)
{
    // Search row for gate
    Node otherNode;
    bool gateFound = false;
    for(int i = 0; i < table[node].size(); i++)
    {
        if(table[node][i] == type)
        {
            otherNode = i;
            gateFound = true;
            break;
        }
    }

    // Return the next node, or NOT_FOUND if not found
    if(gateFound)
        return Gate(otherNode, table[otherNode][node]);
    else
        return Gate(NOT_FOUND, N);
}
LambdaNodes::Gate LambdaNodes::followGate(Gate gate)
{
    return followGate(gate.node, gate.type);
}

/* Returns a list of gates of all outgoing connections from specified node.
*/
std::vector<LambdaNodes::Gate> LambdaNodes::getGatesTo(LambdaNodes::Node node)
{
    std::vector<Gate> gates;
    // Iterate through node's column
    for(int i = 0; i < table.size(); i++)
        if(table[node][i] != N)
            // A connection to the node has been found
            gates.push_back(Gate(i, table[i][node]));
    return gates;
}

/* Return a list of all nodes a given node is connected to.
*/
std::vector<LambdaNodes::Node> LambdaNodes::getConnectedNodes(Node node)
{
    std::vector<Node> nodes;
    for(int i = 0; i < table[node].size(); i++)
        if(table[node][i] != N)
            nodes.push_back(i);
    return nodes;
}

/* Create a node with specified type, and return it.
*/
LambdaNodes::Node LambdaNodes::createNode(LambdaNodes::NodeType type)
{
    // Insert a new table entry for the node
    int length = table.at(0).size();
    Node newNode = length;
    table.push_back(std::vector<GateType>(length));

    // Add another column
    for(auto& i : table)
    {
        i.push_back(N);
    }

    // Add entry in types list for the node
    types.push_back(type);

    return newNode;
}

/* Searches a node's table row for a gate of a particular type, and breaks the
   connection. Once the gate is found, the reverse connection will also be broken.
*/
void LambdaNodes::disconnectGate(Node node, GateType gateType)
{
    // Find gate
    Node connectedNode = followGate(node, gateType).node;

    // Clear the connection if there is one
    if(connectedNode != NOT_FOUND)
    {
        table[node][connectedNode] = N;
        table[connectedNode][node] = N;
    }
}

/* Overloaded functions for connecting nodes together
*/
void LambdaNodes::connect(Node node1, GateType type1, GateType type2, Node node2)
{
    // Remove existing connections that use the specified gates
    disconnectGate(node1, type1);
    disconnectGate(node2, type2);

    // Check for double connections
    if(table[node1][node2] != N)
    {
        if(types[node1] == SPLIT && types[node2] == SPLIT)
        {
            // Connection between SPLIT nodes
            std::cout << "error: attempted to create a double connection between SPLIT node(s).\n";
            return;
        }
        else if(types[node1] == SPLIT || types[node2] == SPLIT)
        {
            // Check for an attempted triple connection
            GateType existingType1 = table[node1][node2];
            if(existingType1 == OO || existingType1 == JJ)
            {
                std::cout << "error: attempted to connect JOIN node and SPLIT node together thrice.\n";
                return;
            }

            // Swap the nodes so the second is the split (makes code simpler)
            if(types[node1] == SPLIT)
            {
                Node tempNode = node1;
                node1 = node2;
                node2 = tempNode;
                GateType tempType = type1;
                type1 = type2;
                type2 = tempType;
            }

            // Create a special connection between the two nodes
            table[node1][node2] = JJ;
            table[node2][node1] = OO;
        }
        else if(types[node1] == JOIN && types[node2] == JOIN)
        {
            // Check for a number of different cases, and resolve each separately
            GateType existingType1 = table[node1][node2];
            GateType existingType2 = table[node2][node1];
            if(existingType1 != X && existingType1 != A && existingType1 != B)
            {
                // Triple connection
                std::cout << "error: attempted to create a triple connection between JOIN node(s).\n";
                return;
            }
            else if(node1 == node2)
            {
                // Double connection between node and itself
                // This should never happen
                std::cout << "error: attempted to create double connection between JOIN node and itself. Not possible!.\n";
                return;
            }
            else if(
                existingType1 == X && existingType2 == X ||
                type1 == X && type2 == X ||
                (type1 == A || type1 == B) &&
                    type1 == type2 &&
                    (existingType1 == A || existingType1 == B) &&
                    existingType1 == existingType2
            )
            {
                // Cluster resolves to a single connection
                // Replace both nodes with a single connection
                // Break the connection between the two nodes
                disconnectGate(node1, existingType1);
                // Get connections leading to neighbooring nodes
                auto extraGates1 = getGatesTo(node1);
                auto extraGates2 = getGatesTo(node2);
                if(extraGates1.size() == 0 || extraGates2.size() == 0)
                {
                    std::cout << "error: missing connections found when resolving a redundant node.\n";
                    return;
                }
                Gate extraGate1 = extraGates1[0];
                Gate extraGate2 = extraGates2[0];
                // Connect the external gates together
                connect(extraGate1, extraGate2);
            }
            else if(
                (type1 == X || existingType1 == X) &&
                (type2 == X || existingType2 == X)
            )
            {
                // Both X gates will be internal, forming a dead cluster
                std::cout << "error: attempted to form a dead cluster.\n";
            }
            else if(
                (type1 == A || type1 == B) &&
                type1 == existingType2 &&
                (existingType1 == A || existingType1 == B) &&
                existingType1 == type2
            )
            {
                // A reverse cluster has been formed
                table[node1][node2] = R;
                table[node2][node1] = R;
            }
            else
            {
                // I think we've narrowed it down to a useful double connection
                // Use the node with the internal X gate as node1
                // Check if that's how it currently is
                if(type1 != X && existingType1 != X)
                {
                    // The current setup doesn't match, so swap the node variables
                    Node tempNode = node1;
                    node1 = node2;
                    node2 = tempNode;
                    GateType tempType = type1;
                    type1 = type2;
                    type2 = tempType;
                    tempType = existingType1;
                    existingType1 = existingType2;
                    existingType2 = tempType;
                }
                // Encode state of connection using special gates
                // Record the state of connection from node1's perspective
                if(type1 == A || existingType1 == A)
                    table[node1][node2] = AX;
                else
                    table[node1][node2] = BX;
                // Record the state of connection from node2's perspective
                if(
                    type1 == X && type2 == A ||
                    existingType1 == X && existingType2 == A
                )
                    table[node2][node1] = A2X;
                else
                    table[node2][node1] = B2X;
            }
        }
        else
        {
            // This should never happen
            std::cout << "error: attempted to double connect nodes that aren't type JOIN or SPLIT.\n";
            return;
        }
    }
    else
    {
        // No existing connection
        // Check if the node is being connected to itself
        if(node1 == node2)
        {
            table[node1][node1] = I;
        }
        else
        {
            // Good heavens! A nice, regular connection at last!
            table[node1][node2] = type1;
            table[node2][node1] = type2;
        }
    }
}
void LambdaNodes::connect(Node node1, GateType type1, Gate gate2)
{ connect(node1, type1, gate2.type, gate2.node); }
void LambdaNodes::connect(Gate gate1, GateType type2, Node node2)
{ connect(gate1.node, gate1.type, type2, node2); }
void LambdaNodes::connect(Gate gate1, Gate gate2)
{ connect(gate1.node, gate1.type, gate2.type, gate2.node); }

/* Expands special gates, and returns a pair of gates to be used in a join operation.
*/
LambdaNodes::GatePair LambdaNodes::prepareNeighborsForJoin(Node node, std::vector<Node> neighbors)
{
    if(neighbors.size() == 1)
    {
        // Get gate type of incoming connections
        GateType incomingType = table[neighbors[0]][node];
        GateType outgoingType = table[node][neighbors[0]];
        // Remove neighbor's connection to the node
        table[neighbors[0]][node] = N;
        // Check for a variety of cases
        if(outgoingType == R)
            return GatePair(
                Gate(neighbors[0], B),
                Gate(neighbors[0], A));
        else if(outgoingType == JJ)
            return GatePair(
                Gate(neighbors[0], A),
                Gate(neighbors[0], B));
        else if(outgoingType == A2X && incomingType == BX)
            return GatePair(
                Gate(neighbors[0], X),
                Gate(neighbors[0], B));
        else if(outgoingType == A2X && incomingType == AX)
            return GatePair(
                Gate(neighbors[0], X),
                Gate(neighbors[0], A));
        else if(outgoingType == B2X && incomingType == BX)
            return GatePair(
                Gate(neighbors[0], B),
                Gate(neighbors[0], X));
        else if(outgoingType == B2X && incomingType == AX)
            return GatePair(
                Gate(neighbors[0], A),
                Gate(neighbors[0], X));
        else
        {
            std::cout << "error: while expanding a special gate, an illegal gate was encountered.\n";
            return GatePair(Gate(NOT_FOUND, N), Gate(NOT_FOUND, N));
        }
    }
    else if(neighbors.size() == 2)
    {
        // Let's assume that the node wasn't doubly connected to any of its two
        // neighbors because that should be impossible.
        return GatePair(
            Gate(neighbors[0], table[neighbors[0]][node]),
            Gate(neighbors[1], table[neighbors[1]][node]));
    }
    else
    {
        std::cout << "error: while preparing nodes, node had an illegal number of connections.\n";
        return GatePair(Gate(NOT_FOUND, N), Gate(NOT_FOUND, N));
    }
}

/* Join two JOIN nodes together, while accounting for a number of odd special cases.
   This function assumes that the first node was the first the iterator arrived at.
*/
void LambdaNodes::join(Node node1, Node node2)
{
    /* Check for some possible errors
    if(node1 == node2)
    {
        std::cout << "error: attempted to join a node with itself.\n";
        return;
    }
    if(table[node1][node2] != X || table[node2][node1] != X)
    {
        std::cout << "error: attempted to join nodes that weren't connected by their X gates.\n";
        return;
    }
    // */

    // Disconnect nodes
    disconnectGate(node1, X);

    // Get connections to neighboring nodes
    auto connections1 = getConnectedNodes(node1);
    auto connections2 = getConnectedNodes(node2);

    // Check for identity function case
    if(connections2[0] == I)
    {
        // Connect node1's neighbors together
        // Prepare node1's neighbors
        GatePair pair = prepareNeighborsForJoin(node1, connections1);
        // Connect pair of gates to each other
        connect(pair.a, pair.b);
        // Finish
        return;
    }
    
    // Prepare ends
    GatePair pair1 = prepareNeighborsForJoin(node1, connections1);
    GatePair pair2 = prepareNeighborsForJoin(node2, connections2);

    // Join ends
    connect(pair1.a, pair2.a);
    connect(pair1.b, pair2.b);
}

/* Searches through graph starting at a given gate, and collects all encountered
   nodes into a list.
*/
LambdaNodes::Cluster LambdaNodes::selectCluster(Gate gate)
{
    // Define a cluster to contain all the nodes found
    Cluster cluster;

    // Define a queue to contain gates that need to be searched
    std::queue<Gate> queue;
    queue.push(gate);

    // Search through graph until queue runs out
    while(queue.size() > 0)
    {
        // Take the next gate on the queue
        Gate currentGate = queue.front();
        queue.pop();
        // Get the node the gate points to
        Node nextNode = followGate(currentGate).node;
        // Check if node is already part of cluster
        if(std::find(cluster.begin(), cluster.end(), nextNode) == cluster.end())
        {
            // Node not found, so we need to add it
            // Add node to cluster
            cluster.push_back(nextNode);
            // Add its gates to the queue
            auto connectedNodes = getConnectedNodes(nextNode);
            for(int i = 0; i < connectedNodes.size(); i++)
            {
                // Skip current node
                if(connectedNodes[i] == currentGate.node) continue;
                // Add gate to the queue
                queue.push(Gate(
                    nextNode,
                    table[nextNode][connectedNodes[i]]));
            }
        }
    }

    return cluster;
}

/* Copies a cluster of nodes attached to a gate to another gate.
*/
void LambdaNodes::copy(Gate sourceGate, Gate destinationGate)
{
    // Determine the source cluster
    Cluster sourceNodes = selectCluster(sourceGate);

    // Add new nodes
    int prevSize = table.size();
    for(int i = 0; i < sourceNodes.size(); i++)
        createNode(types[sourceNodes[i]]);

    // Copy over connection information
    for(int i = 0; i < sourceNodes.size(); i++)
        for(int j = 0; j < sourceNodes.size(); j++)
            table[prevSize + i][prevSize + j] =
                table[sourceNodes[i]][sourceNodes[j]];

    // Attach new cluster to destination gate
    connect(destinationGate, followGate(sourceGate).type, prevSize);
}
