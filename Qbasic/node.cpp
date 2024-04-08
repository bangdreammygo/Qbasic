#include "node.h"

node::node(QString str)
{
    name=str;
}
//////////////析构函数
void node::clear()
{
    if(left==nullptr&&right==nullptr)delete this;
    if(left)left->clear();
    if(right)right->clear();
}

node::~node()
{

}
