#ifndef ERROR_H
#define ERROR_H

#include <QObject>
#include <QString>
#include <QMessageBox>

enum ErrType {
    WARN,
    ERROR,
    ALERT
};

enum ErrResult {
    IGNORE = QMessageBox::Ignore,
    CLOSE = QMessageBox::Close
};

class Error : public QObject
{
    Q_OBJECT
public:
    explicit Error(ErrType type = ErrType::ERROR,
                   QObject *parent = nullptr);
    ~Error();
    void ShowMessage(const QString& msg);
    int result();
signals:

private:
    QMessageBox* m_msgBox;
};

#endif // ERROR_H
