#include "dialogconsole.h"
#include "ui_dialogconsole.h"
#include "stable.h"

DialogConsole::DialogConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConsole),
    listModel( NULL)
{
    ui->setupUi(this);
    listModel = new QStringListModel(ui->listView);
    ui->listView->setModel(listModel);
    ui->tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    setWindowFlags(Qt::Window);


    connect(ui->pushButtonRefresh,SIGNAL(clicked()),SLOT(updateDatabases()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)),SLOT(onDatabaseChanged(QString)));
//    connect(ui->listView,SIGNAL(clicked(QModelIndex)),SLOT(updateCurrentTable(QModelIndex)));
    connect(ui->listView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),SLOT(onSelectionChanged(QItemSelection)));
    connect(ui->listView,SIGNAL(customContextMenuRequested(QPoint)),SLOT(onListViewContextMenu(QPoint)));
    connect(ui->pushButtonQuery,SIGNAL(clicked()),SLOT(onQuery()));
    connect(ui->plainTextEditQuery,SIGNAL(textChanged()),SLOT(onQueryTextChanged()));

    QAction *actionClose = new QAction(this);
    this->addAction(actionClose);
    actionClose->setShortcut(QKeySequence(Qt::Key_F12));
    connect(actionClose,SIGNAL(triggered()),SLOT(hide()));

    QAction *actionCopyAll = new QAction(QIcon(":/Icons/Copy.png"),tr("Copy All"),this);
    ui->tableView->addAction(actionCopyAll);
    connect(actionCopyAll,SIGNAL(triggered()),SLOT(copyTableAll()));
}

DialogConsole::~DialogConsole()
{
    delete ui;
}

void DialogConsole::updateDatabases()
{
    //Get current state;
    QString currentConnectionName = ui->comboBox->currentText();
    int index = ui->listView->currentIndex().row();
    QStringList list = listModel->stringList();

    //update
    ui->comboBox->clear();
    ui->comboBox->addItems(QSqlDatabase::connectionNames());

    //Continue getting preview state;
    if (index<0)
        return;
    QString currentTableName = list[index];

    index = ui->comboBox->findText(currentConnectionName);
    if (index == -1)
        return;
    ui->comboBox->setCurrentIndex(index);

    if (listModel->stringList().contains(currentTableName))
    {
        index = listModel->stringList().indexOf(currentTableName);
        ui->listView->selectionModel()->select(listModel->index(index,0),QItemSelectionModel::Select);
    }
}

void DialogConsole::onQuery()
{
    QSqlQuery query(db);
    if (query.exec(ui->plainTextEditQuery->toPlainText())==false)
    {
        QMessageBox::critical(this,tr("Query failed!"),query.lastError().text());
        return;
    }
    QSqlQueryModel* tableModel = new QSqlQueryModel(ui->tableView);
    if (ui->tableView->model())
        delete ui->tableView->model();
    tableModel->setQuery(query);
    ui->tableView->setModel(tableModel);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void DialogConsole::onQueryTextChanged()
{
    ui->pushButtonQuery->setEnabled(!ui->plainTextEditQuery->toPlainText().isEmpty());
}

void DialogConsole::onDatabaseChanged(QString connName)
{
    if (connName.isEmpty())
    {
        listModel->setStringList(QStringList());
        return ;
    }
    db = QSqlDatabase::database(connName);
    if (!db.isValid())
    {
        QMessageBox::critical(this,tr("Error"),db.lastError().text());
        return;
    }
    QStringList list = db.tables();
    listModel->setStringList(list);
}

void DialogConsole::onListViewContextMenu(QPoint pt)
{
    QModelIndex index = ui->listView->indexAt(pt);
    if (!index.isValid())
        return;

    int row = index.row();
    QStringList list = listModel->stringList();
    ui->listView->setCurrentIndex(ui->listView->indexAt(pt));
    updateCurrentTable(ui->listView->currentIndex());

    QMenu menu;
    menu.addAction(tr("Delete all data in table '%1'").arg(list[row]),this,SLOT(deleteDataInCurrentTable()));
    menu.exec(QCursor::pos());
}

void DialogConsole::deleteDataInCurrentTable()
{
    int row = ui->listView->currentIndex().row();
    QStringList list = listModel->stringList();
    if (row>=list.size())
        return;

    QSqlQuery query(db);
    query.exec(QString("Delete from %1").arg(list[row]));
    updateCurrentTable(ui->listView->currentIndex());
}

void DialogConsole::onSelectionChanged(QItemSelection selection)
{
    updateCurrentTable(selection[0].indexes()[0]);
}

void DialogConsole::copyTableAll()
{
    auto model = ui->tableView->model();
    if (model==NULL || model->rowCount()==0) return;


    QString selected_text;
    for (int i=0;i<model->rowCount();++i)
    {
        for (int j=0;j<model->columnCount();++j)
        {
            selected_text += (model->data(model->index(i,j)).toString() + "\t");
        }
        selected_text += "\n";
    }

    QApplication::clipboard()->setText(selected_text);
}

void DialogConsole::updateCurrentTable(const QModelIndex &index)
{
    QString tableName = listModel->data(index,Qt::DisplayRole).toString();
    QSqlTableModel* tableModel = new QSqlTableModel(ui->tableView,db);
    if (ui->tableView->model())
        delete ui->tableView->model();
    ui->tableView->setModel(tableModel);
    tableModel->setTable(tableName);
    tableModel->select();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}
