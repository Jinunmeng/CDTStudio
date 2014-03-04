#ifndef FORMMST_H
#define FORMMST_H

#include <QWidget>
#include "mstmethodinterface.h"

namespace Ui {
class FormMST;
}

class FormMST : public QWidget
{
    Q_OBJECT
public:
    explicit FormMST(QWidget *parent = 0);
    ~FormMST();

    void setInterface(MSTMethodInterface* interface);
signals:
    void finished();
private slots:
    void on_pushButtonStart_clicked();    
    void setGroupboxText(QString text);
private:
    Ui::FormMST *ui;    
    MSTMethodInterface* interface;
};

#endif // FORMMST_H