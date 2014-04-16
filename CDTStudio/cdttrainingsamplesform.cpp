#include "cdttrainingsamplesform.h"
#include "ui_cdttrainingsamplesform.h"
#include "stable.h"
#include "cdtmaptoolselecttrainingsamples.h"
#include "cdtimagelayer.h"
#include "mainwindow.h"
#include "cdtprojectwidget.h"

QDataStream &operator <<(QDataStream &out,const CategoryInformation &categoryInformation)
{
    out<<categoryInformation.id<<categoryInformation.categoryName<<categoryInformation.color;
    return out;
}

QDataStream &operator >>(QDataStream &in, CategoryInformation &categoryInformation)
{
    in>>categoryInformation.id>>categoryInformation.categoryName>>categoryInformation.color;
    return in;
}

CDTTrainingSamplesForm::CDTTrainingSamplesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CDTTrainingSamplesForm),
    categoryModel(new QSqlRelationalTableModel(this,QSqlDatabase::database("category"))),
    sampleModel(new QSqlQueryModel(this)),
    delegateColor(new CDTCategoryDelegate(this)),
    delegateImageID(new QSqlRelationalDelegate(this)),
    lastMapTool(NULL),
    currentMapTool(NULL)
{
    ui->setupUi(this);

    QToolBar* toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16,16));
    toolBar->addActions(QList<QAction*>()
                        <<ui->actionEdit<<ui->actionRevert<<ui->actionSubmit);
    toolBar->addSeparator();
    toolBar->addActions(QList<QAction*>()
                        <<ui->actionInsert<<ui->actionRemove<<ui->actionRemove_All);
    ui->verticalLayout->insertWidget(0,toolBar);

    connect(ui->toolButtonRefresh,SIGNAL(clicked()),SLOT(updateComboBox()));

    ui->tableView->setModel(categoryModel);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
    categoryModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Class Name"));
    categoryModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Color"));
    connect(categoryModel,SIGNAL(primeInsert(int,QSqlRecord&)),SLOT(onPrimeInsert(int,QSqlRecord&)));

    ui->listView->setModel(sampleModel);
}

CDTTrainingSamplesForm::~CDTTrainingSamplesForm()
{
    delete ui;
}

void CDTTrainingSamplesForm::updateTable()
{
    categoryModel->setTable("category");
    categoryModel->setFilter("imageID='"+imageLayerID.toString()+"'");
    categoryModel->select();

    ui->tableView->setItemDelegateForColumn(2,delegateColor);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(3);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void CDTTrainingSamplesForm::updateComboBox()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (ui->comboBox->model()) delete ui->comboBox->model();
    ui->comboBox->setModel(model);

    model->setQuery("select name,id from segmentationlayer where imageID = '"+imageLayerID.toString()+"'",
                    QSqlDatabase::database("category"));

    ui->toolButtonNewSample->setEnabled(ui->comboBox->count()!=0);
    ui->toolButtonRemoveSelected->setEnabled(ui->comboBox->count()!=0);
    ui->toolButtonEditSample->setEnabled(ui->comboBox->count()!=0);
    if (ui->comboBox->count()==0)
        sampleModel->clear();
}

void CDTTrainingSamplesForm::updateListView()
{
    int index = ui->comboBox->currentIndex();
    if (index<0) return;

    QSqlQueryModel * model = (QSqlQueryModel *)(ui->comboBox->model());
    QString segmentationID = model->data(model->index(index,1)).toString();

    sampleModel->setQuery("select name,id from sample_segmenation where segmenationid='"+segmentationID+"'",QSqlDatabase::database("category"));
}

void CDTTrainingSamplesForm::setImageID(QUuid uuid)
{
    imageLayerID = uuid;

    QList<CDTImageLayer*> layers = CDTImageLayer::getLayers();
    foreach (CDTImageLayer* layer, layers) {
        if (layer->id()==uuid)
        {
            connect(categoryModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    layer,SIGNAL(imageLayerChanged()));
            connect(categoryModel,SIGNAL(beforeInsert(QSqlRecord&)),
                    layer,SIGNAL(imageLayerChanged()));
            connect(categoryModel,SIGNAL(beforeDelete(int)),
                    layer,SIGNAL(imageLayerChanged()));
        }
        else
        {
            disconnect(categoryModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       layer,SIGNAL(imageLayerChanged()));
            disconnect(categoryModel,SIGNAL(beforeInsert(QSqlRecord&)),
                       layer,SIGNAL(imageLayerChanged()));
            disconnect(categoryModel,SIGNAL(beforeDelete(int)),
                       layer,SIGNAL(imageLayerChanged()));
        }
    }

    updateComboBox();
    updateTable();
}

bool CDTTrainingSamplesForm::isValid()
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    bool ret = query.exec("select count(*) from imagelayer where id = '"+ imageLayerID.toString() +"'");
    if (!ret)
        qDebug()<<query.lastError().text();

    query.next();
    if (query.value(0).toInt()==1)
        return true;

    updateTable();
    return false;
}

QUuid CDTTrainingSamplesForm::currentCategoryID()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid())
        return QUuid();
    QString id = categoryModel->data(categoryModel->index(currentIndex.row(),0)).toString();
    if (id.isNull())
        return QUuid();
    return QUuid(id);
}

void CDTTrainingSamplesForm::on_actionInsert_triggered()
{
    if (!this->isValid()) return;

    categoryModel->insertRow(categoryModel->rowCount());
    ui->actionInsert->setEnabled(false);
}

