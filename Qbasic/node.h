#ifndef NODE_H
#define NODE_H
#include"Config.h"
#include<QVector>
#include<QString>
#include<QDebug>

class node
{
public:
    node(QString str);
    void clear();
    ~node();
    node*left=nullptr;
    node*right=nullptr;
    QString name;
    int value;
    int height=1;
};

#endif // NODE_H
