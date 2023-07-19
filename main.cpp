#include <iostream>

#include "lambda_nodes.h"

int main(int argc, char** argv)
{
    Node foo{HEAD};
    Node bar{JOIN};
    Node::connect(foo.X, bar.X);
    Node::connect(bar.A, bar.B);
    std::cout << (bar.A.connection->node->type == JOIN) << '\n';
    return 0;
}
