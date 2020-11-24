#include "error.h"
#include <QDebug>
#include "mainwindow.h"

Error::Error(ErrType type, QObject *parent) : QObject(parent){
    switch (type){
        case ErrType::WARN:
                m_msgBox = new QMessageBox(QMessageBox::Warning, "Предупреждение",
                                           "", QMessageBox::Ignore|QMessageBox::Close);
        break;
        case ErrType::ERROR:
        m_msgBox = new QMessageBox(QMessageBox::Critical, "Ошибка",
                                   "", QMessageBox::Ok);
        break;
    }
}

Error::~Error(){
    delete m_msgBox;
}

void Error::ShowMessage(const QString& msg){
    m_msgBox->setText(msg);
    m_msgBox->exec();
}

int Error::result(){
   return  m_msgBox->result();
}
