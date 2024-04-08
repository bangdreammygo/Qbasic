#include "leaglejudger.h"

leagleJudger::leagleJudger()
{

}
/////////////////////////////判断变量名合法性的函数//////////////////
bool leagleJudger::isNameLeagle(QString name)
{
    //空字符串返回false
    if(name.size()==0)return false;
    //先化简
    name=name.simplified();
    int i=0;
    bool flag=true;
    //跳过前置空格
    while(i<name.size()&&name[i]==' ')i++;
    if(i==name.size())return false;
    //首字母只能是字母或者下划线
    if(name[i]=='_'||(name[i]>='a'&&name[i]<='z')||(name[i]>='A'&&name[i]<='Z')){flag=true;}
    else return false;
    i++;
    for(;i<name.size();i++){
        if(name[i]=='_')continue;
        else if(name[i]<='9'&&name[i]>='0')continue;
        else if(name[i]<='z'&&name[i]>='a')continue;
        else if(name[i]<='Z'&&name[i]>='A')continue;
        flag=false;
        break;
    }
    //非法变量名
    if(name=="IF")return false;
    if(name=="THEN")return false;
    if(name=="LET")return false;
    if(name=="GOTO")return false;
    if(name=="INPUT")return false;
    if(name=="RUN")return false;
    if(name=="LOAD")return false;
    if(name=="CLEAR")return false;
    if(name=="PRINT")return false;
    if(name=="QUIT")return false;
    if(name=="HELP")return false;
    if(name=="REM")return false;
    if(name=="LIST")return false;
    if(name=="END")return false;
    if(name=="MOD")return false;
    return flag;
}
////////////////////////////判断表达式里面的各个部分是不是合法的//////////////////
bool leagleJudger::isExpressionVarleagle(QVector<QString> var)
{
         bool flag=true;
         for(int i=0;i<var.size();i++){
         //符号
         if(var[i]=="-")continue;
         if(var[i]=="+")continue;
         if(var[i]=="*")continue;
         if(var[i]=="/")continue;
         if(var[i]=="(")continue;
         if(var[i]==")")continue;
         if(var[i]=="**")continue;
         if(var[i]=="MOD")continue;
         //数字
         if(isNumberName(var[i]))continue;
         //变量名
         if(isNameLeagle(var[i]))continue;
         flag=false;
         break;
     }

    return flag;
}
//////////////////////////////////判断是不是一个数字的名字//////////
bool leagleJudger::isNumberName(QString num)
{
    bool flag=true;
    //筛查负数
    if(num[0]=='-'){
      if(num.size()==1)flag=false;
      for(int j=1;j<num.size();j++){
          if(num[j]>='0'&&num[j]<='9'){
              continue;
          }
          else{
              flag=false;
              break;
          }
      }
      return flag;
    }
    //筛查正数
    for(int i=0;i<num.size();i++){
        if(num[i]<='9'&&num[i]>='0')continue;
        else {flag=false;break;}
    }
    return flag;
}
///////////////////////////////////判断是不是操作符//////
bool leagleJudger::isoperator(QString str)
{
    if(str=="+")return true;
    if(str=="-")return true;
    if(str=="**")return true;
    if(str=="*")return true;
    if(str=="/")return true;
    if(str=="MOD")return true;
    if(str=="(")return true;
    if(str==")")return true;
    return false;
}
