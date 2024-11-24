#include <QRandomGenerator>
#include "SecCodeGenerator.hpp"

SecCodeGenerator::SecCodeGenerator(QObject* parent)
    : QObject(parent)
    , curSecCode_("")
    , curPath_("")
{

}

QString SecCodeGenerator::generateSecCode(const QString& path)
{
    int num1 = QRandomGenerator::global()->bounded(10);
    int num2 = QRandomGenerator::global()->bounded(10);
    int num3 = QRandomGenerator::global()->bounded(10);
    int num4 = QRandomGenerator::global()->bounded(10);
    QString result = QString("%1%2%3%4")
        .arg(num1)
        .arg(num2)
        .arg(num3)
        .arg(num4);
    curSecCode_ = result;
    curPath_ = path;
    return result;
}

std::string SecCodeGenerator::getSecCode()
{   
    return curSecCode_.toStdString();
}

std::string SecCodeGenerator::getCurPath() 
{
    return curPath_.toStdString();
}