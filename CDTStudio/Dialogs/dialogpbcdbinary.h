#ifndef DIALOGPBCDBINARY_H
#define DIALOGPBCDBINARY_H

#include <QDialog>

struct QUuid;
class QSqlQueryModel;

namespace Ui {
class DialogPBCDBinary;
}

class DialogPBCDBinary : public QDialog
{
    Q_OBJECT

public:
    friend class DialogPBCD;
    explicit DialogPBCDBinary(QUuid projectID,QWidget *parent = 0);
    ~DialogPBCDBinary();

private slots:
    void onT1ImageChanged(int row);
    void onT2ImageChanged(int row);

    void onAddBandPair();
    void onAutoBand();
    void onRemoveBands();
    void onSelectionChanged();
    void updatePushbuttonRemoveAll();    
    void clearBandPairs();

    void updateGroupBoxMerge();
    void updateDoubleThreshold();

    void onAutoThresholdToggled(bool toggled);
    void onManualThresholdToggled(bool toggled);

    void generateXML();

private:
    Ui::DialogPBCDBinary *ui;

    QUuid prjID;
    QSqlQueryModel *modelImage;
    bool isDoubleThreshold;
};

#endif // DIALOGPBCDBINARY_H
