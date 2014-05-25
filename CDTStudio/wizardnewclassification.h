#ifndef WIZARDNEWCLASSIFICATION_H
#define WIZARDNEWCLASSIFICATION_H

#include <QWizard>

namespace Ui {
class WizardNewClassification;
}
class QSqlQueryModel;
class QTreeWidgetItem;
class QStringListModel;

class WizardNewClassification : public QWizard
{
    Q_OBJECT

public:
    explicit WizardNewClassification(QWidget *parent = 0);
    ~WizardNewClassification();

private:
    void initClassifiers();
    void updateFeatures(QString segID);
    static QStringList attributeNames();
    bool validateCurrentPage();


private slots:
    void onSegmentationChanged(int index);
    void onClassifierChanged(int index);
    void onCurrentPageChanged(int pageID);
    void onButtonClicked(int buttonID);
    void updateSelectedFeature();

private:
    Ui::WizardNewClassification *ui;
    QSqlQueryModel *modelSample;
    QStringListModel *modelSelectedFeature;
    QList<QWidget*> paramWidgets;
    bool finished;
};

#endif // WIZARDNEWCLASSIFICATION_H
