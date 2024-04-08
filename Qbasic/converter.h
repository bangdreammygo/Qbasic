//////////////////////////////////////////一个转换器类，把字符串表达式转成一段一段的
#ifndef CONVERTER_H
#define CONVERTER_H
#include"Config.h"
#include<QVector>
#include<QString>
#include<QDebug>
class converter
{
public:
    converter();
    QVector<QString> doConvert(QString exp2);
};

#endif // CONVERTER_H
