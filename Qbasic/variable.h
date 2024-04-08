#ifndef VARIABLE_H
#define VARIABLE_H
#include<QString>
#include<QVector>
#include"Config.h"
#include"leaglejudger.h"
class variable
{
public:
    variable(QString n);
    variable(QString n,int v);
    //////////////成员变量
    //变量名
    QString name;
    //变量出现次数
    int times=0;
    //变量值
    int varValue;
    //来个判断器
    leagleJudger judge;
};

#endif // VARIABLE_H
