#include <cstddef>
#include <iostream>

#include "lambda_nodes.h"

// Define the structs
struct LambdaNodes::Gate {
    Node node;
    GateType type;
    Gate(Node node, GateType type)
        : node(node)
        , type(type)
    {}
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
        std::cout << types[i] << " : ";
        for(GateType type : table[i])
            std::cout << type << ' ';
        std::cout << '\n';
    }
}

/* Follow a gate and return the next node, or return -1
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

    // Return the next node, or -1 if not found
    if(gateFound)
        return Gate(otherNode, table[otherNode][node]);
    else
        return Gate(-1, N);
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
        if(table[i][node] != N)
            // A connection to the node has been found
            gates.push_back(Gate(i, table[i][node]));
    return gates;
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
    if(connectedNode != -1)
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
{
    connect(node1, type1, gate2.type, gate2.node);
}
void LambdaNodes::connect(Gate gate1, GateType type2, Node node2)
{
    connect(gate1.node, gate1.type, type2, node2);
}
void LambdaNodes::connect(Gate gate1, Gate gate2)
{
    connect(gate1.node, gate1.type, gate2.type, gate2.node);
}
