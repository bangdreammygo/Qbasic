
#include "converter.h"

converter::converter()
{

}
/////////////////////////////////////////将表达式转成一段一段的函数
QVector<QString> converter::doConvert(QString exp2)
{
    QVector<QString>tmp;
    int i=0;
    //先把整个表达式读出来并化简
    QString exp=exp2;
    exp=exp.simplified();
    //初始化work字符串
    QString work;
    work.clear();
    while(i!=exp.size()){
      //处理到了+ -  /  （  ）  返回
      if(exp[i]=='+'){
          work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
              work.clear();
          }
          work.clear();
          tmp.push_back("+");
          i++;
          continue;
      }
      //特别注意一下负数的处理模式
      else if(exp[i]=='-'){
          //最前面的负号
          if(i==0&&i<exp.size()-1&&((exp[i+1]>='0'&&exp[i+1]<='9')||exp[i+1]==' ')){work+=exp[i];i++ ; continue;}
          else if(i==0&&i<exp.size()-1&&exp[i+1]=='('){tmp.push_back("-1");tmp.push_back("*");i++;continue;}
          //有括号的负号
          else if(!tmp.empty()&&tmp.back()=="("&&(work.remove(' ').size()==0)&&((exp[i+1]>='0'&&exp[i+1]<='9')||(exp[i+1]==' '))){work+=exp[i];i++ ; continue;}
          //其他
          else{
              work=work.simplified();
              if(work.remove(' ').size()>0){
                  tmp.push_back(work);
                  work.clear();
              }
              work.clear();
              tmp.push_back("-");
              i++;
              continue;
          }
      }
      else if(exp[i]=='/'){
          work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
          }
          work.clear();
          tmp.push_back("/");
          i++;
          continue;
      }
      else if(exp[i]=='('){
          work=work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
          }
          work.clear();
          tmp.push_back("(");
          i++;
          continue;
      }
      else if(exp[i]==')'){
          work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
              work.clear();
          }
          work.clear();
          tmp.push_back(")");
          i++;
          continue;
      }
      //**和*
      if(i<exp.size()-1&&exp[i+1]=='*'&&exp[i]=='*'){
          work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
          }
          work.clear();
          tmp.push_back("**");
          i+=2;
          continue;
      }
      else if(i<exp.size()-1&&exp[i]=='*'&&exp[i+1]!='*'){
          work=work.simplified();
          if(work.remove(' ').size()>0){
              tmp.push_back(work);
          }
          work.clear();
          tmp.push_back("*");
          i++;
          continue;
      }
      //更新work这个字符串
      work+=exp[i];
      //MOD(最难的一集)
      if(i>=3&&(exp[i]=='D')){
          if(exp[i-1]=='O'&&exp[i-2]=='M'&&exp[i-3]==' '){
              QString ex;
              ex.clear();
             for(int t=0;t<work.size()-4;t++){
                 ex+=work[t];
             }
             work.clear();
             ex=ex.simplified();
             if(ex.remove(' ').size()>0)tmp.push_back(ex);
             tmp.push_back("MOD");
             i++;
             continue;
          }
      }
      i++;
    }
    //最后还差一个要PUSH进去
    if(work.remove(' ').size()>0){
        work=work.simplified();
        tmp.push_back(work);
    }
    return tmp;
}
