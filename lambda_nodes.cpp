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
void LambdaNodes::disconnectGate(Gate gate)
{
    disconnectGate(gate.node, gate.type);
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
        return GatePair(followGate(node, A), followGate(node, B));
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
    if(table[node2][connections2[0]] == I)
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

/* Applies one function to another.
*/
LambdaNodes::Gate LambdaNodes::apply(Gate func1, Gate func2)
{
    Node join = createNode(JOIN);
    connect(join, X, func1);
    connect(join, B, func2);
    return Gate(join, A);
}

/* Builds an I combinator
*/
LambdaNodes::Gate LambdaNodes::funcI()
{
    Node front = createNode(JOIN);
    connect(front, A, B, front);
    return Gate(front, X);
}

/* Builds a K combinator
*/
LambdaNodes::Gate LambdaNodes::funcK()
{
    Node first = createNode(JOIN);
    Node second = createNode(JOIN);
    Node dummy = createNode(JOIN);
    connect(first, A, X, second);
    connect(first, B, A, second);
    connect(second, B, X, dummy);
    return Gate(first, X);
}

/* Builds an S combinator
*/
LambdaNodes::Gate LambdaNodes::funcS()
{
    Node first = createNode(JOIN);
    Node second = createNode(JOIN);
    Node third = createNode(JOIN);
    Node splitThird = createNode(SPLIT);
    Node apply1to3 = createNode(JOIN);
    Node apply2to3 = createNode(JOIN);
    Node applyFinal = createNode(JOIN);
    connect(first, A, X, second);
    connect(second, A, X, third);
    connect(splitThird, S, B, third);
    connect(apply1to3, X, B, first);
    connect(apply1to3, B, A, splitThird);
    connect(apply2to3, X, B, second);
    connect(apply2to3, B, B, splitThird);
    connect(applyFinal, X, A, apply1to3);
    connect(applyFinal, B, A, apply2to3);
    connect(applyFinal, A, A, third);
    return Gate(first, X);
}

/* Moves a "pulse" through the graph, starting at the head node. Its movement will
   follow specific rules, and it will preform some sort of operation on the graph
   when it meets certain conditions.
*/
bool LambdaNodes::propagatePulse(int limit)
{
    // Start with a newline for asthetics
    std::cout << "\nSTART PULSE\n";// todo remove?

    // Create a gate to start the pulses from
    Gate currentGate = Gate(0, H);

    GateType previousGateType = N;
    for(int i = 0; i < limit; i++)
    {
        // Follow the gate
        Gate nextGate = followGate(currentGate);
        if(nextGate.node == NOT_FOUND)
        {
            // Since the regular gate type didn't work, use whatever's there
            auto connections = getConnectedNodes(currentGate.node);
            if(connections.size() == 2)
            {
                if(table[currentGate.node][connections[0]] == previousGateType)
                    nextGate = Gate(connections[1], table[connections[1]][currentGate.node]);
                else
                    nextGate = Gate(connections[0], table[connections[0]][currentGate.node]);
            }
            else
            {
                std::cout << "error: attempt to follow gate during pulse propagation failed.\n";
                return true; // halt
            }
        }
        std::cout << currentGate.node << " -> " << nextGate.node << ' '; // todo remove?

        // Check for a variety of rules
        if(types[nextGate.node] == JOIN)
        {
            if(currentGate.type == X && nextGate.type == X)
            {
                // Pair of JOIN nodes with X gates connected
                join(currentGate.node, nextGate.node);
                std::cout << "join " << currentGate.node << " & " << nextGate.node << ' ';
                return false;
            }
            else if(nextGate.type == X)
            {
                // Turn around
                previousGateType = currentGate.type;
                currentGate = Gate(nextGate.node, X);
            }
            else if(nextGate.type == A)
                // Leave out the next B gate
                currentGate = Gate(nextGate.node, B);
            else if(nextGate.type == B)
                // Leave out the next B gate
                currentGate = Gate(nextGate.node, X);
            else if(nextGate.type == A2X)
            {
                // Decide which direction to go
                if(currentGate.type == AX)
                {
                    // Turn around
                    previousGateType = currentGate.type;
                    currentGate = Gate(currentGate.node, B);
                }
                else
                {
                    // Keep going
                    currentGate = Gate(nextGate.node, X);
                }
            }
            else if(nextGate.type == B2X)
            {
                // Keep going
                currentGate = Gate(nextGate.node, X);
            }
            else
            {
                std::cout << "error: pulse didn't know what to do at JOIN node.\n";
                return true; // halt
            }
        }
        else if(types[nextGate.node] == SPLIT)
        {
            // Make sure we're not entering through the S gate
            if(nextGate.type == S)
            {
                std::cout << "error: attempted to enter SPLIT node through S gate.\n";
                return true; // halt
            }
            else
            {
                std::cout << "split at " << nextGate.node << ' ';

                // Save current gate as cluster attachment point
                Gate attachmentPoint = Gate(
                    currentGate.node,
                    table[currentGate.node][nextGate.node]);

                // Disconnect the node we came from from the split node
                if(attachmentPoint.type == JJ)
                {
                    // Expand the special gates OO and JJ, using A as the attachmentPoint
                    // point if possible, and B as the current gate the split node is
                    // attached to if possible.
                    if(previousGateType == X)
                    {
                        table[attachmentPoint.node][nextGate.node] = B;
                        table[nextGate.node][attachmentPoint.node] = B;
                        attachmentPoint.type = A;
                    }
                    else if(previousGateType == A)
                    {
                        table[attachmentPoint.node][nextGate.node] = B;
                        table[nextGate.node][attachmentPoint.node] = B;
                        attachmentPoint.type = X;
                    }
                    else
                    {
                        table[attachmentPoint.node][nextGate.node] = X;
                        table[nextGate.node][attachmentPoint.node] = B;
                        attachmentPoint.type = A;
                    }
                }
                else
                {
                    disconnectGate(nextGate);
                }

                // Find cluster to be copied
                currentGate = Gate(nextGate.node, S);
                while(true)
                {
                    // Get next node
                    Gate next = followGate(currentGate);

                    // Check if we found a non-split node yet
                    if(types[next.node] != SPLIT)
                        break;
                    else
                        currentGate = Gate(next.node, S);
                }

                // Copy cluster to attachment point
                copy(currentGate, attachmentPoint);

                // Dissolve split node
                // get all gates pointing to the split node
                auto connections = getGatesTo(nextGate.node);
                // connect the gates together so the split node is removed
                connect(connections[0], connections[1]);

                // Finish
                return false;
            }
        }
        else if(types[nextGate.node] == HEAD)
        {
            // Returned to HEAD node, halt
            std::cout << "halt ";
            return true;
        }
        std::cout << '\n'; // todo remove?

        // Save the gate type of the node we're entering to prevent backtracking
        previousGateType = nextGate.type;
    } // end for loop iterator

    // The limit was reached without finding a halt condition, so halt
    return true;
}

/* Creates "pulses" until a halt condition is met. It will also periodically
   prune the graph.
*/
void LambdaNodes::run(int limit)
{
    // Loop until halt condition
    while(!propagatePulse(100)){}

    // End with a newline for asthetics
    std::cout << '\n';
}
void LambdaNodes::run()
{
    run(100);
}
