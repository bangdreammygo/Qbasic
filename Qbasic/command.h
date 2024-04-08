#ifndef COMMAND_H
#define COMMAND_H
#include<QString>
#include<QVector>
#include"Config.h"
#include"variable.h"
#include<QDebug>
#include"converter.h"
#include"leaglejudger.h"
#include<QMap>
#include"node.h"
#include<QQueue>
#include<QStack>
#include<algorithm>
#include<qmath.h>
/////////////////////指令类
class Command
{
public:
    ///////////////////////////////函数

    //debug树用的
    void printtree(node*n);
    //构造树用的
    QVector<QString> generatTree(node*r);
    void giveAHeight(node*n,int h);
    //构造函数
    Command(int t=0,QString cmd=" ",int Type=0);
    void clear(node*p);
    //处理变量命名是否合法的函数
    bool isNameLeagle(QString name);
    //封装一个专门处理表达式的函数？
    int tackleExpression(QVector<QString>exp);
    void justteckle(QVector<QString>exp);//这个是不计算结果，只生成树的主
    //处理REM
    void doREM();
    //处理input(一二阶段)
    void doInput_1stage();
    void doInput_2stage(int finalValue);
    void justInputtree();
    //处理LET
    void initLet();
    void reallydoLet();
    void justLettree();
    //处理print
    void initPrint();
    void reallydoPrint();
    void justPrinttree();
    //处理goto
    void initGoto();
    //处理END
    void initEnd();
    //处理if
    void initIf();
    void reallydoIf();
    void justiftree();

    /////////////////生成各个树的函数封装
    QVector<QString> generatLettree(node*r);
    QVector<QString> generatPrinttree(node*r);
    QVector<QString> generatIfTree(node*r);
    /////////////////////////辅助的工具成员
    //表达式转化函数
    converter varConver;
    //合法性判断函数
    leagleJudger judgeer;
    ////////////////////////////下面是正经成员
    QString wholeCmd;//总指令
    QString simCmd;
    int cmdType;//指令类型
    int inputLine;//指令对应的行数
    //记录内部的指令是否是合法的指令
    bool isLeagle=false;
    /////////////下面是一些指令通用的性质
    int v1result;//一般需要至少一个变量执行结果
    int v2result;//(在if中会需要第二个)
    QVector<QString>lagTree;//用于打印的语法树
    node*root;//语法树本体
    QVector<QString>tmpVaribales;//记录计算过程中的变量，判断是否存在未定义的变量名
    ///////来点不同的性质
    //注释的具体信息
    QString remMessage;
    //input 的变量名字和数值大小
    QString inputName;
    int inputValue;
    //let的计算过程中，整个表达式的拆分（一段一段的）
    QVector<QString>letExpression;
    QString letName;//LET的变量的名字
    //print的计算过程中，整个表达式的拆分
    QVector<QString>printExpression;
    //goto的地址
    int gotoAddress;
    //if的前往的地址
    int ifAddress;
    //整个表达式的所有变量以及常量
    QMap<QString,variable>allvar;
    //记录目前这条命令是否已经被执行过（不用二次构建语法树）
    bool ishaveDone=false;
    //存放if指令的两条表达式
    QVector<QString>ifExp1;
    QVector<QString>ifExp2;
    //存放if的变量表达式
    QString opIf;
    //记录if语句最终的结果
    bool isifTrue;
    //记录这条指令执行了几次
    int finaltimes=0;
    int thenTimes=0;//then的次数
    bool isChecked=false;
    bool isTreed=false;
    int cnts=0;
};

#endif // COMMAND_H