void CDTTrainingSamplesForm::on_actionRemove_triggered()
{
    if (!this->isValid()) return;
    int row = ui->tableView->currentIndex().row();
    if (row <0) return;
    categoryModel->removeRow(row);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void CDTTrainingSamplesForm::on_actionRemove_All_triggered()
{
    if (!this->isValid()) return;
    categoryModel->removeRows(0,categoryModel->rowCount());
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void CDTTrainingSamplesForm::on_actionRevert_triggered()
{
    if (!this->isValid()) return;
    categoryModel->revertAll();
    ui->actionInsert->setEnabled(true);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void CDTTrainingSamplesForm::on_actionSubmit_triggered()
{
    if (categoryModel->submitAll()==false)
        qDebug()<<"Submit failed:"<<categoryModel->lastError();
    ui->actionInsert->setEnabled(true);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void CDTTrainingSamplesForm::onPrimeInsert(int row, QSqlRecord &record)
{
    record.setValue(0,QUuid::createUuid().toString());
    record.setValue(1,tr("New Class"));
    record.setValue(2,QColor(qrand()%255,qrand()%255,qrand()%255));
    record.setValue(3,imageLayerID.toString());
    record.setGenerated(0,true);
    record.setGenerated(1,true);
    record.setGenerated(2,true);
    record.setGenerated(3,true);
}

void CDTTrainingSamplesForm::on_actionEdit_triggered(bool checked)
{
    if (!this->isValid()) return;

    ui->actionInsert->setEnabled(checked);
    ui->actionRemove->setEnabled(checked);
    ui->actionRemove_All->setEnabled(checked);
    ui->actionRevert->setEnabled(checked);
    ui->actionSubmit->setEnabled(checked);
    if (checked)
        ui->tableView->setEditTriggers(QTableView::DoubleClicked|QTableView::AnyKeyPressed);
    else
        ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
}



void CDTTrainingSamplesForm::on_toolButtonRefresh_clicked()
{
    updateComboBox();
}

void CDTTrainingSamplesForm::on_comboBox_currentIndexChanged(int index)
{
    updateListView();
}

void CDTTrainingSamplesForm::on_toolButtonEditSample_toggled(bool checked)
{
    if (checked)
    {
        lastMapTool = MainWindow::getCurrentMapCanvas()->mapTool();
        currentMapTool =
                new CDTMapToolSelectTrainingSamples(MainWindow::getCurrentMapCanvas());
        MainWindow::getCurrentMapCanvas()->setMapTool(currentMapTool);
        MainWindow::getCurrentProjectWidget()->menuBar()->setEnabled(false);

        int index = ui->listView->currentIndex().row();
        if (index>=0)
        {
            QString sampleid   = sampleModel->data(sampleModel->index(index,1)).toString();
            currentMapTool->setSampleID(sampleid);
        }
    }
    else
    {
        MainWindow::getCurrentMapCanvas()->setMapTool(lastMapTool);
        currentMapTool->clearRubberBand();
        if(currentMapTool) delete currentMapTool;
        MainWindow::getCurrentProjectWidget()->menuBar()->setEnabled(true);
    }


}

void CDTTrainingSamplesForm::on_toolButtonSampleRename_clicked()
{
    if (!this->isValid()) return;
    int index = ui->listView->currentIndex().row();
    if (index<0)
        return;

    QString sampleid   = sampleModel->data(sampleModel->index(index,1)).toString();
    QString sampleName = sampleModel->data(sampleModel->index(index,0)).toString();

    sampleName =
            QInputDialog::getText(this,tr("New Sample Name"),tr("Name:"),QLineEdit::Normal,sampleName);
    if (sampleName.isEmpty())
        return;

    QSqlQuery q(QSqlDatabase::database("category"));
    q.prepare("update sample_segmenation set name = ? where id = ?");
    q.bindValue(0,sampleName);
    q.bindValue(1,sampleid);
    q.exec();

    updateListView();
}

void CDTTrainingSamplesForm::on_toolButtonNewSample_clicked()
{
    if (!this->isValid()) return;

    QString sampleName =
            QInputDialog::getText(this,tr("New Sample Name"),tr("Name:"),QLineEdit::Normal,tr("New Sample"));
    if (sampleName.isEmpty())
        return;

    QSqlQueryModel * model = (QSqlQueryModel *)(ui->comboBox->model());
    QString segmentationID = model->data(model->index(ui->comboBox->currentIndex(),1)).toString();

    QSqlQuery q(QSqlDatabase::database("category"));
    q.prepare("insert into sample_segmenation values(?,?,?)");
    q.bindValue(0,QUuid::createUuid().toString());
    q.bindValue(1,sampleName);
    q.bindValue(2,segmentationID);
    q.exec();

    updateListView();
}

void CDTTrainingSamplesForm::on_toolButtonRemoveSelected_clicked()
{
    if (!this->isValid()) return;
    int index = ui->listView->currentIndex().row();
    if (index<0)
        return;

    QString sampleid   = sampleModel->data(sampleModel->index(index,1)).toString();
    QSqlQuery q(QSqlDatabase::database("category"));
    q.prepare("delete from sample_segmenation where id = ?");
    q.bindValue(0,sampleid);
    q.exec();

    updateListView();
}

void CDTTrainingSamplesForm::on_listView_clicked(const QModelIndex &index)
{
    if (currentMapTool)
    {
        QString sampleid   = sampleModel->data(sampleModel->index(index.row(),1)).toString();
        currentMapTool->setSampleID(sampleid);

    }
}