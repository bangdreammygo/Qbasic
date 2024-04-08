/////////////////////////////////用来评估合法性的类///////////////////
#ifndef LEAGLEJUDGER_H
#define LEAGLEJUDGER_H
#include"Config.h"
#include<QVector>
#include<QString>
#include<QDebug>

class leagleJudger
{
public:
    leagleJudger();
    bool isNameLeagle(QString name);
    bool isExpressionVarleagle(QVector<QString>var);
    bool isNumberName(QString num);
    bool isoperator(QString str);
};

#endif // LEAGLEJUDGER_H
