#include "command.h"
/////////////////////////////////////////////没什么实际意义，debug用的//////////////////////////
void Command::printtree(node *n)
{
    if(n->left!=nullptr)printtree(n->left);
    if(n->right!=nullptr)printtree(n->right);
}
////////////////////////////////////////////给一个根，生成qstring形式的表达式树//////////////////
QVector<QString> Command::generatTree(node*r)
{
   //造一个变量来存放构造的树
   QVector<QString> tmpTree;
   //构造辅助队列
   QQueue<node*>  nodeQueue;
   nodeQueue.enqueue(r);
   QString tmp="    ";
   //用最粗浅的思路，我先遍历一遍树，把树的节点的高度都确定出来，然后再毫无顾及地层序遍历
   if(cmdType!=IF)giveAHeight(r,1);
   else giveAHeight(r,0);
   //现在每个节点都已经有了自己的高度了，现在开始遍历
   while(!nodeQueue.empty()){
    //队头出队
    node*tmpNode=nodeQueue.dequeue();
    //儿子进队
    if(tmpNode->left)nodeQueue.enqueue(tmpNode->left);
    if(tmpNode->right)nodeQueue.enqueue(tmpNode->right);
    QString tack;
    //前置空格
    for(int i=0;i<tmpNode->height;i++){
        tack+=tmp;
    }
    tack+=tmpNode->name;
    tmpTree.push_back(tack);
   }
   return tmpTree;
}
//////////////////////////////////赋予树高度////////////////////////////
void Command::giveAHeight(node *n, int h)
{
    if(n==nullptr)return;
    n->height=h;
    if(n->left)giveAHeight(n->left,h+1);
    if(n->right)giveAHeight(n->right,h+1);
}
////////////////////////////////////////////构造函数/////////////////////////
Command::Command(int t, QString cmd ,int Type)
{
    inputLine=t;
    wholeCmd=cmd;
    cmdType=Type;
    //根据输入的类型不同来分别自动处理掉这些输入
    //REM
    if(cmdType==REM){
        doREM();
    }
    //INPUT
    else if(cmdType==INPUT){
        doInput_1stage();
    }
    //LET
    else if(cmdType==LET){
        initLet();
    }
    //PRINT
    else if(cmdType==PRINT){
        initPrint();
    }
    //GOTO
    else if(cmdType==GOTO){
        initGoto();
    }
    //END
    else if(cmdType==END){
        initEnd();
    }
    //IF
    else if(cmdType==IF){
        initIf();
    }
}
////////////////////////////////////////////释放内存////////////////////
void Command::clear(node *p)
{
    if(p->left)clear(p->left);
    if(p->right)clear(p->right);
    delete p;
}
////////////////////////////////////封装的一个处理表达式的函数并具备构造语法树能力的函数////////////////////
int Command::tackleExpression(QVector<QString> expr)
{
    //先把expr转成node*型的vector
    QVector<node*>exp;
    for(int i=0;i<expr.size();i++){
        //符号节点直接入栈
        if(judgeer.isoperator(expr[i]))exp.push_back(new node(expr[i]));
        //其余变量需要把值也搞进去（此前已经检查过非法性了，没问题的）
        else{
            node*ex=new node(expr[i]);
            if(judgeer.isoperator(ex->name))exp.push_back(ex);
            else{
                ex->value=allvar.find(expr[i]).value().varValue;
                exp.push_back(ex);
            }
        }
    }
    //操作符栈
    QStack<node*>op;
    //操作数栈
    QStack<node*>num;
    //一个标记，标记整个表达式是否合法
    try{
        ///////////////////////////////////先保证把整个表达式都取出来(不一定把整个栈运算完了)
        while(!exp.empty()){
            //取出目前的队头（所以其实我应该用queue才更好）
            QString thing;
            node*tmp=exp.front();
            thing=exp.front()->name;
            exp.pop_front();
            //是操作符进操作符栈
            if(judgeer.isoperator(thing)){
                //栈空就直接进去,且不能是右括号
                if(op.empty()&&thing!=")"){op.push(new node(thing));continue;}
                //如果是右括号，需要不断出栈直到左括号匹配
                if(thing==")"){
                   //如果栈顶是空的，也是错的
                   if(op.empty())throw"wrong expression";
                   //如果是没有其他运算符,栈顶就是左括号，除非现在num非空，不然就是错的
                   else if(op.top()->name=="("){
                       if(num.isEmpty()){qDebug()<<"debug";throw "wrong expression";}
                       else if(!num.empty()&&judgeer.isNumberName(QString::number(num.top()->value)))op.pop();
                       else{ qDebug()<<"DEBUG"; qDebug()<<num.top()->name;throw "wrong expression";}
                   }
                   else{
                      while(op.top()->name!="("){
                          if(op.empty())throw "wrong expression";//符号栈都空了还没有实现匹配，说明输错了
                          //否则就一直出栈运算
                          QString ope=op.pop()->name;
                          //加法处理
                          if(ope=="+"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1+num2;
                            //则可以新建节点了
                            node*result=new node("+");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //减法处理
                          else if(ope=="-"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1-num2;
                              //则可以新建节点了
                              node*result=new node("-");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                          }
                          //乘法处理
                          else if(ope=="*"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1*num2;
                              //则可以新建节点了
                              node*result=new node("*");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                          }
                          //乘方处理
                          else if(ope=="**"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //底数不为0
                            if(num1==0&&num2==0)throw "wrong expression";
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=qPow(num1,num2);
                            //则可以新建节点了
                            node*result=new node("**");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //除法处理
                          else if(ope=="/"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //除数不为0
                            if(num2==0)throw "wrong expression";
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1/num2;
                            //则可以新建节点了
                            node*result=new node("/");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //取模运算
                          else if(ope=="MOD"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //0不能用来取模
                            if(num2==0)throw"wrong expression";
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1%num2;
                            //统一符号
                            if(num2>0&&res<0)res+=num2;
                            else if(num2<0&&res>0)res+=num2;
                            //则可以新建节点了
                            node*result=new node("MOD");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                      }
                      //出循环了,现在栈顶是（,出栈
                      op.pop();
                   }
                   //括号处理完毕，进入下一次循环
                   continue;
                }
                //左括号直接进栈
                else if(thing=="(")op.push(new node("("));
                //如果是加号
                else if(thing=="+"){
                    //左括号享有不用出栈运算的资格
                    if(op.top()->name=="(") op.push(new node("+"));
                    else{//其他都得运算（肯定不是空栈，否则执行不到这里）
                        //取出当前栈顶的运算符号
                        QString ope=op.pop()->name;
                        //加法处理
                        if(ope=="+"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1+num2;
                          //则可以新建节点了
                          node*result=new node("+");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //减法处理
                        else if(ope=="-"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1-num2;
                            //则可以新建节点了
                            node*result=new node("-");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘法处理
                        else if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1*num2;
                            //则可以新建节点了
                            node*result=new node("*");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //底数不为0
                          if(num1==0&&num2==0)throw "wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=qPow(num1,num2);
                          //则可以新建节点了
                          node*result=new node("**");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //除数不为0
                          if(num2==0)throw "wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1/num2;
                          //则可以新建节点了
                          node*result=new node("/");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //0不能用来取模
                          if(num2==0)throw"wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1%num2;
                          //统一符号
                          if(num2>0&&res<0)res+=num2;
                          else if(num2<0&&res>0)res+=num2;
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把加号入栈
                        op.push(new node("+"));
                    }
                }
                //如果是减号，等同处理
                else if(thing=="-"){
                    //左括号享有不用出栈运算的资格
                    if(op.top()->name=="(") op.push(new node("-"));
                    else{//其他都得运算（肯定不是空栈，否则执行不到这里）
                        //取出当前栈顶的运算符号
                        QString ope=op.pop()->name;
                        //加法处理
                        if(ope=="+"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1+num2;
                          //则可以新建节点了
                          node*result=new node("+");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //减法处理
                        else if(ope=="-"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1-num2;
                            //则可以新建节点了
                            node*result=new node("-");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘法处理
                        else if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1*num2;
                            //则可以新建节点了
                            node*result=new node("*");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //底数不为0
                          if(num1==0&&num2==0)throw "wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=qPow(num1,num2);
                          //则可以新建节点了
                          node*result=new node("**");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //除数不为0
                          if(num2==0)throw "wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1/num2;
                          //则可以新建节点了
                          node*result=new node("/");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //0不能用来取模
                          if(num2==0)throw"wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1%num2;
                          //统一符号
                          if(num2>0&&res<0)res+=num2;
                          else if(num2<0&&res>0)res+=num2;
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把加号入栈
                        op.push(new node("-"));
                    }
                }
                //如果是乘号，则内测只计算除，取模，乘方
                else if(thing=="*"){
                    if(op.top()->name=="(")op.push(new node("*"));
                    else if(op.top()->name=="+")op.push(new node("*"));
                    else if(op.top()->name=="-")op.push(new node("*"));
                    else {
                        //取出当前栈顶的运算符号(不会是空栈，可以安心)
                        QString ope=op.pop()->name;
                        //乘法处理
                        if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=num1*num2;
                            //则可以新建节点了
                            node*result=new node("*");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            int num1=var1->value;
                            int num2=var2->value;
                            //底数不为0
                            if(num1==0&&num2==0)throw "wrong expression";
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            int res=qPow(num1,num2);
                            //则可以新建节点了
                            node*result=new node("**");
                            result->value=res;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //除数不为0
                          if(num2==0)throw "wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1/num2;
                          //则可以新建节点了
                          node*result=new node("/");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          int num1=var1->value;
                          int num2=var2->value;
                          //0不能用来取模
                          if(num2==0)throw"wrong expression";
                          //由于事先检查过了变量的合法性，所以它一定能算出来值
                          int res=num1%num2;
                          //统一符号
                          if(num2>0&&res<0)res+=num2;
                          else if(num2<0&&res>0)res+=num2;
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->value=res;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把乘号入栈
                        op.push(new node("*"));
                    }
                }
                //除号同理
                else if(thing=="/"){
                        if(op.top()->name=="(")op.push(new node("/"));
                        else if(op.top()->name=="+")op.push(new node("/"));
                        else if(op.top()->name=="-")op.push(new node("/"));
                        else {
                            //取出当前栈顶的运算符号(不会是空栈，可以安心)
                            QString ope=op.pop()->name;
                            //乘法处理
                            if(ope=="*"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                int num1=var1->value;
                                int num2=var2->value;
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                int res=num1*num2;
                                //则可以新建节点了
                                node*result=new node("*");
                                result->value=res;
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //乘方处理
                            else if(ope=="**"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                int num1=var1->value;
                                int num2=var2->value;
                                //底数不为0
                                if(num1==0&&(num2==0))throw "wrong expression";
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                int res=qPow(num1,num2);
                                //则可以新建节点了
                                node*result=new node("**");
                                result->value=res;
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //除法处理
                            else if(ope=="/"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //除数不为0
                              if(num2==0)throw "wrong expression";
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1/num2;
                              //则可以新建节点了
                              node*result=new node("/");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //取模运算
                            else if(ope=="MOD"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //0不能用来取模
                              if(num2==0)throw"wrong expression";
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1%num2;
                              //统一符号
                              if(num2>0&&res<0)res+=num2;
                              else if(num2<0&&res>0)res+=num2;
                              //则可以新建节点了
                              node*result=new node("MOD");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //最后把除号入栈
                            op.push(new node("/"));
                        }
                }
                //取模也是同理
                else if(thing=="MOD"){
                        if(op.top()->name=="(")op.push(new node("MOD"));
                        else if(op.top()->name=="+")op.push(new node("MOD"));
                        else if(op.top()->name=="-")op.push(new node("MOD"));
                        else {
                            //取出当前栈顶的运算符号(不会是空栈，可以安心)
                            QString ope=op.pop()->name;
                            //乘法处理
                            if(ope=="*"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                int num1=var1->value;
                                int num2=var2->value;
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                int res=num1*num2;
                                //则可以新建节点了
                                node*result=new node("*");
                                result->value=res;
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //乘方处理
                            else if(ope=="**"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                int num1=var1->value;
                                int num2=var2->value;
                                //底数不为0
                                if(num1==0&&num2==0)throw "wrong expression";
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                int res=qPow(num1,num2);
                                //则可以新建节点了
                                node*result=new node("**");
                                result->value=res;
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //除法处理
                            else if(ope=="/"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //除数不为0
                              if(num2==0)throw "wrong expression";
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1/num2;
                              //则可以新建节点了
                              node*result=new node("/");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //取模运算
                            else if(ope=="MOD"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              int num1=var1->value;
                              int num2=var2->value;
                              //0不能用来取模
                              if(num2==0)throw"wrong expression";
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              int res=num1%num2;
                              //统一符号
                              if(num2>0&&res<0)res+=num2;
                              else if(num2<0&&res>0)res+=num2;
                              //则可以新建节点了
                              node*result=new node("MOD");
                              result->value=res;
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //最后把取模号入栈
                            op.push(new node("MOD"));
                        }
                }
                //最后是乘方运算，只在内部是乘方的时候需要计算
                else if(thing=="**"){
                    if(op.top()->name!="**")op.push(new node("**"));
                    else{//没必要再取栈顶查看了，肯定就是乘方符号了
                        op.pop();
                        if(num.empty())throw"wrong expression";
                        node* var2=num.pop();
                        if(num.empty())throw"wrong expression";
                        node* var1=num.pop();
                        int num1=var1->value;
                        int num2=var2->value;
                        //底数不为0
                        if(num1==0&&num2==0)throw "wrong expression";
                        //由于事先检查过了变量的合法性，所以它一定能算出来值
                        int res=qPow(num1,num2);
                        //则可以新建节点了
                        node*result=new node("**");
                        result->value=res;
                        result->left=var1;
                        result->right=var2;
                        num.push(result);
                        //最后**入栈
                        op.push(new node("**"));
                    }
                }
            }
            //不是操作符就直接进变量栈就好
            else{
                node*tmpl=new node(thing);
                tmpl->value=tmp->value;
                num.push(tmpl);
            }
        }
        ///////////////////////////////////////while运算结束////////////////////
        //现在的情况是，取的vector空了，但是num和op里面还没有空，所以要处理这两个
        while(num.size()!=1){
            if(op.empty())throw "wrong expression";
            QString ope=op.pop()->name;
            //加法处理
            if(ope=="+"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              int num1=var1->value;
              int num2=var2->value;
              //由于事先检查过了变量的合法性，所以它一定能算出来值
              int res=num1+num2;
              //则可以新建节点了
              node*result=new node("+");
              result->value=res;
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //减法处理
            else if(ope=="-"){
                if(num.empty())throw"wrong expression";
                node* var2=num.pop();
                if(num.empty())throw"wrong expression";
                node* var1=num.pop();
                int num1=var1->value;
                int num2=var2->value;
                //由于事先检查过了变量的合法性，所以它一定能算出来值
                int res=num1-num2;
                //则可以新建节点了
                node*result=new node("-");
                result->value=res;
                result->left=var1;
                result->right=var2;
                num.push(result);
            }
            //乘法处理
            else if(ope=="*"){
                if(num.empty())throw"wrong expression";
                node* var2=num.pop();
                if(num.empty())throw"wrong expression";
                node* var1=num.pop();
                int num1=var1->value;
                int num2=var2->value;
                //由于事先检查过了变量的合法性，所以它一定能算出来值
                int res=num1*num2;
                //则可以新建节点了
                node*result=new node("*");
                result->value=res;
                result->left=var1;
                result->right=var2;
                num.push(result);
            }
            //乘方处理
            else if(ope=="**"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              int num1=var1->value;
              int num2=var2->value;
              //底数不为0
              if(num1==0&&num2==0)throw "wrong expression";
              //由于事先检查过了变量的合法性，所以它一定能算出来值
              int res=qPow(num1,num2);
              //则可以新建节点了
              node*result=new node("**");
              result->value=res;
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //除法处理
            else if(ope=="/"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              int num1=var1->value;
              int num2=var2->value;
              //除数不为0
              if(num2==0)throw "wrong expression";
              //由于事先检查过了变量的合法性，所以它一定能算出来值
              int res=num1/num2;
              //则可以新建节点了
              node*result=new node("/");
              result->value=res;
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //取模运算
            else if(ope=="MOD"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              int num1=var1->value;
              int num2=var2->value;
              //0不能用来取模
              if(num2==0)throw"wrong expression";
              //由于事先检查过了变量的合法性，所以它一定能算出来值
              int res=num1%num2;
              //统一符号
              if(num2>0&&res<0)res+=num2;
              else if(num2<0&&res>0)res+=num2;
              //则可以新建节点了
              node*result=new node("MOD");
              result->value=res;
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
        }
        //现在的情况是，num已经只剩一个了，但是op还不确定
        if(op.size()>0)throw "wrong expression";
        //好了，现在的情况是，所有表达式处理完毕了
    }
    catch(const char*fault){
        throw  fault;
    }
    root=num.pop();
    return root->value;
}
////////////////////////////////////////////////我只是想生成一棵树，不管计算了//////////////////
void Command::justteckle(QVector<QString> expr)
{
    //先把expr转成node*型的vector
    QVector<node*>exp;
    for(int i=0;i<expr.size();i++){
        //符号节点直接入栈
        if(judgeer.isoperator(expr[i]))exp.push_back(new node(expr[i]));
        //其余变量需要把值也搞进去（此前已经检查过非法性了，没问题的）
        else{
            node*ex=new node(expr[i]);
            if(judgeer.isoperator(ex->name))exp.push_back(ex);
            else{
                exp.push_back(ex);
            }
        }
    }
    //操作符栈
    QStack<node*>op;
    //操作数栈
    QStack<node*>num;
    //一个标记，标记整个表达式是否合法
    try{
        ///////////////////////////////////先保证把整个表达式都取出来(不一定把整个栈运算完了)
        while(!exp.empty()){
            //取出目前的队头（所以其实我应该用queue才更好）
            QString thing;
            node*tmp=exp.front();
            thing=exp.front()->name;
            exp.pop_front();
            //是操作符进操作符栈
            if(judgeer.isoperator(thing)){
                //栈空就直接进去,且不能是右括号
                if(op.empty()&&thing!=")"){op.push(new node(thing));continue;}
                //如果是右括号，需要不断出栈直到左括号匹配
                if(thing==")"){
                   //如果栈顶是空的，也是错的
                   if(op.empty())throw"wrong expression";
                   //如果是没有其他运算符,栈顶就是左括号，除非现在num刚刚入栈负数，不然就是错的
                   else if(op.top()->name=="("){
                       if(num.isEmpty()){qDebug()<<"debug";throw "wrong expression";}
                       else if(!num.empty()&&judgeer.isNumberName(QString::number(num.top()->value)))op.pop();
                       else{ qDebug()<<"DEBUG"; qDebug()<<num.top()->name;throw "wrong expression";}
                   }
                   else{
                      while(op.top()->name!="("){
                          if(op.empty())throw "wrong expression";//符号栈都空了还没有实现匹配，说明输错了
                          //否则就一直出栈运算
                          QString ope=op.pop()->name;
                          //加法处理
                          if(ope=="+"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("+");;
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //减法处理
                          else if(ope=="-"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("-");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                          }
                          //乘法处理
                          else if(ope=="*"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //由于事先检查过了变量的合法性，所以它一定能算出来值
                              //则可以新建节点了
                              node*result=new node("*");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                          }
                          //乘方处理
                          else if(ope=="**"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("**");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //除法处理
                          else if(ope=="/"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("/");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                          //取模运算
                          else if(ope=="MOD"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("MOD");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                          }
                      }
                      //出循环了,现在栈顶是（,出栈
                      op.pop();
                   }
                   //括号处理完毕，进入下一次循环
                   continue;
                }
                //左括号直接进栈
                else if(thing=="(")op.push(new node("("));
                //如果是加号
                else if(thing=="+"){
                    //左括号享有不用出栈运算的资格
                    if(op.top()->name=="(") op.push(new node("+"));
                    else{//其他都得运算（肯定不是空栈，否则执行不到这里）
                        //取出当前栈顶的运算符号
                        QString ope=op.pop()->name;
                        //加法处理
                        if(ope=="+"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("+");;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //减法处理
                        else if(ope=="-"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("-");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘法处理
                        else if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            //则可以新建节点了
                            node*result=new node("*");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("**");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("/");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把加号入栈
                        op.push(new node("+"));
                    }
                }
                //如果是减号，等同处理
                else if(thing=="-"){
                    //左括号享有不用出栈运算的资格
                    if(op.top()->name=="(") op.push(new node("-"));
                    else{//其他都得运算（肯定不是空栈，否则执行不到这里）
                        //取出当前栈顶的运算符号
                        QString ope=op.pop()->name;
                        //加法处理
                        if(ope=="+"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("+");;
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //减法处理
                        else if(ope=="-"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //则可以新建节点了
                            node*result=new node("-");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘法处理
                        else if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            //则可以新建节点了
                            node*result=new node("*");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("**");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("/");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把减号入栈
                        op.push(new node("-"));
                    }
                }
                //如果是乘号，则内测只计算除，取模，乘方
                else if(thing=="*"){
                    if(op.top()->name=="(")op.push(new node("*"));
                    else if(op.top()->name=="+")op.push(new node("*"));
                    else if(op.top()->name=="-")op.push(new node("*"));
                    else {
                        //取出当前栈顶的运算符号(不会是空栈，可以安心)
                        QString ope=op.pop()->name;
                        //乘法处理
                        if(ope=="*"){
                            if(num.empty())throw"wrong expression";
                            node* var2=num.pop();
                            if(num.empty())throw"wrong expression";
                            node* var1=num.pop();
                            //由于事先检查过了变量的合法性，所以它一定能算出来值
                            //则可以新建节点了
                            node*result=new node("*");
                            result->left=var1;
                            result->right=var2;
                            num.push(result);
                        }
                        //乘方处理
                        else if(ope=="**"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("**");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //除法处理
                        else if(ope=="/"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("/");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //取模运算
                        else if(ope=="MOD"){
                          if(num.empty())throw"wrong expression";
                          node* var2=num.pop();
                          if(num.empty())throw"wrong expression";
                          node* var1=num.pop();
                          //则可以新建节点了
                          node*result=new node("MOD");
                          result->left=var1;
                          result->right=var2;
                          num.push(result);
                        }
                        //最后把乘号入栈
                        op.push(new node("*"));
                    }
                }
                //除号同理
                else if(thing=="/"){
                        if(op.top()->name=="(")op.push(new node("/"));
                        else if(op.top()->name=="+")op.push(new node("/"));
                        else if(op.top()->name=="-")op.push(new node("/"));
                        else {
                            //取出当前栈顶的运算符号(不会是空栈，可以安心)
                            QString ope=op.pop()->name;
                            //乘法处理
                            if(ope=="*"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                //则可以新建节点了
                                node*result=new node("*");
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //乘方处理
                            else if(ope=="**"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("**");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //除法处理
                            else if(ope=="/"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("/");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //取模运算
                            else if(ope=="MOD"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("MOD");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //最后把除号入栈
                            op.push(new node("/"));
                        }
                }
                //取模也是同理
                else if(thing=="MOD"){
                        if(op.top()->name=="(")op.push(new node("MOD"));
                        else if(op.top()->name=="+")op.push(new node("MOD"));
                        else if(op.top()->name=="-")op.push(new node("MOD"));
                        else {
                            //取出当前栈顶的运算符号(不会是空栈，可以安心)
                            QString ope=op.pop()->name;
                            //乘法处理
                            if(ope=="*"){
                                if(num.empty())throw"wrong expression";
                                node* var2=num.pop();
                                if(num.empty())throw"wrong expression";
                                node* var1=num.pop();
                                //由于事先检查过了变量的合法性，所以它一定能算出来值
                                //则可以新建节点了
                                node*result=new node("*");
                                result->left=var1;
                                result->right=var2;
                                num.push(result);
                            }
                            //乘方处理
                            else if(ope=="**"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("**");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //除法处理
                            else if(ope=="/"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("/");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //取模运算
                            else if(ope=="MOD"){
                              if(num.empty())throw"wrong expression";
                              node* var2=num.pop();
                              if(num.empty())throw"wrong expression";
                              node* var1=num.pop();
                              //则可以新建节点了
                              node*result=new node("MOD");
                              result->left=var1;
                              result->right=var2;
                              num.push(result);
                            }
                            //最后把取模号入栈
                            op.push(new node("MOD"));
                        }
                }
                //最后是乘方运算，只在内部是乘方的时候需要计算
                else if(thing=="**"){
                    if(op.top()->name!="**")op.push(new node("**"));
                    else{//没必要再取栈顶查看了，肯定就是乘方符号了
                        op.pop();
                        if(num.empty())throw"wrong expression";
                        node* var2=num.pop();
                        if(num.empty())throw"wrong expression";
                        node* var1=num.pop();
                        //则可以新建节点了
                        node*result=new node("**");
                        result->left=var1;
                        result->right=var2;
                        num.push(result);
                        //最后**入栈
                        op.push(new node("**"));
                    }
                }
            }
            //不是操作符就直接进变量栈就好
            else{
                node*tmpl=new node(thing);
                tmpl->value=tmp->value;
                num.push(tmpl);
            }
        }
        ///////////////////////////////////////while运算结束////////////////////
        //现在的情况是，取的vector空了，但是num和op里面还没有空，所以要处理这两个
        while(num.size()!=1){
            if(op.empty())throw "wrong expression";
            QString ope=op.pop()->name;
            //加法处理
            if(ope=="+"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              //则可以新建节点了
              node*result=new node("+");
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //减法处理
            else if(ope=="-"){
                if(num.empty())throw"wrong expression";
                node* var2=num.pop();
                if(num.empty())throw"wrong expression";
                node* var1=num.pop();
                //则可以新建节点了
                node*result=new node("-");
                result->left=var1;
                result->right=var2;
                num.push(result);
            }
            //乘法处理
            else if(ope=="*"){
                if(num.empty())throw"wrong expression";
                node* var2=num.pop();
                if(num.empty())throw"wrong expression";
                node* var1=num.pop();
                //则可以新建节点了
                node*result=new node("*");
                result->left=var1;
                result->right=var2;
                num.push(result);
            }
            //乘方处理
            else if(ope=="**"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              //则可以新建节点了
              node*result=new node("**");
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //除法处理
            else if(ope=="/"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              //则可以新建节点了
              node*result=new node("/");
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
            //取模运算
            else if(ope=="MOD"){
              if(num.empty())throw"wrong expression";
              node* var2=num.pop();
              if(num.empty())throw"wrong expression";
              node* var1=num.pop();
              //则可以新建节点了
              node*result=new node("MOD");
              result->left=var1;
              result->right=var2;
              num.push(result);
            }
        }
        //现在的情况是，num已经只剩一个了，但是op还不确定
        if(op.size()>0)throw "wrong expression";
        //好了，现在的情况是，所有表达式处理完毕了
    }
    catch(const char*fault){
        throw  fault;
    }
    root=num.pop();
    return ;
}
////////////////////////////////REM的专用函数//////////////////
void Command::doREM()
{
    remMessage.clear();
    int i=0;
    while(wholeCmd[i]!='M')i++;
    i++;
    while(i<wholeCmd.size()&&wholeCmd[i]==' ')i++;
    for(;i<wholeCmd.size();i++){
        remMessage+=wholeCmd[i];
    }
    //注释肯定怎么都是对的
    isLeagle=true;
    lagTree.clear();
    //构建打印的语法树
    lagTree.push_back(QString(" %1 REM").arg(QString::number(inputLine)));
    QString leaf;
    for(int i=0;i<4;i++)leaf+=' ';
    leaf+=remMessage;
    lagTree.push_back(leaf);
}
/////////////////////////////INPUT专用函数////////////////////
//一阶段(第二次输入之前)
void Command::doInput_1stage()
{
   int i=0;
   //跳过前面的指令部分（第一遍筛查已经筛掉了不合法的输入）
   while(i<wholeCmd.size()&&wholeCmd[i]!='T')i++;
   if(i==wholeCmd.size()-1){isLeagle=false;return;}
   i++;
   //再跳过后续跟着的空格
   while(i<wholeCmd.size()&&wholeCmd[i]==' ')i++;
   //如果没有后续了，说明指令非法
   if(i==wholeCmd.size()){isLeagle=false;return;}
   QString name;
   while(i<wholeCmd.size()&&wholeCmd[i]!=' ')name+=wholeCmd[i++];
   //截取到变量名了
   //跳过后续的空格
   while(i<wholeCmd.size()&&wholeCmd[i]==' ')i++;
   //如果现在还没到结尾，说明后续还有不规范输入,该指令非法
   if(i!=wholeCmd.size()){isLeagle=false;return;}
   //如果得到的name不合法，依旧说明指令非法
   if(!judgeer.isNameLeagle(name)){isLeagle=false;return;}
   //如果以上测试都通过了，恭喜，这是一个合法的变量输入
   inputName=name;
   isLeagle=true;
   return;
}
//二阶段(第二次输入之后)
void Command::doInput_2stage(int finalValue)
{
    //非法输入，直接为error
    if(isLeagle==false){
        lagTree.push_back(QString(" %1 ERROR").arg(QString::number(inputLine)));
    }
    //合法输入，正常push进去就好
    else{
        lagTree.clear();
        lagTree.push_back(QString(" %1 INPUT").arg(QString::number(inputLine)));
        lagTree.push_back(QString("    %1").arg(inputName));
        inputValue=finalValue;
    }
}
///////////////没执行，直接搞树//////////////
void Command::justInputtree()
{
    lagTree.clear();
    lagTree.push_back(QString("%1 INPUT").arg(QString::number(inputLine)));
    lagTree.push_back(QString("    %1").arg(inputName));
}
////////////////////////////LET的专用函数/////////////////
void Command::initLet()
{
    //首先要清空var，因为可能后续不是第一次执行这条指令
    allvar.clear();
    //先把表达式部分截取出来
    int i=0;
    //跳过前面的已检测部分
    while(i<wholeCmd.size()&&wholeCmd[i]!='T')i++;
    i++;
    //截取变量名
    QString res;
    while(i<wholeCmd.size()&&wholeCmd[i]!='='){
        res+=wholeCmd[i];
        i++;
    }
    if(judgeer.isNameLeagle(res)==false){
        isLeagle=false;
        return;
    }
    //将变量名赋值
    letName=res.remove(' ');
    if(i==wholeCmd.size()||i==wholeCmd.size()-1){
        isLeagle=false;
        return;
    }
    //目前停在等号，要跳过等号
    i++;
    //截取表达式
    QString exp;
    while(i<wholeCmd.size()){
        exp+=wholeCmd[i];
        i++;
    }
    letExpression=varConver.doConvert(exp);
    if(!judgeer.isExpressionVarleagle(letExpression)){
        isLeagle=false;
        return;
    }
    //表达式初步合法
    isLeagle=true;
    //合法过后要把所有变量（常量可以不用）都塞进容器里等待筛选
    for(int i=0;i<letExpression.size();i++){
        //常量塞进去
        if(judgeer.isNumberName(letExpression[i])){
            allvar.insert(letExpression[i],variable(letExpression[i]));
        }
        //变量塞进去
        else if(judgeer.isNameLeagle(letExpression[i])){
            allvar.insert(letExpression[i],variable(letExpression[i]));
        }
        else continue;
    }
}
//真正开始做运算了
void Command::reallydoLet()
{
    try{
        v1result=tackleExpression(letExpression);
        //现在root已经到手了，可以构建语法树了
        if(!isTreed){
            lagTree.clear();
            lagTree=generatLettree(root);
            ishaveDone=false;
            isTreed=true;
        }
        //然后现在就是统计变量出现次数
         for(int i=0;i<letExpression.size();i++){
             if(judgeer.isNameLeagle(letExpression[i])){
                 allvar.find(letExpression[i])->times+=1;
             }
         }
        //释放内存
        clear(root);
    }
    catch(const char*fault){
        //总之必须生成一棵合理的树
        justLettree();
        isLeagle=false;
        throw fault;
    }
}
///////////////////////////////////////只是用来生成LET树的/////////////////
void Command::justLettree()
{
    try{
        justteckle(letExpression);
        //现在root已经到手了，可以构建语法树了
        lagTree.clear();
        lagTree=generatLettree(root);
        //释放内存
        clear(root);
    }
    catch(const char*fault){
        isLeagle=false;
        lagTree.clear();
        lagTree.push_back(QString::number(inputLine));
        lagTree.push_back("    error");
    }
}
////////////////////////////PRINT的专用函数////////////
void Command::initPrint()
{
    //首先要清空var，因为可能后续不是第一次执行这条指令
    allvar.clear();
    //先把表达式部分截取出来
    int i=0;
    //跳过前面的已检测部分
    while(i<wholeCmd.size()&&wholeCmd[i]!='T')i++;
    i++;
    //截取后面的表达式部分
    QString exp;
    while(i<wholeCmd.size()){
        exp+=wholeCmd[i];
        i++;
    }
    //放进转换器里面转成一段一段的
    printExpression=varConver.doConvert(exp);
    //判断表达式的初步合法性
    if(!judgeer.isExpressionVarleagle(printExpression)){
        isLeagle=false;
        return;
    }
    for(int i=0;i<printExpression.size();i++){
        //常量塞进去
        if(judgeer.isNumberName(printExpression[i])){
            allvar.insert(printExpression[i],variable(printExpression[i]));
        }
        //变量塞进去
        else if(judgeer.isNameLeagle(printExpression[i])){
            allvar.insert(printExpression[i],variable(printExpression[i]));
        }
        else continue;
    }
    //初步合法
    isLeagle=true;
}
//真正进行print计算
void Command::reallydoPrint()
{
    try{
        v1result=tackleExpression(printExpression);
        //树还没有构建好
        if(!isTreed){
            lagTree.clear();
            lagTree=generatPrinttree(root);
            isTreed=true;
        }
        //更新变量使用次数
        for(int i=0;i<printExpression.size();i++){
            if(judgeer.isNameLeagle(printExpression[i])){
                allvar.find(printExpression[i])->times++;
            }
        }
        //释放内存
        clear(root);
    }

    catch(const char*fault){
        justPrinttree();
        isLeagle=false;
        throw fault;
    }
}
///////////////只是用来生成Print树的////////////
void Command::justPrinttree()
{
    try{
        justteckle(printExpression);
        //树还没有构建好
        lagTree.clear();
        lagTree=generatPrinttree(root);
        //释放内存
        clear(root);
    }

    catch(const char*fault){
        isLeagle=false;
        lagTree.clear();
        lagTree.push_back(QString::number(inputLine));
        lagTree.push_back("    error");
    }
}
/////////////////////////////初始化goto///////////
void Command::initGoto()
{
    int i=0;
    //先找到GOTO的位置
    while(i<wholeCmd.size()&&wholeCmd[i]!='T')i++;
    i++;//停在了O
    if(i==wholeCmd.size()){isLeagle=false;return;}
    i++;//过掉了O
    if(i==wholeCmd.size()){isLeagle=false;return;}
    //现在剩下的理应是数字了
    QString num;
    while(i<wholeCmd.size())num+=wholeCmd[i++];
    num=num.simplified();
    //筛查数字
    if(judgeer.isNumberName(num)){
        num=num.remove(' ');
        gotoAddress=num.toInt();
        isLeagle=true;
        //构建正确的语法树
        lagTree.clear();
        lagTree.push_back(QString("%1 GOTO").arg(QString::number(inputLine)));
        lagTree.push_back(QString("    %1").arg(QString::number(gotoAddress)));
    }
    else{
        isLeagle=false;
        //直接构建错误语法树
        lagTree.clear();
        lagTree.push_back(QString::number(inputLine));
        lagTree.push_back("    error");
        return;
    }
}
///////////////////////////初始化END////////////
void Command::initEnd()
{
    int i=0;
    QString test=wholeCmd.simplified();
    while(i<test.size()&&test[i]!='D')i++;
    //目前停在了D
    if(i==test.size()-1){
        lagTree.clear();
        lagTree.push_back(QString("%1 END").arg(QString::number(inputLine)));
        isLeagle=true;
    }
    else{
        isLeagle=false;
        lagTree.clear();
        lagTree.push_back(QString::number(inputLine));
        lagTree.push_back("    error");
    }
}
/////////////////////////////初始化if/////////////////////
void Command::initIf()
{
    //先跳过前面的行号和  IF
    int i=0;
    while(i<wholeCmd.size()&&wholeCmd[i]!='F')i++;
    //现在停在了IF的F这里
    //后面没输入了，报错返回
    if(i==wholeCmd.size()){isLeagle=false;return;}
    //接下来要截取表达式
     i++;
     QString exp1;
     //分界线：<或>或=
     while(i<wholeCmd.size()&&(wholeCmd[i]!='>'&&wholeCmd[i]!='='&&wholeCmd[i]!='<')){
         exp1+=wholeCmd[i];
         i++;
     }
     //目前停在了符号这里
     if(i==wholeCmd.size()){isLeagle=false;return;}
     opIf=wholeCmd[i++];
     //目前停在了表达式2的前面
     if(i==wholeCmd.size()){isLeagle=false;return;}
     //目前已经截取到了exp1的表达式了,先转换一下
     ifExp1=varConver.doConvert(exp1);
     //判断表达式的初步合法性
     if(!judgeer.isExpressionVarleagle(ifExp1)){
         isLeagle=false;
         return;
     }
     //合法则把表达式1的变量放进map里
     for(int i=0;i<ifExp1.size();i++){
         //常量塞进去
         if(judgeer.isNumberName(ifExp1[i])){
             allvar.insert(ifExp1[i],variable(ifExp1[i]));
         }
         //变量塞进去
         else if(judgeer.isNameLeagle(ifExp1[i])){
             allvar.insert(ifExp1[i],variable(ifExp1[i]));
         }
         else continue;
     }
     //下面筛查表达式2，没办法强行找标志符，只能硬查
     QString exp2;
     //至少会预留出4个空位来给THEN
     while(i<wholeCmd.size()-4){
         if(wholeCmd[i]!='T'){
             //不是then，直接加进去就行
             exp2+=wholeCmd[i++];
         }
         //筛查then
         else{
             if(wholeCmd[i+1]=='H'&&wholeCmd[i+2]=='E'&&wholeCmd[i+3]=='N')break;
             else exp2+=wholeCmd[i++];
         }
     }
     //现在停在了THEN的T这里
     if(i>=wholeCmd.size()-4){isLeagle=false;return;}
     //可以确定接下来的四个字符就是THEN
     //跳过then
     i+=4;
     //判断表达式的初步合法性
     ifExp2=varConver.doConvert(exp2);
     if(!judgeer.isExpressionVarleagle(ifExp2)){
         isLeagle=false;
         return;
     }
     //合法则把表达式2的变量放进map里
     for(int i=0;i<ifExp2.size();i++){
         //常量塞进去
         if(judgeer.isNumberName(ifExp2[i])){
             allvar.insert(ifExp2[i],variable(ifExp2[i]));
         }
         //变量塞进去
         else if(judgeer.isNameLeagle(ifExp2[i])){
             allvar.insert(ifExp2[i],variable(ifExp2[i]));
         }
         else continue;
     }
     QString address;
     //截取最后的跳转地址
     while(i<wholeCmd.size())address+=wholeCmd[i++];
     address=address.simplified();
     //判断是不是合法数字
     if(!judgeer.isNumberName(address)){isLeagle=false;return;}
     //把跳转位置存进去
     address=address.remove(' ');
     ifAddress=address.toInt();
     isLeagle=true;
}
///////////////////////////////////////////处理if的函数///////////////////
void Command::reallydoIf()
{
    try{
        v1result=tackleExpression(ifExp1);
        node*r1=root;
        v2result=tackleExpression(ifExp2);
        //判断一下是否跳转
        if(opIf==">"){
            if(v1result>v2result)isifTrue=true;
            else isifTrue=false;
        }
        else if(opIf=="="){
            if(v1result==v2result)isifTrue=true;
            else isifTrue=false;
        }
        else{
            if(v1result<v2result)isifTrue=true;
            else isifTrue=false;
        }
        //没构造过树才构造
            node*r2=root;
            root=new node(QString("%1  IF THEN ").arg(QString::number(inputLine)));
            root->left=r1;
            root->right=r2;
            //树还没有构建好
            lagTree.clear();
            lagTree=generatIfTree(root);
            //插入符号和目标节点
            QString tmp="    ";
            lagTree.insert(2,tmp+opIf);
            if(lagTree.size()==4)lagTree.push_back(tmp+QString::number(ifAddress));
            else lagTree.insert(4,tmp+QString::number(ifAddress));
        //更新变量使用次数
        for(int i=0;i<ifExp1.size();i++){
            if(judgeer.isNameLeagle(ifExp1[i])){
                allvar.find(ifExp1[i])->times++;
            }
        }
        for(int i=0;i<ifExp2.size();i++){
            if(judgeer.isNameLeagle(ifExp2[i])){
                allvar.find(ifExp2[i])->times++;
            }
        }
        //释放内存
        clear(root);
    }
    catch(const char*fault){
        justiftree();
        isLeagle=false;
        throw fault;
    }
}
/////////////////////////////只生成树而已///////////////////////////
void Command::justiftree()
{
    try{
        if(!isTreed){
            justteckle(ifExp1);
            node*r1=root;
            justteckle(ifExp2);
            node*r2=root;
            root=new node(QString("%1  IF THEN ").arg(QString::number(inputLine)));
            root->left=r1;
            root->right=r2;
            //树还没有构建好
            lagTree.clear();
            lagTree=generatIfTree(root);
            //插入符号和目标节点
            QString tmp="    ";
            lagTree.insert(2,tmp+opIf);
            if(lagTree.size()==4)lagTree.push_back(tmp+QString::number(ifAddress));
            else lagTree.insert(4,tmp+QString::number(ifAddress));
        }
        //释放内存
        clear(root);
    }
    catch(const char*fault){
        isLeagle=false;
        lagTree.clear();
        lagTree.push_back(QString::number(inputLine));
        lagTree.push_back("    error");
    }
}
/////////////////////////////////////生成let的树
QVector<QString> Command::generatLettree(node*r)
{
    //先把后面的root生成树搞定
    QVector<QString>tmptree;
    tmptree=generatTree(r);
    QString tmp="    ";
    tmptree.insert(0,(tmp+letName));
    //把最开始的放进去
    tmptree.insert(0,QString("%1 LET =").arg(QString::number(inputLine)));
    return tmptree;
}
/////////////////////////////////生成print的树
QVector<QString> Command::generatPrinttree(node *r)
{
    //先把后面的root生成树搞定
    QVector<QString>tmptree;
    tmptree=generatTree(r);
    tmptree.insert(0,QString("%1 PRINT").arg(QString::number(inputLine)));
    return tmptree;
}
///////////////////////////////生成if的树//////////////////
QVector<QString> Command::generatIfTree(node *r)
{
    //先把后面的root生成树搞定
    QVector<QString>tmptree;
    tmptree=generatTree(r);
    return tmptree;
}
