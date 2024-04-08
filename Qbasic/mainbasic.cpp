#include "mainbasic.h"
#include "ui_mainbasic.h"
//////////////////////////////////构造函数
mainBasic::mainBasic(QWidget *parent) : QWidget(parent), ui(new Ui::mainBasic)
{
    ui->setupUi(this);
    //初始化
    initBasic();
}
///////////////////////////////初始化basic的函数////////////////////////////
void mainBasic::initBasic()
{
   this->setWindowTitle("Qbasic 解释器");
   this->setWindowIcon(QIcon(":/icon/visual-basic-logo-png-1.jpg"));
   //开始连接信号槽
   //连接输入函数（核心函数）
    connect(ui->cmdLine,&QLineEdit::returnPressed,[=](){
       //非运行模式下，且未进入输入模式
        if(!isRun&&!isInput){
            //加入错误处理
            try{
                //核心函数直接开始处理指令
                tackleCommand();
            }
            //如果是非法输入，会抛出错误
            catch(char const* fault){
                QMessageBox::critical(this,"wrong",fault);
            }
            //更新一下code board
            updateBoard();
           if(!isRun&&!isInput)ui->cmdLine->clear();
        }
        //非运行模式但是要输入
        else if(!isRun&&isInput){
            try{
                doImmediateInput();
            }
            catch(const char*fault){
                QMessageBox::critical(this,"你干嘛~哎呦",fault);
            }
            if(isInput){
              ui->cmdLine->setText(QString("%1 =?").arg(tmpinput.inputName));
            }
            else {
                ui->cmdLine->clear();
            }
        }
        //运行模式，但是还要先输入变量
        else if(isRun&&isInput){
            try{
                tackleCommand();
            }
            catch(const char*fault){
                 QMessageBox::critical(this,"你干嘛~哎呦",fault);
            }
            if(isInput){
                ui->cmdLine->setText(QString("%1 =?").arg(cmdMap.find(lastRunline).value().inputName));
            }
            else{
                //读取输入结束了，进入下一条命令的执行
                updaterunBoard();
                if(currentRunLine==lastRunline)emit endnow();
                else emit runnow();
            }
        }
        //运行模式下能输入的反馈
        else{
                //如果是clear可以清空
                if(ui->cmdLine->text().simplified()=="CLEAR")doClear();
                //如果是quit可以退出
                else if(ui->cmdLine->text().simplified()=="QUIT")this->close();
                //又不是输入又是运行模式，所以应该禁止输入
                else ui->cmdLine->setText("运行中，请勿输入其他代码,或先清除当前程序");
        }
    });
    //连接run函数
    connect(this,&mainBasic::runnow,[=](){
        haveRun=false;
        doRun();
    });
    //连接结束函数
    connect(this,&mainBasic::endnow,[=](){
       haveRun=true;
       runEnd();
    });
    //剩下就是连接几个按钮了
    //连接CLEAR按钮(注意禁用区间)
    connect(ui->btnClear,&QPushButton::clicked,[=](){
        if(haveRun)doClear();
        else{
             QMessageBox::critical(this,"你干嘛~哎呦","程序运行中，请勿乱操作");
        }
    });
    //连接run的按钮
    connect(ui->btnRun,&QPushButton::clicked,[=](){
        if(!isRun){
            isRun=true;
            if(cmdMap.empty()){
                updaterunBoard();
                emit endnow();
            }
            else{currentRunLine=cmdMap.begin().value().inputLine;
                 updaterunBoard();
                 emit runnow();}
        }
        else{
           QMessageBox::critical(this,"你干嘛~哎呦","程序运行中，请勿乱操作");
        }
    });
    //连接load的按钮
    connect(ui->btnLoad,&QPushButton::clicked,[=](){
        if(!isRun)doload();
    });
}
//////////////////////////////////////执行run函数///////////////////////////
/////////////////////////调整运行思路：从循环变为单步执行，但反复调用///////////////////////////////
void mainBasic::doRun()
{
        bool flag=true;
        bool isitInput=false;
        //要相应修改指令执行次数
        //目前已统计次数的指令：REM END GOTO INPUT LET PRINT IF THEN
        //先找到当前需要执行的命令
        QMap<int,Command>::iterator it=cmdMap.find(currentRunLine);
        if(it==cmdMap.end())emit endnow();
        for(;it!=cmdMap.end();){
        currentRunLine=it.value().inputLine;
        //END
        if(it.value().cmdType==END){
            it.value().finaltimes+=1;
            break;
        }
        //REM
        else if(it.value().cmdType==REM){
            it.value().finaltimes+=1;
            it++;
        }
        //INPUT
        else if(it.value().cmdType==INPUT){
            isitInput=true;
            it.value().finaltimes+=1;
            updaterunBoard();
            //这里先把下一条命令找到
            if(++it!=cmdMap.end()){
                lastRunline=currentRunLine;
                currentRunLine=it.value().inputLine;
            }
            else{
                lastRunline=currentRunLine;
            }
            runInput();
            break;
        }
        //LET
        else if(it.value().cmdType==LET)
        {
            try{
                it.value().finaltimes+=1;
                //执行本条命令
                runLet();
                it++;
            }
             catch(char const* fault){
               updaterunBoard();
               QMessageBox::critical(this,"执行错误","Let语句错误");
               break;
             }
        }
        //print
        else if(it.value().cmdType==PRINT){
            try{
                it.value().finaltimes+=1;
                runPrint();
                it++;
            }
             catch(char const* fault){
                updaterunBoard();
                QMessageBox::critical(this,"执行错误","Print语句错误");
                break;
             }
        }
        //goto
        else if(it.value().cmdType==GOTO){
            try{
                it.value().finaltimes+=1;
                runGoto();
                it=cmdMap.find(currentRunLine);
            }
            catch(const char*fault){
                it.value().isLeagle=false;
                it.value().lagTree.clear();
                it.value().lagTree.push_back(QString("%1 GOTO").arg(QString::number(currentRunLine)));
                it.value().lagTree.push_back(QString("    %1").arg(QString::number(it.value().gotoAddress)));
                updaterunBoard();
                QMessageBox::critical(this,"执行错误","跳转地址为空");
                break;
             }
            catch(int a){
                 QMessageBox::critical(this,"执行错误","你不正常输入地址我怎么跳转");
                 updaterunBoard();
                 break;
                 }
        }
        //if
        else if(it.value().cmdType==IF){
            try{
                //执行跳转
                it.value().finaltimes+=1;
                int tmp=currentRunLine;
                runIF();
                if(tmp!=currentRunLine)it=cmdMap.find(currentRunLine);
                else {flag=false;break;}
            }
             catch(char const* fault){
                updaterunBoard();
                QMessageBox::critical(this,"执行错误",fault);
                break;
             }

        }
      }
      //如果是跳转失败
      if(!flag) {emit endnow();return;}
      //如果是又不是跳转失败又不是输入模式
      if(!isitInput)emit endnow();
}
/////////////////////////////////////结束时调用，意思意思///////////////
void mainBasic::runEnd()
{
    ui->cmdLine->clear();
    updateAlltree();
    updaterunBoard();
    //清扫一波没有执行的语法树
    updateEndBoard();
    ui->resultBoard->append("your program has run to end.");
    //最后为了程序的复用，清空一些东西
    printVar.clear();
    allVaribales.clear();
    currentRunLine=0;
    haveInput=false;
    isInput=false;
    //把指令全部初始化
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        it->finaltimes=0;
        it->thenTimes=0;
        if(it.value().cmdType==LET){it.value().initLet();it.value().justLettree();}
        else if(it.value().cmdType==IF){it.value().initIf();it.value().justPrinttree();}
        else if(it.value().cmdType==PRINT){it.value().initPrint();it.value().justPrinttree();}
        else if(it.value().cmdType==INPUT){it.value().doInput_1stage();it.value().justInputtree();}
        else if(it.value().cmdType==GOTO)it.value().initGoto();
        else if(it.value().cmdType==END)it.value().initEnd();
        else if(it.value().cmdType==REM)it.value().doREM();
    }
    isRun=false;
}
//////////////////////////////////////////////////////////处理相关指令的函数////////////////////////////////////////////////
void mainBasic::tackleCommand()
{
    //如果是input的模式进行中要特殊处理
    try{
        if(isInput){
            doInput_stage2();
            return;
        }
    }
    catch(char const* fault){
        throw fault;
    }
    //读取当前的输入
    currentCmd=ui->cmdLine->text();
    //第一部分，读取正常的代码输入
    if(isLeagalline(currentCmd)){
        //删除（其中删除操作已经在isleagle中实现了）
        if(isDelete) {isDelete=false;return;}
        //非删除
        cmdMap.insert(currentInputLine,Command(currentInputLine,currentCmd,cmdType));
        //根据不同的指令类型构建语法树和初始化指令
        if(cmdType==REM){
            //直接调用对应的处理函数处理它
            doRem();
        }
        ////////////////////////////////////尤其需要注意input变量的特殊性，导致所有其他的变量可能都需要到运行时才会有赋值，所以map里的值只能先不填
        else if(cmdType==INPUT){
           try{ doInput();}
            catch(char const* fault){
                throw fault;
            }
        }
        //处理let
        else if(cmdType==LET){
           try{ doLet();}
            catch(char const* fault){
                cmdMap.remove(currentInputLine);
                throw fault;
            }
        }
        //处理print
        else if(cmdType==PRINT){
            try{ doPrint();}
             catch(char const* fault){
                 cmdMap.remove(currentInputLine);
                 throw fault;
             }
        }
        //处理GOTO
        else if(cmdType==GOTO){
        }
        //处理IF
        else if(cmdType==IF){
            try{ doIf();}
             catch(char const* fault){
                 cmdMap.remove(currentInputLine);
                 throw fault;
             }
        }
        //最后再return 回去
        return;
    }
    //第二部分，读取立即执行的代码输入，这里更加复杂，因为需要立即执行！
    //先判断是否合法，否则直接丢出去
    ///////没有全部实现还需要后期补写
    if(isLeagalimmediate(currentCmd)){
        //构建出来一个临时的命令
        Command tmpCmd(0,currentCmd,cmdType);
        //分别处理不同指令
       if(cmdType==LIST){
           qDebug()<<"空执行LIST指令";
           return;
        }
        //逻辑实现，这个最后再写一个页面就行
        else if(cmdType==HELP){
            doHelp();
        }
        else if(cmdType==QUIT){
            this->close();
        }
        else if(cmdType==CLEAR){
            doClear();
        }
        ////////下面这些暂时还实现不了，等待后续实现(PRINT , INPUT , LET ,LOAD)
        else if(cmdType==RUN){
            isRun=true;
            if(cmdMap.empty()){
                updaterunBoard();
                emit endnow();
            }
            else{currentRunLine=cmdMap.begin().value().inputLine;
                 updaterunBoard();
                 emit runnow();}
        }
        //实现立即input（已经实现）
        else if(cmdType==INPUT){
           if(tmpCmd.isLeagle==false)throw "非法输入";
           //立即执行input
           isInput=true;
           tmpinput=tmpCmd;
           ui->cmdLine->setText(QString("%1 =?").arg(tmpinput.inputName));
        }
        //实现立即let（已实现）
        else if(cmdType==LET){
           try{
               //初步筛查
               if(tmpCmd.isLeagle==false)throw"非法指令";
               //检查目前里面的变量是否是已经存在于库中的变量
               for(QMap<QString,variable>::iterator it=tmpCmd.allvar.begin();it!=tmpCmd.allvar.end();it++){
                   //常数不用检查
                   if(judger.isNumberName(it.value().name))continue;
                   if(allVaribales.find(it.key())==allVaribales.end()){throw "输入非法变量";}
                   //将变量赋值过去
                   it.value().varValue=allVaribales.find(it.key()).value().varValue;
               }
               //变量检查已经过关了，所以可以执行语法树
               try{tmpCmd.reallydoLet();}
               catch (const char*fault){throw fault;}
               //目前来看语法树也构建好了，现在要把变量放进容器里了(前提是表达式合法的时候才会塞进去)
               //重新覆盖值之前要把变量的出现次数统计出来,方便后续记录次数
               if(tmpCmd.isLeagle){
                   QString name=tmpCmd.letName;
                   int vvalue=tmpCmd.v1result;
                   allVaribales.insert(name,variable(name,vvalue));
               }
               //更新面板
               ui->resultBoard->clear();
               for(int i=0;i<printVar.size();i++){
                   ui->resultBoard->append(QString::number(printVar[i]));
               }
           }
           catch(const char*fault){
               throw fault;
           }
        }
        //实现立即print（已实现）
        else if(cmdType==PRINT){
           try{
               //初步筛查
               if(tmpCmd.isLeagle==false)throw"非法指令";
               //检查目前里面的变量是否是已经存在于库中的变量
               for(QMap<QString,variable>::iterator it=tmpCmd.allvar.begin();it!=tmpCmd.allvar.end();it++){
                   //常数不用检查
                   if(judger.isNumberName(it.value().name))continue;
                   if(allVaribales.find(it.key())==allVaribales.end()){throw "输入非法变量";}
                   //将变量赋值过去
                   it.value().varValue=allVaribales.find(it.key()).value().varValue;
               }
               //变量检查已经过关了，所以可以执行语法树
               try{
                   tmpCmd.reallydoPrint();
                   //实际上这个时候就已经把变量给算好了，放在了v1varibale里面
                   if(tmpCmd.isLeagle)printVar.push_back(tmpCmd.v1result);
                   //更新一下面板
                   ui->resultBoard->clear();
                   for(int i=0;i<printVar.size();i++){
                       ui->resultBoard->append(QString::number(printVar[i]));
                   }
              }
               catch (const char*fault){throw fault;}
           }
           catch(const char*fault){
               throw fault;
           }
       }
        //实现最难的load功能
        else if(cmdType==LOAD){
           doload();
       }
       //在处理好之后返回
        return;
    }
    throw "非法输入!请重新输入";
}
/////////////////////////////判断是否合法///////////////（只需要判断基本的指令合法性）：基本已经实现了
bool mainBasic::isLeagalline(QString str)
{
    //行号处理
    QString str_num;
    int i=0;
    //跳过前置空格
    while(i<str.size()&&str[i]==' '){i++;}
    while( i<str.size()&&str[i]>='0'&&str[i]<='9'){
        str_num+=str[i++];
    }
    int num_str=str_num.toInt();
    //是否为删除指令
    if(i==str.size()){
        cmdMap.remove(num_str);
        isDelete=true;
        return true;
    }
    //检查后置空格
    if(str[i]!=' '){return false;}//无空格，非法输入
    else currentInputLine=num_str;
    //接下来应该判断这个操作的具体类型，最后才是精简表达式，或者说其实也可以不精减，内部构造函数的时候自己去精简
    QString opType;
    //跳过现在的无效空格
    while(i<str.size()&&str[i]==' '){i++;}
    //直接删除操作
    if(i==str.size()){
        cmdMap.remove(currentInputLine);
        isDelete=true;
        return true;
    }
    //把目前的具体操作符读进来
    for(;i<str.length()&&str[i]!=' ';i++){
        opType+=str[i];
    }
    //REM的具体操作
    if(opType=="REM"){
      cmdType=REM;
      return true;
    }
    //处理let操作
    else if(opType=="LET"){
        cmdType=LET;
        return true;
    }
    //处理print操作
    else if(opType=="PRINT"){
        cmdType=PRINT;
        return true;
    }
    //处理input操作
    else if(opType=="INPUT"){
        cmdType=INPUT;
        return true;
    }
    //处理goto操作
    else if(opType=="GOTO"){
        cmdType=GOTO;
        return true;
    }
    //处理if操作
    else if(opType=="IF"){
        cmdType=IF;
        return true;
    }
    //处理end操作
    else if(opType=="END"){
        cmdType=END;
        return true;
    }
    return false;
}
/////////////////////////////////////////////////判断立即操作是否合法////////////////////////////
bool mainBasic::isLeagalimmediate(QString str)
{
    int i=0;
    //空格
    while(i<str.length()&&str[i]==' ')i++;
    if(i==str.length())return false;
    QString op;
    for(;i<str.length()&&str[i]!=' ';i++){
        op+=str[i];
    }
    if(op=="RUN"){
        if(str.remove(' ')=="RUN")
        cmdType=RUN;
        return true;
    }
    else if(op=="LOAD"){
        if(str.remove(' ')=="LOAD")
       {cmdType=LOAD;
        return true;}
    }
    else if(op=="LIST"){
        if(str.remove(' ')=="LIST")
       {cmdType=LIST;
        return true;}
    }
    else if(op=="CLEAR"){
        if(str.remove(' ')=="CLEAR")
       {cmdType=CLEAR;
        return true;}
    }
    else if(op=="HELP"){
        if(str.remove(' ')=="HELP")
       {cmdType=HELP;
        return true;}
    }
    else if(op=="QUIT"){
        if(str.remove(' ')=="QUIT")
       {cmdType=QUIT;
        return true;}
    }
    else if(op=="PRINT"){
        cmdType=PRINT;
        return true;
    }
    else if(op=="LET"){
        cmdType=LET;
        return true;
    }
    else if(op=="INPUT"){
        cmdType=INPUT;
        return true;
    }
    return false;
}
/////////////////////////////更新代码面板
void mainBasic::updateBoard()
{
    //展示代码
    ui->codeBoard->clear();
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        ui->codeBoard->append(it.value().wholeCmd);
    }
}
///////////////////////////////更新运行时的代码面板
void mainBasic::updaterunBoard()
{
    ui->codeBoard->clear();
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        ui->codeBoard->append(it.value().wholeCmd);
    }
    ui->cmdLine->clear();
    ui->resultBoard->clear();
    for(int i=0;i<printVar.size();i++){
        ui->resultBoard->append(QString::number(printVar[i]));
    }
    //首先需要把这些指令中那些没有被执行，但是也没有错误的(没调用二阶段，但是一阶段初筛也没错)给生成树了
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        if(it.value().isLeagle&&it.value().lagTree.size()==0){
            //注意：GOTO和REM不需要！！！！！！！
            if(it.value().cmdType==LET)it.value().justLettree();
            else if(it.value().cmdType==PRINT)it.value().justPrinttree();
            else if(it.value().cmdType==INPUT)it.value().justInputtree();
            else if(it.value().cmdType==IF) it.value().justiftree();
        }
        else continue;
    }
    ui->treeBoard->clear();
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        for(int i=0;i<it.value().lagTree.size();i++){
            ui->treeBoard->append(it.value().lagTree[i]);
        }
    }
    ui->treeBoard->append("your tree has come to end.");
}
/////////////////////////最终面板更新（包含有GOTO导致的可能的没有显示语法树）//////////////
void mainBasic::updateEndBoard()
{
    //首先需要把这些指令中那些没有被执行，但是也没有错误的(没调用二阶段，但是一阶段初筛也没错)给生成树了
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        if(it.value().isLeagle&&it.value().lagTree.size()==0){
            //注意：GOTO和REM不需要！！！！！！！
            if(it.value().cmdType==LET)it.value().justLettree();
            else if(it.value().cmdType==PRINT)it.value().justPrinttree();
            else if(it.value().cmdType==INPUT)it.value().justInputtree();
            else if(it.value().cmdType==IF) it.value().justiftree();
        }
        else continue;
    }
    //再把树给重新整理一遍（输出部分无需整理）
    ui->treeBoard->clear();
    for(QMap<int,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        for(int i=0;i<it.value().lagTree.size();i++){
           if(it.value().cmdType==LET&&i==1){
               if(allVaribales.find(it.value().letName)==allVaribales.end()){
                    QString tmp=it.value().lagTree[i];
                    tmp+="  使用了0次";
                    ui->treeBoard->append(tmp);
               }
               else{
                   QString tmp=it.value().lagTree[i];
                   tmp+=QString("  使用了%1次").arg(QString::number(allVaribales.find(it.value().letName).value().times));
                   ui->treeBoard->append(tmp);
               }
           }
           else ui->treeBoard->append(it.value().lagTree[i]);
        }
    }
    ui->treeBoard->append("your tree has come to end.");
}
/////////////////////////清除函数
void mainBasic::doClear()
{
    allVaribales.clear();
    cmdMap.clear();
    printVar.clear();
    isRun=false;
    ui->cmdLine->clear();
    ui->resultBoard->clear();
    ui->treeBoard->clear();
    ui->codeBoard->clear();
    currentRunLine=0;
    haveInput=false;
    isInput=false;
}
///////////////////////////////实现立即input/////////////////
void mainBasic::doImmediateInput()
{
    //读取输入
    QString getBack=ui->cmdLine->text();
    //处理输入（要判断是否为有效输入）
    int i=0;
    //这里是已经停在问号这里了
    QString test1;
    while(i<getBack.size()-1&&getBack[i]!='?')test1+=getBack[i++];
    if(i==getBack.size()-1)throw "illeaagle input";
    if(test1.simplified()!=QString("%1 =").arg(tmpinput.inputName))throw "你为什么要删我提示符?";
    //跳过问号
    i++;
    //过掉空格
    while(i<getBack.size()&&getBack[i]==' ')i++;
    if(i==getBack.size())throw "illeaagle input";
    QString result;
    if(getBack[i]=='-')result+=getBack[i++];
    for(;i<getBack.size()&&getBack[i]<='9'&&getBack[i]>='0';i++){
        result+=getBack[i];
    }
    while(i<getBack.size()&&getBack[i]==' ')i++;
    if(i!=getBack.size())throw "illeaagle input";
    int finalresult=result.toInt();
    //将值放进allvar里面
    allVaribales.insert(tmpinput.inputName,variable(tmpinput.inputName,finalresult));
    isInput=false;
}
//////////////////////////////立即函数里最难的load函数
void mainBasic::doload()
{
   cmdMap.clear();
   QString filepath=QFileDialog::getOpenFileName(this,"选择需要打开的代码","C:/Users/asus/Desktop");
   if(filepath.isEmpty())return;
   //读取文件信息
   QFile file(filepath);
   file.open(QIODevice::ReadOnly);
   QTextStream stream(&file);
   bool flag=true;
   //循环读入文件
   while(!stream.atEnd()){
       QString str=stream.readLine();
       QString test=str;
       //如果读取到的是空，则跳过
       if(test.remove(' ').size()==0)continue;
       //非空则需要考虑是否添加
       try{
         tackLoadcmd(str);
       }
       catch(const char*f){
           flag=false;
       }
   }
   if(flag)QMessageBox::information(this,"提示","读取成功");
   else    QMessageBox::critical(this,"读取错误","存在严重非法代码，已经为您去除");
   updateBoard();
}
//////////////////////////////帮助/////////////////////////////////
void mainBasic::doHelp()
{
    //https://baike.baidu.com/item/BASIC/207698
    QMessageBox::information(this,"帮助","你可以自由的在本解释器上进行代码创作，通过下方的LOAD按钮加载已经写好的代码，通过RUN运行;代码通过下方命令行进行输入，通过输入具体指令进行操作。详情具体的信息，可通过该文件夹下的qbasic.pdf文档进行查阅，或查阅该网站：https://baike.baidu.com/item/BASIC/207698 进行进一步查阅。");
}
///////////////////////////更新所有指令的使用次数/////////
void mainBasic::updateAlltree()
{
    for(QMap<int ,Command>::iterator it=cmdMap.begin();it!=cmdMap.end();it++){
        if(it.value().cmdType!=IF){
            QString tmp=QString("  使用了  %1次").arg(QString::number(it->finaltimes));
            it.value().lagTree[0]+=tmp;
        }
        else{
          QString tmp=QString(" 分别使用了   %1  %2  次").arg(QString::number(it->finaltimes)).arg(QString::number(it->thenTimes));
          it.value().lagTree[0]+=tmp;
        }
    }
}
////////////////////////////LOAD的指令处理函数（懒狗拿tackle直接改的，所以看着多，实际是直接复用的）/////////
void mainBasic::tackLoadcmd(QString cmd)
{
    currentCmd=cmd;
    //第一部分，读取正常的代码输入
    if(isLeagalline(currentCmd)){
        //删除（其中删除操作已经在isleagle中实现了）
        if(isDelete) {isDelete=false;return;}
        //非删除
        cmdMap.insert(currentInputLine,Command(currentInputLine,currentCmd,cmdType));
        //根据不同的指令类型构建语法树和初始化指令
        if(cmdType==REM){
            //直接调用对应的处理函数处理它
            doRem();
        }
        ////////////////////////////////////尤其需要注意input变量的特殊性，导致所有其他的变量可能都需要到运行时才会有赋值，所以map里的值只能先不填
        else if(cmdType==INPUT){
           try{ doInput();}
            catch(char const* fault){
                throw fault;
            }
        }
        //处理let
        else if(cmdType==LET){
           try{ doLet();}
            catch(char const* fault){
                cmdMap.remove(currentInputLine);
                throw fault;
            }
        }
        //处理print
        else if(cmdType==PRINT){
            try{ doPrint();}
             catch(char const* fault){
                 cmdMap.remove(currentInputLine);
                 throw fault;
             }
        }
        //处理GOTO
        else if(cmdType==GOTO){
        }
        //处理IF
        else if(cmdType==IF){
            try{ doIf();}
             catch(char const* fault){
                 cmdMap.remove(currentInputLine);
                 throw fault;
             }
        }
        //最后再return 回去
        return;
    }
    //否则有问题，直接退出
    throw "fault";
}
////////////////////////////////实现rem////////////////////
void mainBasic::doRem()
{
    qDebug()<<"这里处理REM，但是REM有内置函数在构造时自动处理掉了";
}
///////////////////////////////实现input//////////////
//初步筛查合法性
void mainBasic::doInput()
{
    //直接错误执行
    if(cmdMap.find(currentInputLine).value().isLeagle==false){cmdMap.remove(currentInputLine);throw "iileagle input";}
}
//////////////////////////////////////把变量放进变量map里
void mainBasic::runInput()
{
    //唤醒input模式
    isInput=true;
    //目前进入提示符状态
    ui->cmdLine->setText(QString("%1 =?").arg(cmdMap.find(lastRunline).value().inputName));
}
///////////////////////////二阶段input///////////////
//把变量放在command内部，不要插入变量map里面
void mainBasic::doInput_stage2()
{
    //读取输入
    QString getBack=ui->cmdLine->text();
    //处理输入（要判断是否为有效输入）
    int i=0;
    //这里是已经停在问号这里了
    QString test1;
    while(i<getBack.size()-1&&getBack[i]!='?')test1+=getBack[i++];
    if(i==getBack.size()-1)throw "illeaagle input";
    if(test1.simplified()!=QString("%1 =").arg(cmdMap.find(lastRunline).value().inputName))throw "你为什么要删我提示符?";
    //跳过问号
    i++;
    //过掉空格
    while(i<getBack.size()&&getBack[i]==' ')i++;
    if(i==getBack.size())throw "illeaagle input";
    QString result;
    if(getBack[i]=='-')result+=getBack[i++];
    for(;i<getBack.size()&&getBack[i]<='9'&&getBack[i]>='0';i++){
        result+=getBack[i];
    }
    while(i<getBack.size()&&getBack[i]==' ')i++;
    if(i!=getBack.size())throw "illeaagle input";
    int finalresult=result.toInt();
    //把input里的变量赋值
    cmdMap.find(lastRunline).value().doInput_2stage(finalresult);
    //将值放进allvar里面
    //先找找有没有这个变量
    int t=0;
    if(allVaribales.find(cmdMap.find(lastRunline).value().inputName)!=allVaribales.end())t=allVaribales.find(cmdMap.find(lastRunline).value().inputName).value().times;
    allVaribales.insert(cmdMap.find(lastRunline).value().inputName,variable(cmdMap.find(lastRunline).value().inputName,cmdMap.find(lastRunline).value().inputValue));
    allVaribales.find(cmdMap.find(lastRunline).value().inputName).value().times=t;
    isInput=false;
}
/////////////////////////实现let///////////////////////
void mainBasic::doLet()
{

   if(cmdMap.find(currentInputLine).value().isLeagle==false){throw "iileagle input";}

}
//////////////////////////////////开始run过后的执行//////////////
void mainBasic::runLet()
{
    //检查目前里面的变量是否是已经存在于库中的变量
    for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
        //常数不用检查
        if(judger.isNumberName(it.value().name))continue;
        if(allVaribales.find(it.key())==allVaribales.end()){throw "iileagle input";}
        //将变量赋值过去(次数也要)
        it.value().varValue=allVaribales.find(it.key()).value().varValue;
        it.value().times=allVaribales.find(it.key()).value().times;
    }
    //变量检查已经过关了，所以可以执行语法树
    try{cmdMap.find(currentRunLine).value().reallydoLet();}
    catch (const char*fault){throw fault;}
    //目前来看语法树也构建好了，现在要把变量放进容器里了(前提是表达式合法的时候才会塞进去)
    //重新覆盖值之前要把变量的出现次数统计出来,方便后续记录次数
    if(cmdMap.find(currentRunLine).value().isLeagle){
        QString name=cmdMap.find(currentRunLine).value().letName;
        int vvalue=cmdMap.find(currentRunLine).value().v1result;
        int t=0;
        if(allVaribales.find(name)!=allVaribales.end())t=allVaribales.find(name).value().times;
        allVaribales.insert(name,variable(name,vvalue));
        allVaribales.find(name).value().times=t;
        for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
            //常数不用管
            if(judger.isNumberName(it.value().name))continue;
            //将变量赋值过去(次数也要)
            allVaribales.find(it.value().name).value().times=it.value().times;
        }
    }
}
/////////////////////////实现print////////////////////////////
void mainBasic::doPrint()
{
    if(cmdMap.find(currentInputLine).value().isLeagle==false){throw "iileagle input";}
}
///////////////////run起来print////////////////////////////
void mainBasic::runPrint()
{
    //检查目前里面的变量是否是已经存在于库中的变量
    for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
        //常数不用检查
        if(judger.isNumberName(it.value().name))continue;
        if(allVaribales.find(it.key())==allVaribales.end()){throw "iileagle input";}
        //将变量赋值过去
        it.value().varValue=allVaribales.find(it.key()).value().varValue;
        it.value().times=allVaribales.find(it.key()).value().times;
    }
    try{
        cmdMap.find(currentRunLine).value().reallydoPrint();
    }
    catch(const char*fault){
        throw fault;
    }
    //实际上这个时候就已经把变量给算好了，放在了v1varibale里面
    if(cmdMap.find(currentRunLine).value().isLeagle){
        printVar.push_back(cmdMap.find(currentRunLine).value().v1result);
        //然后就是变量次数赋值
        for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
            //常数不用管
            if(judger.isNumberName(it.value().name))continue;
            //将变量赋值过去(次数也要)
            allVaribales.find(it.key()).value().times=it.value().times;
        }
    }
}
///////////////////////////////////////简简单单run一下goto///////////////////////////
void mainBasic::runGoto()
{
    int a=7;
    if(cmdMap.find(currentRunLine).value().isLeagle==false)throw a;
    //获取当前需要跳转的地址
    int address=cmdMap.find(currentRunLine).value().gotoAddress;
    //确认该地址不为空
    if(cmdMap.find(address)==cmdMap.end())throw "无效地址";
    else currentRunLine=address;
}
//////////////////////////////初始化if指令///////////////////
void mainBasic::doIf()
{
    //直接错误执行
    if(cmdMap.find(currentInputLine).value().isLeagle==false){cmdMap.remove(currentInputLine);throw "iileagle input";}
}
/////////////////////////执行if指令///////////////////
void mainBasic::runIF()
{
    //检查目前里面的变量是否是已经存在于库中的变量
    for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
        //常数不用检查
        if(judger.isNumberName(it.value().name))continue;
        if(allVaribales.find(it.key())==allVaribales.end()){throw "if语句出错!";}
        //将变量赋值过去(次数也是)
        it.value().varValue=allVaribales.find(it.key()).value().varValue;
        it.value().times=allVaribales.find(it.key()).value().times;
    }
    //正式执行if判断语句
    try{
        cmdMap.find(currentRunLine).value().reallydoIf();
        //更新变量使用次数
        for(QMap<QString,variable>::iterator it=cmdMap.find(currentRunLine).value().allvar.begin();it!=cmdMap.find(currentRunLine).value().allvar.end();it++){
            //常数不用管
            if(judger.isNumberName(it.value().name))continue;
            //将变量赋值过去(次数也要)
            allVaribales.find(it.key()).value().times=it.value().times;
        }
        //判断是否执行跳转
        if(cmdMap.find(currentRunLine).value().isifTrue==true){
            cmdMap.find(currentRunLine).value().thenTimes+=1;
            //判断是否跳转地址不对
            int address=cmdMap.find(currentRunLine).value().ifAddress;
            if(cmdMap.find(address)==cmdMap.end())throw "if跳转地址无效";
            else currentRunLine=address;
        }
        //若不执行跳转，则继续执行下一条
        else{
            //先找到当前需要执行的命令
            QMap<int,Command>::iterator it=cmdMap.find(currentRunLine);
            //找到下一条
            if(++it!=cmdMap.end()){
                currentRunLine=it.value().inputLine;
            }
        }
    }
    catch(const char*fault){
        throw fault;
    }
}


















////////////////////////////////析构函数//////////////////////////////////
mainBasic::~mainBasic()
{
    delete ui;
}

