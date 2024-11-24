#ifndef __SEC_CODE_GENERATOR_HPP__
#define __SEC_CODE_GENERATOR_HPP__
#include <QObject>
#include <QString>

class SecCodeGenerator: public QObject
{
    Q_OBJECT
public:
    explicit SecCodeGenerator(QObject* parent = nullptr);
    Q_INVOKABLE QString generateSecCode(const QString& path);

    std::string getSecCode();
    std::string getCurPath();

private:
    QString curSecCode_;
    QString curPath_;
};

#endif//__SEC_CODE_GENERATOR_HPP__
