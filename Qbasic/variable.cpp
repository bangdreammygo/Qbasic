#include "variable.h"

variable::variable(QString n)
{
       name=n;
       if(judge.isNumberName(name)){
           varValue=name.toInt();
       }
}
variable::variable(QString n, int v)
{
    name=n;
    varValue=v;
}
