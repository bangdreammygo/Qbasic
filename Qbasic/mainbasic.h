////////////////////////////////////////这个类是Qbasic的主场景类，所有的显示都是在这个类下完成
#ifndef MAINBASIC_H
#define MAINBASIC_H
#include<QString>
#include<QIcon>
#include<QPixmap>
#include<map>
#include<QMap>
#include<vector>
#include<QPair>
#include <QWidget>
#include<QMessageBox>
#include<QFileDialog>
#include"command.h"
#include"Config.h"
#include"QDebug"
#include"variable.h"
#include<QFileDialog>
#include<QFile>
#include<QTextStream>
QT_BEGIN_NAMESPACE
namespace Ui { class mainBasic; }
QT_END_NAMESPACE

class mainBasic : public QWidget
{
    Q_OBJECT
public:
    /////////////////////////////////////////////////////////////成员函数//////////////////////////////////////////
    //构造函数
    mainBasic(QWidget *parent = nullptr);
    //初始化显示函数
    void initBasic();
    //////////////////////////////////////核心函数:run函数
    void doRun();
    /////////////////写个结束函数意思意思//////////////
    void runEnd();
    //////////////////////////////////////核心函数：处理cmd的函数///////////////////
    void tackleCommand();
    //判断是否是合法的有行号输入
    bool isLeagalline(QString str);
    //判断是否为合法的立即操作输入
    bool isLeagalimmediate(QString str);
    //更新代码的面板
    void updateBoard();
    //在run起来后更新面板
    void updaterunBoard();
    //还有最终更新的面板
    void updateEndBoard();
    //几个立即函数的实现
    void doClear();
    void doImmediateInput();
    void doload();
    void doHelp();
    //最后更新一下使用次数
    void updateAlltree();

    //配合load的函数
    void tackLoadcmd(QString cmd);

    //几个行号函数的实现
    void doRem();
    void doInput();
    void runInput();
    void doInput_stage2();
    void doLet();
    void runLet();
    void doPrint();
    void runPrint();
    void runGoto();
    void doIf();
    void runIF();
    //析构，没啥用
    ~mainBasic();
    ////////////////////////////////////////////////////////////成员变量/////////////////////////////////////////////
    /////////////////几个工具
    leagleJudger judger;
    converter    varCon;
    //记录当前的命令
    QString currentCmd="\0";
    //记录目前的输入行号
    int currentInputLine=0;
    //记录目前的执行行号
    int currentRunLine=0;
    //记录上一条行号
    int lastRunline=0;
    //记录目前的input到的行号
    int currentInputline=0;
    //记录目前的指令是否是删除指令
    bool isDelete=false;
    //记录当前指令的化简后的版本
    QString currentSim;
    //记录目前的所有行号以及指令
    QMap<int,Command>cmdMap;
    //记录当前指令的类型
    int cmdType;
    //记录目前run起来没有
    bool isRun=false;
    //记录是不是还没有运行到最后一行
    bool haveRun=true;
    //记录是不是在input
    bool isInput=false;
    //记录本次运行是否已经输入完所有变量的值
    bool haveInput=false;
    //存放立即输入的input的变量
    Command tmpinput;
    //优化性能用的，检查是否已经构造过树了，如果构造过就不再浪费时间
    bool isChecked=false;
    bool isTreed=false;
    ///////////////////////////记录当前的变量
    QMap<QString,variable>allVaribales;
    ///////////////////////////做一个需要被print的vector
    QVector<int>printVar;
private:
    Ui::mainBasic *ui;
signals:
    void runnow();
    void endnow();
};
#endif // MAINBASIC_H
